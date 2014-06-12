<?php

/**
 * @file
 * Handles the display and submission of this modules admin form.
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
function islandora_red_biodiversidad_admin(array $form, array &$form_state) {
    $map_options = function($o) {
      return filter_xss($o['label']);
    };
    $options = array_map($map_options, islandora_basic_collection_get_collections());
	$options[]= array('none' => t('None'));
  
  $form = array(
    'islandora_red_biodiversidad_setting_institution_code' => array(
      '#type' => 'textfield',
      '#title' => t('Default Institution Code for Darwin Core Records'),
      '#default_value' => variable_get('islandora_red_biodiversidad_setting_institution_code', 'CBUCN'),
      '#description' => t('The default Institution Code to prefill Darwin Core Forms. Recomended is to register on http://grbio.org and get there a valid one'),
    ),
    'islandora_red_biodiversidad_setting_public_collection' => array(
     '#type' => 'select',
      '#title' => t('Default public Islandora collection'),
      '#default_value' => variable_get('islandora_red_biodiversidad_setting_public_collection', 'none'),
      '#description' => t("The selected collection will de parsed differentle, showing Taxon Records and EOL associated info in addition to linked islandora objects. The purpouse of this is to generate a different and simpler view for Objects belonging to this solution pack"),
      '#options' => $options, 
	),
    'islandora_red_biodiversidad_setting_use_objectid_as_record_id' => array(
      '#type' => 'checkbox',
      '#title' => t('Use unique generated Islandora Object ID for Darwin Core ID fields'),
      '#default_value' => variable_get('islandora_red_biodiversidad_setting_use_objectid_as_record_id', FALSE),
      '#description' => t("This options enables the use the generated Islandora Object ID as main Darwin Core ID for fields like dwc:occurrenceID, dwc:identificationID, etc. If enabled the corresponding field in the ingest form will be disabled. If disabled, we will try to autoincrement the last one, if possible."),
    ),
  );
  return system_settings_form($form);
}
