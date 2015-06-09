<?php
/**
 * @file
 * Functions for newspaper Solr replacements.
 */

/**
 * Generates the Solr list of newspaper issues.
 */
function __islandora_custom_solr_theme_islandora_newspaper(array $variables) {
  module_load_include('inc', 'islandora_newspaper', 'includes/utilities');
  drupal_add_js('misc/collapse.js');
  $object = $variables['object'];
  $issues = islandora_custom_solr_get_issues($object);
  $grouped_issues = islandora_newspaper_group_issues($issues);
  $output = array(
    'controls' => array(
      '#theme' => 'links',
      '#attributes' => array(
        'class' => array('links', 'inline'),
      ),
      '#links' => array(
        array(
          'title' => t('Expand all months'),
          'href' => "javascript://void(0)",
          'html' => TRUE,
          'external' => TRUE,
          'attributes' => array(
            'onclick' => "Drupal.toggleFieldset(jQuery('fieldset.month.collapsed'));",
          ),
        ),
        array(
          'title' => t('Collapse all months'),
          'href' => "javascript://void(0)",
          'html' => TRUE,
          'external' => TRUE,
          'attributes' => array(
            'onclick' => "Drupal.toggleFieldset(jQuery('fieldset.month:not(.collapsed)'));",
          ),
        ),
      ),
    ),
    'tabs' => array(
      '#type' => 'vertical_tabs',
    ),
  );
  $tabs = &$output['tabs'];
  foreach ($grouped_issues as $year => $months) {
    $tabs[$year] = array(
      '#title' => $year,
      '#type' => 'fieldset',
    );
    foreach ($months as $month => $days) {
      $month_name = t("@date", array(
        "@date" => date("F", mktime(0, 0, 0, $month, 1, 2000)),
      ));
      $tabs[$year][$month] = array(
        '#title' => $month_name,
        '#type' => 'fieldset',
        '#attributes' => array(
          'class' => array('collapsible', 'collapsed', 'month'),
        ),
      );
      foreach ($days as $day => $issues) {
        foreach ($issues as $issue) {
          $tabs[$year][$month][$day][] = array(
            '#theme' => 'link',
            '#prefix' => '<div>',
            '#suffix' => '</div>',
            '#text' => t("@month @day, @year", array(
                '@year' => $year,
                '@month' => $month_name,
                '@day' => $day,
                )),
            '#path' => "islandora/object/{$issue['pid']}",
            '#options' => array(
              'attributes' => array(),
              'html' => FALSE,
            ),
          );
        }
      }
      ksort($tabs[$year][$month]);
    }
    ksort($tabs[$year]);
  }
  ksort($tabs);
  return drupal_render($output);
}

/**
 * Gets all the issues that the given newspaper owns.
 *
 * The results are ordered by their RELS-EXT dateIssued property.
 * Older to newer.
 * Using your Solr index instead of the triplestore
 *
 * @param AbstractObject $object
 *   An AbstractObject representing a Fedora object.
 *
 * @return array
 *   An associative array in the form of:
 *   - pid: The unique persistent identifier for the issue.
 *     - pid: The unique persistent identifier for the issue.
 *     - label: A descriptive label for the issue.
 *     - sequence: The sequence number of the issue, starts at 1.
 *     - issued: A DateTime object repersenting the date the issue was released.
 */
function islandora_custom_solr_get_issues(AbstractObject $object) {
  module_load_include('inc', 'islandora_solr_search', 'includes/query_processor');
  $solr_build = new IslandoraSolrQueryProcessor();

  $parent_field = variable_get('islandora_newspaper_parent_issue_solr_field', 'RELS_EXT_isMemberOf_uri_ms');
  $solr_param = array(
    'rows' => 30000,
    'limit' => 30000,
    'fl' => 'mods_originInfo_dateIssued_dt, fgs_label_s, PID, RELS_EXT_isSequenceNumber_literal_ms',
    'hl' => 'false',
    'facet' => 'false',
  );
  $solr_query = "($parent_field:\"info:fedora/{$object->id}\"+AND+RELS_EXT_hasModel_uri_ms:\"info:fedora/islandora:newspaperIssueCModel\")";
  $solr_build->buildQuery($solr_query, $solr_param);
  $solr_build->solrParams = array_replace_recursive($solr_build->solrParams, $solr_param);
  if (isset($solr_build->solrParams['fq'])) {
    for ($x=0; $x < count($solr_build->solrParams['fq']); $x+=1 ) { 
      if (strpos('-RELS_EXT_hasModel_uri_ms:"info:fedora/islandora:newspaperIssueCModel"', $solr_build->solrParams['fq'][$x]) !== FALSE) {
        unset($solr_build->solrParams['fq'][$x]);
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
  // Map the results using a default Datetime for missing issued dates.
  $map_results = function($o) {
    try {
      @$issued = new DateTime($o['mods_originInfo_dateIssued_dt']);
    }
    catch (Exception $e) {
      // Use the current time as a place holder.
      $issued = new DateTime();
      $msg  = 'Failed to get issued date from SPARQL query for @pid';
      $vars = array('@pid' => $o['PID']);
      watchdog_exception('islandora_newspaper', $e, $msg, $vars, WATCHDOG_ERROR);
    }
    if (array_key_exists('RELS_EXT_isSequenceNumber_literal_ms', $o) && is_array($o['RELS_EXT_isSequenceNumber_literal_ms']) && isset($o['RELS_EXT_isSequenceNumber_literal_ms'][0])) {
      $o['RELS_EXT_isSequenceNumber_literal_ms'] = $o['RELS_EXT_isSequenceNumber_literal_ms'][0];
    }
    else {
      $o['RELS_EXT_isSequenceNumber_literal_ms'] = 0;
    }
    return array(
      'pid' => $o['PID'],
      'label' => $o['fgs_label_s'],
      'sequence' => $o['RELS_EXT_isSequenceNumber_literal_ms'],
      'issued' => $issued,
    );
  };
  $issues = array_map($map_results, $results);
  // Grab the PIDs...
  $get_pid = function($o) {
    return $o['pid'];
  };
  $pids = array_map($get_pid, $issues);
  // Make the PIDs the keys.
  return count($pids) ? array_combine($pids, $issues) : array();
}

function islandora_custom_solr_newspaper_get_pages(AbstractObject $object) {
  module_load_include('inc', 'islandora_solr_search', 'includes/query_processor');
  $solr_build = new IslandoraSolrQueryProcessor();
  
  $parent_field = 'RELS_EXT_isMemberOf_uri_ms';
  $solr_param = array(
    'rows' => 30000,
    'limit' => 30000,
    'fl' => 'fgs_label_s, PID, RELS_EXT_isSequenceNumber_literal_ms',
    'hl' => 'false',
    'facet' => 'false',
  );
  $solr_query = "($parent_field:\"info:fedora/{$object->id}\"+AND+RELS_EXT_hasModel_uri_ms:\"info:fedora/islandora:newspaperPageCModel\")";
  $solr_build->buildQuery($solr_query, $solr_param);
  $solr_build->solrParams = array_replace_recursive($solr_build->solrParams, $solr_param);

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
  // Map the results using a default Datetime for missing issued dates.
  $map_results = function($o) {
    if (array_key_exists('RELS_EXT_isSequenceNumber_literal_ms', $o) && is_array($o['RELS_EXT_isSequenceNumber_literal_ms']) && isset($o['RELS_EXT_isSequenceNumber_literal_ms'][0])) {
      $o['RELS_EXT_isSequenceNumber_literal_ms'] = $o['RELS_EXT_isSequenceNumber_literal_ms'][0];
    }
    else {
      $o['RELS_EXT_isSequenceNumber_literal_ms'] = 0;
    }
    return array(
      'pid' => $o['PID'],
      'label' => $o['fgs_label_s'],
      'page' => $o['RELS_EXT_isSequenceNumber_literal_ms'],
    );
  };
  $pages = array_map($map_results, $results);

  // Grab the PIDs...
  $get_pid = function($o) {
    return $o['pid'];
  };
  $pids = array_map($get_pid, $pages);

  // If we have some pages, combine our remapped results to produce an array
  // mapping pids to the values for that pid.
  $pages = count($pids) ? array_combine($pids, $pages) : array();

  // Sort the pages into their proper order.
  $sort = function($a, $b) {
    $a = (is_array($a) && isset($a['page'])) ? $a['page'] : 0;
    $b = (is_array($b) && isset($b['page'])) ? $b['page'] : 0;
    if ($a == $b) {
      return 0;
    }
    return ($a < $b) ? -1 : 1;
  };
  uasort($pages, $sort);

  return $pages;
}

/**
 * Theme the page selector.
 */
function theme_islandora_custom_solr_newspaper_page_select(array $variables) {
  $path = drupal_get_path('module', 'islandora_newspaper');
  drupal_add_js($path . '/js/islandora_newspaper.js');
  $object = $variables['object'];
  $results = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');
  $result = reset($results);
  $parent = $result ? islandora_object_load($result['object']['value']) : FALSE;
  $pages = $parent ? islandora_custom_solr_newspaper_get_pages($parent) : FALSE;
  if (!$pages) {
    return;
  }

  $variables = array(
    '#options' => array(),
    '#attributes' => array('class' => array('page-select')),
    '#value' => $object->id,
  );
  foreach ($pages as $pid => $page) {
    $variables['#options'][$pid] = $page['page'];
  }
  $prefix = '<strong>' . t('Image') . ':</strong> ';
  return $prefix . t('!page_selector of @total', array(
    '!page_selector' => theme('select', array('element' => $variables)),
    '@total' => count($pages),
  ));
}
