<?php

/**
* @file
* Theme hooks.
*/

/**
* Implements hook_preprocess().
* Idea based on islandora_solr_geo module
* @see leaflet and solr
* Map requires a latlong solr field. Using XSL we combined decimal latitud and longitud from Darwin Core dcterms:Location type schema into one named dwc.latlong_p of class latlong type(so we can make spatial queries to solr)
* dwc.latlong contains to dynamic fields, dwc.latlong_p_0_coordinate and dwc.latlong_p_1_coordinate who actually store the lat/long coordinates in double.
* @see solrconfig/DwC_to_solr.xslt and solrconfig/schema.xml
* Note: there is something i don´t like about the standard Rel_ext to Solr XSLT inf fedoragsearch. There is no way we could use a !join to get related objects, 
* because the values inside the Solr Index for RELS_EXT relations have already de info:fedora part prepended and our ID not.
* This limits our use of Solr to get relations based on joins
* @todo make an additional solr field that stores data in this form :fieldname predicate name, value = object->id whitout the info:fedora part. 
*/
function islandora_red_biodiversidad_preprocess_islandora_red_biodiversidad_map(array &$variables)
{
	$module_path = drupal_get_path('module', 'islandora_red_biodiversidad');
	module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
	module_load_include('inc', 'islandora_red_biodiversidad', 'includes/utilities');
	    
	$islandora_object = $variables['islandora_object'];
	$object_pid = $islandora_object->id;
	drupal_set_title($islandora_object->label);
	$pathtopoints=array();
		
	$mappings = array(
		'location' => array(
			'cmodel' => 'biodiversity:LocationCModel',
			'latlong_lat' => 'dwc.latlong_p_0_coordinate',
			'latlong_long' => 'dwc.latlong_p_1_coordinate',
			'latlong' => 'dwc.latlong_p',
		),
    
	);
 
	$results = array();

	$content_model_field = variable_get(
	'islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms'
);
   
$biodiversidad_query_field = $mappings['location']['latlong_lat'];
$stub="";
	    
$query_processor = new IslandoraSolrQueryProcessor();
$query_processor->solrQuery = format_string(
'!biodiversidad_query_field:(!biodiversidad_query)', array(
	'!biodiversidad_query_field' => $biodiversidad_query_field,
	'!biodiversidad_query' => $stub.'*',
)
);
$query_processor->solrStart = 0;
$query_processor->solrLimit = 10000;

//limit to 10000 for now, leaflet can manage aprox 50.000 cluster points concurrent.
//Let's add some base restrictions to comply with Solr module permissiones
		
$xacml_restrictions=islandora_red_biodiversidad_getxacml_solrquery_restrictions();
		
		
		
$query_processor->solrParams = array(
'fl' => $mappings['location']['latlong_lat'].",".$mappings['location']['latlong_long'].",PID,fgs_label_mt",
'fq' => format_string(
'!content_model_field:"info:fedora/!content_model"', array(
	'!content_model_field' => $content_model_field,
	'!content_model' => $mappings['location']['cmodel'],
)
),
);
        
if (!empty($xacml_restrictions)) {
$query_processor->solrParams['fq'] = $query_processor->solrParams['fq']." AND ( ".$xacml_restrictions." )";
}
		
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
//This gives us all the locations, but i could need conditional on where i'm standing(calling) also the occurence data and the associated taxons. 
//Now do some SPARQL Magic...

if (module_exists('islandora_ontologies'))
{
		
module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
$models =$islandora_object->models;
foreach($models  as $contentmodelpid)
{
	//dpm($contentmodelpid);
	if ($cache = cache_get('islandora_ontologies_adjacency'.$contentmodelpid))
	{	
		//dpm('cache hit');
		$result=$cache->data;
		//dpm('hit');
	}		
	else
	{
		$fullgraph=islandora_ontologies_get_graph_path($contentmodelpid);
		if (count($fullgraph)>0)
		{
			foreach ($fullgraph as $ontologiepid=>$localgraph)
			{
				if (count($localgraph)>0)
				{
					$g = new islandora_ontologies_adjacency_graph($localgraph);
					$result[$ontologiepid]=$g->getdepthFirst("info:fedora/".$contentmodelpid);
				}
				
			}
						
			cache_set('islandora_ontologies_adjacency'.$contentmodelpid, $result, 'cache');	
			
		}
	}	
			
	   
	  
}

//If it´s not a location model object what we are visiting the we must reverse the paths($result) and remove everything until we get to a location model
if (!empty($result))
{
if (!(array_intersect(array('biodiversity:LocationCModel'), $models)))  {


	foreach ($result as $ontologie=>&$graphs2)
			{
				foreach($graphs2 as $key=>&$rels2)
					{
						$rels2=array_reverse($rels2);
						foreach($rels2 as $cmodel=>&$rel)
							{
								if ($cmodel!="info:fedora/biodiversity:LocationCModel")
									{
										unset($graphs2[$key][$cmodel]);
										
									}	
								else									{
										unset($graphs2[$key][$cmodel][0]);
										break 1;
									}
							}
						$rels2=array_reverse($rels2);	
					}	
					//Get rid of duplicate paths after removing unnecesarry cmodels
			$result[$ontologie]=array_map("unserialize", array_unique(array_map("serialize", $graphs2)));
			}	
	 
 }




//dpm($result);
$o='o';
$s='s';
$query="";
$relationsandcmodels_sparqlmapping_hash=array();//This structure is a hash map between Sparql Variables and rels+cmodels form our paths so we don`t have to extract this info from Mulgara.
$locationinpath=array();
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
					$otmp="<info:fedora/$object_pid>";	
					}
				else
				{				
								
					if 	($cur==0)
						{
						$otmp="UNION {<info:fedora/$object_pid>";	
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
					
					
						//	{$query.="}";}
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

	//dpm($fullquery);
	$repository = $islandora_object->repository;
	$current_is_location=false;

    //There is a speciall case: If the object we are currently vistiting is a location, then in our graph paths there will be no locationcmodel! (it´s our starting point). It´s obvios, but we must show in this case all the info.
	if (in_array('biodiversity:LocationCModel',$islandora_object->models))
	{
	$current_is_location=true;
	}
	//dpm($fullquery);
	$connection = islandora_get_tuque_connection();
	
	//$escaped_pid = str_replace(':', '_', $pid);
	if (($connection) && !empty($fullquery)){
		

		$results = $connection->repository->ri->sparqlQuery($fullquery);
			
		$related_sparql_results=array();
		$pathtopoints=array();
		$pathoflocation_pid="";
		foreach ($results as $result) {
			$related_sparql_closeure="";
			$related_sparql="";
			$list=array();
			
			$harvestthispath=false;
			//Precheck if location is in path. If so, then process objects for this location. If the current Object belongs to LocationCMODEL, the all paths starting from it are related, so we must compute an exception
			if  ($current_is_location)
				{
				if (!array_key_exists($islandora_object->id,$pathtopoints))
					{
					$pathtopoints[$islandora_object->id]=array();
				    }
					$harvestthispath=true;
					$pathoflocation_pid=$islandora_object->id;	
					
				}
			else
				{	
				foreach($result as $sparqlvar=>$hit)
				{	
					if ($hit['type']=='pid')	
					{
						if ($relationsandcmodels_sparqlmapping_hash[$sparqlvar][0]=='info:fedora/biodiversity:LocationCModel')
						{
						
							if (!array_key_exists($hit['value'],$pathtopoints))
							{
								$pathtopoints[$hit['value']]=array();
							}
							$harvestthispath=true;
							$pathoflocation_pid=$hit['value'];
						}
					
					}	
				}
			
			 }
			//If the path has a location, then harvest all other objects and add it to the array key of this location. 
			//If not assume direct relation to this object, means a path thats goes in another direction of our object is the center of a star =) nice deduction of myself!.
			//This also means that if there are relations to a location starting from here then we must assume that all objects that go in any other direction are related to all found locations and not a particular one.
			if ($harvestthispath)
			{
				//dpm("entrando");
				foreach($result as $sparqlvar=>$hit)
				{								
					if (($hit['type']=='pid'))
					{
					    try {
					    $object = $islandora_object->repository->getObject($hit['value']);
						$model_label=$relationsandcmodels_sparqlmapping_hash[$sparqlvar][0];
					    $model_label=preg_match('/^(.*[\/\#])([^\/\#]+)$/', $model_label, $m)? $m[2]: $model_label;
						
						$model_label=preg_match('/^(.*[\:\/])([^\:\/]+)$/', $model_label, $m)? $m[2]: $model_label;
						
						//$model_label=str_replace('biodiversity:','dwc:',$model_label);
						$model_label=str_replace('CModel','',$model_label)."s";
						$pathtopoints[$pathoflocation_pid][]=array('type' => 'point','popup' => l($object->label, 'islandora/object/'.$hit['value']),'clusterGroup'=> $model_label);
						
					    }
					    catch (RepositoryException $e) {
					   
					        watchdog_exception('Islandora Red de Biodiversidad', $e, 'Got an exception while trying to get relation for @pid to @targetpid on @rel on map', array(
						        '@pid' => $object_pid, '@targetpid' => $hit['value'], '@rel'=>$relationsandcmodels_sparqlmapping_hash[$sparqlvar][1][0]
						      ), WATCHDOG_ERROR);
					  
						}
						
						
						
						
					}										
						
				}					
			$harvestthispath=false;
			$pathoflocation_pid="";
			}
										
										
				
		}
		//dpm($results);
        //dpm($relationsandcmodels_sparqlmapping_hash);


	}

	//dpm($pathtopoints);
	$pathtopoints=array_map("unserialize", array_unique(array_map("serialize", $pathtopoints)));
//End sparql

}//end check for $results, if no ontology is given or no paths then we won´t get anything from up there. 

$locations2=array();
$locations3=array();
		
		
$j=0;
$clustergroup='';
foreach ($json['response']['docs'] as $key=>$choice) {
$j++;
//dpm($choice);

if ((isset($choice[$mappings['location']['latlong_lat']])) && (isset($choice[$mappings['location']['latlong_long']]))) {
if ($choice['PID']==$object_pid)
	{
	$clustergroup='Current Object Location';	
	}
	else
	{
	$clustergroup='All locations in this Repository';	
	}	


	
$point=array(
	'type' => 'point',
	'lat' => (float) $choice[$mappings['location']['latlong_lat']],
	'lon' => (float) $choice[$mappings['location']['latlong_long']],
	'leaflet_id' => 'point-'.$key,
	// 'flags'=>0x01,
	'popup' => l($choice['fgs_label_mt'][0], 'islandora/object/'.$choice['PID']),
	'clusterGroup'=>$clustergroup,
);
if (array_key_exists($choice['PID'],$pathtopoints))
	{
		//remove duplicates before adding
	$pathtopoints[$choice['PID']]=array_map("unserialize", array_unique(array_map("serialize", $pathtopoints[$choice['PID']])));
	
	foreach ($pathtopoints[$choice['PID']] as $objectsinpoint)	
		{
			$objectsinpoint['lat']=(float) $choice[$mappings['location']['latlong_lat']];
			$objectsinpoint['lon']=(float) $choice[$mappings['location']['latlong_long']];
			array_push($locations, $objectsinpoint);	
		}
		
		
	}		  
			  
array_push($locations, $point);	

			
}
}
	
		
	
if (!empty($locations)) {
$styles = module_invoke_all('leaflet_map_info'); 
			
		
		
		
$variables['results'] =leaflet_render_map($styles['islandora_red_biodiversidad'], $locations, '500px');	
}  
	
}

/* Here we transform Darwin Core to HTML using XLST for displaying */
function islandora_red_biodiversidad_preprocess_islandora_dwc(array &$variables) {
drupal_add_js('misc/form.js');
drupal_add_js('misc/collapse.js');
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

$datastreams = array();
foreach ($islandora_object as $ds) {
  try {
    $pid = $islandora_object->id;
    $id = $ds->id;
    $label = $ds->label;
    $download_path = islandora_datastream_get_url($ds, 'download');
    $datastreams[$id]['id'] = $id;
    $datastreams[$id]['label'] = $label;
    $datastreams[$id]['label_link'] = islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $ds) ?
      l($label, $download_path) :
      $label;
    $datastreams[$id]['download_url'] = $download_path;
    $datastreams[$id]['mimetype'] = $ds->mimetype;
    $datastreams[$id]['size'] = islandora_datastream_get_human_readable_size($ds);
    $datastreams[$id]['created_date'] = $ds->createdDate->format("Y-m-d");
    $datastreams[$id]['class'] = drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $id));
  }
  catch (RepositoryException $e) {

  }
}
$variables['datastreams'] = $datastreams;

  
if (isset($islandora_object['DwC']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['DwC']))
{ 
$dwc = $islandora_object['DwC']->content;
$xsl = new DOMDocument();
$transform="dwcclass2html.xsl";
$xsl->load($module_path.'/xsl/'.$transform);
$xslt = new XSLTProcessor();
$xslt->importStyleSheet($xsl);
$html = $xslt->transformToXML(simplexml_load_string($dwc));
$variables['islandora_dwc_html'] = '<article><dl class="dl-horizontal">'.$html.'</dl></article>';
	
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

if (!empty($title))
{
	$eoldatahtml=array();
				
			
	$eoldata=islandora_red_biodiversidad_eol_getdata($title);
	if (!empty($eoldata))	
		{	
	$eoldata=array_filter($eoldata);
	foreach($eoldata as $key=>$values)
	{
		$tmp="";
		$tmp_acum=array();
		$eoldatapre=array();	
				
				
		switch($key)
		{
			case 'references':
			$eoldatapre['eol_3']=array(
				'#type' => 'fieldset',
				'#id' => 'eol-references',
				'#tree' => TRUE,
				'#title'=>t('Bibliographic References'),
				'#group' => 'eol_info',
		        '#collapsible' => TRUE,
		        '#collapsed' => TRUE,	
				'#weight'=>3,	
				'content' => 
				array(
					'#markup' => t(''),
				),
			);
			$tmp.="<ol>";
			foreach($values as $items)
			{
							
				$tmp.="<li>".$items."</li>";	
			}
			$tmp.="</ol>";
			$eoldatapre['eol_3']['content']['#markup']=$tmp;
						
			break;
					
			case 'synonyms':
			$eoldatapre['eol_2']=array(
				'#type' => 'fieldset',
				'#id' => 'eol-synonyms',
				'#tree' => TRUE,
				'#title'=>t('Synonyms'),
				'#group' => 'eol_info',
		        '#collapsible' => TRUE,
		        '#collapsed' => TRUE,	
				'#weight'=>2,		
				'content' => 
				array(
					'#markup' => t(''),
				),
			);
					
					
			foreach($values as $items)
			{
				if ($items['relationship']=='synonym')	
				{
					$tmp_acum[]=$items['synonym'];
				}
			}
			if(count($tmp_acum)>0)
			{
				$tmp=implode(", ",$tmp_acum);	
				$tmp="<p>".$tmp."</p>";
			}	
			else
			{
				$tmp=t('Unknown');	
			}
			$eoldatapre['eol_2']['content']['#markup']=$tmp;
						
			break;
			case 'vernacularNames':
			$eoldatapre['eol_1']=array(
				'#type' => 'fieldset',
				'#id' => 'eol-vernacular',
				'#tree' => TRUE,
				'#title'=>t('Vernacular Names'),
				'#group' => 'eol_info',
				'content' => 
				array(
					'#markup' => t(''),
				),
		        '#collapsible' => TRUE,
		        '#collapsed' => TRUE,	
				'#weight'=>1,	
			);
					
					
			foreach($values as $items)
			{
						
				$tmp_acum[]=$items['vernacularName'].' ('.$items['language'].')';
							
			}
			if(count($tmp_acum)>0)
			{
				$tmp=implode(", ",$tmp_acum);	
				$tmp="<p>".$tmp."</p>";
			}	
			else
			{
				$tmp=t('Unknown');	
			}
			$eoldatapre['eol_1']['content']['#markup']=$tmp;
				
			break;
					
			case 'dataObjects':
			$eoldatapre['eol_0']=array(
				'#type' => 'fieldset',
				'#id' => 'eol-overview',
				'#tree' => TRUE,
				'#title'=>t('Overview'),
				'#group' => 'eol_info',
		        '#collapsible' => TRUE,
		        '#collapsed' => FALSE,	
				'#weight'=> -2,		
				'content' => 
				array(
					'#markup' => t(''),
				),
			);
			foreach($values as $items)
			{
				if ($items['dataType']=="http://purl.org/dc/dcmitype/StillImage")
					{
						$items["title"]=isset($items["title"])?$items["title"]:'';
						$items["license"]=isset($items["license"])?$items["license"]:'';
						$items["rightsHolder"]=isset($items["rightsHolder"])?$items["rightsHolder"]:'';
						$tmp=theme('image',array(
							'title' => t($items["title"]. 'license:'.$items["license"]. 'Rights Holder:'.$items["rightsHolder"]),
							'path' => $items['eolMediaURL'],
						));	
						$tmp.='<div><p>'.t($items["title"]. '<br>license: '.$items["license"]. '<br>Rights Holder: '.$items["rightsHolder"]).'</p></div>';	
						$eoldatapre['eol_0']['content']['#markup']=$tmp;
						$eoldatapre['eol_0']['#weight']=-999;
					}
							
						
				}
					
					
					
				    
						
			}				
								
			//array_multisort($eoldatahtml[]
			$eoldatahtml['fieldset_eol'][]=$eoldatapre;
				
					
		}
		       
			      
		$eoldatahtml['fieldset_eol']['#type'] = 'vertical_tabs';
		$eoldatahtml['fieldset_eol']['#group'] = 'eol_info';
			$eoldatahtml['fieldset_eol']['#weight']=1;
			
			
		$variables['eol_render_element']= drupal_render($eoldatahtml);
	}
		}	
			
			
	
}
}
	
	

$variables['parent_collections'] = islandora_get_parents_from_rels_ext($islandora_object);
$variables['metadata'] = islandora_retrieve_metadata_markup($islandora_object);
$variables['description'] = islandora_retrieve_description_markup($islandora_object);
   
if (isset($islandora_object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['TN'])) {
$variables['islandora_thumbnail_url'] = url("islandora/object/{$islandora_object->id}/datastream/TN/view");

	   
}
//QR generation
if (in_array('biodiversity:OccurrenceCModel',$islandora_object->models))
{   
$variables['QR']=islandora_red_biodiversidad_pidtoQR($object_pid);
} 
  
}
/* Preprocess for DWC-A
/* Here we transform Darwin Core to HTML using XLST for displaying */
function islandora_red_biodiversidad_preprocess_islandora_dwca(array &$variables) {
	drupal_add_js('misc/form.js');
	drupal_add_js('misc/collapse.js');
	module_load_include('inc', 'islandora', 'includes/datastream');
	module_load_include('inc', 'islandora', 'includes/utilities');
	module_load_include('inc', 'islandora', 'includes/metadata');
	$module_path = drupal_get_path('module', 'islandora_red_biodiversidad');
	drupal_add_css("$module_path/css/islandora_red_biodiversidad.css");
	$islandora_object = $variables['islandora_object'];
	$object_pid = $islandora_object->id;
	$variables['parent_collections'] = islandora_get_parents_from_rels_ext($islandora_object);
	$variables['metadata'] = islandora_retrieve_metadata_markup($islandora_object);
	$variables['description'] = islandora_retrieve_description_markup($islandora_object);
	
	$datastreams = array();
	foreach ($islandora_object as $ds) {
	  try {
	    $pid = $object_pid;
	    $id = $ds->id;
	    $label = $ds->label;
	    $download_path = islandora_datastream_get_url($ds, 'download');
	    $datastreams[$id]['id'] = $id;
	    $datastreams[$id]['label'] = $label;
	    $datastreams[$id]['label_link'] = islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $ds) ?
	      l($label, $download_path) :
	      $label;
	    $datastreams[$id]['download_url'] = $download_path;
	    $datastreams[$id]['mimetype'] = $ds->mimetype;
	    $datastreams[$id]['size'] = islandora_datastream_get_human_readable_size($ds);
	    $datastreams[$id]['created_date'] = $ds->createdDate->format("Y-m-d");
	    $datastreams[$id]['class'] = drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $id));
	  }
	  catch (RepositoryException $e) {

	  }
	}
	$variables['datastreams'] = $datastreams;
	if (isset($islandora_object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['TN'])) {
		$variables['islandora_thumbnail_url'] = url("islandora/object/{$islandora_object->id}/datastream/TN/view");
	}
  
  
  
	if (isset($islandora_object['DWCA']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['DWCA']))
	{ 
		$variables['dwca-link'] = l(t('Download Zipped DWC-A'), "islandora/object/{$islandora_object->id}/datastream/DWCA/download");
	}
	
	
}


/*
Having an Ontology defined or not, we display related objects on a tab for every Object belonging to his solution pack provided CMODELS
*/

function islandora_red_biodiversidad_preprocess_islandora_dwc_related(array &$variables) {
$module_path = drupal_get_path('module', 'islandora_red_biodiversidad');
module_load_include('inc', 'islandora_red_biodiversidad', 'includes/utilities');
  

  
$islandora_object = $variables['islandora_object'];

$pid = $islandora_object->id;
drupal_set_title($islandora_object->label);
$result=array();

if (module_exists('islandora_ontologies'))
{
		
module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
$models = $islandora_object->models;
foreach($models  as $contentmodelpid)
{
	//dpm($contentmodelpid);
	if ($cache = cache_get('islandora_ontologies_adjacency'.$contentmodelpid))
	{	
		
		$result=$cache->data;
		
		
	}		
	else
	{
		//If no cache present we get the full graph paths for this ContentModel
		$fullgraph=islandora_ontologies_get_graph_path($contentmodelpid);
		if (count($fullgraph)>0)
		{
			foreach ($fullgraph as $ontologiepid=>$localgraph)
			{
				if (count($localgraph)>0)
				{
					//Having the paths for every associated ontologies, we traverse this structure using getdepthfirst. This way we get an array with every "road" or consecutive "edges" a.k.a relations 
					//that connect this type of Content Model (origin) to every other one in the full Graph. This will be used to make a composite Sparql Query
					$g = new islandora_ontologies_adjacency_graph($localgraph);
					$result[$ontologiepid]=$g->getdepthFirst("info:fedora/".$contentmodelpid);
				
				}
				
			}
				//lastly we set the cache. This is necesarry because we don´t wan´t to recalculate every time the same structure. Only if a Ontology changes, or new ones are added, the cache is cleaned.		
			cache_set('islandora_ontologies_adjacency'.$contentmodelpid, $result, 'cache');	
			
		}
	}	
			
	   
	  
}
/* We also need for this solution pack our actual main CMODEL for the current Object.
This is a special case, because in theory we could haven Ontologies for all CMODELS, incluiding fedora-system

*/
$current_objects_cmodel=array_diff($models, array('fedora-system:FedoraObject-3.0'), array('biodiversity:biodiversityCModel')); //
//dpm($result);

$o='o';
$s='s';
$query="";
$relationsandcmodels_sparqlmapping_hash=array();//This structure is a hash map between Sparql Variables and rels+cmodels form our paths so we don`t have to extract this info from Mulgara.
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
					
					
						//	{$query.="}";}
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
		
	
			
		
		
//dpm($fullquery);		
}
	

	$repository = $islandora_object->repository;
	$related = array();
	/*$rels = $islandora_object->relationships->get(); //Note, leaving this out for now, using mulgara instead. Will prop. reuse in the future 
	
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
	}*/
	
    
	$related_sparql_results=array();
	$connection = islandora_get_tuque_connection();
	$escaped_pid = str_replace(':', '_', $pid);
	//dpm($relationsandcmodels_sparqlmapping_hash);
	if (($connection) && !empty($fullquery)){
		

			$results = $connection->repository->ri->sparqlQuery($fullquery);
			
				$related_sparql_results=array();
				
				$vlot_settings=array(); //this array holds the node and edges data for the vlot Javscript belonging to islandora_ontologies
				//@TODO make all related objects functions in islandora_ontologies module and not in this particular one.
				$current_contentmodel_vlot=preg_match('/^(.*[\/\#])([^\/\#]+)$/',$current_objects_cmodel[0], $m)? $m[2]: $current_objects_cmodel[0]; //Using the current`s object main CMODEL for this solution Pack. It´s an array, so fetch only the first one...should be only one present, but who knows!
				
				
				$vlot_settings[$islandora_object->id]=array('relates_to'=>array(),'label'=>$islandora_object->label,'nodetype'=>'current_node','node_cmodel'=>$current_contentmodel_vlot,'link'=>"");//We are constructing and adjacency graph(again) from Sparql results and this is our current object
			
				
				foreach ($results as $result) {//iterate over every computed path
				    $related_sparql_closeure="";
					$related_sparql="<div>$islandora_object->label".t(' relates via:');
					//store current step in this path as previous object's id
					$previous_object=$islandora_object->id;
					
					$list=array();
					$level=0;
					$object=null;
					foreach($result as $sparqlvar=>$hit)
					{
						
					if ($hit['type']=='pid')	
						{
							$object_label="";
							$object_link="";
							$level++;
						    try { //if an object no longer exists, then we should just stop iterating over this path, because sparql could not have traveled any deeper
								///@TODO NOV-2014 we must take in account external entities as an option.
						 		$object = $islandora_object->repository->getObject($hit['value']);
								$object_label=$object->label;
								$object_link=l($object_label, "islandora/object/".$hit['value']);
						   	 	
								}
						    catch (RepositoryException $e) 
								{
						        watchdog_exception('Islandora Red de Biodiversidad', $e, 'Got an exception while trying to get relation for @pid to @targetpid on @rel', array(
							        '@pid' => $pid, '@targetpid' => $hit['value'], '@rel'=>$relationsandcmodels_sparqlmapping_hash[$sparqlvar][1][0]
							      ), WATCHDOG_ERROR);
						    	  $object_label = "";
								  $object_link="Non longer existing object (".$hit['value'].")";
								  unset($previous_object);//means we start over
								}
						
						
						$model_label=$relationsandcmodels_sparqlmapping_hash[$sparqlvar][0];
						$relation=$relationsandcmodels_sparqlmapping_hash[$sparqlvar][1][0];	//Aca hay que revisar....pueden ser varios y estoy sacando el primero
					    //dpm($hit);
						
						//Make CMODEL something people can actually read 
					    $model_label=preg_match('/^(.*[\/\#])([^\/\#]+)$/', $model_label, $m)? $m[2]: $model_label;
						//$model_label=preg_match('/^(.*[\:\/])([^\:\/]+)$/', $model_label, $m)? $m[2]: $model_label;
						
						$rel_label=preg_match('/^(.*[\/\#])([^\/\#]+)$/',$relation, $m)? $m[2]: $relation;
						
									
						//Now add this object as adjacent to the previous one, if not already done
											
						$rel_label=preg_match('/^(.*[\:\/])([^\:\/]+)$/', $rel_label, $m)? $m[2]: $rel_label;	
						
						
						if ($object)
							{
						if (isset($previous_object))	
							{
							if (!(isset($vlot_settings[$previous_object]['relates_to'][$object->id])))
								{
								$vlot_settings[$previous_object]['relates_to'][$object->id]=array($relation=>$rel_label);
								}
							else
								{
								if (!isset($vlot_settings[$previous_object]['related'][$object->id][$relation]))
									{
									$vlot_settings[$previous_object]['relates_to'][$object->id][$relation]=$rel_label;
									}
								}	
							}
						//Now check if this node is already defined or not. We don´t wan´t to overwrite if it´s already there
						if (!isset($vlot_settings[$object->id]))
							{
							$vlot_settings[$object->id]=array('relates_to'=>array(),'label'=>$object->label,'node_cmodel'=>$model_label,'nodetype'=>'node','link'=>url("islandora/object/{$object->id}",array('absolute'=>TRUE)));
					
							}
						
						
						$previous_object=$object->id; //now set previous as current
							}
						$splitedlabel=implode(' ',preg_split('/(?=[A-Z])/', $rel_label, -1, PREG_SPLIT_NO_EMPTY));
						$rel_label=!empty($splitedlabel)?$splitedlabel:$rel_label;
						//If the Ontology is nice written...then this makes wonders:
						
						$related_sparql=$related_sparql."<ul><li class=\"fa-hover\"><i class=\"fa fa-arrow-right fa-x2\"></i><span class=\"islandora-red-biodiversidad-info\">$rel_label</span>: ".$object_link;
					    $related_sparql_closeure.='</li></ul>';
						}
						
						
					}
				$related_sparql_results[]=array('#type'=>'markup','#markup'=>$related_sparql.$related_sparql_closeure."</div>");	
			}
		
			
			
				
				
				
		}
			
			$variables['related'] = $related;
			if (count($related_sparql_results)>0)
				{
				$vlot_graph_style=islandora_ontologies_vlot_get_info('islandora_red_biodiversidad'); //@see hook_islandora_ontologies_vlot_info and islandora_ontologies_vlot_get_info
				
				
				//$treelist = new islandora_ontologies_adjacency_graph_to_html($vlot_settings);
				//$treelisthtml=$treelist->getdepthFirst($islandora_object->id);
				
			
				//$variables['related_sparql_results'] = $treelisthtml;//
				$variables['related_sparql_results']=drupal_render($related_sparql_results);
				$variables['vlot'] =islandora_ontologies_build_vlot($vlot_graph_style, $vlot_settings, '924','800');
			
				$variables['vlot']=drupal_render($variables['vlot']);
				
				}
			else
				{
				$variables['related_sparql_results'] = t('There are no related objects from or to this one.');	
				}	
			//dpm($variables['related_sparql_results']);
	}	
		
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

