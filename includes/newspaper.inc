<?php

/**
 * @file
 * Functions for newspaper Solr replacements.
 */

/**
 * Generates the Solr list of newspaper issues.
 */
function __islandora_custom_solr_theme_islandora_newspaper(array $variables) {
  global $base_url;
  module_load_include('inc', 'islandora_newspaper', 'includes/utilities');
  drupal_add_js('misc/collapse.js');
  $object = $variables['object'];
  $date_field = variable_get('islandora_custom_solr_newspaper_issued_date', 'RELS_EXT_dateIssued_literal_ms');
  $use_ajax = FALSE;
  if (module_exists('islandora_solr')) {
    module_load_include('inc', 'islandora_solr', 'includes/utilities');
    $use_ajax = (variable_get('islandora_custom_solr_replace_newspaper', FALSE) &&
      variable_get('islandora_custom_solr_newspaper_ajax', FALSE) &&
      islandora_solr_is_date_field($date_field));
  }
  if ($use_ajax) {
    $years = islandora_custom_solr_newspaper_get_years($object);
    $first_year = reset($years);
    $issues = islandora_custom_solr_get_issues($object, $first_year);
    drupal_add_library('system', 'drupal.ajax');
    drupal_add_js(drupal_get_path('module', 'islandora_custom_solr') . '/js/newspaper_tabs.js', 'file');
    drupal_add_js(array(
      'islandora_custom_solr' => array(
        'newspaper_issue_pid' => $object->id,
        'base_url' => $base_url,
        'first_year' => $first_year
      )), 'setting');
  }
  else {
    $issues = islandora_custom_solr_get_issues($object);
  }
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
  if ($use_ajax) {
    $tabs['#prefix'] = '<div id="islandora_custom_solr_ajax_nav">';
    $tabs['#suffix'] = '</div>';
    _islandora_custom_solr_render_ajax_issues($tabs, $grouped_issues, $years, $first_year);
  }
  // else load all issues at once.
  else {
    _islandora_custom_solr_render_all_issues($tabs, $grouped_issues);
  }
  return $output;
}

/**
 * @param array $tabs
 *   The render array of vertical tabs.
 * @param array $grouped_issues
 *   The array of all issues to display
 */
function _islandora_custom_solr_render_all_issues(array &$tabs, array $grouped_issues) {
  foreach ($grouped_issues as $year => $months) {
    $tabs[$year] = array(
      '#title' => $year,
      '#type' => 'fieldset',
      '#attributes' => array(
        'class' => array('vertical-tab-years'),
      ),
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
}

/**
 * Updates the render array to add the years and the first years issues.
 *
 * @param array $tabs
 *   The render array to update.
 * @param array $grouped_issues
 *   The first years issues.
 * @param array $years
 *   The array of years available.
 * @param $first_year
 *   The first year.
 */
function _islandora_custom_solr_render_ajax_issues(array &$tabs, array $grouped_issues, array $years, $first_year) {
  foreach ($years as $year) {
    $tabs[$year] = array(
      '#title' => $year,
      '#type' => 'fieldset',
      '#attributes' => array(
        'class' => array('vertical-tab-years'),
        'id' => 'fieldset-content-' . $year,
      ),
    );
    if ($first_year == $year) {
      foreach ($grouped_issues[$year] as $month => $days) {
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
    }
    ksort($tabs[$year]);
  }
}

/**
 * @param AbstractObject $object
 *   The newspaper to display issues for.
 * @param $display_year
 *   The year of issues to retrieve.
 *
 * @return array
 *   The render array of the issues to display.
 */
function __islandora_custom_solr_islandora_newspaper_ajax(AbstractObject $object, $display_year) {
  module_load_include('inc', 'islandora_newspaper', 'includes/utilities');
  $issues = islandora_custom_solr_get_issues($object, $display_year);
  $grouped_issues = islandora_newspaper_group_issues($issues);
  $output = array();
  foreach ($grouped_issues[$display_year] as $month => $days) {
    $month_name = t("@date", array(
      "@date" => date("F", mktime(0, 0, 0, $month, 1, 2000)),
    ));
    $output[$month] = array(
      '#title' => $month_name,
      '#type' => 'fieldset',
      '#attributes' => array(
        'class' => array('collapsible', 'collapsed', 'month'),
      ),
    );
    foreach ($days as $day => $issues) {
      foreach ($issues as $issue) {
        $output[$month][$day][] = array(
          '#theme' => 'link',
          '#prefix' => '<div>',
          '#suffix' => '</div>',
          '#text' => t("@month @day, @year", array(
              '@year' => $display_year,
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
    ksort($output[$month]);
  }

  $data = drupal_render($output);
  $output = array(
    '#type' => 'ajax',
    '#commands' => array(
      ajax_command_html("#fieldset-content-$display_year", $data),
    ),
  );
  return $output;
}

/**
 * Return array of years for a newspaper.
 *
 * @param AbstractFedoraObject $object
 *   The newspaper object to get the years available.
 *
 * @return array
 *   Sorted array of year values only.
 */
function islandora_custom_solr_newspaper_get_years(AbstractFedoraObject $object) {

  if (user_is_anonymous()) {
    $cache_roles = 'anonymous';
  }
  else {
    global $user;
    $roles = $user->roles;
    // Sort so we keep ordering consistent to reuse cache.
    asort($roles);
    $cache_roles = implode('_', $roles);
  }

  $years_cache = &drupal_static(__FUNCTION__);
  // Separate cache object for each newspaper.
  $cache_name = 'islandora_custom_solr_newspaper_years_' . str_replace(':', '_', $object->id);

  $years = array();

  if (!isset($years_cache)) {
    $cache = cache_get($cache_name);
    if (!$cache) {
      // Nothing in cache so initialize it with these roles.
      $fetch = $cache_roles;
    }
    else {
      $years_cache = $cache->data;
    }
  }

  if (!isset($years_cache[$cache_roles])) {
    // No cache for this set of roles.
    $fetch = $cache_roles;
  }
  else {
    $years = $years_cache[$cache_roles];
  }

  if (isset($fetch)) {
    $parent_field = variable_get('islandora_custom_solr_newspaper_relationship', 'RELS_EXT_isMemberOf_uri_ms');
    $date_field = variable_get('islandora_custom_solr_newspaper_issued_date', 'RELS_EXT_dateIssued_literal_ms');
    $solr_param = array(
      'rows' => 0,
      'limit' => 0,
      'fl' => $date_field,
      'hl' => 'false',
      'facet' => 'true',
      'facet.field' => $date_field,
      'f.' . $date_field . '.facet.mincount' => 1,
      'f.' . $date_field . '.facet.limit' => -1,
      'f.' . $date_field . '.facet.sort' => 'index',
    );
    $solr_query = format_string("(!parent_field:(\"info:fedora/!pid\" OR \"!pid\")
    AND RELS_EXT_hasModel_uri_ms:\"info:fedora/islandora:newspaperIssueCModel\")",
      array(
        '!parent_field' => $parent_field,
        '!pid' => $object->id,
      )
    );

    $solr_build = new IslandoraSolrQueryProcessor();
    $solr_build->buildQuery($solr_query, $solr_param);
    $solr_build->solrParams = array_replace_recursive(islandora_solr_remove_base_filters($solr_build->solrParams), $solr_param);

    try {
      $solr_build->executeQuery(FALSE);
      $results = $solr_build->islandoraSolrResult['facet_counts']['facet_fields'][$date_field];
    } catch (Exception $e) {
      $results = array();
      drupal_set_message(check_plain(t('Error searching Solr index')) . ' ' . $e->getMessage(), 'error');

    }

    $map = function (&$val) {
      $dt = new DateTime($val);
      $val = $dt->format('Y');
    };
    $years = array_keys($results);
    array_walk($years, $map);
    $years = array_unique($years);
    sort($years);
    $years_cache[$cache_roles] = $years;
    cache_set($cache_name, $years_cache);
  }

  return $years;
}

/**
 * Gets all the issues that the given newspaper owns.
 *
 * The results are ordered by their RELS-EXT dateIssued property.
 * Older to newer.
 * Using your Solr index instead of the triplestore.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing a Fedora object.
 *
 * @param string $year
 *   The 4 digit year to limit results to.
 *
 * @return array
 *   An associative array in the form of:
 *   - pid: The unique persistent identifier for the issue.
 *     - pid: The unique persistent identifier for the issue.
 *     - label: A descriptive label for the issue.
 *     - sequence: The sequence number of the issue, starts at 1.
 *     - issued: A DateTime object repersenting the date the issue was released.
 */
function islandora_custom_solr_get_issues(AbstractObject $object, $year = NULL) {

  $parent_field = variable_get('islandora_custom_solr_newspaper_relationship', 'RELS_EXT_isMemberOf_uri_ms');
  $date_field = variable_get('islandora_custom_solr_newspaper_issued_date', 'RELS_EXT_dateIssued_literal_ms');
  $seq_field = variable_get('islandora_custom_solr_newspaper_sequence_field', 'RELS_EXT_isSequenceNumber_literal_ms');
  $start = -1;
  // Number of records to return at once.
  $rows = 10000;
  $count = 0;
  $results = array();

  $solr_build = new IslandoraSolrQueryProcessor();

  $solr_query = format_string("(!parent_field:(\"info:fedora/!pid\" OR \"!pid\")
AND RELS_EXT_hasModel_uri_ms:\"info:fedora/islandora:newspaperIssueCModel\")",
    array(
      '!parent_field' => $parent_field,
      '!pid' => $object->id,
    )
  );

  do {
    $start += 1;
    $solr_param = array(
      'rows' => $rows,
      'limit' => $rows,
      'fl' => format_string('!date, !sequence, PID, fgs_label_s', array(
        '!date' => $date_field,
        '!sequence' => $seq_field,
      )),
      'start' => ($rows * $start),
      'hl' => 'false',
      'facet' => 'false',
    );

    if (!is_null($year) && preg_match('/^\d{4}$/', $year)) {
      $solr_param['fq'] = array("$date_field:[ $year-01-01T00:00:00Z TO $year-12-31T23:59:59Z ]");
      $solr_param['sort'] = "{$date_field} ASC";
    }

    $solr_build->buildQuery($solr_query, $solr_param);
    $solr_build->solrParams = array_replace_recursive(islandora_solr_remove_base_filters($solr_build->solrParams), $solr_param);

    try {
      $solr_build->executeQuery(FALSE);
      $count = $solr_build->islandoraSolrResult['response']['numFound'];
      $results = array_merge($results, $solr_build->islandoraSolrResult['response']['objects']);
    }
    catch (Exception $error) {
      drupal_set_message(check_plain(t('Error searching Solr index')) . ' ' . $error->getMessage(), 'error');
    }

  } while ($count > ($rows * $start + $rows) && !isset($error));

  // Check the Solr doc array top and inside 'solr_doc'.
  $check_fn = function ($o, $name) {
    $value = "";
    if (isset($o[$name])) {
      $value = $o[$name];
    }
    elseif (isset($o['solr_doc'][$name])) {
      $value = $o['solr_doc'][$name];
    }
    if (is_array($value)) {
      return reset($value);
    }
    else {
      return $value;
    }
  };

  // Map the results using a default Datetime for missing issued dates.
  $map_results = function ($o) use ($date_field, $check_fn, $seq_field) {
    try {
      @$issued = new DateTime($check_fn($o, $date_field));
    }
    catch (Exception $e) {
      // Use the current time as a place holder.
      $issued = new DateTime();
      $msg = 'Failed to get issued date from Solr query for @pid';
      $vars = array('@pid' => $o['PID']);
      watchdog_exception('islandora_newspaper', $e, $msg, $vars, WATCHDOG_ERROR);
    }
    $sequence = $check_fn($o, $seq_field);
    if (empty($sequence)) {
      $sequence = 0;
    }
    return array(
      'pid' => $o['PID'],
      'label' => $o['object_label'],
      'sequence' => $sequence,
      'issued' => $issued,
    );
  };

  $issues = array_map($map_results, $results);
  // Grab the PIDs...
  $get_pid = function ($o) {
    return $o['pid'];
  };

  $pids = array_map($get_pid, $issues);
  // Make the PIDs the keys.
  if (count($pids) > 0) {
    $issues = array_combine($pids, $issues);
  }
  else {
    $issues = array();
  }
  return $issues;
}

/**
 * Get the newspaper object directly from the newspaperPage object.
 *
 * @param AbstractObject $object
 *   The newspaperPageCModel object.
 *
 * @return mixed
 *   an array of the PID or FALSE
 */
function islandora_custom_solr_get_newspaper(AbstractObject $object) {
  module_load_include('inc', 'islandora_solr', 'includes/utilities');
  $parent_field = 'RELS_EXT_isMemberOf_uri_ms';
  $solr_param = array(
    'rows' => 20,
    'limit' => 20,
    'fl' => 'PID',
    'hl' => 'false',
    'facet' => 'false',
    'fq' => array('RELS_EXT_hasModel_uri_ms:"info:fedora/islandora:newspaperCModel"'),
  );
  $pid = islandora_solr_lesser_escape($object->id);
  $solr_query = "{!join from=ancestors_ms to=PID}PID:{$pid}";
  $results = islandora_custom_solr_query($solr_query, $solr_param, array('hl' => FALSE, 'facet' => FALSE));
  return reset($results);
}
