<?php
/**
* Procesador de excel desde GBIF
*/
@include_once 'sites/all/libraries/phpexcel/PHPExcel.php';
function  red_biodiversidad_ingest_specimens_fromexcel($file, $force,$object)
{
	try {
	    $inputFileType = PHPExcel_IOFactory::identify($file->uri);
	    $objReader = PHPExcel_IOFactory::createReader($inputFileType);
	    $objPHPExcel = $objReader->load($file->uri);
	} catch(Exception $e) {
		drupal_set_message(t('No se ha podido leer el archivo @filename con error @error', array('@filename' => $file->filename,'@error'=>$e->getMessage())));
	}	
	$sheet = $objPHPExcel->getSheetByName('Occurrence'); 
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
		drupal_set_message(t('Datos Headers @rowData', array('@rowData' => print_r($info['excel_row_headers'],true))));
		drupal_set_message(t('Datos Info @rowInfo', array('@rowInfo' => print_r($info['excel_rows'],true))));
	     return $info;				
		}
	else
		{
    	drupal_set_message(t('El archivo no contiene suficientes datos '));
    	return;	
		}
}


/*
function ingest_specimen_by_id($specimenId, $force){
	
	// decide what type of number this is
	$specimenType = get_specimen_type_from_id($specimenId);
	
	if($specimenType == 'UNKNOWN'){
		drupal_set_message(t("Sorry. Can't tell if $specimenId is a herbarium specimen, accession or plant. Giving up!" ));
		return;
	}
	
	
	// call the function to get the specimen object
	// fixme - do different things depending on specimen type
	db_set_active('bgbase_dump');
	$result = db_query('SELECT * FROM {darwin_core} WHERE CatalogNumber = :barcode', array(':barcode' => $specimenId));
	db_set_active();	
	
	
	foreach($result as $record){
		// bump that object off to the ingest method.
		if(ingest_specimen($record, $specimenType, $force)){
			drupal_set_message(t( "Ingest complete for specimen id: $specimenId" ));
		}
	}
	
}

function get_specimen_type_from_id($specimenId){
	
	if(preg_match('/^E[0-9]{8}$/', $specimenId)) return 'HERBARIUM';
	if(preg_match('/^[0-9]{8}$/', $specimenId) ) return 'ACCESSION';
	if(preg_match('/^[0-9]{8}[A-Za-z]+/', $specimenId) ) return 'PLANT';
	return 'UNKNOWN';
	
}

function ingest_specimen($specimenRow, $specimenType, $force){
	
		// this will be called for thousands of records so we don't want
		// to create new XSLT processors every time so we will hold 
		// references to them in here
		$xlstProcessors = &drupal_static(__FUNCTION__);
		
		module_load_include('inc', 'islandora', 'includes/IslandoraTuque');		
		
		// work out what the objectId and collectionId is for this specimen
		$objectId = false;
		$objectLabel = false;
		$collectionId = false;
		$contentModelId = 'rbge:specimen_content_model'; // same content model for all specimens?
		switch ($specimenType) {
			case 'HERBARIUM':
				$objectId = 'herb:' . $specimenRow->CatalogNumber;
				$objectLabel = $specimenRow->CatalogNumber . ": " . strip_tags($specimenRow->ScientificName);
				$collectionId = 'rbge:herbarium_collection';
				break;
			case 'ACCESSION':
				$objectId = 'accession:' . $specimenRow->CatalogNumber;
				break; 
				// fixme add support for plants
			default:
				$s = t("Sorry. Can't import specimen of type $specimenType. Giving up!" );
				drupal_set_message($s);
				watchdog('Specimen Sync Fail', $s);
				return;
				break;
		}
		
		// try and get the repo object
		$object = islandora_object_load($objectId);
		
		// if we don't get it make it.
		if(!$object){

			// get a handle on the repository
	       $my_islandora_tuque = new IslandoraTuque();
		   $repository = $my_islandora_tuque->repository;

		   $object = $repository->constructObject($objectId); // create an object with the given PID
		   $object->label = $objectLabel;
	       $object->owner = 'admin';
		   $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', $contentModelId);
		   $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $collectionId);
		    
	       $object = islandora_add_object($object);

	        if(!$object){
				$s = t("Sorry. Failed to create object in repository. Giving up!" );
				drupal_set_message($s);
				watchdog('Specimen Sync Fail', $s);
				return;
				
			} 
			
			// a specimen must have a raw specimen xml datastream -- currently empty
			$specimenDS = $object->constructDatastream('RBGE_SPECIMEN', 'M');
			$specimenDS->label = 'RBGE Specimen raw XML';
			$specimenDS->mimeType = 'text/xml';
			$specimenDS->checksumType = 'MD5'; // used to check for changes in sync run
			$specimenDS->versionable = FALSE; // we don't track versions at the moment as we are auto updating and it might get messy with trivial versions being created
			$object->ingestDatastream($specimenDS);
			
			// a specimen must have a MODS datastream -- currently empty
			$modsDS = $object->constructDatastream('MODS', 'M');
			$modsDS->label = 'RBGE Specimen MODS Metadata';
			$modsDS->mimeType = 'text/xml';
			$modsDS->versionable = FALSE; // it is a derivative so no need to version track
			$object->ingestDatastream($modsDS);
						
			// a specimen must have a Darwin Core DS -- currently empty
			$dwcDS = $object->constructDatastream('DWC', 'M');
			$dwcDS->label = 'RBGE Specimen Simple Darwin Core Metadata';
			$dwcDS->mimeType = 'text/xml';
			$dwcDS->versionable = FALSE; // it is a derivative so no need to version track
			$object->ingestDatastream($dwcDS);
			
			
	    }
		
		// update the properties 
		$object->label = $objectLabel;
		
		// we have the object so let's update the datastream 
		$rawSpecimenXML = get_specimen_xml($specimenRow, $specimenType);
		
		$existingMd5 = $object['RBGE_SPECIMEN']->checksum;
		$newMd5 = md5($rawSpecimenXML);
		
		if($existingMd5 == $newMd5){
			
			if($force){
				drupal_set_message(t( "MD5s of new and old match but FORCE is set so updating specimen: $specimenRow->CatalogNumber" ), 'warning');
			}else{
				drupal_set_message(t( "MD5s of new and old match so not updating specimen: $specimenRow->CatalogNumber" ), 'warning');
				return false;
			}
			
			
		}
		
		$object['RBGE_SPECIMEN']->content = $rawSpecimenXML;
		
		// we are going to create the other DS from this raw XML
		
		// to play nice with other stuff we need a mods datastream
		$xml = new DOMDocument;
		$xml->loadXML($rawSpecimenXML);
		
		// create a more standardised Darwin Core DS
		if(!isset($xsltProcessors['raw2dwc'])){
			$xsl = new DOMDocument;
			$xsl->load(drupal_get_path('module', 'rbge_specimen') . '/xml/specimen2dwc.xsl');
			// Configure the transformer
			$xsltProcessors['raw2dwc'] = new XSLTProcessor;
			$xsltProcessors['raw2dwc']->importStyleSheet($xsl); // attach the xsl rules
		}
		
		// get a document instance of it
		$dwcXml = $xsltProcessors['raw2dwc']->transformToDoc($xml);
		
		// save the DwC xml to the datastream
		$object['DWC']->content = $dwcXml->saveXML();


		// create a Dublin Core Datastream from the simple DarwinCore
		if(!isset($xsltProcessors['dwc2dc'])){
			$xsl = new DOMDocument;
			$xsl->load(drupal_get_path('module', 'rbge_specimen') . '/xml/dwc2dc.xsl');
			// Configure the transformer
			$xsltProcessors['dwc2dc'] = new XSLTProcessor;
			$xsltProcessors['dwc2dc']->importStyleSheet($xsl); // attach the xsl rules
		}
		$object['DC']->content = $xsltProcessors['dwc2dc']->transformToXML($dwcXml);

		
		// now do the MODs DS - do we really need this?
		if(!isset($xsltProcessors['raw2mods'])){
			$xsl = new DOMDocument;
			$xsl->load(drupal_get_path('module', 'rbge_specimen') . '/xml/specimen2mods.xsl');
			// Configure the transformer
			$xsltProcessors['raw2mods'] = new XSLTProcessor;
			$xsltProcessors['raw2mods']->importStyleSheet($xsl); // attach the xsl rules
		}

		$object['MODS']->content = $xsltProcessors['raw2mods']->transformToXML($xml);
			
		return true;
		
}


function get_specimen_xml($specimenRow, $specimenType){

// make an initial document	
$xmlstr = <<<XML
<?xml version='1.0' standalone='yes'?>
<specimen type="$specimenType" xmlns="http://data.rbge.org.uk/ns/specimen/raw">
</specimen>
XML;
	
	$xml = new SimpleXMLElement($xmlstr);
	
	$properties = get_object_vars($specimenRow);
	
	foreach($properties as $key => $value){
		$xml->addChild($key,  htmlspecialchars($value) );
	}
	
	// add in some static values depending on the specimen type
	switch ($specimenType) {
		case 'HERBARIUM':
			$xml->addChild('BasisOfRecord', 'PreservedSpecimen');
			$xml->addChild('DatasetName', 'Royal Botanic Garden Edinburgh Herbarium');
			break;
		case 'ACCESSION':
			$xml->addChild('BasisOfRecord', 'LivingSpecimen');
			$xml->addChild('DatasetName', 'Royal Botanic Garden Edinburgh Living Collection');
			break; 
			// fixme add support for plants
		case 'PLANT':
			$xml->addChild('BasisOfRecord', 'LivingSpecimen');
			$xml->addChild('DatasetName', 'Royal Botanic Garden Edinburgh Living Collection');
			break;
	}	
	
	// add some generic static values
	$xml->addChild('TypeOfThing', 'PhysicalObject');
	$xml->addChild('CollectionCode', 'E');
	$xml->addChild('CollectionID', 'http://biocol.org/urn:lsid:biocol.org:col:15670');
	$xml->addChild('OwnerInstitutionCode', 'E');
	$xml->addChild('InstitutionCode', 'E');
	$xml->addChild('VerbatimCoordinateSystem',  'degrees minutes seconds');
	
	return $xml->asXML();
}
*/

