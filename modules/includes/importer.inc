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

 
 
  
  /**
   * Get the number of items to import from $this->file.
   *
   * @see IslandoraBatchImporter::getNumber()
   */
  public function getNumber() {
  
    
	     return count($this->file['object_info']); //Devuelve el numero de registros totales
	     dpm("total objectos",count($this->file['object_info']));
  }

  /**
   * Get the form for this importer.
   *
   * @see IslandoraBatchImporter::getForm()
   */
  public static function getForm(array &$form_state) {
    // Get the list of valid content models.
    $parent = islandora_object_load($form_state['storage']['parent_pid']);

    $cm_options = array();
    $ns_options = array(
      NULL => t('Default namespace'),
    );
    if (isset($parent['COLLECTION_POLICY'])) {
      $policy_stream = $parent['COLLECTION_POLICY'];
      $policy = new CollectionPolicy($policy_stream->content);

      $cm_options = array();
      $ns_options = array(NULL => t('Default namespace'));
      foreach ($policy->getContentModels() as $pid => $info) {
        $cm_options[$pid] = array('title' => $info['name']);

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

    return array(
      'fs' => array(
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
        'fs' => array(
          '#type' => 'fieldset',
          '#title' => t('Content Model'),
          '#description' => t('The content model(s) to assign to the imported objects.'),
          'content_model' => array(
            '#type' => 'tableselect',
            '#header' => array('title' => t('Name')),
            '#multiple' => TRUE,
            '#options' => $cm_options,
          ),
		  'extrainfo'=>array(
              '#type' => 'markup',
              '#value' =>'<div><em>Note:</em>The excel File must contain a sheet named "Occurrence". Every column named different as a DWC XML schema field will be included as comma separated pair "columnheader=value" inside dwc:dynamicProperties xml field</div>',
		  ),
        ),
        'namespace' => array(
          '#type' => 'select',
          '#title' => t('Object Namespace'),
          '#description' => t('The namespace in which the imported objects will be created.'),
          '#options' => $ns_options,
        ),
        'submit' => array(
          '#type' => 'submit',
          '#value' => t('Import'),
        ),
      ),
    );
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
      form_set_error('content_model', t('Need to select at least one content model.'));
    }

    return !empty($form_state['values']['file']) && !empty($content_models);
  }

  /**
   * Get the required info to pass into the file parameter of the constructor.
   *
   * @see IslandoraBatchImporter::getBatchInfo()
   */
  public static function getBatchInfo(array &$form_state) {
    // Load the file.
    $file = file_load($form_state['values']['file']);

    // Grab the content models.
    // Cast to an array, in case there are none?
    $content_models = array_filter((array) $form_state['values']['content_model']);
    //We will define all the transform data here, in case we must deal with separate content models inside a same excel row!
    
	$red_biodiversidad_transform_path = drupal_get_path('module', 'islandora_red_biodiversidad') . '/xsl';
	$transform_path = drupal_get_path('module', 'gbifexcel_importer') . '/xsl';
	$red_biodiversidad_template_path = drupal_get_path('module', 'islandora_red_biodiversidad') . '/xml';
    $contentmodels_xml_proccessing_data=array(
        'biodiversity:OccurrenceCModel' => array(
          'title' => 'dwc:occurrenceID',
		  'template' => $red_biodiversidad_template_path.'/dwc_occurrence_template.xml',
		  'dwc2dc' => $red_biodiversidad_transform_path.'/dwcclassoccurrence2dc.xsl',
        ),
        'biodiversity:LocationCModel' => array(
          'title' => 'dwc:locationID',
		  'template' => $red_biodiversidad_template_path.'/dcterms_location_template.xml',
		  'dwc2dc' => $red_biodiversidad_transform_path.'/dctermsclasslocation2dc.xsl',
        ),
        'biodiversity:IdentificationCModel' => array(
           'title' => 'dwc:identificationID',
		   'template' => $red_biodiversidad_template_path.'/dwc_identification_template.xml',
		   'dwc2dc' => $red_biodiversidad_transform_path.'/dwcclassidentification2dc.xsl',
        ),
        'biodiversity:EventCModel' => array(
           'title' => 'dwc:eventID',
		   'template' => $red_biodiversidad_template_path.'/dwc_event_template.xml',
		   'dwc2dc' => $red_biodiversidad_transform_path.'/dwcclassevent2dc.xsl',
        ),
        'biodiversity:SimpledwcCModel' => array(
            'title' => 'dwc:occurrenceID', //Sames as OccurrenceCModel
		    'template' => $red_biodiversidad_template_path.'/dwc_simple_template.xml',
		    'dwc2dc' => $red_biodiversidad_transform_path.'/dwc2dc.xsl',
        ),
        'biodiversity:TaxonCModel' => array(
          'title' => 'dwc:scientificName',	
		  'template' => $red_biodiversidad_template_path.'/dwc_taxon_template.xml',
		  'dwc2dc' => $red_biodiversidad_transform_path.'/dwcclasstaxon2dc.xsl',
    ),
);
	foreach($content_models as $content_model)
		{
		$content_models_with_processing_info[$content_model]=$contentmodels_xml_proccessing_data[$content_model];	
		}
		
    
	
	
	
	$object_realinfo=static::getIngestInfo($file,$content_models_with_processing_info); 
   
	$info = array(
      'file' => $file,
      'pid_namespace' => $form_state['values']['namespace'],
      'content_model' => $content_models_with_processing_info,
      'object_info' => $object_realinfo,
      'processed_objects' => array(),
    );
    return $info;
	
  }

 
  /**
   * Get ingest info.
   *
   * @return array
   *   The ingest info.
   */
  protected static function getIngestInfo($file,$content_models_with_processing_info) {
	  try {
	    	    $inputFileType = PHPExcel_IOFactory::identify(drupal_realpath($file->uri));
	    	    $objReader = PHPExcel_IOFactory::createReader($inputFileType);
	    	    $objPHPExcel = $objReader->load(drupal_realpath($file->uri));
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
					$dom= new DOMDocument;
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
	   	    	$rowHeaders = $sheet->rangeToArray('A1:'.$highestColumn.'1', NULL, TRUE, TRUE,FALSE);//Devuelve los titulos de cada columna
				
				//Ok, now me hust see which fields belong to which cmodel, just intersect the array preserving the original row data.
				$all_fields=array();
				$rowHeaders[0]=array_map('stripslashes',$rowHeaders[0]);
				$rowHeaders[0]=array_map('utf8_encode',$rowHeaders[0]);
				error_log(var_export($rowHeaders[0],true),0);	
				foreach($dwc_fields as $cmodels=>$dwcfields)
					{	
					$existing_fields_inheader[$cmodels]=array_intersect($rowHeaders[0],$dwcfields);	//we preserve the excel row keys
					//also get an array with all dwc fields...necessary for the next step
					$all_fields=array_merge($all_fields,$dwcfields);
					}
				//Deal with those fields not present anywhere.
				//If we have this model we can use dwc:dynamicProperties to store those nasty intruders.
				error_log(var_export($existing_fields_inheader,true),0);
				
				if (array_key_exists('biodiversity:OccurrenceCModel',$dwc_fields))
					{
						$whatsleft=array_intersect($rowHeaders[0],$all_fields);	
						
					}	
				//Now we got for every cmodel a list of fields present in this excel file. great. Now fill the info. One row can generate multiple objects.	
				
				
					$objectcount=0;	     
			 	for ($fila = 2; $fila <= (int)$highestRow; $fila++)
			 		{
					
					
			 		$rowData=$sheet->rangeToArray('A' . $fila . ':' . $highestColumn.$fila, ' ', TRUE, TRUE, FALSE);//Devuelve fila por fila
					$flat = trim(implode('', $rowData[0]));
					error_log(var_export($rowData[0],true),0);	
					//check for empty row...if found stop there.
					if (strlen($flat)==0){break;}
					//And now just iterate over $existing_fields_inheader and fill our data. 
					unset($datoscombinados);
					
					if (sizeof($existing_fields_inheader)>0)
					{	
					foreach($existing_fields_inheader as  $cmodel =>$cmheaders)
						{
						$datoscombinados=array_combine($cmheaders,array_intersect_key($rowData[0],$cmheaders));//We first get only the data possible to this contenmodel from the row,then we combine the headernames(keys) with the data.
						if (sizeof($datoscombinados)>0)
							{
								$info[$objectcount]=array('data'=>$datoscombinados,'cmodel'=>$cmodel);
							}
						//So we store the desired cmodel and the data in one place...nice.$file aka row is used to keep track fom where the data came in the excel to make
						$objectcount++;
						}
					
					
					}
				}
			     $objPHPExcel->disconnectWorksheets();
				 unset($sheet);
				 unset($objPHPExcel);
				 unset($objReader);
				
			     return $info;				
 		    	
 		    	
				}
			else
				{
		    	drupal_set_message(t('El archivo no contiene suficientes datos '));
		    	return;	
				}
			  
	  
		

    
  }
  public function preprocess() {
    $preprocessed = array();
    $item_class = $this->itemClass;
	
    $total = $this->getNumber();
	$internal_setID_counter=0;
	$date = date_create();
	$this->setId=1;
	
    for ($i = 0; $i < $total; $i++) {
      $item = $item_class::getOne($this->file);
      if ($item) {
		$internal_setID_counter++;
        $this->parameters['namespace'] = $this->getNamespace($item);
        $preprocessed[] = $object = new IslandoraImporterBatchObject($this->connection, $item, $this->parameters);
        $object->addRelationships();
		//Added local generated id (using uui parameter from islandoras configuration)
		$object->id=$this->getIdentifier($this->connection, $this->parameters['namespace']);
        //$object->object_info=$item;//Solo pasaremos el objeto actual...esto está mal en la implementación parent!
		$this->addToDatabase($object, $object->getResources());
		if ($internal_setID_counter==50)//Diego Pino, Excel files can be large. Set group of 5 records to a batch set so we don´t run out of memory in Drupal or get PDO errors
			{
			
			$this->setId++;
			$internal_setID_counter=0;
			}
      }
    }

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
	  protected $title =  NULL;
	  protected $dwc = NULL;
	  protected static $DC2MODS;
	  protected static $DWC2DC;
	  protected static $DWCTEMPLATE;
	  protected static $TITLEFIELD;
	 
      
	  /**
	   * Constructor.
	   */
	  protected function __construct($source) {
	    parent::__construct($source);
	    $this->pidNamespace = $this->source['pid_namespace'];
	    $this->contentModel = (array) $this->source['content_model'];
		$transform_path = drupal_get_path('module', 'gbifexcel_importer') . '/xsl';
	    self::$DC2MODS = "$transform_path/simpleDC2MODS.xsl";
		
		
	  }

	  /**
	   * Get an item from the source.
	   *
	   * @see IslandoraImportObject::getOne()
	   * Esta funcion es la que realmente crea la info para el nuevo stream DwC
	   */
	  public static function getOne(&$info) {
		$key = key($info['object_info']);
		$record = array(
	      'pid_namespace' => $info['pid_namespace'],
	      'file' => $info['file'],
	      'content_model' => $info['object_info'][$key]['cmodel'],
	     
		);
		
		//$info['object_info'] has $key for every object, second key for the corresponding row, and third key for the contentmodel...clever!
		//$info['object_info'][objectcuont]=array('excelrow'=>$fila,'data'=>$datoscombinados,'cmodel'=>$cmodel);
		self::$DWC2DC=$info['content_model'][$info['object_info'][$key]['cmodel']]['dwc2dc'];
	    self::$DWCTEMPLATE=$info['content_model'][$info['object_info'][$key]['cmodel']]['template'];
		self::$TITLEFIELD=$info['content_model'][$info['object_info'][$key]['cmodel']]['title'];
	    unset($info['object_info'][$key]);
	    return (empty($record) ? FALSE : new static($record));
		
	    
	  }
	  
	  
	  protected function getDwC()//Usaremos esta función para crear un DwC XML a partir de cada fila entregada por el excel
	  	{
	  	if ($this->dwc === NULL) {
			$dom= new DOMDocument;
			
			$dom->load(self::$DWCTEMPLATE);
			$dwc_xpath = new DOMXPath($dom);
			$dwc_xpath->preserveWhiteSpace = false;
			$dwc_xpath->registerNamespace('dwc', 'http://rs.tdwg.org/dwc/terms/');
			error_log(var_export($this->source['object_info']['data'],true),0);
			$datosexcel=$this->source['object_info']['data'];//data
			
			if (sizeof($datosexcel)>0)
			{
			foreach($datosexcel as $keydw => $valuedw)
				{
				//drupal_set_message(t('Query data es @campos', array('@campos' => $campos)));
				$keydw=utf8_encode(stripslashes($keydw));
				$valuedw=utf8_encode($valuedw);
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
			
	      );
	    }
	    else {
	      $errors[] = array(
	        t('Failed to produce DC record for @pid.'),
	      );
	    }
	    $obtained_mods = $this->getMODS();
	    if ($obtained_mods) {
	      $name = file_create_filename('MODS record.xml', 'public://');
	      $files[] = $file = file_save_data($obtained_mods, $name);
	      $datastreams[] = array(
	        'dsid' => 'MODS',
	        'label' => 'MODS Record',
	        'mimetype' => 'text/xml',
	        'control_group' => 'M',
	        'datastream_file' => file_create_url($file->uri),
			
	      );
	    }
	    else {
	      $errors[] = array(
	        t('Failed to produce MODS record for @pid.'),
	      );
	    }

	    return $datastreams;
	  }

	  /**
	   * Generates a MODS document representing the imported data.
	   *
	   * @see IslandoraImportObject::getMODS()
	   */
	  public function getMODS() {
	    if ($this->mods === NULL) {
	     	 $xml = $this->getDC();
			 $title=$this->title;
		  	 if ($this->isDC($xml)) {//verifico si el DC quedo bien hecho
				 error_log(var_export(self::$DC2MODS,true),0);
	 	        $this->mods = static::runXSLTransform(array('input' => $xml, 'xsl' => self::$DC2MODS));
	 	        
			 } if (empty($this->mods)) { 
			 $this->mods = <<<EOXML
<mods xmlns:mods="http://www.loc.gov/mods/v3" xmlns="http://www.loc.gov/mods/v3">
  <titleInfo>
    <title>$title</title>
  </titleInfo>
</mods>
EOXML;
			 
		  }
		  
	}	 
	    return $this->mods;
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
		  if ($xml) {
	        $dc_doc = new DOMDocument();
	        $dc_doc->loadXML($xml);
	        $dc_xpath = new DOMXPath($dc_doc);
	        $dc_xpath->registerNamespace('dc', 'http://purl.org/dc/elements/1.1/');
			$result=$dc_xpath->evaluate('//dc:title[text()]');
	      	if ($result->length > 0){$this->title= $result->item(0)->nodeValue;}
			else
				{
				$result=$dc_xpath->evaluate('//dc:type[text()]');
				if ($result->length > 0){$this->title= "Unkown ".$result->item(0)->nodeValue." ";}
				}
				unset($dc_xpath);
				unset($dc_doc);
			}
	    }

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
				error_log(var_export($this->source['content_model_process_info'],true),0);	
  	 	        $this->dc = static::runXSLTransform(array('input' => $xml, 'xsl' => self::$DWC2DC));
	 	        
  			 }
   	         if (empty($this->dc)) {
			 
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