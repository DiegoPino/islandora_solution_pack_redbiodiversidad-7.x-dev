<?php

/**
 * @file
 * Handles a form for choosing a Source Collection to create a DWCA, used for biodiversity:DWCACMODEL as the first ingest step
 */

/**
 *  Creates the form with the source collection list
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_red_biodiversidad_source_collection_form(array $form, array &$form_state) {
    module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
	$collectionOBJ=$form_state['islandora']['shared_storage']['collection']; //we only get the first one if there are multiple
	$form = array();
	$form['source_collection'] = array(
      'collection' => array(
        '#title' => t('Source Collection'),
        '#description' => t('Defines from which collection we are getting our Biodiversity Objects to generate a GBIF Darwin Core Archive'),
        '#type' => 'select',
        '#options' => islandora_basic_collection_get_other_collections_as_form_options($collectionOBJ),
      ),
  );
  return $form;
  
  
}
function islandora_red_biodiversidad_source_collection_form_submit(array $form, array &$form_state) {
  $form_state['islandora']['shared_storage']['source_collection'] = $form_state['values']['collection'];
  //We set our source collection. This will be set as collectionId value in the ingest form in the metadata section
  
}