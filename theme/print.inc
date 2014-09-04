<?php

/**
* @file
* Theme hooks.
*/

/**
* Implements hook_preprocess() for DWC Printing.

*/

/* Here we transform Darwin Core to HTML using XLST for displaying */
function islandora_red_biodiversidad_preprocess_islandora_red_biodiversidad_print(array &$variables) {

module_load_include('inc', 'islandora', 'includes/datastream');
module_load_include('inc', 'islandora', 'includes/utilities');
module_load_include('inc', 'islandora', 'includes/metadata');
$islandora_object = $variables['islandora_object'];
$object_pid = $islandora_object->id;
//Let´s get our primary content model
$title="";
$variables['eol_render_element']=array();

$module_path = drupal_get_path('module', 'islandora_red_biodiversidad');

drupal_add_css("$module_path/css/islandora_red_biodiversidad.css");	 
  
if (isset($islandora_object['DwC']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['DwC']))
{ 
$dwc = $islandora_object['DwC']->content;
$xsl = new DOMDocument();
$transform="dwcclass2html.xsl";
$xsl->load($module_path.'/xsl/'.$transform);
$xslt = new XSLTProcessor();
$xslt->importStyleSheet($xsl);
$html = $xslt->transformToXML(simplexml_load_string($dwc));
$variables['islandora_dwc_html'] = $html;
	
if (in_array('biodiversity:TaxonCModel',$islandora_object->models))
{
			
if (empty($islandora_object->label) || ($islandora_object->label==$object_pid))//something is wrong with our xslt o this object was not originated from our FORM
{
	$domdwc= new DOMDocument;
	$domdwc->loadXML($dwc);
	$domdwc->preserveWhiteSpace = false;
	$dwc_xpath = new DOMXPath($domdwc);
	$dwc_xpath->registerNamespace('dwc', 'http://rs.tdwg.org/dwc/xsd/simpledarwincore/');
	$result=$dwc_xpath->evaluate('//dwc:scientificName[text()]');
	if ($result->length > 0){
		$title=$result->item(0)->nodeValue;
	}
}
else
{
	$title=$islandora_object->label;
}	


}
	
	

$variables['description'] = islandora_retrieve_description_markup($islandora_object);
	   
}
//QR generation
  
$variables['QR']=islandora_red_biodiversidad_pidtoQR($object_pid);

  
}


