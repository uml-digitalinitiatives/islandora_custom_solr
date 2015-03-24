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
  $form = array(
    'islandora_custom_solr_replace_newspaper' => array(
      '#type' => 'checkbox',
      '#title' => t('Enable newspaper replacements'),
      '#default_value' => variable_get('islandora_custom_solr_replace_newspaper', FALSE),
      '#return_value' => TRUE,
    ),
    'islandora_custom_solr_replace_books' => array(
      '#type' => 'checkbox',
      '#title' => t('Enable book replacements'),
      '#default_value' => variable_get('islandora_custom_solr_replace_books', FALSE),
      '#return_value' => TRUE,
    ),
  );
  return system_settings_form($form);
}
