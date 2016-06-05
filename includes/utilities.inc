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
	    $model_label="Simple Darwin Core";
	    return($model_label);
    }
	elseif (array_intersect(array('biodiversity:DWCACModel'), $models)) {
	    $model_label="Darwin Core Archive";
	    return($model_label);
    }
	
	return($model_label);

}
/**
 * This function just returns every CMODEL implemented by this Solution Pack using the islandora_required_objects hook.

 */

function islandora_red_biodiversidad_list_implemented_cmodels(IslandoraTuque $connection)
	{
	$objectlist=array();
	$info=islandora_red_biodiversidad_islandora_required_objects($connection);
    if (isset($info['islandora_red_biodiversidad']['objects']))
		{
		foreach ($info['islandora_red_biodiversidad']['objects'] as $object) 
			{
			$existing_object = islandora_object_load($object->id);
			if ($existing_object) 
				{
				if (array_intersect(array('fedora-system:ContentModel-3.0'), $existing_object->models)) 
					{
					$objectlist[$existing_object->id]=array('pid'=>$existing_object->id, 'label'=>$existing_object->label);//Same structure as islandora_get_content_models();
	    			}		
				}
			}
		}	
	return $objectlist;
}


/**
 * Gets the max Main ID for a Darwin Core Class Stream. It requires that this field is indexed as "_s" (string) in Solr. We can´t use multivalued fields for sorting. 
* @param a cmodel_pid from this module
* @see  /solrconfig/DwC_to_solr.xslt and schema.xml. We store very dwcore field as _s and the later copy it to _ms and _mt  for tokenized access(like needed by autocomplete).
*/
function islandora_red_biodiversidad_getlast_dwc_mainid_viaSolr($cmodel_pid)
{
    module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
    $institutioncode = variable_get('islandora_red_biodiversidad_setting_institution_code', 'CBUCN');
	$lastID="";//Assume some large autoincrementable number in case we don´t have anything in our solr index	
	$mappings = array(
      'biodiversity:OccurrenceCModel' => array(
        'field' => 'dwc.occurrenceID',
		    'default' => 'occurrence',
      ),
      'biodiversity:LocationCModel' => array(
        'field' => 'dwc.locationID',
		'default' => 'location',
      ),
      'biodiversity:IdentificationCModel' => array(
         'field' => 'dwc.identificationID',
		 'default' => 'identification',
      ),
      'biodiversity:MaterialSampleCModel' => array(
         'field' => 'dwc.materialSampleID',
		 'default' => 'materialsample',
      ),
      'biodiversity:GeologicalContextCModel' => array(
         'field' => 'dwc.geologicalContextID',
		 'default' => 'geologicalcontext',
      ),
      'biodiversity:EventCModel' => array(
         'field' => 'dwc.eventID',
		 'default' => 'event',
      ),
      'biodiversity:SimpledwcCModel' => array(
      'field' => 'dwc.occurrenceID', //Sames as OccurrenceCModel
		  'default' => 'sdwc_occurrence',
      ),
      'biodiversity:TaxonCModel' => array(
        'field' => 'dwc.taxonID',
		'default' => 'taxon',
		//We should not be calling this function on ingest. Rarely Taxon ID are defined as new(there is almost always a unique existing ID from GBIF, EOL, etc). Also if we already have one of those in our solr, then incrementing it would be a very bad idea! But we implemented some fallback anyway for our import module. Our People-scientist never give taxons an unique ID!
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
    $stub="[$institutioncode TO *]";
   
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
		  if (isset($choice[$mappings[$cmodel_pid]['field']."_s"]))
		  	{
			$lastID=$choice[$mappings[$cmodel_pid]['field']."_s"];
			}
	  }
    }
	//Exception for taxon ID
	if ($cmodel_pid=="biodiversity:TaxonCModel")//I'm not going to autoincrement something that could be originated from somewhere else. 
	{
		//so check if it´s starts with our $institutioncode...this could be changed many times by the administrator, but it´s the only way to be semi-sure.
		if (0 !== strpos($lastID, ' $institutioncode:')) {
			$lastID="";
			//it does not begin with our PID, so set it empty and let the next code handle it
		}
	}
	
    if (empty($lastID))
		{
		$lastID=$institutioncode.":".$mappings[$cmodel_pid]['default'].":0000";//In case our solr does not give us any data, or we are dealing with the first object of it´s kind 
		}
		
		;//Assume some large autoincrementable number in case we don´t have anything in our solr index	)
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

//Similar islandora_xacml_api_islandora_solr_query. But we don´t wan´t to use the limits and other defaults used by the solr api, so we implement our permission check independently

function islandora_red_biodiversidad_getxacml_solrquery_restrictions() {
  global $user;

  if ($user->uid != 0) {
    $user_name = $user->name;
  }
  else {
    $user_name = 'anonymous';
  }

  $fq = variable_get('islandora_xacml_api_rels_viewable_user', 'rels.isViewableByUser') . ":\"" . $user_name . "\" OR ";

  // Check if any of my roles are allowed.
  foreach ($user->roles as $role) {
    $fq .= variable_get('islandora_xacml_api_rels_viewable_role', 'rels.isViewableByRole') . ":\"" . $role . "\" OR ";
  }
  // Check if the object is open (unrestricted).
  $fq .= "((*:* -" . variable_get('islandora_xacml_api_rels_viewable_user', 'rels.isViewableByUser') . ":[* TO *]) AND (*:* -" . variable_get('islandora_xacml_api_rels_viewable_role', 'rels.isViewableByRole') . ":[* TO *]))";
  return $fq;
}
/** This function uses Solr !join sintax to get parent documents, mimics a simple sparql query
 *Assumes our schema.xml and foxmltosolr.xslt is used 
*/

function islandora_red_biodiversidad_getparents_via_solr($rels_ext_fields, $pid )
{
  if (count($rels_ext_fields)==0) {
    return array(); 
    }
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery("*:*");
  $qp->solrParams['fl'] = 'PID,PID_fullurl,fgs_label_s';
  $filters=array();
  
  //Build the first !join
  $direct_rels=array_shift($rels_ext_fields);
  
  $luceneReservedCharacters = preg_quote('+-&|!(){}[]^"~*?:\\');
    //Not nice: we are using a nested query so we need to double escape our only value, the famouse PID
  $pid= preg_replace_callback(
        '/([' . $luceneReservedCharacters . '])/',
        function($matches) {
            return '\\' . $matches[0];
        },
        $pid);
  $filters[] = format_string('_query_:{!join from=!direct_rels to=PID_uri}PID:!pid', array(
      '!direct_rels' => $direct_rels,
      '!pid' => islandora_solr_lesser_escape($pid),
    ));
  
  //*:* AND _query_:"{!join to=PID_uri from=RELS_EXT_identifies_uri_ms}PID_uri:* AND _query_:{!join from=RELS_EXT_taxonOfId_uri_ms to=PID_uri}PID:umce\\:a79c6f05\-87ce\-49e3\-b01a\-b4ac6e15ecba"*:*
  foreach($rels_ext_fields as $rels_ext_field) {
    
    $filters[] = format_string('_query_:"{!join to=PID_uri from=!rels_ext_field}PID_uri:*', array(
        '!rels_ext_field' => $rels_ext_field,
      ));
  }
  $filters[]='*:*';
 
 
  $filters_string=implode(' AND ',array_reverse($filters)).'"*:*';
  $qp->solrParams['fq'] = $filters_string;
  $xacml_restrictions=islandora_red_biodiversidad_getxacml_solrquery_restrictions();
  if (!empty($xacml_restrictions)) {
  $qp->solrParams['fq'] = $qp->solrParams['fq']." AND ( ".$xacml_restrictions." )";
  }
  
  
  $qp->solrLimit = 10000;
  $qp->executeQuery(FALSE);
 
  $results=$qp->islandoraSolrResult;
  if (isset($results['response']) && ($results['response']['numFound']>0)) {
    return array('objects'=>$results['response']['objects'],'numFound'=>$results['response']['numFound']);
    
  }
  return array();
}
/**
 * Autocomplete for collection selection.
 *
 * @param string $string
 *   Contents of textfield being completed
 */
function islandora_red_biodiversidad_get_collection_autocomplete($string = '') {
  $string = str_replace(':', '\:', $string);
  $params = array(
    'fl' => array('dc.title, PID'),
  );
  $string = $string ? " +dc.title:$string*" : '';
  $query = '(+RELS_EXT_hasModel_uri_ms:"info:fedora/islandora:collectionCModel"' . $string . ')';
  $query .= '(+RELS_EXT_hasModel_uri_ms:"info:fedora/islandora:collectionCModel"' . str_replace('dc.title', 'PID', $string) . ')';
  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  try {
    $results = $solr->search($query, 0, 1000, $params);
    $json = json_decode($results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    watchdog_exception('Islandora Biodiversity', $e, 'Got an exception while searching entities for callback.', array(), WATCHDOG_ERROR);
  }
  $list = array();
  foreach ($json['response']['docs'] as $choice) {
    $list[$choice['PID']] = $choice['dc.title'][0] . " ~ " . $choice['PID'];
  }

  drupal_json_output($list);
}



