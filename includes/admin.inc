<?php

/**
 * @file
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_custom_solr_admin_settings_form(array $form, array &$form_state) {
  $use_ajax = FALSE;
  if (module_exists('islandora_solr')) {
    module_load_include('inc', 'islandora_solr', 'includes/utilities');
    $use_ajax = (!islandora_solr_is_date_field(variable_get('islandora_custom_solr_newspaper_issued_date', '')));
  }
  // Need the module to do the replacement.
  $req_modules = array(
    'newspaper' => module_exists('islandora_newspaper'),
    'book' => module_exists('islandora_book'),
    'compound' => module_exists('islandora_compound_object'),
  );
  $messages = array(
    'newspaper' => (!$req_modules['newspaper'] ? "Requires Islandora Newspaper Solution Pack." : ""),
    'book' => (!$req_modules['book'] ? "Requires Islandora Book Solution Pack." : ""),
    'compound' => (!$req_modules['compound'] ? "Requires Islandora Compuond Object Solution Pack." : ""),
  );

  $form = array(
    'newspaper_wrapper' => array(
      '#type' => 'fieldset',
      '#title' => t('Newspapers'),
      'replace_newspaper' => array(
        '#type' => 'checkbox',
        '#title' => t('Enable newspaper replacements'),
        '#disabled' => (!$req_modules['newspaper']),
        '#description' => t('Replaces newspapers resource index queries with Solr ones. @msg',
          array('@msg' => $messages['newspaper'])),
        '#default_value' => (!$req_modules['newspaper'] ? FALSE : variable_get('islandora_custom_solr_replace_newspaper', FALSE)),
        '#return_value' => TRUE,
      ),
      'newspapers_options' => array(
        '#type' => 'container',
        '#states' => array(
          'visible' => array(
            ':input[name="replace_newspaper"]' => array('checked' => TRUE),
          ),
        ),
        'newspaper_date_field' => array(
          '#type' => 'textfield',
          '#title' => t('Issued date field'),
          '#description' => t('Solr field that contains the date issued of the newspaper.'),
          '#size' => 100,
          '#default_value' => variable_get('islandora_custom_solr_newspaper_issued_date', 'RELS_EXT_dateIssued_literal_ms'),
          '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
          '#states' => array(
            'required' => array(
              ':input[name="replace_newspaper"]' => array('checked' => TRUE),
            ),
          ),
        ),
        'newspaper_sequence_field' => array(
          '#type' => 'textfield',
          '#title' => t('Sequence field'),
          '#description' => t('Field to use when sorting pages'),
          '#size' => 100,
          '#default_value' => variable_get('islandora_custom_solr_newspaper_sequence', 'RELS_EXT_isSequenceNumber_literal_ms'),
          '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
          '#states' => array(
            'required' => array(
              ':input[name="replace_newspaper"]' => array('checked' => TRUE),
            ),
          ),
        ),
        'newspaper_relation_field' => array(
          '#type' => 'textfield',
          '#title' => t('Newspaper relationship field'),
          '#description' => t('Solr field that contains the newspaper relationships.'),
          '#size' => 100,
          '#default_value' => variable_get('islandora_custom_solr_newspaper_relationship', 'RELS_EXT_isMemberOf_uri_ms'),
          '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
          '#states' => array(
            'required' => array(
              ':input[name="replace_newspaper"]' => array('checked' => TRUE),
            ),
          ),
        ),
        'newspaper_sequence_field' => array(
          '#type' => 'textfield',
          '#title' => t('Newspaper sequence field'),
          '#description' => t('Solr field holding alternate sequence numbers.'),
          '#size' => 100,
          '#default_value' => variable_get('islandora_custom_solr_newspaper_sequence_field', 'RELS_EXT_isSequenceNumber_literal_ms'),
          '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
          '#states' => array(
            'required' => array(
              ':input[name="replace_newspaper"]' => array('checked' => TRUE),
            ),
          ),
        ),
        'newspaper_ajax_issues' => array(
          '#type' => 'checkbox',
          '#title' => t('Load newspaper issues via AJAX.'),
          '#description' => t('Load years of issues via AJAX when the year is clicked. Requires a Solr date field for "Issued date field".'),
          '#return_value' => TRUE,
          '#disabled' => $use_ajax,
          '#default_value' => variable_get('islandora_custom_solr_newspaper_ajax', FALSE),
        ),
      ),
    ),
    'books_wrapper' => array(
      '#type' => 'fieldset',
      '#title' => t('Books'),
      'replace_books' => array(
        '#type' => 'checkbox',
        '#title' => t('Enable book replacements'),
        '#disabled' => (!$req_modules['book']),
        '#description' => t('Replaces book resource index queries with Solr ones. @msg',
          array('@msg' => $messages['book'])),
        '#default_value' => (!$req_modules['book'] ? FALSE : variable_get('islandora_custom_solr_replace_books', FALSE)),
        '#return_value' => TRUE,
      ),
      'books_options' => array(
        '#type' => 'container',
        '#states' => array(
          'visible' => array(
            ':input[name="replace_books"]' => array('checked' => TRUE),
          ),
        ),
        'book_relation_field' => array(
          '#type' => 'textfield',
          '#title' => t('Book relationship field'),
          '#description' => t('Solr field that contains the relationship of pages to book.'),
          '#size' => 100,
          '#default_value' => variable_get('islandora_custom_solr_book_relationship', 'RELS_EXT_isMemberOf_uri_ms'),
          '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
          '#states' => array(
            'required' => array(
              ':input[name="replace_book"]' => array('checked' => TRUE),
            ),
          ),
        ),
      ),
    ),
    'compound_wrapper' => array(
      '#type' => 'fieldset',
      '#title' => t('Compound Objects'),
      'replace_compound' => array(
        '#type' => 'checkbox',
        '#title' => t('Enable compound replacements'),
        '#disabled' => (!$req_modules['compound']),
        '#description' => t('Replaces compound resource index queries with Solr ones. @msg',
          array('@msg' => $messages['compound'])),
        '#default_value' => (!$req_modules['compound'] ? FALSE : variable_get('islandora_custom_solr_replace_compound', FALSE)),
        '#return_value' => TRUE,
      ),
      'compound_options' => array(
        '#type' => 'container',
        '#states' => array(
          'visible' => array(
            ':input[name="replace_compound"]' => array('checked' => TRUE),
          ),
        ),
        'compound_relation_field' => array(
          '#type' => 'textfield',
          '#title' => t('Compound relationship field'),
          '#description' => t('Solr field that contains the relationship of compound children to parent.'),
          '#size' => 100,
          '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
          '#default_value' => variable_get('islandora_custom_solr_compound_relationship', 'RELS_EXT_isConstituentOf_uri_mt'),
          '#states' => array(
            'required' => array(
              ':input[name="replace_compound"]' => array('checked' => TRUE),
            ),
          ),
        ),
      ),
    ),
    'advisory_message' => array(
      '#markup' => t('Note: Saving will clear all drupal caches.'),
      '#prefix' => '<p>',
      '#suffix' => '</p>',
    ),
    'submit_action' => array(
      '#type' => 'submit',
      '#value' => t('Save configuration'),
    ),
  );
  return $form;
}

/**
 * Implements hook_FORM_ID_validate().
 */
function islandora_custom_solr_admin_settings_form_validate(array $form, array &$form_state) {
  $error = array();
  // Newspaper fields.
  if ($form_state['values']['replace_newspaper'] == 1) {
    if (empty($form_state['values']['newspaper_date_field'])) {
      $error['newspaper_date_field'] = t('Issued date field cannot be blank');
    }
    if (empty($form_state['values']['newspaper_relation_field'])) {
      $error['newspaper_relation_field'] = t('Newspaper relationship field cannot be blank');
    }
    if (empty($form_state['values']['newspaper_sequence_field'])) {
      $error['newspaper_sequence_field'] = t('Newspaper sequence field cannot be blank');
    }
  }
  // Book fields.
  if ($form_state['values']['replace_books'] && empty($form_state['values']['book_relation_field'])) {
    $error['book_relation_field'] = t('Book relationship field is required.');
  }
  // Compound fields.
  if ($form_state['values']['replace_compound'] == 1 && empty($form_state['values']['compound_relation_field'])) {
    $error['compound_relation_field'] = t('Compound relationship field is required.');
  }

  if (count($error) > 0) {
    foreach ($error as $field => $message) {
      form_set_error($field, $message);
    }
  }
}

/**
 * Implements hook_FORM_ID_submit().
 */
function islandora_custom_solr_admin_settings_form_submit(array $form, array &$form_state) {
  variable_set('islandora_custom_solr_replace_newspaper', ($form_state['values']['replace_newspaper'] == 1));
  variable_set('islandora_custom_solr_replace_books', ($form_state['values']['replace_books'] == 1));
  variable_set('islandora_custom_solr_replace_compound', ($form_state['values']['replace_compound'] == 1));
  variable_set('islandora_custom_solr_compound_relationship', $form_state['values']['compound_relation_field']);
  variable_set('islandora_custom_solr_newspaper_relationship', $form_state['values']['newspaper_relation_field']);
  variable_set('islandora_custom_solr_book_relationship', $form_state['values']['book_relation_field']);
  variable_set('islandora_custom_solr_newspaper_issued_date', $form_state['values']['newspaper_date_field']);
  variable_set('islandora_custom_solr_newspaper_sequence_field', $form_state['values']['newspaper_sequence_field']);
  variable_set('islandora_custom_solr_newspaper_ajax', ($form_state['values']['newspaper_ajax_issues'] == 1));
  drupal_flush_all_caches();
  drupal_set_message('Configuration saved.');
}

/**
 * Solr compound relationship element state validation callback.
 *
 * @param array $element
 *   The form element.
 * @param array $form_state
 *   The form state.
 */
function islandora_custom_solr_validate_solr_relationship($element, &$form_state) {
  if ($form_state['values']['islandora_custom_solr_replace_compound'] &&
    empty($form_state['values']['islandora_custom_solr_compound_solr_relationship'])) {
    form_error($element, t('Compound Solr relationship field is required.'));
  }
}
