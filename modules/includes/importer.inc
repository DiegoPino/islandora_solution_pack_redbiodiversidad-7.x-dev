<?php

/**
 * @file
 * Defines the RedbiodiversidadBatchImporter por GBIF Excel Files
 */

/**
 * XLS or XLSX
 * @author Diego Pino Navarro
 */
@include_once 'sites/all/libraries/phpexcel/PHPExcel.php';
class RedbiodiversidadBatchImporter extends IslandoraBatchImporter {
 protected $itemClass = 'RedbiodiversidadBatchImportObject';
 
 protected static $internal_pidcache=array();//We need PIDS generated on getingestinfo
 //Because this methid is static, we need to access autside the object scope
  
 
 protected function getIdentifier(IslandoraTuque $tuque, $namespace) {
      module_load_include('inc', 'islandora', 'includes/utilities');
	  if (empty($this->context['results']['pid_cache'])) {
        // Get enough PIDs for half of the remaining items.
        // (plus one, so we'll always get at least one).
		//Instead of using the build in parent function use the Islandora API to get the identifier using de uuid variable
        $this->context['results']['pid_cache']=(array) $tuque->repository->getNextIdentifier($namespace, variable_get('islandora_basic_collection_generate_uuid', FALSE), intval((($this->context['sandbox']['max'] - $this->context['sandbox']['progress']) / 2) + 1)
	);//@see Repository.php
		
      }

      return array_shift($this->context['results']['pid_cache']);
    }
	//Static function to get PIDs. 
	protected static function getInternalIdentifier(IslandoraTuque $tuque, $namespace,$cachesize) {
	      module_load_include('inc', 'islandora', 'includes/utilities');
		  if (empty(self::$internal_pidcache)) {
	        // Get enough PIDs given an initial $cachesize
	        // (plus one, so we'll always get at least one).
			//Instead of using the build in parent function use the Islandora API to get the identifier using de uuid variable
	        self::$internal_pidcache=(array) $tuque->repository->getNextIdentifier($namespace, variable_get('islandora_basic_collection_generate_uuid', FALSE), $cachesize + 1);
	      }

	      return array_shift(self::$internal_pidcache);
	    }
 
 
  
  /**
   * Get the number of items to import from $this->file.
   *
   * @see IslandoraBatchImporter::getNumber()
   */
  public function getNumber() {
    
	 $file_contents = file_get_contents(drupal_realpath($this->file['object_info']->uri));
	 $data=unserialize($file_contents); 
	 $numdata=count($data);
	 unset($data);
	 unset($file_contents);
	  
	  error_log("total de registros a importar".$numdata,0);
	     return $numdata; //Devuelve el numero de registros totales
	   
  }

  /**
   * Get the form for this importer.
   *
   * @see IslandoraBatchImporter::getForm()
   */
  public static function getForm(array &$form_state) {
    // Get the list of valid content models.
    //$form = parent::getForm($form_state);
    $parent = islandora_object_load($form_state['storage']['parent_pid']);
    module_load_include('inc', 'islandora_red_biodiversidad', 'includes/utilities');
    $cm_options = array();
    $ns_options = array(
      NULL => t('Default namespace'),
    );
    if (isset($parent['COLLECTION_POLICY'])) {
      $policy_stream = $parent['COLLECTION_POLICY'];
      $policy = new CollectionPolicy($policy_stream->content);
      $cm_options = array();
      $ns_options = array(NULL => t('Default namespace'));
      $connection2 = islandora_get_tuque_connection();
      $implemented_models=islandora_red_biodiversidad_list_implemented_cmodels($connection2);
      unset($connection2);
      unset($implemented_models['biodiversity:DWCACModel']);// we can´t have a Darwin Core Archive as center of itself...ok!
      unset($implemented_models['biodiversity:biodiversityCModel']);
      $realcontentmodels=array_intersect_key($policy->getContentModels(),$implemented_models);
            
      foreach ($realcontentmodels as $pid => $info) {
        $cm_options[$pid] =	array(
			'title' => $info['name'],
	        'collection' => array( 
	  		  'data'=> array(
	        		'#type' => 'textfield',
	        	  	'#size' => 40,
					'#attributes'=>'',
	        	  	'#description' => t('Enter collection name or PID, leave blank to use only parent Collection'),
	       	   		'#autocomplete_path' => 'islandora/relationships/autocomplete/target/islandora:collectionCModel', //reusing import entity csv autocomplete
	  				'#value'=>'',
	        	  	'#required' => TRUE,
					'#message'=>'',
	  			'#name'=> 'dest_collection['.$pid.']',
					'#id'=>'dest_collection_'.md5($pid),
	  			)
	  		),
        'replacebyobject'=> array( 
	  		  'data'=> array(
	        		'#type' => 'textfield',
	        	  	'#size' => 40,
					      '#attributes'=>'',
	        	  	'#description' => t('Enter Object name or PID that you want to use insted of the excel data for this CMODEL<br>. All other row data will be connected to this one. Leave blank to use the provided data'),
	       	   		'#autocomplete_path' => "islandora/relationships/autocomplete/target/{$pid}", //reusing import entity csv autocomplete
	  				'#value'=>'',
	        	  	'#required' => TRUE,
					'#message'=>'',
	  			'#name'=> 'replace_object['.$pid.']',
					'#id'=>'replace_object_'.md5($pid),
	  			)
	  		)
            
            
		 	);
      if ($pid=='biodiversity:SimpledwcCModel') {
        //exception. We can't replace this one by an existing one
        $cm_options[$pid]['replacebyobject']['data']['#attributes']= array('disabled' => TRUE);
        $cm_options[$pid]['replacebyobject']['data']['#description'] = t('Simple Darwin Core can not be replaced by an existing Object');
      }
	  
        $cm_namespace = $info['namespace'];
        if (!empty($cm_namespace)) {
          $cm_namespace = $info['namespace'];
          $cm_namespace = explode(':', $info['namespace']);
          $namespace = trim($cm_namespace[0]);
		  if (empty($namespace))
			  {
			  	$namespace="islandora";
			  } //?¿Still empty???
          $ns_options[$namespace] = $namespace;
        }
      }
    }
    
	
	
	
    $form['fs'] = array(
        '#type' => 'fieldset',
        '#title' => t('Redbiodiversidad Batch XLS GBIF Importer'),
        '#description' => t('Select the XLS or XLSX containing the GBIF formated data'),
        'file' => array(
          '#type' => 'managed_file',
          '#title' => t('XLS file containing the rows to import'),
          '#upload_validators' => array(
            'file_validate_extensions' => array('xls','xlsx'),
          ),
        ),
      'fs2' => array(
          '#type' => 'fieldset',
          '#title' => t('Content Model'),
          '#description' => t('The content model(s) to assign to the imported objects.'),
          'content_model' => array(
            '#type' => 'tableselect',
            '#header' => array('title' => t('Name'),'collection'=>t('Additional destination collection'),'replacebyobject'=>'Use an existing Object instead of Excel Data'),
            '#multiple' => TRUE,
            '#options' => $cm_options,
          ),
          'extrainfo'=>array(
            '#type' => 'markup',
            '#value' =>'<div><em>Note:</em>'.t('The excel File must contain a sheet named "Occurrence" and columns must be named after Darwin Core Schema elements (without the dwc: part). Every column named different as a DWC XML schema field will be included as comma separated pair "columnheader=value" inside dwc:dynamicProperties xml field.We try to be as smart as posible, but if a set of columns belonging to a particular Biodiversity CMODEL have the exact same data in different rows, we must assume they are representing the same object and only one will be created and connected multiple times. Same if you mean the opposite case. If the values differ, then we will create two objects, even when the difference is minimum').'</div>',
          ),
        ),
      );
        
        $form['namespace'] = array(
          '#type' => 'select',
          '#title' => t('Object Namespace'),
          '#description' => t('The namespace in which the imported objects will be created.'),
          '#options' => $ns_options,
        );
        $form['dest_collection'] = array( 
          '#type' => 'value', 
        );
        $form['replace_object'] = array( 
          '#type' => 'value', 
        );
        $form['submit'] = array(
          '#type' => 'submit',
          '#value' => t('Import'),
        );
         $form_state['storage']['parent_pid'] = $parent->id;
  //$form['#submit'][1] = 'gbifexcel_importer_form_submit';
  return $form;  
  }
  
  /**
   * Determine if we're ready to run our batch process.
   *
   * @see IslandoraBatchImporter::readyForBatch()
   */
  public static function readyForBatch(array &$form_state) {
    if (empty($form_state['values']['file'])) {
      form_set_error('file', t('Need a XLS or XLSX file!'));
    }

    $content_models = array_filter($form_state['values']['content_model']);
    if (empty($content_models)) {
      form_set_error('content_model', t('Need to select at least one Content Model for your new objects.'));
    }
	
	if (empty($form_state['values']['namespace']))
		{
		  form_set_error('namespace', t('Need to select a namespace for your new object\'s PIDs.'));	
			
		}
	//Now check if the additional destination Collection exists.
	
	$dest_collection =  array_filter($form_state['values']['dest_collection']);
	$dest_errors=array();
	foreach($content_models as $cmodel)
		{		
			$pid='';
      $dest_collection_access=false;
			$pid = array_key_exists($cmodel, $dest_collection) ? $dest_collection[$cmodel] : NULL;
			if (($form_state['storage']['parent_pid']!=$pid) && !(empty($pid))) //if it's the same as the parent collection the no check required, 
				{
					$form_state['complete form']['fs']['fs2']['content_model']['#options'][$cmodel]['collection']['data']['#value']=$pid;	
					$islandora_object=islandora_object_load($pid); 
					if ($islandora_object)
						{
			    		$models = $islandora_object->models;
			    		if (array_intersect(array('islandora:collectionCModel'), $models)) 
							{
			      	$dest_collection_access=islandora_object_access(ISLANDORA_MANAGE_PROPERTIES, $islandora_object);
							if ($dest_collection_access)
								{
								$form_state['complete form']['fs']['fs2']['content_model']['#options'][$cmodel]['collection']['data']['#attributes']['class']=array();	
								
								}
							else
								{
								 form_set_error('dest_collection['.$pid.']', t('Sorry, the choosen Object %s is not a Collection', array('%s' => $pid)));	
								$dest_errors[]=true;
								}	
			    			}
						else
							{
					 	     form_set_error('dest_collection['.$pid.']', t('Sorry, the choosen Object %s is not a Collection', array('%s' => $pid)));
							 $form_state['complete form']['fs']['fs2']['content_model']['#options'][$cmodel]['collection']['data']['#attributes']['class'][] = 'error'; 	
							$dest_errors[]=true;
							}
						}
					else //Object does not exist
						{
						 $form_state['complete form']['fs']['fs2']['content_model']['#options'][$cmodel]['collection']['data']['#attributes']['class'][] = 'error'; 
						 form_set_error('dest_collection['.$pid.']', t('Sorry, the choosen Colecction Object %s does not exists', array('%s' => $pid)));	
	 				     $dest_errors[]=true;
							
						}	
			
				}
	
		}
  	//Now check if the Objects we wan't to use as replacement are there, and if we got at least a real Excel data field.
	
  	$dest_objects = array_filter($form_state['values']['replace_object']);
  	$dest_objects_errors=array();
    if (count($dest_objects)==count($content_models)) {
		  form_set_error('fs2',t('You need at least 1 CMODEL assigned to real Excel data'));
		  $dest_objects_errors[]=true;
    }
    else {
    foreach($content_models as $cmodel)
  		{		
  			$pid='';
        $dest_object_access=false;
  			$pid = array_key_exists($cmodel,$dest_objects) ? $dest_objects[$cmodel] : NULL;
  			if (($form_state['storage']['parent_pid']!=$pid) && !(empty($pid))) //if it's the same as the parent collection the no check required, 
  				{
  					$form_state['complete form']['fs']['fs2']['content_model']['#options'][$cmodel]['replacebyobject']['data']['#value']=$pid;	
  					$islandora_object=islandora_object_load($pid); 
  					if ($islandora_object)
  						{
  			    		$models = $islandora_object->models;
  			    		if (array_intersect(array($cmodel), $models)) 
  							{
  			      	$dest_object_access=islandora_object_access(ISLANDORA_MANAGE_PROPERTIES, $islandora_object);
  							if ($dest_object_access)
  								{
  								$form_state['complete form']['fs']['fs2']['content_model']['#options'][$cmodel]['replacebyobject']['data']['#attributes']['class']=array();	
								
  								}
  							else
  								{
  								 form_set_error('dest_object['.$pid.']', t('Sorry, you don\'t have manage rights for choosen Object %s ', array('%s' => $pid)));	
  								$dest_objects_errors[]=true;
  								}	
  			    			}
  						else
  							{
 								 form_set_error('dest_object['.$pid.']', t('Sorry, the choosen Object %s is not of type %c', array('%s' => $pid,'%c'=>$cmodel)));	
  							 $form_state['complete form']['fs']['fs2']['content_model']['#options'][$cmodel]['replacebyobject']['data']['#attributes']['class'][] = 'error'; 	
  							$dest_objects_errors[]=true;
  							}
  						}
  					else //Object does not exist
  						{
  						 $form_state['complete form']['fs']['fs2']['content_model']['#options'][$cmodel]['replacebyobject']['data']['#attributes']['class'][] = 'error'; 
  						 form_set_error('dest_collection['.$pid.']', t('Sorry, the choosen Object %s does not exists', array('%s' => $pid)));	
  	 				     $dest_objects_errors[]=true;
							
  						}	
			
  				}
	
  		}
		}
    return (!empty($form_state['values']['file']) && !empty($content_models) && !empty($form_state['values']['namespace']) && empty($dest_errors) && empty($dest_objects_errors));
  }

  /**
   * Get the required info to pass into the file parameter of the constructor.
   *
   * @see IslandoraBatchImporter::getBatchInfo()
   */
  public static function getBatchInfo(array &$form_state) {
    // Load the file.
    module_load_include('inc', 'islandora_red_biodiversidad', 'includes/utilities');
	  
	  $excel_file = file_load($form_state['values']['file']);

    // Grab the content models.
    // Cast to an array, in case there are none?
    $content_models = array_filter((array) $form_state['values']['content_model']);
    //We will define all the transform data here, in case we must deal with separate content models inside a same excel row!
    $dest_collection =  $form_state['values']['dest_collection'];
    $replace_objects =  $form_state['values']['replace_object'];
	
	
	$red_biodiversidad_transform_path = drupal_get_path('module', 'islandora_red_biodiversidad') . '/xsl';
	$transform_path = drupal_get_path('module', 'gbifexcel_importer') . '/xsl';
	$red_biodiversidad_template_path = drupal_get_path('module', 'islandora_red_biodiversidad') . '/xml';
    $contentmodels_xml_proccessing_data=array(
        'biodiversity:OccurrenceCModel' => array(
          'title' => 'occurrenceID',
		  'dest_collection'=>$dest_collection['biodiversity:OccurrenceCModel'],
      'replace_objects'=>$replace_objects['biodiversity:OccurrenceCModel'],
		  'title_alt' => array('collectionID','collectionCode','catalogNumber'),	
		  'template' => $red_biodiversidad_template_path.'/dwc_occurrence_template.xml',
		  'dwc2dc' => $red_biodiversidad_transform_path.'/dwcclassoccurrence2dc.xsl',
        ),
        'biodiversity:LocationCModel' => array(
          'title' => 'locationID',
		  'title_alt' => array('locality'),
		  'dest_collection'=>$dest_collection['biodiversity:LocationCModel'],
      'replace_objects'=>$replace_objects['biodiversity:LocationCModel'],
		  'template' => $red_biodiversidad_template_path.'/dcterms_location_template.xml',
		  'dwc2dc' => $red_biodiversidad_transform_path.'/dctermsclasslocation2dc.xsl',
        ),
        'biodiversity:IdentificationCModel' => array(
           'title' => 'identificationID',
		   'title_alt'=>array('identifiedBy','dateIdentified'),
		   'dest_collection'=>$dest_collection['biodiversity:IdentificationCModel'],
       'replace_objects'=>$replace_objects['biodiversity:IdentificationCModel'],
		   'template' => $red_biodiversidad_template_path.'/dwc_identification_template.xml',
		   'dwc2dc' => $red_biodiversidad_transform_path.'/dwcclassidentification2dc.xsl',
        ),
        'biodiversity:EventCModel' => array(
           'title' => 'eventID',
		   'title_alt'=>array('fieldNumber','eventDate'),
		   'dest_collection'=>$dest_collection['biodiversity:EventCModel'],
       'replace_objects'=>$replace_objects['biodiversity:EventCModel'],
		   'template' => $red_biodiversidad_template_path.'/dwc_event_template.xml',
		   'dwc2dc' => $red_biodiversidad_transform_path.'/dwcclassevent2dc.xsl',
        ),
        'biodiversity:SimpledwcCModel' => array(
            'title' => 'occurrenceID', //Sames as OccurrenceCModel
			'title_alt'=> array('basisOfRecord','catalogNumber'),
			'dest_collection'=>$dest_collection['biodiversity:SimpledwcCModel'],
      'replace_objects'=>NULL,
		    'template' => $red_biodiversidad_template_path.'/dwc_simple_template.xml',
		    'dwc2dc' => $red_biodiversidad_transform_path.'/dwcclass2dc.xsl',
        ),
        'biodiversity:TaxonCModel' => array(
          'title' => 'taxonID',	
		  'title_alt'=> array('scientificName'),
		  'dest_collection'=>$dest_collection['biodiversity:TaxonCModel'],
      'replace_objects'=>$replace_objects['biodiversity:TaxonCModel'],
		  'template' => $red_biodiversidad_template_path.'/dwc_taxon_template.xml',
		  'dwc2dc' => $red_biodiversidad_transform_path.'/dwcclasstaxon2dc.xsl',
    ),
);
    $use_ontologies=FALSE;	
if (module_exists('islandora_ontologies'))
	{
	$use_ontologies=TRUE;	
		module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
	}
	foreach($contentmodels_xml_proccessing_data as $content_model=>$data)
		{
		
		//We are going to try to be smart...get the last ID (dwc:class id) from Solr. If ID is absent from our Excel, then increment this one
		$lastid=islandora_red_biodiversidad_getlast_dwc_mainid_viaSolr($content_model);
		$contentmodels_xml_proccessing_data[$content_model]['lastIDfromSolr']=$lastid[1];
		
			
		if ($use_ontologies)
			{
			$contentmodels_xml_proccessing_data[$content_model]['localrel']=islandora_ontologies_get_all_for_cmodel($content_model);//This returns an array with the ontology pid as key containing array 
			//The full property (Array) =>
			//msg (Array)  "of no use here..."
			//onClass (String) "info:fedora/biodiversity:LocationCModel"
			//ns Namespace
			//localname property local name
			//allowsmore (Boolean) TRUE if possible...if this key is absent means the we can´t add this relation
			//If case of dupicate objects we can´t discard/join new objects because we could end with two relations to the same...should we check this in getingestinfo?
      //This is a reality, sometimes Different Event Objects have exactly the same dates and info, but belong to different occurrence sets and locations. we check if our ontology has the max min data 
      //and make this compliant.
			}
		}
	  // Test implementation, try to use ontologies, if ontologie module is enabled
		
		//Ok, now things are going to get complicated. If we wan´t to be able to relate objects using existing ontologies some compromises must be done
		//- An excel file is a bidimensional representation, so be must deal with an Star Model, one center object that relates to others...but this could lead to duplicated objects...
	  //Or: assume that every selected cmodel is a center of something that relates (possible) to all others...
		//To avoid duplication we could just get an MD5 from an entire record an create an MD5 index of objects by CMODEL. Every time we wan´t to add a new object we first check if this md5 exist. If so avoid creation...
		//We keep track of every other object originated from the same excel row and construct our relations from there. If the to be create object already exists, we replace the relation with this one.
		//Must: we need the object->id to be defined during getIngestInfo, not on preprocess
		
	
	$objects_realinfo=static::getIngestInfo($excel_file,$contentmodels_xml_proccessing_data,$content_models,$form_state['values']['namespace']); 
	//@todo mark the excel file for deleting
	$file = array( //No really a file...but i keep using this name because the parent class defined this structure as such....
      'pid_namespace' => $form_state['values']['namespace'],
      'content_model' => $contentmodels_xml_proccessing_data,//In this case multiple....
      'object_info' => $objects_realinfo, //July 2014, This is the uri of the file that contains the serialized data, no longer passed as an array, too huge for the queue DB.
      'processed_objects' => array(),
    );
    return $file;
	
  }

 
  /**
   * Get ingest info.
   *
   * @return array
   *   The ingest info.
   */
  protected static function getIngestInfo($excelfile,$content_models_with_processing_info,$content_models,$pid_namespace) {
	  
    try {
	    	    $inputFileType = PHPExcel_IOFactory::identify(drupal_realpath($excelfile->uri));
	    	    $objReader = PHPExcel_IOFactory::createReader($inputFileType);
	    	    $objPHPExcel = $objReader->load(drupal_realpath($excelfile->uri));
	    	} catch(Exception $e) {
	    		drupal_set_message(t('No se ha podido leer el archivo @filename con error @error', array('@filename' => $this->file->filename,'@error'=>$e->getMessage())));
	    		
				return;
			}	
	    	$sheet = $objPHPExcel->getSheetByName('Occurrence'); 
	    	$highestRow = $sheet->getHighestRow(); 
	    	$highestColumn = $sheet->getHighestColumn();
			$dwc_fields=array();
			//First get all the xml fields allowed from every defined contentmodel template
			foreach($content_models_with_processing_info as $cmodels=>$cmodels_info)
				{
					$dom= new DOMDocument('1.0', 'UTF-8');
					$dom->load($cmodels_info['template']);
					$dwc_xpath = new DOMXPath($dom);
					$dwc_xpath->preserveWhiteSpace = false;
					$dwc_xpath->registerNamespace('dwc', 'http://rs.tdwg.org/dwc/terms/');//FAKE? 
			
					$camposdwcexistentes2=$dwc_xpath->query("//dwc:*");	
					if (sizeof($camposdwcexistentes2)>0)
					{	
					foreach($camposdwcexistentes2 as $itemdwc)
						{
						$dwc_fields[$cmodels][]=$itemdwc->localName;//Bye prefix
						
						}//Aca deberia agregar todos los campos que no están definidos dentro de un campo DWC adecuado como nombre=valor,
					}
					unset($dwc_xpath);
					unset($dom);
					unset($camposdwcexistentes2);
				}
				
			
	    	//  Loop through each row of the worksheet in turn
	    	$info = array();
			if (($highestRow)>1)//Es decir tengo al menos el header con los campos y un registro
				{
				$internalTuque=islandora_get_tuque_connection();
   	    $rowHeaders = $sheet->rangeToArray('A1:'.$highestColumn.'1', NULL, TRUE, TRUE,FALSE);//Devuelve los titulos de cada columna
				
				//Ok, now we must see which fields belong to which cmodel, just intersect the array preserving the original row data.
				
				$rowHeaders_utf8=array_map('stripslashes',$rowHeaders[0]);
				$rowHeaders_utf8=array_map('utf8_encode',$rowHeaders_utf8);
				//error_log("Mis excel headers".var_export($rowHeaders_utf8,true),0);	
				$whatsleft=$rowHeaders_utf8;
				foreach ($dwc_fields as $dwcfields)//Iterate over all CMODELS, even if not selected
					{
					//All rows that don´t belong to any Biodiversity Content Model	
					$whatsleft=array_diff($whatsleft,$dwcfields);
					}
				
				$rels_index_cmodel_to_cmodel=array();///dimension1 Subject cmodel, dimension2 Target cmodel(with the info:fedora part...sadly), value= relation property
				//Now only iterate over our selected Cmodels
				foreach ($content_models as $cmodels)
					{	
  				if (empty($content_models_with_processing_info[$cmodels]['replace_objects'])) {

					$tmpexistingfields=array_intersect($rowHeaders_utf8,$dwc_fields[$cmodels]);
					if (sizeof($tmpexistingfields)>0)
						{
						$existing_fields_inheader[$cmodels]=$tmpexistingfields;	//we preserve the excel row keys
						//we will use this structure to keep track of md5 to avoid duplicates
						$md5_index_by_cmodel[$cmodels]=array();//Fill this up
						//see what possible relations we can use and remap them so we can acces them more easilly
						foreach($content_models_with_processing_info[$cmodels]['localrel'] as $ontologie => $prop)//We can´t deal with multiple ontologies here...@TODO fix it somehow..
							{
								foreach($prop as $propname=>$propinfo)
								{
								if (array_key_exists('allowsmore',$propinfo))
									{
									if 	(!empty($propinfo['onClass']))
										{
										$rels_index_cmodel_to_cmodel[$cmodels][str_replace("info:fedora/","",$propinfo['onClass'])]=array('ns'=>$propinfo['ns'],'localname'=>$propinfo['localname']);	
										}
									}	
								}	
							}
					
						}
					}
          else { //means we got a replacement object for a bunch of excel columns and rows.
						//we need the relations but not the headers anymore.
            foreach($content_models_with_processing_info[$cmodels]['localrel'] as $ontologie => $prop)//We can´t deal with multiple ontologies here...@TODO fix it somehow..
							{
								foreach($prop as $propname=>$propinfo)
								{
								if (array_key_exists('allowsmore',$propinfo))
									{
									if 	(!empty($propinfo['onClass']))
										{
										$rels_index_cmodel_to_cmodel[$cmodels][str_replace("info:fedora/","",$propinfo['onClass'])]=array('ns'=>$propinfo['ns'],'localname'=>$propinfo['localname']);	
										}
									}	
								}	
							}
          }
						
					}
				//Deal with those fields not present anywhere.
				//If we have this occurrence cmodel we can use dwc:dynamicProperties to store those nasty intruders.
				$pidcachesize=count($existing_fields_inheader)+1;//We set our cache size to the existing cmodels (possible number of objects per row)+1!
					
				
				
						
				//Now we got for every cmodel a list of fields present in this excel file. great. Now fill the info. One row can generate multiple objects.	
				//Also check for duplicates.
				
				$objectcount=0;
        $tmpobject=array();//to store replacement objects, so we don't have to load them everytime.     
			 	for ($fila = 2; $fila <= (int)$highestRow; $fila++)
			 		{
					$thisrow=array();//we empty this every row...it´s just an acumulator of what we have for every row
					
			 		$rowData=$sheet->rangeToArray('A' . $fila . ':' . $highestColumn.$fila, NULL, TRUE, TRUE, FALSE);//Devuelve fila por fila
					$flat = trim(implode('', $rowData[0]));
					
					//check for empty row...if found stop there.
					if (strlen($flat)==0){break;}
					//And now just iterate over $existing_fields_inheader and fill our data. 
					unset($datoscombinados);
					//We first get only the data possible to this contenmodel from the row,then we combine the headernames(keys) with the data.
					
					
					
					if (sizeof($existing_fields_inheader)>0)
					{	
            foreach($existing_fields_inheader as $cmodel => $cmheaders)
            {
              $datoscombinadostmp=array();
              $datoscombinados=array();
              $datoscombinadostmp=array_combine($cmheaders,array_intersect_key($rowData[0],$cmheaders));
              $datoscombinadostmp=array_map('trim',(array) $datoscombinadostmp); //This removes pre and post whitespaces in each member
              $datoscombinados=array_filter((array) $datoscombinadostmp, 'strlen');//This filters out FALSE, NULL, '', but leaves 0 in place
              //basisOfRecord is a common field to all different Darwin Core classes. The thing about this, is that even
              //not defined records/empty fields (like an non registered event i.e) will always get at least one value when mapping fields to header values.
              //This is incorrect and will get me a lot of false relations and unexisting objects. Our templates have defaults for this Record, so we will check if it's in our header
              //if it's, then we can't relay on having > 0 data. 
              
              if (sizeof($datoscombinados)>1) { //Only check for $whatsleft if we got values, we can't define a DWC record based only on this extra values
                if (($cmodel=='biodiversity:OccurrenceCModel') || ($cmodel=='biodiversity:SimpledwcCModel'))
                {
                  if (sizeof($whatsleft)>0)
                  {
                    $nondwcfields=array_combine($whatsleft,array_intersect_key($rowData[0],$whatsleft));	
                    $nondwcfieldsasarrayvalues=array();
                    foreach ($nondwcfields as $key => $values)
                    {
                      $nondwcfieldsasarrayvalues[] = $key.'='.$values;	
                    }
                    if (sizeof($nondwcfieldsasarrayvalues)>0)
                    {
                      $datoscombinados['dynamicProperties']=implode(",",$nondwcfieldsasarrayvalues);
                    }
                  }	
                }
              }
              if (sizeof($datoscombinados)>1) //At least two fields...
              {
                //Now get an md5 representation of data	
                //April 2015. I found a bug. We are not respecting Ontology restrictions using only MD5. If we only enforce md5 as check, then it's possible we add not allowed relations to a same object many times
                //So we need a way of discerning here if the object's md5 is already present AND the existing object does not already have all allowed relations in place...this is complicated!
                //Typical example. An incomplete darwin core Event Record could end being related to multiple locations.
                // MD5 check defines it's the same object (no eventID given to discern) but two different locations can't be associated to the same event.
                $md5=md5(implode(array_values($datoscombinados)));	
                if (!array_key_exists($md5,$md5_index_by_cmodel[$cmodel]))
                {
                  //we are creating a new object
                  if (!array_key_exists($content_models_with_processing_info[$cmodel]['title'],$datoscombinados))//means we dont have an ID for this record
                  {
                    $datoscombinados[$content_models_with_processing_info[$cmodel]['title']]=++$content_models_with_processing_info[$cmodel]['lastIDfromSolr'];
                  }
                  $file[$objectcount]=array('data'=>$datoscombinados,'cmodel'=>$cmodel);		
                  $file[$objectcount]['pid']=self::getInternalIdentifier($internalTuque, $pid_namespace,$pidcachesize);	
                  $file[$objectcount]['localrels']=array();	
                  //check here if we got an DWC ID from Excel, if not autoincrement from Solr	
	   							
												
                  $thisrow[$cmodel]=$objectcount;
								
                  $md5_index_by_cmodel[$cmodel][$md5]=$thisrow[$cmodel];	
                  $objectcount++;
                }
                else
                {
                  //we can reuse and existing object
                  //to resolve the bug we found , we should at this point make a ontology check. If the object does not allow more relations we mark it so. If it allows more relations we reuse it.
                  //For now, users will have to deal with DarwinCore ID's to define this.
                  $thisrow[$cmodel]=$md5_index_by_cmodel[$cmodel][$md5];
                
                
                
                  //error_log('We allready have this record',0);	
                }
              }
              //So we store the desired cmodel and the data in one place...nice.$file aka row is used to keep track from where the data came from excel 
						
            }
					
					
          }
					//Collect what we have for this row and build relations...
					foreach($thisrow as $cmodel => $objectcountid)
						{
							//Check if we have and additional destination Collection defined for this object's CMODEL, if so, add this relation here
						if  (!empty($content_models_with_processing_info[$cmodel]['dest_collection'])) //not optimal, should be check only once?
							{
								$file[$objectcountid]['localrels'][]=array(
									'localname'=>'isMemberOfCollection',
									'ns'=>FEDORA_RELS_EXT_URI,
									'targetobject'=>$content_models_with_processing_info[$cmodel]['dest_collection'],	
								);	
							
							}
						//Iterate over possible localrels.	
						if (isset($rels_index_cmodel_to_cmodel[$cmodel]))
							{
						if (sizeof($rels_index_cmodel_to_cmodel[$cmodel])>0)
							{
								
								foreach($rels_index_cmodel_to_cmodel[$cmodel] as $target=>$prop)
									{
									if (isset($thisrow[$target]))//Only add the relation if there is an target object of this cmodel present in this row.
											{
												$file[$objectcountid]['localrels'][]=array(
													'localname'=>$prop['localname'],
													'ns'=>$prop['ns'],
													'targetobject'=>$file[$thisrow[$target]]['pid'],
									      );	
									    }
                  if (!empty($content_models_with_processing_info[$target]['replace_objects']))   //If we got a replacement object we add relations to it. 
									  $file[$objectcountid]['localrels'][]=array(
										  'localname'=>$prop['localname'],
										  'ns'=>$prop['ns'],
										  'targetobject'=>$content_models_with_processing_info[$target]['replace_objects'],
                     
                    
                    
                    
                    );	
                  }
								if ((isset($file[$objectcountid]['localrels'])) &&	is_array($file[$objectcountid]['localrels']))
									{
									$file[$objectcountid]['localrels'] = array_map("unserialize", array_unique(array_map("serialize", $file[$objectcountid]['localrels'])));//We get rid of duplicates here.
									//error_log('Got relations for:'.var_export($file[$objectcountid]['localrels'],true),0);	
									}
								}
							}
						}
            //Now try to do the inverse operation.Get all replace_objects, load them and add the back relations(uh, oh!).Hope we don't run out of time, or we will have to add an additional
            //batch operation for this..
            foreach($rels_index_cmodel_to_cmodel as $cmodel=>$rels){
              if (!empty($content_models_with_processing_info[$cmodel]['replace_objects'])) {
								foreach($rels_index_cmodel_to_cmodel[$cmodel] as $target=>$prop)
									{
  									if (isset($thisrow[$target]))//Only add the relation if there is an target object of this cmodel present in this row.
  											{
                  				if (!isset($tmpobject[$cmodel]))
                            {
                            $tmpobject[$cmodel] = islandora_object_load($content_models_with_processing_info[$cmodel]['replace_objects'])->relationships;
                            $tmpobject[$cmodel]->autoCommit = FALSE;//lets delay the commit
                            }
                  				
                          if (count($tmpobject[$cmodel]->get($prop['ns'],$prop['localname'],$file[$thisrow[$target]]['pid'])==0))
                            {
                          //means we don´t have already this relation in place...we can proceed	
                            $tmpobject[$cmodel]->add($prop['ns'],$prop['localname'],$file[$thisrow[$target]]['pid']);
                            }
                        } 
                  }   
              }
            }
          
				
				}
			   $objPHPExcel->disconnectWorksheets();
         foreach($tmpobject as $cmode=>$relation)
           {
             $relation->commitRelationships();
           }   
         unset($tmpobject);
         unset($sheet);
				 unset($objPHPExcel);
				 unset($objReader);
				 unset($rels_index_cmodel_to_cmodel);
				 unset($thisrow);
				 unset($internalTuque);
				 
				 if (sizeof($file)==0)
				 	{
				 	drupal_set_message(t('This excel files does not contain enough data for this CMODEL(s)'));
				 	return;
					}
					//return it anyway...empty or not
			     	//July 2014. We are having serious trouble importing large datasets. The reason. Drupals Batch serializes the whole preprocessor. That means our $file array gets inserted as a whole thing inside the queue DB.
					//In case we are dealing with 1000+ rows, this array is huge, and we get a PDO exception. It matters nothing that islandora then processes every object as a individual entitity.
					//Solution: we will have to write this array to disk serialized. Then on every getOne request open the file, get a record, re-serialize and write to disk again. I really don´t like this approach.
					
					$toDisk = serialize($file);
					$newfileName='dwc_gbif_importer_preprocesseddata_'.time().'txt';
					$newfile = file_save_data($toDisk,'public://' .$newfileName);
					return $newfile;	//info...not the input file...i should have used $info instead...but i left this so readers can keep track of variables in the original class definition...		
				
 		    	
				}
			else
				{
		    	drupal_set_message(t('This excel files does not contain enough data for this CMODEL(s)'));
		    	return;	
				}
			  
	  
		

    
  }//had to implement my own...i wanted UUID ID for objects and local relations based on ontologies...
  public function preprocess() {
    $preprocessed = array();
    $item_class = $this->itemClass;
 	  $file_contents = file_get_contents(drupal_realpath($this->file['object_info']->uri));
    $wholedata=unserialize($file_contents); 		
    $total = count($wholedata);
    $localconnection=$this->connection;
	//$internal_setID_counter=0;
	$date = date_create();
	//$this->setId=1;
	$tempfile=$this->file;
	$tempfile['object_info']=$wholedata;//July 2014. This looks messy. I know the getone should do the work, but opening the file and serializing for every getone is getting over my max execution time limit. So i will try it this way,
	//error_log(var_export($this->connection,true),0);
	
	//return $preprocessed;
    for ($i = 0; $i < $total; $i++) {
      $item = $item_class::getOne($tempfile);
      if ($item) {
        $this->parameters['namespace'] = $this->getNamespace($item);
        $preprocessed[] = $object = new IslandoraImporterBatchObject($localconnection, $item, $this->parameters);
        $object->addRelationships();
		//Added local generated id (using uui parameter from islandoras configuration)
		    $object->id=$item->getCurrentPid();
		//$object->addOntologiesRelationships();
  	  	$localrels=$item->getRels();
	  foreach ($localrels as $rels)
  	  	{
		if ($rels['ns']==FEDORA_RELS_EXT_URI)
			{
			$object->relationships->add($rels['ns'], $rels['localname'], $rels['targetobject']);		
			}
		else
			{
			$object->relationships->add($rels['ns'], $rels['localname'], $rels['targetobject'],RELS_TYPE_URI);	
			}
		
		}
		
		$this->addToDatabase($object, array());
		
		unset($item);
		unset($object);
	  }
    }
	  unset ($tempfile);
	  unset ($wholedata);
    return $preprocessed;
  }
}
/**
 * GBIF Excel import object.
 *
 * Actually does the heavy-lifting during the import.
 * @author Diego Pino Navarro
 */
class RedbiodiversidadBatchImportObject extends IslandoraImportObject {

	  protected $mods;
	  protected $title = NULL;
	  protected $dwc = NULL;
	  protected static $DC2MODS = NULL;
	  protected $DWC2DC = NULL ;
	  protected $DWCTEMPLATE = NULL;
	  protected $TITLEFIELD;
	  protected $rels;
	  protected $pid;
	 
      
	  /**
	   * Constructor.
	   */
	  protected function __construct($source) {
	    parent::__construct($source);
	    $this->pidNamespace = $this->source['pid_namespace'];//source==record...
	    $this->contentModel=$this->source['content_model'];
		$transform_path = drupal_get_path('module', 'gbifexcel_importer') . '/xsl';
	    self::$DC2MODS = "$transform_path/simpleDC2MODS.xsl";
		$this->DWC2DC = $this->source['dwc2dc'];
	  $this->DWCTEMPLATE=$this->source['template'];
		$this->TITLEFIELD=$this->source['titlefield'];
		$this->rels=$this->source['localrels'];
		$this->pid=$this->source['pid'];
		
	  }

	  /**
	   * Get an item from the source.
	   *
	   * @see IslandoraImportObject::getOne()
	   * Esta funcion es la que realmente crea la info para el nuevo stream DwC
	   */
	  public function getCurrentPid()
	  	{
	  	return $this->pid;
	  	}
  	  public function getRels()
  	  	{
  	  	return $this->rels;
  	  	}
	  
	  public static function getOne(&$info) {//remember...no really a file.but an array of info...so i will just change this name....it´s my implementation of the class anyway
		
		
  		$record = array();
		$key = key($info['object_info']);
  		$record = array(//A.k.a source chunk..
  	      'pid_namespace' => $info['pid_namespace'],
  	      'data' => $info['object_info'][$key]['data'],
  	      'content_model' => $info['object_info'][$key]['cmodel'],
  	      'dwc2dc' => $info['content_model'][$info['object_info'][$key]['cmodel']]['dwc2dc'],//every cmodel has a different one
  		    'template'=>	$info['content_model'][$info['object_info'][$key]['cmodel']]['template'],
  		    'titlefield'=> $info['content_model'][$info['object_info'][$key]['cmodel']]['title'],
  		    'localrels'=>$info['object_info'][$key]['localrels'],
  		    'pid'=>$info['object_info'][$key]['pid'],
  		);
		 	
  	    unset($info['object_info'][$key]);//Ok, we got one, unset this from the  big big source
  	    return (empty($record) ? FALSE : new static($record));		    
	  }
	  
	  /**
	   * Generates a MODS document representing the imported data.
	   *
	   * @see IslandoraImportObject::getMODS()
	   */
	  public function getMODS()
	  {}
	  
	  
	  protected function getDwC()//Usaremos esta función para crear un DwC XML a partir de cada fila entregada por el excel
	  	{
			
		if ($this->dwc === NULL) {
			$dom= new DOMDocument('1.0', 'UTF-8');
			$dom->load($this->DWCTEMPLATE);
			$dwc_xpath = new DOMXPath($dom);
			$dwc_xpath->preserveWhiteSpace = false;
			$dwc_xpath->registerNamespace('dwc', 'http://rs.tdwg.org/dwc/terms/');
			//error_log("getdwc: mis datos source".var_export($this->source['data'],true),0);
			$datosexcel=$this->source['data'];//data
			
			if (sizeof($datosexcel)>0)
			{
			foreach($datosexcel as $keydw => $valuedw)
				{
				//drupal_set_message(t('Query data es @campos', array('@campos' => $campos)));
				$keydw=utf8_encode(stripslashes($keydw));
				$valuedw=htmlspecialchars(trim($valuedw), ENT_QUOTES,'UTF-8');
					$camposdwcexistentes2=$dwc_xpath->query("//dwc:$keydw");	
					if (sizeof($camposdwcexistentes2)>0)
					{	
					foreach($camposdwcexistentes2 as $itemdwc)
						{
						if(empty($itemdwc->nodeValue)){$itemdwc->nodeValue=$valuedw;}//preserve the defaults of the templates, @todo, ask in the input form
						}
					}
				}
			}
				$this->dwc=$dom->saveXML();
				unset($dwc_xpath);
				unset($dom);
				}
				
				
				
					
				
				return	$this->dwc;
			
				}
			  	
	  
  	  protected function isDwC($xml) {
		  $rootdoc=$this->getLocalNameOfRootElement($xml);
		  if (($rootdoc=='DarwinRecordSet')||($rootdoc=='SimpleDarwinRecordSet'))
		  	{
		  	return true;	
		  	}
  	    else
			{
			return false;	
			}
  	  }
	    

	  /**
	   * Acquire base datastreams structure.
	   *
	   * @see IslandoraImportObject::getDatastreams()
	   * Aca tuvimos que invertir la generacion de XML, primero generamos DwC, luego DC y luego MODS
	   */
	  
	  protected function getDatastreams(&$errors = NULL, &$files = NULL) {
	    module_load_include('inc', 'islandora', 'includes/utilities');
        
		
		
		//$to_return = parent::getDatastreams($errors, $files);
		$obtained_dwc =$this->getDwC();
	    if ($obtained_dwc) {
	      $name = file_create_filename('DwC record.xml', 'public://');
	      $files[] = $file = file_save_data($obtained_dwc, $name);
	      $datastreams[] = array(
	        'dsid' => 'DwC',
	        'label' => 'DwC Record',
	        'mimetype' => 'text/xml',
	        'control_group' => 'M',
			'filename' => $name,
	        'datastream_file' => file_create_url($file->uri),
			'file' => $file,
			
	      );
	    }
	    else {
	      $errors[] = array(
	        t('Failed to produce DwC record for @pid.'),
	      );
	    }
  
		
	    $obtained_dc = $this->getDC();
	    if ($obtained_dc) {
	      $name = file_create_filename('DC record.xml', 'public://');
	      $files[] = $file = file_save_data($obtained_dc, $name);
	      $datastreams[] = array(
	        'dsid' => 'DC',
	        'label' => 'DC Record',
	        'mimetype' => 'text/xml',
	        'control_group' => 'X',
	        'datastream_file' => file_create_url($file->uri),
			'file' => $file,
			
	      );
	    }
	    else {
	      $errors[] = array(
	        t('Failed to produce DC record for @pid.'),
	      );
	    }
	    
        //error_log("mis errores generando datastreams",var_export($errors,true),0);
	    return $datastreams;
	  }

	  

	  /**
	   * Generate DC to describe the imported data.
	   *
	   * @see IslandoraImportObject::getDC()
	   *
	   * @return string
	   *   A string containing DC XML.
	   */
	  
	  public function getTitle() {
	    if (($this->title === NULL) || empty($this->title) ){
			//this is a little redundat...we should have already all in place...if not...the why should we try again?
			$xml = $this->getDC();
			//error_log("mi DC ".var_export($xml,true),0);
		  if ($xml) {			
			$dc_doc = new DOMDocument('1.0', 'UTF-8');
	        $dc_doc->loadXML($xml);
	        $dc_xpath = new DOMXPath($dc_doc);
	        $dc_xpath->registerNamespace('dc', 'http://purl.org/dc/elements/1.1/');
			$result=$dc_xpath->evaluate('dc:title[text()]');
	      	if ($result->length > 0){$this->title= trim($result->item(0)->nodeValue);}
			else
				{
				$result=$dc_xpath->evaluate('dc:type[text()]');
				if ($result->length > 0){$this->title= "Unkown ".trim($result->item(0)->nodeValue)." ";}
				}
				unset($dc_xpath);
				unset($dc_doc);
			}
			else
			{
			//error_log('No DC XML',0);	
			}
	    }
        //error_log('Este es mi titulo'. $this->title,0);
	    return $this->title;
	  }
	  public function getDC() {
  	    if ($this->dc === NULL) {
			if ($this->dwc===NULL)
				{$xml = $this->getDwC();}
			else
				{
				$xml=$this->dwc;
				}
  		  	 if ($this->isDwC($xml)) {//verifico si el DwC quedo bien hecho
				//error_log("MI XSLT para DWC".var_export($this->DWC2DC,true),0);	
  	 	    $this->dc = static::runXSLTransform(array('input' => $xml, 'xsl' => $this->DWC2DC));
	 	        
  			 }
   	         if (empty($this->dc)) {
				 //error_log('vacio!',0);
			 }
		 }
	    return $this->dc;
	  }

	  /**
	   * Gets the label to be used for the given Datastream.
	   *
	   * @param string $dsid
	   *   The datastream identifier.
	   * @param array $datastream
	   *   The datatream as defined by self::getDatastreams().
	   *
	   * @return string
	   *   The label to use for the datastream.
	   */
	  protected function getLabel($dsid, $datastream) {
	   
	    return isset($datastream['label']) ? $datastream['label'] : "$dsid datastream";
	  }

	 

	 
	  protected function isMODS($xml) {
	    $root_elements = array('mods', 'modsCollection');
	    return in_array($this->getLocalNameOfRootElement($xml), $root_elements);
	  }

	 
	  /**
	   * Checks if the given file content is actually a DC document.
	   */
	  protected function isDC($xml) {
	    return $this->getLocalNameOfRootElement($xml) == 'dc';
	  }

	  /**
	   * Gets the local name of the root element of the given xml document.
	   *
	   * @param string $xml
	   *   An xml document.
	   *
	   * @return string
	   *   The local name of the root element, if found. Otherwise NULL.
	   */
	  protected function getLocalNameOfRootElement($xml) {
	    try {
	      $xml = new SimpleXMLElement($xml);
	      return $xml->getName();
	    }
	    catch (Exception $e) {
	      return NULL;
	    }
	  }
	}