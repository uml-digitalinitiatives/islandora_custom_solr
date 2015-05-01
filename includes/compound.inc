<?php
/**
 * @file
 * Functions for compound objects.
 */

/**
 * Return an array of pids that are part of a compound object.
 */
function islandora_custom_solr_compound_object_get_parts($pid, $ret_title = FALSE) {
  module_load_include('inc', 'islandora_solr_search', 'includes/query_processor');
  $solr_build = new IslandoraSolrQueryProcessor();

  $escaped_pid = str_replace(':', '_', $pid);
  $objects = array();

  $parent_field = variable_get('islandora_custom_solr_compound_relationship', 'RELS_EXT_isConstituentOf_uri_mt');  
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

  // Sort the objects into their proper order.
  $sort = function($a, $b) {
    $a = $a["RELS_EXT_isSequenceNumberOf{$escaped_pid}_literal_ms"];
    $b = $b["RELS_EXT_isSequenceNumberOf{$escaped_pid}_literal_ms"];
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
    $object = islandora_object_load($result['object']['value']);
    if (islandora_object_access(ISLANDORA_VIEW_OBJECTS, $object)) {
      if ($ret_title) {
        $objects[$result['PID']] = array(
          'pid' => $result['PID'],
          'title' => $result['fgs_label_s'],
          'seq' => $result["RELS_EXT_isSequenceNumberOf{$escaped_pid}_literal_ms"],
        );
      }
      else {
        $objects[] = $result['PID'];
      }
    }
  }

  return $objects;
}