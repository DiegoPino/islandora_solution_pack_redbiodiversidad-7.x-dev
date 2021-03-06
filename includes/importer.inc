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

  /**
   * Get the number of items to import from $this->file.
   *
   * @see IslandoraBatchImporter::getNumber()
   */
  public function getNumber() {
  	/*try {
  	    $inputFileType = PHPExcel_IOFactory::identify($this->file->uri);
  	    $objReader = PHPExcel_IOFactory::createReader($inputFileType);
  	    $objPHPExcel = $objReader->load($this->file->uri);
  	} catch(Exception $e) {
  		drupal_set_message(t('No se ha podido leer el archivo @filename con error @error', array('@filename' => $this->file->filename,'@error'=>$e->getMessage())));
  	}	
  	$sheet = $objPHPExcel->getSheet(1); 
  	$highestRow = $sheet->getHighestRow(); 
  	$highestColumn = $sheet->getHighestColumn();
  	//  Loop through each row of the worksheet in turn
  	for ($row = 1; $row <= $highestRow; $row++){ 
  	    //  Read a row of data into an array
  	    $rowData = $sheet->rangeToArray('A' . $row . ':' . $highestColumn . $row,
  	                                    NULL,
  	                                    TRUE,
  	                                    FALSE);
  		//drupal_set_message(t('@rowData', array('@rowData' => print_r($rowData,true))));
		
  		} 
	  
    return $highestRow;*/
	    
	     return count($this->file['object_info']['excel_rows']); //Devuelve el numero de registros totales
	   
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

    $info = array(
      'file' => $file,
      'pid_namespace' => $form_state['values']['namespace'],
      'content_model' => $content_models,
      'object_info' => static::getIngestInfo($file),
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
  protected static function getIngestInfo($file) {
	  try {
	    	    $inputFileType = PHPExcel_IOFactory::identify(drupal_realpath($file->uri));
	    	    $objReader = PHPExcel_IOFactory::createReader($inputFileType);
	    	    $objPHPExcel = $objReader->load(drupal_realpath($file->uri));
	    	} catch(Exception $e) {
	    		drupal_set_message(t('No se ha podido leer el archivo @filename con error @error', array('@filename' => $this->file->filename,'@error'=>$e->getMessage())));
	    		return;
			}	
	    	$sheet = $objPHPExcel->getSheet(1); 
	    	$highestRow = $sheet->getHighestRow(); 
	    	$highestColumn = $sheet->getHighestColumn();
	    	//  Loop through each row of the worksheet in turn
	    	if (($highestRow)>1)//Es decir tengo la menos el header con los campos y una registro
				{
	   	    	 $rowHeaders = $sheet->rangeToArray('A1'. ':' . $highestColumn . 1, '', TRUE, FALSE);//Devuelve los titulos de cada columna
				 $count = $highestRow-1; //Numero de filas, resto una 
			     $info = array();
			 	for ($row = 2; $row <= $highestRow; $row++)
			 		{
			 		$rowData=$sheet->rangeToArray('A' . $row . ':' . $highestColumn . $row, '', TRUE, FALSE);//Devuelve los titulos de cada columna
			 		$info['excel_rows'][$row-1]=$rowData;
					}
					$info['excel_row_headers']=$rowHeaders;
			     $objPHPExcel->disconnectWorksheets();
			     return $info;				
				}
			else
				{
		    	drupal_set_message(t('El archivo @filename no contiene suficientes datos ');
		    	return;	
				}
			  
	  
		

    
  }
}

/**
 * GBIF Excel import object.
 *
 * Actually does the heavy-lifting during the import.
 * @author Diego Pino Navarro
 */
class RedbiodiversidadImportObject extends IslandoraImportObject {

	  protected $mods = NULL;
	  protected $dc = NULL;
	  protected $dwc = NULL;
	  protected static $DC2MODS;
	  protected static $DWC2MODS;
	  protected static $DWCtemplate;
	  /**
	   * Constructor.
	   */
	  protected function __construct($source) {
	    parent::__construct($source);
	    $this->pidNamespace = $this->source['pid_namespace'];
	    $this->contentModel = (array) $this->source['content_model'];
	    $transform_path = drupal_get_path('module', 'gbifexcel_importer') . '/xsl';
		$template_path = drupal_get_path('module', 'gbifexcel_importer') . '/xml';
	    self::$DWC2DC = "$transform_path/dwc2dc.xsl";
	    self::$DC2MODS = "$transform_path/simpleDC2MODS.xsl";
		self::$DWCtemplate= "$template_path/simpleDwCtemplate.xsl"; //usaremos un template para crear el DWC
		
	  }

	  /**
	   * Get an item from the source.
	   *
	   * @see IslandoraImportObject::getOne()
	   * Esta funcion es la que realmente crea el nuevo stream DwC
	   */
	  public static function getOne(&$info) {
		$record = array(
	      'pid_namespace' => $info['pid_namespace'],
	      'file' => $info['file'],
	      'content_model' => $info['content_model'],
	    );
	    $key = key($info['object_info']['excel_rows']);
	    $record['object_info'] = $info['processed_objects'][$key] = $info['object_info']['excel_rows'][$key];
	    unset($info['object_info']['excel_rows'][$key]);
	    return (empty($record) ? FALSE : new static($record));
	  }
	  
	  
	  protected function getDwC()//Usaremos esta función para crear un DwC XML a partir de cada fila entregada por el excel
	  	{
	  	if ($this->dwc === NULL) {
			$dom= new DOMDocument;
			$dom->load(self::$DWCtemplate);
			$dwc_xpath = new DOMXPath($dom);
			$dwc_xpath->registerNamespace('dwc', 'http://rs.tdwg.org/dwc/xsd/simpledarwincore/');
			//$mods_xpath->query('//dwc:SimpleDarwinRecordSet/dwc:SimpleDarwinRecord/*');
			foreach($this->source['object_info']['excel_row_headers'] as $colum => $campos)
				{
				$mods_xpath_check->evaluate(string("//dwc:SimpleDarwinRecordSet/dwc:SimpleDarwinRecord/dwc:$campos");
				if ($mods_xpath_check)
					{
					$mods_xpath_field->query('//dwc:SimpleDarwinRecordSet/dwc:SimpleDarwinRecord/dwc:$campos');	
					foreach($mods_xpath_field as $item)
						{
							$item->nodeValue=$this->source['object_info']['excel_rows'][$colum];
						}//Aca deberia agregar todos los campos que no están definidos dentro de un campo DWC adecuado como nombre=valor,
					}	
				}
				$dom->saveXML();
				if ($dom)
					{
						$this->dwc=$dom;
					}
				
				
				}
			return	$this->dwc;
		
	  	}
	  
  	  protected function isDwC($xml) {
  	    return $this->getLocalNameOfRootElement($xml) == 'SimpleDarwinRecordSet';
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
		$obtainer_dwc =$this->getDwC();
	    if ($obtained_dwc) {
	      $name = file_create_filename('DwC record.xml', 'public://');
	      $files[] = $file = file_save_data($obtained_dcw, $name);
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
	   * Generates a MODS document repersenting the imported data.
	   *
	   * @see IslandoraImportObject::getMODS()
	   */
	  public function getMODS() {
	    if ($this->mods === NULL) {
	     	 $xml = $this->getDC();
		  	 if ($this->isDC($xml)) {//verifico si el DC quedo bien hecho
	 	        $this->mods = static::runXSLTransform(array('input' => $xml, 'xsl' => self::$DC2MODS));
	 	        
			 }
 	         if (empty($this->mods)) {
			$this->mods = <<<EOXML <mods xmlns:mods="http://www.loc.gov/mods/v3" xmlns="http://www.loc.gov/mods/v3">
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
	  public function getDC() {
  	    if ($this->dc === NULL) {
  	     	 $xml = $this->getDwC();
  		  	 if ($this->isDwC($xml)) {//verifico si el DwC quedo bien hecho
  	 	        $this->dc = static::runXSLTransform(array('input' => $xml, 'xsl' => self::$DWC2DC);
	 	        
  			 }
   	         if (empty($this->dc)) {
			 
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