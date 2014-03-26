<?php
 /*
 * @file
 * Holds helper functions for islandora_red_biodiversidad.
 */

/**
 * This function will add an entity DS composite datastream to an object.
 *
 * This is a workaround of a known issue with temp files being lost in Tuque in
 * batches because of object destruction during the serialization process.
 *
 * @param AbstractObject $islandora_object
 *   The object to add the DS composite datastream to.
 * @see Entities solution pack
 */
function islandora_red_biodiversidad_ingest__red_biodiversidad_ds_composite_datastream(AbstractObject $islandora_object) {
  $module_path = drupal_get_path('module', 'islandora_red_biodiversidad');

  $ds_composite_datastream = $islandora_object->constructDatastream('DS-COMPOSITE-MODEL', 'X');
  $ds_composite_datastream->label = 'DS-COMPOSITE-MODEL';
  $ds_composite_datastream->mimetype = 'text/xml';
  $ds_composite_datastream->setContentFromFile("$module_path/xml/islandora_red_biodiversidad_ds_composite_model.xml", FALSE);
  $islandora_object->ingestDatastream($ds_composite_datastream);
}

/**
 * This function will add the islandora:sp_red_biodiversidad content model to an object.
 *
 * Due to natural inheritance in the data model all red_biodiversidad type
 * objects will have the content model of islandora:sp_red_biodiversidad as well as one
 * for their types.
 *
 * @param AbstractObject $fedora_object
 *   The object to add the relationship to.
 */
function islandora_red_biodiversidad_add_red_biodiversidad_content_model(AbstractObject $fedora_object) {
  $fedora_object->relationships->add(
      FEDORA_MODEL_URI, 'hasModel', 'islandora:sp_red_biodiversidad'
  );
}