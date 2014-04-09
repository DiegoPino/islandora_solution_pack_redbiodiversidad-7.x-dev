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
/**
 * This function will add the a custom ICON based on the content model to an object.
 *
 *
 * @param AbstractObject $fedora_object
 *   The object to add the TN stream
 */
function  islandora_red_biodiversidad_add_thumbnail_icon_bycontent_model(AbstractObject $fedora_object) {
    $models = $fedora_object->models;
    // We want to add TN based on the content model.
     $module_path = drupal_get_path('module', 'islandora_red_biodiversidad');
	if (array_intersect(array('islandora:sp_red_biodiversidad_dwcclasstaxon'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwctaxon.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dwcclassoccurrence'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwcoccurrence.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dwcclassevent'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwcevent.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dctermlocation'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dctermlocation.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dwcclassidentification'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwcidentify.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }

	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dwcclasssample'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwcmaterialsample.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dwcclassgeologicalcontext'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwcgeologicalcontext.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_simpledwc'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwsimple.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
  

}
function  islandora_red_biodiversidad_content_model_tohuman(AbstractObject $fedora_object) {
    $models = $fedora_object->models;
    // We want to add TN based on the content model.
     $model_label="Fedora Object";
	if (array_intersect(array('islandora:sp_red_biodiversidad_dwcclasstaxon'), $models)) {
	    $model_label="dwc:Taxon";
		 return($model_label);
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dwcclassoccurrence'), $models)) {
	    $model_label="dwc:Occurrence";
	   return($model_label);
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dwcclassevent'), $models)) {
	    $model_label="dwc:Event";
	    return($model_label);
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dctermlocation'), $models)) {
	     $model_label="dcterms:Location";
	    return($model_label);
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dwcclassidentification'), $models)) {
	     $model_label="dwc:Identification";
	   return($model_label);
    }

	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dwcclasssample'), $models)) {
	     $model_label="dwc:MaterialSample";
	   return($model_label);
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_dwcclassgeologicalcontext'), $models)) {
	   $model_label="dwc:GeologicalContext";
	   return($model_label);
    }
	elseif (array_intersect(array('islandora:sp_red_biodiversidad_simpledwc'), $models)) {
	    $model_label="SimpleDarwinCore";
	    return($model_label);
    }
	 return($model_label);

}

/**
 * Gets any objects that the given object has a parent relationship with.
 *
 * Parent relationships are defined as (isMemberOf, isMemberOfCollection).
 *
 * This function gets its info from the RELS-EXT directly rather than through an
 * risearch.
 *
 * @param AbstractObject $object
 *   The object whose parents will be returned.
 *
 * @return array
 *   An array of FedoraObject's that the given object has a
 *   (isMemberOf, isMemberOfCollection) relationship with.
 */
function islandora_red_biodiversidad_get_parents_from_rels_ext(AbstractObject $object) {
  try {
    $collections = array_merge(
      $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection'),
      $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf'));
  }
  catch (RepositoryException $e) {
    // @todo some logging would be nice, not sure what this throws.
    return array();
  }
  $map = function($o) {
    return islandora_object_load($o['object']['value']);
  };
  $collections = array_map($map, $collections);
  return array_filter($collections);
}
