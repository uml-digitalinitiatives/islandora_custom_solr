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
    'islandora_custom_solr_replace_newspaper' => array(
      '#type' => 'checkbox',
      '#title' => t('Enable newspaper replacements'),
      '#disabled' => (!$req_modules['newspaper']),
      '#description' => t('Replaces newspapers get_issues RI queries with Solr ones. @msg',
        array('@msg' => $messages['newspaper'])),
      '#default_value' => (!$req_modules['newspaper'] ? FALSE : variable_get('islandora_custom_solr_replace_newspaper', FALSE)),
      '#return_value' => TRUE,
    ),
    'islandora_custom_solr_replace_books' => array(
      '#type' => 'checkbox',
      '#title' => t('Enable book replacements'),
      '#disabled' => (!$req_modules['book']),
      '#description' => t('Replaces book get_pages RI queries with Solr ones. @msg',
        array('@msg' => $messages['book'])),
      '#default_value' => (!$req_modules['book'] ? FALSE : variable_get('islandora_custom_solr_replace_books', FALSE)),
      '#return_value' => TRUE,
    ),
    'islandora_custom_solr_replace_compound' => array(
      '#type' => 'checkbox',
      '#title' => t('Enable compound replacements'),
      '#disabled' => (!$req_modules['compound']),
      '#description' => t('Replaces compound get_parts RI queries with Solr ones. @msg',
        array('@msg' => $messages['compound'])),
      '#default_value' => (!$req_modules['compound'] ? FALSE : variable_get('islandora_custom_solr_replace_compound', FALSE)),
      '#return_value' => TRUE,
    ),
    'islandora_custom_solr_compound_solr_relationship' => array(
      '#type' => 'textfield',
      '#title' => t('Compound Solr relationship field'),
      '#default_value' => variable_get('islandora_custom_solr_compound_solr_relationship', 'RELS_EXT_isConstituentOf_uri_mt'),
      '#element_validate' => array('islandora_custom_solr_validate_solr_relationship'),
      '#states' => array(
        'visible' => array(
          ':input[name="islandora_custom_solr_replace_compound"]' => array('checked' => TRUE),
        ),
        'required' => array(
          ':input[name="islandora_custom_solr_replace_compound"]' => array('checked' => TRUE),
        ),
      ),
    ),
  );
  return system_settings_form($form);
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
