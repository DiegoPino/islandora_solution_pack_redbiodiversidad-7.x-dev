<?php

/**
 * @file
 * Theme hooks.
 */

/**
 * Implements hook_preprocess().
 */
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
     watchdog('rosie', 'on object ' . $rel['object']['value']);
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
WHERE {
  ?object  bioscicol:depends_on <info:fedora/$pid>;
  <fedora-model:hasModel> <info:fedora/islandora:sp_red_biodiversidad>
 
  }

EOQ;
    $results = $connection->repository->ri->sparqlQuery($query);
  // var_dump($query);
//	print_r($results);// Sort the objects into their proper order.
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
		
 	   $model_label=islandora_red_biodiversidad_content_model_tohuman( $object_symmetric);
      if (islandora_object_access(ISLANDORA_VIEW_OBJECTS,  $object_symmetric)) {
	      if ($object_symmetric) {
	        $related[] = array('relation'=>'bioscicol:is_dependant_of ','object'=>$object_symmetric,'model_label'=>$model_label);
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

