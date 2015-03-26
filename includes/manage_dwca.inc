<?php

/**
 * @file
 * Functions for managing Darwin Core Archive Creation based on Objects in a given Collection
 */
function islandora_red_biodiversidad_dwcarchive_manage(AbstractObject $object) {
  $form_name = 'islandora_red_biodiversidad_dwcarchive_manage_form';

  return drupal_get_form($form_name, $object);
}

function islandora_red_biodiversidad_dwcarchive_manage_form(array $form, array &$form_state, AbstractObject $object) {
 

  $form_state['object_id'] = $object->id;
  drupal_set_message($form_state['object_id']);
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
  module_load_include('inc','islandora','includes/utilities');
  module_load_include('inc', 'islandora_red_biodiversidad','includes/utilities');
  $connection = islandora_get_tuque_connection();
  $source_collection_pid=NULL;
  
    //$required_objects = module_invoke('islandora_red_biodiversidad', 'islandora_required_objects', $connection);//Could be used later if we migrate this generation directly to a tab in a Collection
  $implemented_models=islandora_red_biodiversidad_list_implemented_cmodels($connection);
  unset($implemented_models['biodiversity:DWCACModel']);// we can´t have a Darwin COre Archive as center of itself...ok!
  unset($implemented_models['biodiversity:biodiversityCModel']);
  if (isset($object['EML']))
  	{
	$dom= new DOMDocument;
	$dom->loadXML($object['EML']->content);
	$dom->preserveWhiteSpace = false;
	$collectionIdentifier=$dom->getElementsByTagName('collectionIdentifier');
	foreach ($collectionIdentifier as $item)
		{
		$source_collection_pid=$item->nodeValue;
		}

  $all_content_models=islandora_get_content_models();
  $content_models = array_intersect_key($implemented_models,$all_content_models);
  
  $map_options = function($o) {
    return filter_xss($o['label']);
  };
   $all_collections= array_map($map_options, islandora_basic_collection_get_collections());
   
   
//$form = array();


$form['target_cmodel'] = array(
	   '#prefix' => '<div>'.t('This form allows the generation of a Zipped DWC-A file'), 
	   '#suffix' => '</div>',
      '#title' => t('Target Content Model'),
      '#description' => t('Defines which type of Objects we are going to use as center of the star record. GBIF currently only handles Ocurrences and Taxons'),
      '#type' => 'select',
      '#options' => islandora_basic_collection_get_content_models_as_form_options($content_models),
	'#required' =>'TRUE'
    
);
$form['source_collection'] = array(
      '#title' => t('Source Collection'),
      '#description' => t('Defines from which Collection we are harvesting our Objects. Changing this will also update this Object`s EML'),
      '#type' => 'select',
      '#options' => $all_collections,
	  '#default_value' => $source_collection_pid,
	'#required' =>'TRUE'
   
);

$form['pid'] = array(
  '#type' => 'value',
  '#value' => $object->id,
);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );
  }
 
  return $form;
  
}


function islandora_red_biodiversidad_dwcarchive_manage_form_submit(array $form, array &$form_state)
	{
	  
	    $source_collection =islandora_object_load($form_state['values']['source_collection']);
		
	    $destination = islandora_object_load($form_state['values']['pid']);
	     //$results = islandora_basic_collection_get_member_objects($source_collection, 0, 10, 'view', $form_state['values']['target_cmodel']);
	    $batch = islandora_red_biodiversidad_dwcarchive_batch($source_collection,$destination, $form_state['values']['target_cmodel']); 
	    
		batch_set($batch);
	  
	}

function islandora_red_biodiversidad_dwcarchive_batch(AbstractObject $source_collection, AbstractObject $destination,$cmodel) {
  
	
	$context['finished'] = 0;
    $pid=$destination->id;
	
	  
 	  
  $batch = array(
    'finished' => 'islandora_red_biodiversidad_dwcarchive_batch_finished',
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_red_biodiversidad') . '/includes/manage_dwca.inc',
  );

  $message_parameters = array(
    '@source' => $source_collection->label,
    '@destination' => $destination->label,
  );
  
    $batch += array(
      'operations' => array(
        array(
          'islandora_red_biodiversidad_dwcarchive_batch_operation',
          array($source_collection, $destination, $cmodel
          ),
        ),
      ),
      'title' => t('Processing children of @source to DWC-A in @destination.', $message_parameters),
      'init_message' => t('Preparing to share all children of @source with @destination.', $message_parameters),
    );
  

  return $batch;
}
function islandora_red_biodiversidad_dwcarchive_batch_operation(AbstractObject $source_collection, AbstractObject $destination, $cmodel, &$context) {
  $results = islandora_basic_collection_get_member_objects($source_collection, $context['sandbox']['page'], 50, 'view', $cmodel);
  $context['sandbox']['page']++;
  if ($results) {
    list($count, $members) = $results;
    if (!isset($context['sandbox']['total_count'])) {
      $context['sandbox']['total_count'] = $count;
      $context['sandbox']['current'] = 0;
	  
	  $context['results']['collectionIdentifier']=$source_collection->id;
	  $context['results']['dest_object']=$destination;
	   
	  //$csv_file_uri = drupal_realpath(file_create_filename("{$destination->id}_{$cmodel}.txt", 'temporary://'));
  	  $csv_file_uri =  drupal_realpath(drupal_tempnam('temporary://', 'csv_'));
      $context['results']['islandora_dwca_csv_file'] = $csv_file_uri;
  	  //Lets read our template for the first time
	  $red_biodiversidad_template_path = drupal_get_path('module', 'islandora_red_biodiversidad') . '/xml';  
	  $dom = new DOMDocument('1.0', 'UTF-8');
	  $dom->load($red_biodiversidad_template_path.'/dwc_simple_template.xml');
	  $dwc_xpath = new DOMXPath($dom);
	  $dwc_xpath->preserveWhiteSpace = false;
	  $dwc_xpath->registerNamespace('dwc', 'http://rs.tdwg.org/dwc/terms/');//FAKE? 
	  $camposdwcexistentes2=$dwc_xpath->query("//dwc:*");	
	  if (sizeof($camposdwcexistentes2)>0)
	  {	
	  $context['results']['dwc_fields'][]='id';//Always an ID first	  
	  foreach($camposdwcexistentes2 as $itemdwc)
		{
		
		$context['results']['dwc_fields'][]=$itemdwc->localName;//Bye prefix
		}//Aca deberia agregar todos los campos que no están definidos dentro de un campo DWC adecuado como nombre=valor,
	   $fp = fopen($context['results']['islandora_dwca_csv_file'], 'a');
	   fputcsv($fp, $context['results']['dwc_fields'],"\t");
	   fclose($fp);
   	   $meta_file_uri =  drupal_realpath(drupal_tempnam('temporary://', 'meta_'));
       $context['results']['islandora_dwca_meta_file'] = $meta_file_uri;
	   $fp2 = fopen($context['results']['islandora_dwca_meta_file'], 'a');
	   fwrite($fp2,islandora_red_biodiversidad_dwcarchive_create_metadata($context['results']['dwc_fields']));
	   fclose($fp2);
	  }  
  	  
    }
	
	    $fp = fopen($context['results']['islandora_dwca_csv_file'], 'a');
		
	foreach ($members as $result) {
      $child = $result['object']['value'];
      if ($child && $child = islandora_object_load($child)) {
		  $datos=islandora_red_biodiversidad_dwcarchive_xml_to_csv($child, $context['results']['dwc_fields'],TRUE);//Center data of our star
		  
		  $related_objects=islandora_red_biodiversidad_dwcarchive_get_related($child,$cmodel);
		  foreach($related_objects as $related_cmodel=>$related_object)
		  	{
			$related_datos=islandora_red_biodiversidad_dwcarchive_xml_to_csv($related_object, $context['results']['dwc_fields'],FALSE);
			$related_datos=array_filter($related_datos);
		  	$datos=array_replace($datos,$related_datos);	
		  	}
		  
		  fputcsv($fp, $datos,"\t");
		  //error_log('cargando_datos',0);
        
		$context['sandbox']['current']++;
      }
    }
	
 	   fclose($fp);
   		
  }
  $context['finished'] = $context['sandbox']['current'] / $context['sandbox']['total_count'];
  $context['message'] = 'processing ' . $context['sandbox']['current'] . ' children.';
}


function islandora_red_biodiversidad_dwcarchive_batch_start_operation(array $operands, array &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($operands);
  }
  return @$operands[$context['sandbox']['progress']];
}

/**
 * Ends the batch operation.
 *
 * @param array $context
 *   The context of this batch operation.
 *
 * @return bool
 *   TRUE or FALSE depending on the context of the batch.
 */
function islandora_red_biodiversidad_dwcarchive_batch_end_operation(array &$context) {
  $context['sandbox']['progress']++;
  if ($context['sandbox']['progress'] < $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
    return FALSE;
  }
  return TRUE;
}



/**
 * The batch 'finished' callback.
 */
function islandora_red_biodiversidad_dwcarchive_batch_finished($success, $results, $operations) {
  global $user;
  if (!$success) {
    // Generic Error Message.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
                 '%error_operation' => $error_operation[0],
                 '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  
  }
  else
  {
	  $export_uri = 'public://csv_export_' . time() . '.csv';
	  $export_uri2= 'public://metadata_export' . time() . '.xml';
	  $export_uri3= 'public://EML_export' . time() . '.xml';
	  // We register this file into Drupal so it will be deleted.
	  $file = new stdClass();
	  $file->fid = NULL;
	  $file->uri = $export_uri;
	  $file->filename = drupal_basename($export_uri);
	  $file->filemime = file_get_mimetype($file->uri);
	  $file->uid = $user->uid;
	  $fp = fopen($export_uri, 'w');
	  
	  $tmp_file = fopen($results['islandora_dwca_csv_file'], 'r');
	  stream_copy_to_stream($tmp_file, $fp);
	  fclose($fp);
	  fclose($tmp_file);
	  
	  $file2 = new stdClass();
	  $file2->fid = NULL;
	  $file2->uri = $export_uri2;
	  $file2->filename = drupal_basename($export_uri2);
	  $file2->filemime = file_get_mimetype($file2->uri);
	  $file2->uid = $user->uid;
	  $fp2 = fopen($export_uri2, 'w');
	  
	 
	  
	  $tmp_file2 = fopen($results['islandora_dwca_meta_file'], 'r');
	  stream_copy_to_stream($tmp_file2, $fp2);
	  fclose($fp2);
	  fclose($tmp_file2);
	  
	  //Now get a file for the EML
	  $file3 = new stdClass();
	  $file3->fid = NULL;
	  $file3->uri = $export_uri3;
	  $file3->filename = drupal_basename($export_uri3);
	  $file3->filemime = file_get_mimetype($file3->uri);
	  $file3->uid = $user->uid;
	  //Ok, if everything worked, now modify the EML datastream with the correct collectionIdentifier and get it back as a file to be added into the ZIP
	  
	  $islandora_object=$results['dest_object'];
	  $collectionIdentifier=$results['collectionIdentifier'];
	  if (isset($islandora_object['EML']))//We should have it still here
		
		
		$dom= new DOMDocument;
		$dom->loadXML($islandora_object['EML']->content);
		$dom->preserveWhiteSpace = false;	
		$collectionID=$dom->getElementsByTagName('collectionIdentifier');
  
		foreach($collectionID as $item)
				{	
				$item->nodeValue=$collectionIdentifier;	
				$modified=TRUE;
				}
		if 	($modified)
			{
			$modifiedEML=$dom->saveXML();
			$islandora_object['EML']->setContentFromString($modifiedEML);
			}
			//get the stream into a file
	  $islandora_object['EML']->getContent($export_uri3);
	  
	  
	  
	  
	  //We could manage the ZIP creation in another function, but for now it would be an overkill, just do it here
	  
	  $zip_uri = 'public://DWC-A_' . REQUEST_TIME . '.zip';
	  $zip = new ZipArchive;
	  if ($zip->open(drupal_realpath($zip_uri), ZipArchive::CREATE) === TRUE) {
	      $zip->addFile(drupal_realpath($file->uri), 'occurrence.txt');
		  $zip->addFile(drupal_realpath($file2->uri), 'metadata.xml');
		  $zip->addFile(drupal_realpath($file3->uri), 'eml.xml');
	    
	    $zip->close();
	    drupal_set_message(t('Zip archive successfully created. !link', array('!link' => l(file_create_url($zip_uri), file_create_url($zip_uri)))));
	  }
	  else {
	    drupal_set_message(t('Error creating Zip archive.'), 'error');
	  }

	  //Ok, lets add the zip file to the object as datastream
	  
	 

	  try {
		 if (!isset($islandora_object['DWCA']))
			 { 
	    $datastream = $islandora_object->constructDatastream('DWCA', 'M');
	    $datastream->label = 'DWC-A Zip File';
	    $datastream->mimetype = 'application/zip';
			
	    $datastream->setContentFromFile(drupal_realpath($zip_uri));
	    $islandora_object->ingestDatastream($datastream);
			}
		else
			{
			$islandora_object['DWCA']->setContentFromFile(drupal_realpath($zip_uri));	
			}	
	  }
	  catch (Exception $e) {
	    drupal_set_message(t('@message', array('@message' => check_plain($e->getMessage()))), 'error');
	  }
	
	  
	  
	  file_unmanaged_delete($results['islandora_dwca_csv_file']);
	  file_unmanaged_delete($results['islandora_dwca_meta_file']);
	  drupal_set_message(filter_xss(t('The exported DWCA in CSV is available for download !url.', array(
	    '!url' => l(t('here'), file_create_url($export_uri)),
	  ))));
	  drupal_set_message(filter_xss(t('The exported metatdata in XML is available for download !url.', array(
	    '!url' => l(t('here'), file_create_url($export_uri2)),
	  ))));
 
  }
}

function islandora_red_biodiversidad_dwcarchive_xml_to_csv(AbstractObject $object,$dwc_fields,$basisofrecord)
{
	
   //$object['MODS']->getContent($filename);
       
		$dom= new DOMDocument('1.0', 'UTF-8');
		$dom->loadXML($object['DwC']->content);
		$dwc_xpath = new DOMXPath($dom);
		$dwc_xpath->preserveWhiteSpace = false;
		$dwc_xpath->registerNamespace('dwc', 'http://rs.tdwg.org/dwc/terms/');
		$values=array();
		
		
		if (sizeof($dwc_fields)>0)
		{
		foreach($dwc_fields as $keydw => $valuedw)
			{
			   $values[$keydw]="";
			   if ($keydw==0){$values[0]=$object->id;}
			   if (!$basisofrecord)
			   		{
			   		if ($valuedw=='basisOfRecord')	
						{
						continue;	
						}	
			   		}
			  $camposdwcexistentes2=$dwc_xpath->query("//dwc:$valuedw");	
			  if (sizeof($camposdwcexistentes2)>0)
				{	
				foreach($camposdwcexistentes2 as $itemdwc)
					{
					$values[$keydw]=$itemdwc->nodeValue;
					}
				}		
			
       	 	}
		}
		unset($dom);
		unset($dwc_xpath);
	    
		return $values;
}

function islandora_red_biodiversidad_dwcarchive_create_metadata($dwc_fields)
{
	
   //$object['MODS']->getContent($filename);
        $xml="";
		$red_biodiversidad_template_path = drupal_get_path('module', 'islandora_red_biodiversidad') . '/xml';  
  	  	$dom = new DOMDocument('1.0', 'UTF-8');
 	    $dom->load($red_biodiversidad_template_path.'/dwca_metadata_template.xml');
  	    $dwc_xpath = new DOMXPath($dom);  
        $dwc_xpath->registerNamespace('text', 'http://rs.tdwg.org/dwc/text/');//FAKE? 
        $core=$dwc_xpath->query("//text:core");	
		if (sizeof($core)>0)
			{
				
			foreach($dwc_fields as $keydw => $valuedw)
				{
					if ($keydw!=0)
					{
					$domAttribute = $dom->createAttribute('index');
					$domAttribute2 = $dom->createAttribute('term');
					$element = $dom->createElement('field'); 
					$domAttribute->value=$keydw;
					$domAttribute2->value="http://rs.tdwg.org/dwc/terms/".$valuedw;
					$element->appendChild($domAttribute);
					$element->appendChild($domAttribute2);
					$core->item(0)->appendChild($element);
					}
				}	
				
			}
			$xml=$dom->saveXML();
			return($xml);


}




function islandora_red_biodiversidad_dwcarchive_get_related(AbstractObject $object,$cmodel_star)
{
	module_load_include('inc', 'islandora_ontologies','includes/utilities');
	$pid=$object->id;
	if ($cache = cache_get('islandora_ontologies_adjacency'.$cmodel_star))
		{	
		$result=$cache->data;		
		}		
	else
	{
		//If no cache present we get the full graph paths for this ContentModel
		$fullgraph=islandora_ontologies_get_graph_path($cmodel_star);
		if (count($fullgraph)>0)
		{
			foreach ($fullgraph as $ontologiepid=>$localgraph)
			{
				if (count($localgraph)>0)
				{
					$g = new islandora_ontologies_adjacency_graph($localgraph);
					$result[$ontologiepid]=$g->getdepthFirst("info:fedora/".$cmodel_star);
				}
				
			}
			cache_set('islandora_ontologies_adjacency'.$cmodel_star, $result, 'cache');				
		}
	}			

$o='o';
$s='s';
$query="";
$relationsandcmodels_sparqlmapping_hash=array();
if (!empty($result))
{
foreach ($result as $ontologie=>$graphs)
{		
	foreach ($graphs as $paths=>$rels)
	{
		$o='o';
		$s='s';						
		$cur=0;
		$current_rel="";
		$sparql_object="";	
		foreach($rels as $cmodel=>$rel)
		{
			$querytmp="";	
			$otmp="";			
				
			if (array_key_exists(0,$rel))
			{
				$o=$o=='o'?'s':'o';
				$s=$s=='s'?'o':'s';
				if ($cur>0)
					{			    
				    $relationsandcmodels_sparqlmapping_hash[$sparql_object]=array($cmodel,$current_rel);
					$sparql_object="";	
					$current_rel="";
					}
				
				if (($cur==0) && ($paths==0))
					{
					$otmp="<info:fedora/$pid>";	
					}
				else
					{				
					if 	($cur==0)
						{
						$otmp="UNION {<info:fedora/$pid>";	
						}	
					else
						{	
						if ($cur>1)
							{
							$otmp="}";	
							}
						$otmp.="optional  { ?".$o.$cur."_$paths";
						}
					}	
					$query.=" $otmp <".$rel[0];
					$cur++;
					$query.="> ?$s$cur"."_$paths"."\r\n";
					$sparql_object=$s.$cur."_$paths";	
					$current_rel=$rel;
					if ($cur==sizeof($rels)-1) 
					{
						$query.='}';
						if ($cur>1)
							{
						$query.='}';
							}
					}
						
				}
				else //It´s the last CMODEL, no relation here
				{
					if (!empty($cmodel))
						{
						$relationsandcmodels_sparqlmapping_hash[$sparql_object]=array($cmodel,$current_rel);	
						}
				}		
			}
	}		
}	
	$fullquery = <<<EOQ
	
	SELECT DISTINCT * 
	FROM <#ri>
	WHERE {{
		$query
		}

EOQ;
		
		
	}
	$repository = $object->repository;
	$related = array();
	
	$connection = islandora_get_tuque_connection();
	$escaped_pid = str_replace(':', '_', $pid);
	if (($connection) && !empty($fullquery))
		{
				$results = $connection->repository->ri->sparqlQuery($fullquery);
				$related_sparql_results=array();			
				foreach ($results as $result) 
					{
				   
					$list=array();
					$level=0;
					foreach($result as $sparqlvar=>$hit)
						{
					if ($hit['type']=='pid')	
						{
							$level++;
						    try 
								{
						    	$object = $object->repository->getObject($hit['value']);
						    	}
						    catch (RepositoryException $e) {
						   
						        watchdog_exception('Islandora Red de Biodiversidad', $e, 'Got an exception while trying to get relation for @pid to @targetpid on @rel', array(
							        '@pid' => $pid, '@targetpid' => $hit['value'], '@rel'=>$relationsandcmodels_sparqlmapping_hash[$sparqlvar][1][0]
							      ), WATCHDOG_ERROR);
							}
						$model_label=$relationsandcmodels_sparqlmapping_hash[$sparqlvar][0];
						$related_sparql_results[$model_label]=$object;//This way we only get one Object per CMODEL, only way to convert a multidimensional structure into a star record (bidimensional)
						}			
					}
					
				}
		}

   return ($related_sparql_results);
}




