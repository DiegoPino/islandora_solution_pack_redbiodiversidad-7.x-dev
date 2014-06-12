<?php
 /*
 * @file
 * Holds helper functions for islandora_red_biodiversidad.
 */

/**
 * This function will add an DS composite datastream to an object.
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
 * This function will add an OWL datastream to an object(Content Model)
 * In this solution pack we are using a simililar to EMC's aproach, defining allowed relations(classes) on Content Model level
 * Using ontologies we can describe biodiversity relations for our needs
 * The ontologies are 
 *
 * @param AbstractObject $islandora_object
 *   The object to add the DS composite datastream to.
 * @see http://sourceforge.net/apps/mediawiki/ecm/index.php?title=Ontology_Language
 */
function islandora_red_biodiversidad_ingest__red_biodiversidad_owl_datastream(AbstractObject $islandora_object) {
  $module_path = drupal_get_path('module', 'islandora_red_biodiversidad');
  $owl_datastream = $islandora_object->constructDatastream('OWL', 'X');
  $owl_datastream->label = 'DS-COMPOSITE-MODEL';
  $owl_datastream->mimetype = 'text/xml';
  $owl_datastream->setContentFromFile("$module_path/rdf/islandora_red_biodiversidad_owl.rdf", FALSE);
  $islandora_object->ingestDatastream($owl_datastream);
}



/**
 * This function will add the islandora:sp_red_biodiversidad content model to an object.
 *
 * Due to natural inheritance in the data model all red_biodiversidad type
 * objects will have the content model of islandora:sp_red_biodiversidad as well as one
 * for their Darwin Core types .
 *
 * @param AbstractObject $fedora_object
 *   The object to add the relationship to.
 */
function islandora_red_biodiversidad_add_red_biodiversidad_content_model(AbstractObject $fedora_object) {
  $fedora_object->relationships->add(
      FEDORA_MODEL_URI, 'hasModel', 'biodiversity:biodiversityCModel'
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
	if (array_intersect(array('biodiversity:TaxonCModel'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwctaxon.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('biodiversity:OccurrenceCModel'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwcoccurrence.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('biodiversity:EventCModel'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwcevent.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('biodiversity:LocationCModel'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dctermlocation.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('biodiversity:IdentificationCModel'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwcidentify.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }

	elseif (array_intersect(array('biodiversity:MaterialSampleCModel'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwcmaterialsample.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('biodiversity:GeologicalContextCModel'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwcgeologicalcontext.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('biodiversity:SimpledwcCModel'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwsimple.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	elseif (array_intersect(array('biodiversity:DWCACModel'), $models)) {
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwcarchive.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
	else { /*@TODO, design a default icon */
	    $datastream =  $fedora_object->constructDatastream('TN', 'M');
	    $datastream->label = 'ICON';
	    $datastream->mimetype = 'image/png';
	    $datastream->setContentFromFile("$module_path/images/dwsimple.png", FALSE);
	    $fedora_object->ingestDatastream($datastream);
	   return;
    }
  

}
/**
 * This function just maps the content model PID to a human readable label 
 */
function  islandora_red_biodiversidad_content_model_tohuman(AbstractObject $fedora_object) {
    $models = $fedora_object->models;
    // We want to add TN based on the content model.
     $model_label="Fedora Object";
	if (array_intersect(array('biodiversity:TaxonCModel'), $models)) {
	    $model_label="dwc:Taxon";
		 return($model_label);
    }
	elseif (array_intersect(array('biodiversity:OccurrenceCModel'), $models)) {
	    $model_label="dwc:Occurrence";
	   return($model_label);
    }
	elseif (array_intersect(array('biodiversity:EventCModel'), $models)) {
	    $model_label="dwc:Event";
	    return($model_label);
    }
	elseif (array_intersect(array('biodiversity:LocationCModel'), $models)) {
	     $model_label="dcterms:Location";
	    return($model_label);
    }
	elseif (array_intersect(array('biodiversity:IdentificationCModel'), $models)) {
	     $model_label="dwc:Identification";
	   return($model_label);
    }

	elseif (array_intersect(array('biodiversity:MaterialSampleCModel'), $models)) {
	     $model_label="dwc:MaterialSample";
	   return($model_label);
    }
	elseif (array_intersect(array('biodiversity:GeologicalContextCModel'), $models)) {
	   $model_label="dwc:GeologicalContext";
	   return($model_label);
    }
	elseif (array_intersect(array('biodiversity:SimpledwcCModel'), $models)) {
	    $model_label="SimpleDarwinCore";
	    return($model_label);
    }
	elseif (array_intersect(array('biodiversity:DWCACModel'), $models)) {
	    $model_label="Darwin Core Archive";
	    return($model_label);
    }
	
	return($model_label);

}
/**
 * Gets the max Main ID for a Darwin Core Class Stream. It requires that this field is indexed as "_s" (string) in Solr. We can´t use multivalued fields for sorting. 
* @param a cmodel_pid from this module
* @see  /solrconfig/DwC_to_solr.xslt and schema.xml. We store very dwcore field as _s and the later copy it to _ms and _mt  for tokenized access(like needed by autocomplete).
*/
function islandora_red_biodiversidad_getlast_dwc_mainid_viaSolr($cmodel_pid)
{
    module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
   $lastID="000000";//Assume some large autoincrementable number in case we don´t have anything in our solr index		
	$mappings = array(
      'biodiversity:OccurrenceCModel' => array(
        'field' => 'dwc.occurrenceID',
      ),
      'biodiversity:LocationCModel' => array(
        'field' => 'dwc.locationID',
      ),
      'biodiversity:IdentificationCModel' => array(
         'field' => 'dwc.identificationID',
      ),
      'biodiversity:MaterialSampleCModel' => array(
         'field' => 'dwc.materialSampleID',
      ),
      'biodiversity:GeologicalContextCModel' => array(
         'field' => 'dwc.geologicalContextID',
      ),
      'biodiversity:EventCModel' => array(
         'field' => 'dwc.eventID',
      ),
      'biodiversity:SimpledwcCModel' => array(
          'field' => 'dwc.occurrenceID', //Sames as OccurrenceCModel
      ),
      'biodiversity:TaxonCModel' => array(
        'field' => 'dwc.taxonID',//We won´t we calling this function for this model. Rarely Taxon ID are defined as new(there is almost always a unique existing ID from GBIF, EOL, etc). In case of a 
	),
    );
 	
    $results = array();
	if (!array_key_exists($cmodel_pid,$mappings))//In case we got a wrong contentmodel just give something back
		{
		return array();
		}
    $content_model_field = variable_get(
        'islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms'
    );
   
    $biodiversidad_query_field = $mappings[$cmodel_pid]['field']."_ms";
    $stub="[* TO *]";
   
    $query_processor = new IslandoraSolrQueryProcessor();
    $query_processor->solrQuery = format_string(
        '!biodiversidad_query_field:(!biodiversidad_query)', array(
          '!biodiversidad_query_field' => $biodiversidad_query_field,
          '!biodiversidad_query' => $stub,
        )
    );
    $query_processor->solrStart = 0;
    $query_processor->solrLimit = 1;
    $query_processor->solrParams = array(
      'fl' => 'PID'.','.$mappings[$cmodel_pid]['field']."_s",
	  'sort' => $mappings[$cmodel_pid]['field']."_s"." desc",		  
    );
	
    $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
    $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
    $solr->setCreateDocuments(FALSE);
    try {
      $results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
      $json = json_decode($results->getRawResponse(), TRUE);
    }
    catch (Exception $e) {
      watchdog_exception('Islandora Red de Biodiversidad', $e, 'Got an exception while searching Main DwC ID for an object in Solr for callback.', array(), WATCHDOG_ERROR);
    }
    $list = array();
  
    foreach ($json['response']['docs'] as $choice) {
	
      if (isset($choice['PID'])) {
        $lastID=$choice[$mappings[$cmodel_pid]['field']."_s"];
      }
    }
    dpm($choice);
    return array($mappings[$cmodel_pid]['field'],$lastID);
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
