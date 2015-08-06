<?php
/**
 * @file
 * Common functions.
 */

function islandora_custom_solr_query($query, $solr_params, $options) {
  module_load_include('inc', 'islandora_solr_search', 'includes/query_processor');
  $solr_build = new IslandoraSolrQueryProcessor();

  $default_params = array(
    'rows' => 30000,
    'limit' => 30000,
    'fl' => 'fgs_label_s, PID, RELS_EXT_isSequenceNumber_literal_ms',
    'hl' => 'false',
    'facet' => 'false',
  );

  $solr_params = array_replace_recursive($default_params, $solr_params);
  $solr_build->buildQuery($query, $solr_params);
  $solr_build->solrParams = array_merge_recursive($solr_build->solrParams, $solr_params);
  if (isset($options['hl']) && !$options['hl']) {
    $solr_build->solrParams['hl'] = 'false';
  }
  if (isset($options['facet']) && !$options['facet']) {
    $solr_build->solrParams['facet'] = 'false';
  }
  if (isset($options['object_paging']) && $options['object_paging']) {
    // Object paging requires we return all results, so we start at 0.
    $solr_build->solrStart = 0;
  }
  if (isset($options['stripFQ'])) {
    for ($foo = 0; $foo < count($solr_build->solrParams['fq']); $foo += 1) {
      if (in_array($solr_build->solrParams['fq'][$foo], $options['stripFQ'])) {
        unset($solr_build->solrParams['fq'][$foo]);
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
  return $results;
}
