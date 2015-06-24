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
  $parent_field = variable_get('islandora_custom_solr_newspaper_relationship', 'RELS_EXT_isMemberOf_uri_ms');
  $date_field = variable_get('islandora_custom_solr_newspaper_issued_date', '');
  $solr_param = array(
    'rows' => 30000,
    'limit' => 30000,
    'fl' => $date_field . ', fgs_label_s, PID, RELS_EXT_isSequenceNumber_literal_ms',
    'hl' => 'false',
    'facet' => 'false',
  );
  $solr_query = "($parent_field:\"info:fedora/{$object->id}\"+AND+RELS_EXT_hasModel_uri_ms:\"info:fedora/islandora:newspaperIssueCModel\")";
  $results = islandora_custom_solr_query($solr_query, $solr_param, array('hl' => FALSE,
    'facet' => FALSE,
    'stripFQ' => array(
      '-RELS_EXT_hasModel_uri_ms:"info:fedora/islandora:newspaperIssueCModel"',
    ),
  ));

  // Map the results using a default Datetime for missing issued dates.
  $map_results = function($o) use ($date_field) {
    try {
      @$issued = new DateTime($o[$date_field]);
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
  $parent_field = variable_get('islandora_custom_solr_newspaper_relationship', 'RELS_EXT_isMemberOf_uri_ms');
  $solr_param = array(
    'rows' => 30000,
    'limit' => 30000,
    'fl' => 'fgs_label_s, PID, RELS_EXT_isSequenceNumber_literal_ms',
    'hl' => 'false',
    'facet' => 'false',
  );
  $solr_query = "($parent_field:\"info:fedora/{$object->id}\"+AND+RELS_EXT_hasModel_uri_ms:\"info:fedora/islandora:newspaperPageCModel\")";
  $results = islandora_custom_solr_query($solr_query, $solr_param, array('hl' => FALSE, 'facet' => FALSE));
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

/**
 * Theme a newspaper pages controls.
 */
function template_preprocess_islandora_custom_solr_newspaper_page_controls(array &$variables) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  global $base_url;
  $view_prefix = '<strong>' . t('View:') . ' </strong>';
  $download_prefix = '<strong>' . t('Download:') . ' </strong>';
  $object = $variables['object'];
  $issue = islandora_custom_solr_get_newspaper($object);
  $issue = $issue ? islandora_object_load($issue['PID']) : FALSE;
  $controls = array(
    'page_select' => theme('islandora_custom_solr_newspaper_page_select', array('object' => $object)),
    'page_pager' => theme('islandora_custom_solr_newspaper_page_navigator', array('object' => $object)),
    'issue_pager' => ($issue ?
      theme('islandora_custom_solr_newspaper_issue_navigator', array('object' => $issue)) :
      ''),
  );

  // Text view.
  if (isset($object['OCR']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['OCR'])) {
    $url = islandora_datastream_get_url($object['OCR'], 'view');
    $attributes = array('attributes' => array('title' => t('View text')));
    $controls['text_view'] = $view_prefix . l(t('Text'), $url, $attributes);
  }
  // PDF download.
  if (isset($object['PDF']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['PDF'])) {
    $size = islandora_datastream_get_human_readable_size($object['PDF']);
    $text = t('PDF (@size)', array('@size' => $size));
    $url = islandora_datastream_get_url($object['PDF'], 'download');
    $attributes = array('attributes' => array('title' => t('Download PDF')));
    $controls['pdf_download'] = $download_prefix . l($text, $url, $attributes);
  }
  if (isset($object['JP2']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['JP2'])) {
    // JP2 download.
    $size = islandora_datastream_get_human_readable_size($object['JP2']);
    $text = t('JP2 (@size)', array('@size' => $size));
    $url = islandora_datastream_get_url($object['JP2'], 'download');
    $attributes = array('attributes' => array('title' => t('Download JP2')));
    $controls['jp2_download'] = $download_prefix . l($text, $url, $attributes);

    $controls['clip'] = theme(
      'islandora_openseadragon_clipper',
      array('pid' => $object->id)
    );
  }
  // TIFF download.
  if (isset($object['OBJ']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['OBJ'])) {
    $size = islandora_datastream_get_human_readable_size($object['OBJ']);
    $text = t('TIFF (@size)', array('@size' => $size));
    $url = islandora_datastream_get_url($object['OBJ'], 'download');
    $attributes = array('attributes' => array('title' => t('Download TIFF')));
    $controls['tiff_download'] = $download_prefix . l($text, $url, $attributes);
  }
  $variables['controls'] = $controls;
}

/**
 * Get the newspaper object directly from the newspaperPage object.
 *
 * @param AbstractObject $object
 *   The newspaperPageCModel object
 *
 * @return mixed
 *   an array of the PID or FALSE
 */
function islandora_custom_solr_get_newspaper($object) {
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

/**
 * Theme the issue navigator.
 */
function theme_islandora_custom_solr_newspaper_issue_navigator(array $variables) {
  $object = $variables['object'];
  $newspaper = islandora_custom_solr_get_newspaper($object);
  $newspaper = $newspaper ? islandora_object_load($newspaper['PID']) : FALSE;
  $issues = $newspaper ? islandora_custom_solr_get_issues($newspaper) : array();
  $issues = array_keys($issues);
  if (empty($issues)) {
    $issues[] = $object->id;
  }
  $index = array_search($object->id, $issues);
  $previous_issue = isset($issues[$index - 1]) ? $issues[$index - 1] : NULL;
  $next_issue = isset($issues[$index + 1]) ? $issues[$index + 1] : NULL;
  $links = array();
  if (isset($issues[$index - 1])) {
    $previous_issue = $issues[$index - 1];
    $links[] = array(
      'title' => t('Prev'),
      'href' => url("islandora/object/{$previous_issue}", array('absolute' => TRUE)),
    );
  }
  if (count($issues) > 1) {
    $links[] = array(
      'title' => '<strong>' . t('Issue') . '</strong>',
      'html' => TRUE,
    );
  }
  if (isset($issues[$index + 1])) {
    $next_issue = $issues[$index + 1];
    $links[] = array(
      'title' => t('Next'),
      'href' => url("islandora/object/{$next_issue}", array('absolute' => TRUE)),
    );
  }
  if ($newspaper) {
    $links[] = array(
      'title' => t('All Issues'),
      'href' => url("islandora/object/{$newspaper->id}", array('absolute' => TRUE)),
    );
  }
  $attributes = array('class' => array('links', 'inline'));
  return theme('links', array('links' => $links, 'attributes' => $attributes));
}

/**
 * Prepares variables for islandora_newspaper_page templates.
 *
 * Default template: islandora-newspaper-page.tpl.php.
 *
 * @param array $variables
 *   An associative array containing:
 *   - object: An AbstractObject for which to generate the display.
 */
function template_preprocess_islandora_custom_solr_newspaper_issue(array &$variables) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  module_load_include('inc', 'islandora', 'includes/metadata');
  $object = $variables['object'];
  $pages = islandora_custom_solr_newspaper_get_pages($object);
  $variables['pages'] = array_keys($pages);

  // Get viewer id.
  $variables['viewer_id'] = islandora_get_viewer_id('islandora_newspaper_issue_viewers');

  // Get viewer content.
  $variables['viewer_params'] = array(
    'object' => $object,
    'pages' => $pages,
    'page_progression' => islandora_paged_content_get_page_progression($object),
  );

  $variables['metadata'] = islandora_retrieve_metadata_markup($object);
  $variables['description'] = islandora_retrieve_description_markup($object);

  // @todo: Remove after 7.x-1.5 release.
  module_load_include('inc', 'islandora_newspaper', 'theme/theme');
  $variables['viewer'] = new IslandoraNewspaperDeprecateString($variables, 'viewer', 'template_process_islandora_custom_solr_newspaper_issue');
}

/**
 * Process variables for islandora_newspaper_issue templates.
 *
 * Default template: islandora-newspaper-issue.tpl.php.
 *
 * @param array $variables
 *   An associative array containing:
 *   - object: An AbstractObject for which to generate the display.
 *   - viewer_id: The "ID" of the viewer to use.
 *   - viewer_params: An (associative) array of parameters to pass to the
 *     viewer.
 */
function template_process_islandora_custom_solr_newspaper_issue(array &$variables) {
  // @todo: Remove condition with "IslandoraNewspaperDeprecateString" after
  // 7.x-1.5 release.
  module_load_include('inc', 'islandora_newspaper', 'theme/theme');
  if ($variables['viewer'] instanceof IslandoraNewspaperDeprecateString) {
    $variables['viewer'] = NULL;
    if ($variables['viewer_id'] && $variables['viewer_id'] != 'none') {
      $viewer_callback = islandora_get_viewer_callback($variables['viewer_id']);
      if (function_exists($viewer_callback)) {
        $viewer = $viewer_callback($variables['viewer_params'], $variables['object']);
        if ($viewer) {
          $variables['viewer'] = $viewer;
        }
      }
    }
  }
}

/**
 * Prepares variables for islandora_newspaper_page templates.
 *
 * Default template: islandora-newspaper-page.tpl.php.
 *
 * @param array $variables
 *   An associative array containing:
 *   - object: An AbstractObject for which to generate the display.
 */
function template_preprocess_islandora_custom_solr_newspaper_page(array &$variables) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  module_load_include('inc', 'islandora', 'includes/metadata');
  $object = $variables['object'];

  $params = array();
  if (isset($object['JP2']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['JP2'])) {
    // Get token to allow access to XACML protected datastreams if necessary.
    // Always use token authentication in case there is a global policy.
    module_load_include('inc', 'islandora', 'includes/authtokens');
    $token = islandora_get_object_token($object->id, 'JP2', 2);
    $jp2_url = url("islandora/object/{$object->id}/datastream/JP2/view",
      array(
        'absolute' => TRUE,
        'query' => array('token' => $token),
      ));

    // Display newspaper.
    $params['jp2_url'] = $jp2_url;
  }
  $variables['viewer_id'] = islandora_get_viewer_id('islandora_newspaper_page_viewers');
  $variables['viewer_params'] = $params;

  // @todo: Remove after 7.x-1.5 release.
  module_load_include('inc', 'islandora_newspaper', 'theme/theme');
  $variables['content'] = new IslandoraNewspaperDeprecateString($variables, 'content', 'template_process_islandora_newspaper_page');

  $variables['metadata'] = islandora_retrieve_metadata_markup($object);
  $variables['description'] = islandora_retrieve_description_markup($object);
}

/**
 * Process variables for islandora_newspaper_page templates.
 *
 * Default template: islandora-newspaper-page.tpl.php.
 *
 * @param array $variables
 *   An associative array containing:
 *   - object: An AbstractObject for which to generate the display.
 *   - viewer_id: The "ID" of the viewer to use.
 *   - viewer_params: An (associative) array of parameters to pass to the
 *     viewer.
 */
function template_process_islandora_custom_solr_newspaper_page(array &$variables) {
  // @todo: Remove condition with "IslandoraNewspaperDeprecateString" after
  // 7.x-1.5 release.
  module_load_include('inc', 'islandora_newspaper', 'theme/theme');
  $object = $variables['object'];
  if ($variables['content'] instanceof IslandoraNewspaperDeprecateString) {
    if ($variables['viewer_id'] && $variables['viewer_id'] != 'none') {
      $viewer_callback = islandora_get_viewer_callback($variables['viewer_id']);
      if (function_exists($viewer_callback)) {
        $viewer = $viewer_callback($variables['viewer_params'], $variables['object']);
        if ($viewer) {
          $variables['content'] = $viewer;
        }
      }
    }
    // Fallback to image.
    elseif (isset($object['JPG']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['JPG'])) {
      $params = array(
        'title' => $object->label,
        'path' => url("islandora/object/{$object->id}/datastream/JPG/view"),
      );
      $variables['content'] = theme('image', $params);
    }
    else {
      $variables['content'] = NULL;
    }
  }
  else {
    $variables['content'] = NULL;
  }
}

/**
 * Theme the page navigator.
 */
function theme_islandora_custom_solr_newspaper_page_navigator(array $variables) {
  $object = $variables['object'];
  $results = $object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');
  $result = reset($results);
  $parent = $result ? islandora_object_load($result['object']['value']) : FALSE;
  $pages = $parent ? islandora_custom_solr_newspaper_get_pages($parent) : array();
  $pages = array_keys($pages);
  if (empty($pages)) {
    $pages[] = $object->id;
  }
  $index = array_search($object->id, $pages);
  $previous_page = isset($pages[$index - 1]) ? $pages[$index - 1] : NULL;
  $next_page = isset($pages[$index + 1]) ? $pages[$index + 1] : NULL;
  $links = array();
  if ($previous_page) {
    $links[] = array(
      'title' => t('Prev'),
      'href' => url("islandora/object/{$previous_page}", array('absolute' => TRUE)),
    );
  }
  $links[] = array(
    'title' => '<strong>' . t('Page') . '</strong>',
    'html' => TRUE,
  );
  if ($next_page) {
    $links[] = array(
      'title' => t('Next'),
      'href' => url("islandora/object/{$next_page}", array('absolute' => TRUE)),
    );
  }
  if ($parent) {
    $links[] = array(
      'title' => t('All Pages'),
      'href' => url("islandora/object/{$parent->id}", array('absolute' => TRUE)),
    );
  }
  return theme('links', array(
    'links' => $links,
    'attributes' => array(
      'class' => array(
        'links',
        'inline',
      ),
    ),
  ));
}
