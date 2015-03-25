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
function islandora_solrcloud_admin_form(array $form, array &$form_state) {
    
  $form = array(
    'islandora_solrcloud_setting_delete' => array(
      '#type' => 'checkbox',
      '#title' => t('Remove deleted objects from Solr Index directly using this Module'),
      '#default_value' => variable_get('islandora_solrcloud_setting_delete', FALSE),
      '#description' => t('If enabled we will try to committ a delete action directly to Solr index on object purge. This is neccesary if your unique ID in solr schema is different than the object\'s PID, common in a Solr Cloud configuration'),
    ),
    'islandora_solrcloud_setting_uniqueid' => array(
     '#type' => 'textfield',
      '#title' => t('The Solr field name used as unique ID in our current Solr Schema'),
      '#default_value' => variable_get('islandora_solrcloud_setting_uniqueid', 'pid'),
      '#description' => t("The selected collection will de parsed differentle, showing Taxon Records and EOL associated info in addition to linked islandora objects. The purpouse of this is to generate a different and simpler view for Objects belonging to this solution pack"),
      '#size' => 105,
      '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
    ),
    'islandora_solrcloud_setting_uniqueid_prefix' => array(
     '#type' => 'textfield',
      '#title' => t('A prefix used to build the real <id> inside Solr\'s Schema'),
      '#default_value' => variable_get('islandora_solrcloud_setting_uniqueid_prefix',''),
      '#description' => t("When using a solr uniqueid <id> different than object\'s PID, the index stored value should be formed of a prefix(used to define which REPO generated the object) + PID. By Setting this value here, our internal Solr delete document function 
        can build this ID and send the correct query. E.g : PID=islandora:1, prefix: http://www.myserver.com/islandora/object/, Solr <id> used to delete the document value would be: http://www.myserver.com/islandora/object/islandora:1 "),
    )
  );
  return system_settings_form($form);
}
