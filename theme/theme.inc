<?php

/**
 * @file
 * Theme hooks.
 */

/**
 * Implements hook_preprocess().
* Allmost the same as islandora_solr_geo module
* @See islandora_solr_geo
 */
function islandora_red_biodiversidad_preprocess_islandora_red_biodiversidad_map(array &$variables)
	{
	    $module_path = drupal_get_path('module', 'islandora_red_biodiversidad');
	    module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
	    $mappings = array(
	      'location' => array(
	        'cmodel' => 'islandora:sp_red_biodiversidad_dctermlocation',
	        'latlong' => 'dwc.latlong_coordinatefull',
	      ),
    
	    );
 
	    $results = array();

	    $content_model_field = variable_get(
	        'islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms'
	    );
   
	    $biodiversidad_query_field = $mappings['location']['latlong'];
	    $stub="";
	    
	    $query_processor = new IslandoraSolrQueryProcessor();
	    $query_processor->solrQuery = format_string(
	        '!biodiversidad_query_field:(!biodiversidad_query)', array(
	          '!biodiversidad_query_field' => $biodiversidad_query_field,
	          '!biodiversidad_query' => $stub.'*',
	        )
	    );
	    $query_processor->solrStart = 0;
	    $query_processor->solrLimit = 1000;
	    $query_processor->solrParams = array(
	      'fl' => "$biodiversidad_query_field,PID,fgs_label_ms",
	      'fq' => format_string(
	          '!content_model_field:"info:fedora/!content_model"', array(
	            '!content_model_field' => $content_model_field,
	            '!content_model' => $mappings['location']['cmodel'],
	          )
	      ),
	    );
	    $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
	    $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
	    $solr->setCreateDocuments(FALSE);
	    try {
	      $results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
	      $json = json_decode($results->getRawResponse(), TRUE);
	    }
	    catch (Exception $e) {
	      watchdog_exception('Islandora Red de Biodiversidad', $e, 'Got an exception while searching latlong for object in Solr ', array(), WATCHDOG_ERROR);
	    }
	    $list = array();
	    $locations = array();
	    $mapa_variables = array();
		$lati_long = array();
	    foreach ($json['response']['docs'] as $choice) {
	
	      if (isset($choice[$mappings['location']['latlong']])) {
		      $coords = explode(",", $choice[$mappings['location']['latlong']][0]);
		      array_push($lati_long, array($coords[0], $coords[1]));
		      $balloon_variables = array();
		      //$balloon_variables['solr_result'] = $result;
		      //$balloon_markup = theme('islandora_solr_geo_balloon', $balloon_variables);
		      $location = new IslandoraRedBiodiversidadMapObjectLocation(NULL, $coords[0], $coords[1], '<span>'.$choice['fgs_label_ms'][0].'</span>');
		      array_push($locations, $location);		
			//$list[$choice['PID']] = array('latlong'=>$choice[$mappings['location']['latlong']],'label'=>$choice['fgs_label_ms']);
	      }
	    }
	    $mapa_variables['locations'] = $locations;
	    $mapa_variables['map_options'] = '{"mapTypeId":"roadmap", "zoom":50}';
	    $mapa_variables['map_style'] = "height: 550px; width:100%;";
		
		//$locations = $variables['locations'];
	    //$map_options = $variables['map_options'];
	    //$map_style = $variables['map_style'];
	    $variables['results'] = theme('islandora_mapping_map_multi_item',$mapa_variables);	  
	
	}


function islandora_red_biodiversidad_preprocess_islandora_dwc(array &$variables) {
    drupal_add_js('misc/form.js');
    drupal_add_js('misc/collapse.js');
   module_load_include('inc', 'islandora', 'includes/datastream');
   module_load_include('inc', 'islandora', 'includes/utilities');
   module_load_include('inc', 'islandora', 'includes/metadata');
  $module_path = drupal_get_path('module', 'islandora_red_biodiversidad');

  drupal_add_css("$module_path/css/islandora_red_biodiversidad.css");
  $islandora_object = $variables['islandora_object'];
  $object_pid = $islandora_object->id;
  if ($islandora_object['DwC']) {
	 
	$dwc = $islandora_object['DwC']->content;
    $xsl = new DOMDocument();
	$transform="dwcclass2html.xsl";
    $xsl->load("$module_path/xsl/$transform");
    $xslt = new XSLTProcessor();
    $xslt->importStyleSheet($xsl);
    $html = $xslt->transformToXML(simplexml_load_string($dwc));
	$variables['islandora_dwc_html'] = $html;
	}
	
    $variables['parent_collections'] = islandora_get_parents_from_rels_ext($islandora_object);
    $variables['metadata'] = islandora_retrieve_metadata_markup($islandora_object);
    $variables['description'] = islandora_retrieve_description_markup($islandora_object);
   
	if (isset($islandora_object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['TN'])) {
	      $variables['islandora_thumbnail_url'] = url("islandora/object/{$islandora_object->id}/datastream/TN/view");
	   
    }
   
  
  
}
function islandora_red_biodiversidad_preprocess_islandora_dwc_related(array &$variables) {
 $module_path = drupal_get_path('module', 'islandora_red_biodiversidad');
  module_load_include('inc', 'islandora_red_biodiversidad', 'includes/utilities');
$islandora_object = $variables['islandora_object'];
$repository = $islandora_object->repository;
$rels = $islandora_object->relationships->get();
$related = array();
foreach ($rels as $key => $rel) {
  if (($rel['predicate']['alias'] != 'fedora') and ($rel['predicate']['alias'] != 'fedora-model')) {
    
     try {
       $object = $islandora_object->repository->getObject($rel['object']['value']);
	   $model_label=islandora_red_biodiversidad_content_model_tohuman($object);
     } catch (Exception $e) {
       continue;
     }
     if ($object) {
       $related[] = array('relation'=>$rel['predicate']['alias'].":".$rel['predicate']['value'],'object'=>$object,'model_label'=>$model_label);
     } 
  }
}
/**/
  
 
  $pid=$islandora_object->id;
  $connection = islandora_get_tuque_connection();
  $escaped_pid = str_replace(':', '_', $pid);
  if ($connection) {
    $query = <<<EOQ
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
	PREFIX bioscicol: <http://biscicol.org/terms/biscicol.owl#>
SELECT ?object 
FROM <#ri>
WHERE {{
  ?object <fedora-model:hasModel> <info:fedora/islandora:sp_red_biodiversidad>. 
  ?object  biscicol:depends_on+ <info:fedora/$pid>.}UNION{
  ?object  bioscicol:related_to+ ?pid.}.
 
 
  }

EOQ;
    $results = $connection->repository->ri->sparqlQuery($query);
  //var_dump($query);
//print_r($results);// Sort the objects into their proper order.
   /* $sort = function($a, $b) {
      $a = $a['seq']['value'];
      $b = $b['seq']['value'];
      if ($a === $b) {
        return 0;
      }
      if (empty($a)) {
        return 1;
      }
      if (empty($b)) {
        return -1;
      }
      return $a - $b;
    };
    uasort($results, $sort);*/

    foreach ($results as $result) {
        $object_symmetric = $islandora_object->repository->getObject($result['object']['value']);
		$rels2 = $object_symmetric->relationships->get();
		foreach ($rels2 as $key => $rel) {
		  if (($rel['predicate']['alias'] != 'fedora') and ($rel['predicate']['alias'] != 'fedora-model')) {	    
		     try {
				 
				 if (($object_symmetric->id)!=$pid)
				 	{
 	   				 $model_label=islandora_red_biodiversidad_content_model_tohuman( $object_symmetric);
     				if (islandora_object_access(ISLANDORA_VIEW_OBJECTS,  $object_symmetric)) 
						{
	    				if ($object_symmetric)
							{
								switch($rel['predicate']['namespace'])
								{
								case 'http://biscicol.org/terms/biscicol.owl#':
								
								if 	($rel['predicate']['value']=='depends_on'){$relationlabel='is_dependant_of';}else{$relationlabel=$rel['predicate']['value'];}
								break;
								default:
								$relationlabel=$rel['predicate']['namespace'].":".$rel['predicate']['value'];
								}
									
								
	        				$related[] = array('relation'=>$relationlabel,'object'=>$object_symmetric,'model_label'=>$model_label);
	      			  		} 
	  				    }
  				  }
	  		}
		    catch (Exception $e) {
		      watchdog_exception('Islandora Red de Biodiversidad', $e, 'Got an exception while searching for relations', array(), WATCHDOG_ERROR);
		    }
		}
	
	}
}
}
$variables['related'] = $related;
}



function islandora_red_biodiversidad_preprocess_darwincore(array &$variables) {
  module_load_include('inc', 'islandora_red_biodiversidad', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/metadata');
  //module_load_include('inc', 'islandora_entities', 'includes/entities_rss');

  $ficha = "";
  $object = $variables['object'];
  $object_pid = $object->id;

  if ($object['DwC']) {
    $dwc = $object['DwC']->content;
    $simplexml = simplexml_load_string($dwc);
    $dwcrecord = $simplexml->DarwinRecordSet;
    foreach ($dwcrecord  as $term) {
     
        $ficha =(string) $term;
        $variables['dwcoreterms'] = str_replace("\n", "<br />",  $ficha);
      }
    }
  
  if ($object["TN"]) {
    $variables['tn'] = "/islandora/object/$object_pid/datastream/TN/view";
  }
   $variables['ficha']=$ficha;
  $variables['metadata'] = islandora_retrieve_metadata_markup($object, TRUE);
}

