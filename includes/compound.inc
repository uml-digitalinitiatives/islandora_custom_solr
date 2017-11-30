<?php

/**
 * @file
 * Functions for compound objects.
 */

/**
 * Return an array of pids that are part of a compound object.
 *
 * @param string $pid
 *   the pid of the object to get compound parts for.
 * @param bool $ret_title
 *   whether to return the title and sequence number along with the pid.
 *
 * @return array
 *   array of a single object pid, title and sequence number or
 *   an array of pids
 */
function islandora_custom_solr_compound_object_get_parts($pid, $ret_title = FALSE) {
  $solr_build = new IslandoraSolrQueryProcessor();

  $escaped_pid = str_replace(':', '_', $pid);
  $objects = array();

  $parent_field = variable_get('islandora_custom_solr_compound_solr_relationship', 'RELS_EXT_isConstituentOf_uri_mt');
  $solr_param = array(
    'rows' => 100000,
    'limit' => 100000,
    'fl' => "fgs_label_s, PID, RELS_EXT_isSequenceNumberOf{$escaped_pid}_literal_ms",
    'hl' => 'false',
    'facet' => 'false',
  );
  $solr_query = "($parent_field:\"info:fedora/{$pid}\")";
  $solr_build->buildQuery($solr_query, $solr_param);
  $solr_build->solrParams = array_merge($solr_build->solrParams, $solr_param);
  // We need to remove the solr child object restriction.
  if (isset($solr_build->solrParams['fq'])) {
    foreach ($solr_build->solrParams['fq'] as $idx => $val) {
      if ($val == variable_get('islandora_compound_object_solr_fq', '-RELS_EXT_isConstituentOf_uri_mt:*')) {
        unset($solr_build->solrParams['fq'][$idx]);
        break;
      }
    }
  }

  $path_parts = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($path_parts['host'], $path_parts['port'], $path_parts['path'] . '/');
  $solr->setCreateDocuments(0);

  // Query is executed.
  try {
    $solr_query = ($solr_build->internalSolrQuery) ? $solr_build->internalSolrQuery : $solr_build->solrQuery;
    $method = 'GET';
    $results = $solr->search($solr_query, $solr_build->solrStart, $solr_build->solrLimit, $solr_build->solrParams, $method);
  }
  catch (Exception $e) {
    drupal_set_message(check_plain(t('Error searching Solr index')) . ' ' . $e->getMessage(), 'error');
  }

  if (isset($results)) {
    $solr_results = json_decode($results->getRawResponse(), TRUE);
    $results = $solr_results['response']['docs'];
  }

  $escaped_field = "RELS_EXT_isSequenceNumberOf{$escaped_pid}_literal_ms";
  $map = function ($o) use ($escaped_field) {
    return array(
      'pid' => $o["PID"],
      'title' => $o['fgs_label_s'],
      'seq' => (array_key_exists($escaped_field, $o) ? (string) $o[$escaped_field][0] : 0),
    );
  };

  $results = array_map($map, $results);

  // Sort the objects into their proper order.
  $sort = function ($a, $b) {
    $a = $a['seq'];
    $b = $b['seq'];
    if ($a === $b) {
      return 0;
    }
    if (empty($a)) {
      return 1;
    }
    if (empty($b)) {
      return -1;
    }
    return $a - $b;
  };
  uasort($results, $sort);

  foreach ($results as $result) {
    $object = islandora_object_load($result['pid']);
    if (islandora_object_access(ISLANDORA_VIEW_OBJECTS, $object)) {
      if ($ret_title) {
        $objects[$result['pid']] = array(
          'pid' => $result['PID'],
          'title' => $result['title'],
          'seq' => $result["seq"],
        );
      }
      else {
        $objects[] = $result['pid'];
      }
    }
  }

  return $objects;
}
