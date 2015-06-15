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
  module_load_include('inc', 'islandora_red_biodiversidad', 'includes/callbacks');
  
	    
	$islandora_object = $variables['islandora_object'];
  $limit_to_cmodel=$variables['limit_to_cmodel']; //This allows us to exclude objects from map based on CMODEL. 
  //By default it's empty so we plot every related object based on our ontology
  
  
  
	$pid = $islandora_object->id;
  $currentlabel = $islandora_object->label;
	drupal_set_title($currentlabel );
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
$query_processoronlythis= new IslandoraSolrQueryProcessor(); //We cant' reuse, because after the first query que processor gets modified
$query_processor->solrQuery = format_string(
'!biodiversidad_query_field:(!biodiversidad_query)', array(
	'!biodiversidad_query_field' => $biodiversidad_query_field,
	'!biodiversidad_query' => $stub.'*',
)
);
$query_processor->solrStart = 0;
$query_processor->solrLimit = 30000;

//limit to 30000 for now, leaflet can manage aprox 50.000 cluster points concurrent.
//Let's add some base restrictions to comply with Solr module permissiones
		
$xacml_restrictions=islandora_red_biodiversidad_getxacml_solrquery_restrictions();
		
		
		
$query_processor->solrParams = array(
'fl' => $mappings['location']['latlong_lat'].",".$mappings['location']['latlong_long'].",PID,fgs_label_mt,PID_fullurl,dwc.locality_s,dwc.municipality_s,dwc.county_s,dwc.stateProvince_s,dwc.country_s",
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
//Also get the info from Solr for the current object, fl and fq are the same.
  
 $stubonlythis=urlencode($pid);
 $query_processor->solrQuery = format_string(
    '!biodiversidad_query_field:"!biodiversidad_query"', array(
      '!biodiversidad_query_field' => 'PID',
      '!biodiversidad_query' => $pid,
    )
  );
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 1;
  $resultsonlythis = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams,  'GET');
  $jsononlythis = json_decode($resultsonlythis->getRawResponse(), TRUE);
  

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
$models =$islandora_object->models;
if (module_exists('islandora_ontologies'))
{
		
module_load_include('inc', 'islandora_ontologies', 'includes/utilities');

foreach($models  as $contentmodelpid)
{
	//dpm(islandora_ontologies_get_graph_path($contentmodelpid));
	if ($cache = cache_get('islandora_ontologies_adjacency'.$contentmodelpid))
	{	
		
		$result=$cache->data;
		
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

//dpm($result);
//If it´s not a location model object what we are visiting the we must reverse the paths($result) and remove everything until we get to a location model
//Also we will check direction change:
//We will allow this type of direction changes
// If we start with a verse and not end with an inverse or change to inverse and verse back
//If we start with an inverse and we only change direction once
//

$current_is_location=false;
if (in_array('biodiversity:LocationCModel',$models))
{
$current_is_location=true;
//dpm('es locacion!');
}
if (!empty($result))
{



	foreach ($result as $ontologie=>&$graphs2)
			{
				foreach($graphs2 as $key=>&$rels2)
					{
            if (array_key_exists('info:fedora/biodiversity:LocationCModel',$rels2) && !$current_is_location) {
              $first=reset($rels2);
              $directiontolocation=$first[0][1];
              //dpm($directiontolocation);
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
          }	
					//Get rid of duplicate paths after removing unnecesarry cmodels
			$result[$ontologie]=array_map("unserialize", array_unique(array_map("serialize", $graphs2)));
			}	
	 
//dpm($result);
$query="";
$relationsandcmodels_sparqlmapping_hash=array();//This structure is a hash map between Sparql Variables and rels+cmodels form our paths so we don`t have to extract this info from Mulgara.
$locationinpath=array();
foreach ($result as $ontologie=>$graphs)
{
  //dpm($graphs);
	foreach ($graphs as $paths=>$rels)
	{
		$o='o';
		$s='s';						
		$cur=0;
		$current_rel="";
		$sparql_object="";	
    $allcmodels=array_keys($rels);//Need this to allow look ahead viewing during foreach
		foreach($rels as $cmodel=>$rel)
		{
      $querytmp="";	
			$otmp="";
      $otmp_extras="";			
				
			if (array_key_exists(0,$rel))
			{
				$o=$o=='o'?'s':'o';
				$s=$s=='s'?'o':'s';
				if ($cur>0)
					{
					    
				  $relationsandcmodels_sparqlmapping_hash[$sparql_object]=array($cmodel,$current_rel);
					$sparql_object="";	
					$current_rel="";
          $otmp_extras="";
					}
				
				
							
								
					if ($cur==0)
						{
						$otmp="\n {<info:fedora/$pid>";	
            }	
					else
						{	
						if ($cur>1)
							{
    				  $otmp='';
							}
            $otmp.= " optional { ?".$o.$cur."_$paths";
						}
				
          $cur_extras=$cur+1;
          $otmp_extras=".";
          $otmp_extras.="?".$s.$cur_extras."_$paths <fedora-model:label> ?".$s.$cur_extras."_".$paths."_label ;\n\r";
          $nextcmodel=$allcmodels[$cur+1];
          $otmp_extras.="<fedora-model:hasModel> <{$nextcmodel}> ." ; //Lets add also cmodel check. Why? because sometimes relations alone are ambiguos and do generate cycles. But cmodel+relations are acyclic in our implementation
          //@TODO this is throwing an REPO except<fedora-model:state>, runs fine through the risearch web interface...why?
          //$otmp_extras.="?".$s.$cur_extras."_$paths <fedora-model:state> <info:fedora/fedora-system:def/model#Active> .";
					$query.=" $otmp <".$rel[0][0];
					$cur++;
					$query.="> ?$s$cur"."_$paths $otmp_extras";
                 
					$sparql_object=$s.$cur."_$paths";	
				    
					
					$current_rel=$rel;
					
					
				if ($cur==sizeof($rels)-1) 
					{
            
            $query.="}";
              
						if ($cur>1)
							{
              for($i=1;$i<sizeof($rels)-1;$i++)
                {
                $query.="}";
                }  
							}
            $individual_queries[]=$query;
            $query="";
					
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
  
}

//dpm($individual_queries);
  //There is a speciall case: If the object we are currently vistiting is a location, then in our graph paths query result there will be no locationcmodel! (it´s our starting point). It´s obvious, but we must show in this case all the info.
$everyotherobject=array(); //to store temp all objects that don't have a location
if (isset($individual_queries) && count($individual_queries)>0) {
	$connection = islandora_get_tuque_connection();
	$escaped_pid = str_replace(':', '_', $pid);
	//dpm($relationsandcmodels_sparqlmapping_hash);
if (($connection)){
    $pathtopoints=array();
    foreach($individual_queries as $individual_query) {
      $fullquery = <<<EOQ
      SELECT DISTINCT * 
      FROM <#ri>
      WHERE $individual_query
      
EOQ;
    //@TODO xacml filter still missing. 	
    $queryresults=array();
    $queryresults = $connection->repository->ri->sparqlQuery($fullquery);
     
    //dpm(sizeof($queryresults));
    //$related_sparql_results=array();

    //@TODO make related object function in islandora_ontologies module and not in this particular one.
    foreach ($queryresults as $queryresult) {//iterate over every computed path
		$pathoflocation_pid="";
		$list=array();
			
			$harvestthispath=false;
			//Precheck if location is in path. If so, then process objects for this location. If the current Object belongs to LocationCMODEL, the all paths starting from it are related, so we must compute an exception
			if  ($current_is_location)
				{
     		if (!array_key_exists($pid,$pathtopoints))
					{
					$pathtopoints[$pid]=array();
				  }
					$harvestthispath=true;
					$pathoflocation_pid=$pid;	
					
				}
			else
				{	
         
				foreach($queryresult as $sparqlvar=>$hit)
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
            else
            {
				
            }
					}	
				}
			
			 }
			//If the path has a location, then harvest all other objects and add it to the array key of this location. 
			//If not assume direct relation to this object, means a path thats goes in another direction of our object is the center of a star =) nice deduction of myself!.
			//This also means that if there are relations to a location starting from here then we must assume that all objects that go in any other direction are related to all found locations and not a particular one.
      //2015 @TODO we should check/or not if the relations are well formed? 
			if ($harvestthispath)
			{
				foreach($queryresult as $sparqlvar=>$hit)
				{
					if (($hit['type']=='pid'))
					{
            $current_cmodel=$relationsandcmodels_sparqlmapping_hash[$sparqlvar][0];
            if ((in_array($current_cmodel, $limit_to_cmodel)) || empty($limit_to_cmodel)) {
              $object_label="";
              $object_link="";
              $object_label=$queryresult[$sparqlvar.'_label']['value'];
              $object_pid=$hit['value'];
              $model_label=$current_cmodel;
              //@TODO replace this pregmatch call to a static array...we are overforcing our poor php server
              $model_label=preg_match('/^(.*[\/\#])([^\/\#]+)$/', $model_label, $m)? $m[2]: $model_label;
              $model_label=preg_match('/^(.*[\:\/])([^\:\/]+)$/', $model_label, $m)? $m[2]: $model_label;
              $model_label=str_replace('CModel','',$model_label)."s";
              $pathtopoints[$pathoflocation_pid][$object_pid]=array('type' => 'point','popup' => "<span>Object of type $model_label:</span><br>".l($object_label, 'islandora/object/'.$object_pid),'clusterGroup'=> $model_label);
              }
            }
          }
        }
        else //assume infered relation to every location, store in $everyotherobject
        {
  			
          foreach($queryresult as $sparqlvar=>$hit)
  				{
  					if (($hit['type']=='pid'))
  					{
              $current_cmodel=$relationsandcmodels_sparqlmapping_hash[$sparqlvar][0];
              if ((in_array($current_cmodel, $limit_to_cmodel)) || empty($limit_to_cmodel)) {
                $object_label="";
                $object_link="";
                $object_label=$queryresult[$sparqlvar.'_label']['value'];
                $object_pid=$hit['value'];
                $model_label=$current_cmodel;
                //@TODO replace this pregmatch call to a static array...we are overforcing our poor php server
                $model_label=preg_match('/^(.*[\/\#])([^\/\#]+)$/', $model_label, $m)? $m[2]: $model_label;
                $model_label=preg_match('/^(.*[\:\/])([^\:\/]+)$/', $model_label, $m)? $m[2]: $model_label;
                $model_label=str_replace('CModel','',$model_label)."s";
                $everyotherobject[$object_pid]=array('type' => 'point','popup' => "<span>Object of type $model_label:</span><br>".l($object_label, 'islandora/object/'.$object_pid),'clusterGroup'=> $model_label);
                }
              }
            }
        }
        $harvestthispath=false;
			$pathoflocation_pid="";
			}
		}
	}

	$pathtopoints=array_map("unserialize", array_unique(array_map("serialize", $pathtopoints)));
 
//End sparql

  }//end check for $results, if no ontology is given or no paths then we won´t get anything from up there. 
}
	

$j=0;
$clustergroup='';
$jsonreversed=array();
foreach ($json['response']['docs'] as $key=>$choice) {
  $j++;
  if ((isset($choice[$mappings['location']['latlong_lat']])) && (isset($choice[$mappings['location']['latlong_long']]))) {
    //Inverse hash, we store the PID as key and the json response as value
    $jsonreversed[$choice['PID']]=$key;
    
    if (empty($limit_to_cmodel)) {
    //Only add all locations if we don't have a cmodel filter set by theme
    
      if ($choice['PID']==$pid) {
        $clustergroup='Current Object Location';	
      }
      else {
        $clustergroup='All locations in this Repository';	
      }	
    
    $point=array(
      'type' => 'point',
      'lat' => (float) $choice[$mappings['location']['latlong_lat']],
      'lon' => (float) $choice[$mappings['location']['latlong_long']],
      'leaflet_id' => 'point-'.$key,
      // 'flags'=>0x01,
      'popup' => "<span>Object of type $clustergroup:</span><br>".l($choice['fgs_label_mt'][0], $choice['PID_fullurl']).'<br>'.(float) $choice[$mappings['location']['latlong_lat']].",".(float) $choice[$mappings['location']['latlong_long']],
      'clusterGroup'=>$clustergroup,
    );
    //adding points to settings by clustergroup as clustergroup, means all
    array_push($locations, $point);	
    }
   
  }
}
//Now compute the computed/related objects according to their relations

$variables['wasgeocoded']=false;
$variables['geocoded']='hola';
foreach ($pathtopoints as $locationpid=>$objectsinpoint) {
if (array_key_exists($locationpid,$jsonreversed)) {
  
  //remove duplicates before adding
  if (count($everyotherobject)>0)
    {
      //objectsinpoint=array_merge($objectsinpoint,$everyotherobject);
    }
  
  $choice=$json['response']['docs'][$jsonreversed[$locationpid]];
  foreach ($objectsinpoint as $objectinpoint)
  {
    $objectinpoint['lat']=(float) $choice[$mappings['location']['latlong_lat']];
    $objectinpoint['lon']=(float) $choice[$mappings['location']['latlong_long']];
    $objectinpoint['popup']= $objectinpoint['popup'].'<br>'.(float) $choice[$mappings['location']['latlong_lat']].",".(float) $choice[$mappings['location']['latlong_long']];
    //adding points to settings by cmodel as clustergroup
    array_push($locations, $objectinpoint);	
  }
}
else {
//Means we don't have this location in our Solr points query 
//So lets call our geodecoder and get the info from there using what we got from solr for the current location object
  
  $query_processor->solrQuery = format_string(
   '!biodiversidad_query_field:"!biodiversidad_query"', array(
     '!biodiversidad_query_field' => 'PID',
     '!biodiversidad_query' => $locationpid,
   )
 );
 $query_processor->solrStart = 0;
 $query_processor->solrLimit = 1;
 $resultsonlythis = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams,  'GET');
 $jsononlythis = json_decode($resultsonlythis->getRawResponse(), TRUE);
  
  
  
  
  if ($jsononlythis['response']['numFound']>=1)
    { 
      
      $geodecodeparam[]=$jsononlythis['response']['docs'][0]['dwc.locality_s'];
      $geodecodeparam[]=$jsononlythis['response']['docs'][0]['dwc.municipality_s'];
      $geodecodeparam[]=$jsononlythis['response']['docs'][0]['dwc.county_s'];
      $geodecodeparam[]=$jsononlythis['response']['docs'][0]['dwc.stateProvince_s'];
      $geodecodeparam[]=$jsononlythis['response']['docs'][0]['dwc.country_s'];
      $geodecoded=islandora_red_biodiversidad_nominatimgeocoding(array_filter($geodecodeparam)); 
      if (count($geodecoded)>0)
      {
        $variables['wasgeocoded']=true;
        $variables['geodecoded']="";
        $rows=array();
        $rows2=array();
        //$variables['geodecoded']=theme('table', array('header' => array_keys($geodecoded), 'rows' => $geodecoded));
       
        $message="<p>".t("Lat/long data was not available for object "). $jsononlythis['response']['docs'][0]['fgs_label_mt'][0];
        $message.="<br>".t("so we geodecoded on the fly it's metadata for display purpouses")."</p>";
        drupal_set_message($message,'status',FALSE);
        foreach ($geodecoded as $key=>$value)
          {
            if ($key=='display_name') {
              $variables['geodecoded'].="<h4>$value</h4>";
            }
            else
            {
              switch ($key) {
                case 'address':
                foreach($value as $desc=>$info)
                  {
                    $rows2[]=array($desc,$info);
                  }
                break;
                case 'boundingbox':
                  $rows[]=array($key,implode(";",$value));
                break;
                default: $rows[]=array($key,$value); 
              }
            }
          }
          $variables['geodecoded'].="<h2>".t("Geocoded data for ").$jsononlythis['response']['docs'][0]['fgs_label_mt'][0]."</h2><br>";
          if (count($rows)>0) {
            
          $variables['geodecoded'].=theme('table', array('header' => array(), 'rows' => $rows))."<br>";
          }
          if (count($rows2)>0) {
          $variables['geodecoded'].="<h2>".t('Address')."</h2>".theme('table', array('header' => array(), 'rows' => $rows2));
          }
          if ((count($everyotherobject)>0) && $current_is_location)
            {
              
              $objectsinpoint=array_merge($objectsinpoint,$everyotherobject);
            }
        
        
        
       
        foreach ($objectsinpoint as $objectinpoint)
        {
          $objectinpoint['lat']=(float) $geodecoded['lat'];
          $objectinpoint['lon']=(float) $geodecoded['lon'];
          $objectinpoint['popup']= $objectinpoint['popup'].'<br>'.(float) $geodecoded['lat'].",".(float) $geodecoded['lat'].'<br>'.t('This location was geocoded from metadata');
          //adding points to settings by cmodel as clustergroup
          array_push($locations, $objectinpoint);	
        }
        
      }
    
    }

 //$jsononlythis
  //$geodecoded=islandora_red_biodiversidad_nominatimgeocoding(array $searchparams)
  
  
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
$variables['related_info'] =array();
if (array_intersect(array('biodiversity:TaxonCModel','biodiversity:SimpledwcCModel'),$islandora_object->models)) {
  if (in_array('biodiversity:SimpledwcCModel',$islandora_object->models))//something is wrong with our xslt or this object was not originated from our FORM
  {
  	$namespace='http://rs.tdwg.org/dwc/xsd/simpledarwincore/';
  }
  else {
    module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
    module_load_include('inc', 'islandora_red_biodiversidad', 'includes/utilities');
      //@TODO make this dynamic, same as our other semantic stuff
    $related_ocurrences=islandora_red_biodiversidad_getparents_via_solr(array('RELS_EXT_taxonOfId_uri_ms','RELS_EXT_identifies_uri_ms'),$pid);
    $variables['related_info']['title'] = 'Occurrences for this taxon';
    $variables['related_info']['numFound']=isset($related_ocurrences['numFound'])?$related_ocurrences['numFound']:0;
    $variables['related_info']['objects']=isset($related_ocurrences['objects'])?$related_ocurrences['objects']:array();
    $namespace='http://rs.tdwg.org/dwc/dwcrecord/'; 
  }
  $domdwc= new DOMDocument;
  $domdwc->loadXML($dwc);
  $domdwc->preserveWhiteSpace = false;
  $dwc_xpath = new DOMXPath($domdwc);
  $dwc_xpath->registerNamespace('dwc', $namespace);
  $result=$dwc_xpath->evaluate('//dwc:scientificName[text()]');
  if ($result->length > 0){
  	$title=$result->item(0)->nodeValue;
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
				'#title'=>t('Media and Images'),
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
if (array_intersect(array('biodiversity:OccurrenceCModel','biodiversity:SimpledwcCModel'),$islandora_object->models))
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
  $this_object_pid=$pid;//needed in case we have to switch to another object as startpoint
  drupal_set_title($islandora_object->label);
  $result=array();

  if (module_exists('islandora_ontologies'))
  {
		
  module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
  //Lets test if we got relations starting from this one. If not search if this object is target of someone else
  
  if (count(islandora_ontologies_get_current_localrelations($pid))==0) {//means we have to do things different
$querypre = <<<EOQ
  SELECT DISTINCT ?object ?predicate
  FROM <#ri>
  WHERE {
    ?object ?predicate <info:fedora/$pid> ;
            <fedora-model:hasModel> <info:fedora/biodiversity:biodiversityCModel>
  }
  limit 1
EOQ;
  
  $connection = islandora_get_tuque_connection();
  $querypreresults=array();
  $querypreresults = $connection->repository->ri->sparqlQuery($querypre);
  dpm($querypreresults);
  foreach($querypreresults as $sparqlvar=>$hit)
    {
      try{
        $islandora_object = islandora_object_load($hit['object']['value']);
        $pid = $islandora_object->id;
        }
  		catch (Exception $e) {
  			$variables['related_sparql_results'] = t('There are no related Darwin Core objects starting from or to this one.');	
  		}
    }
  }
  
  $models = $islandora_object->models;
  foreach($models  as $contentmodelpid)
  {
		$fullgraph=islandora_ontologies_get_graph_path($contentmodelpid);
  


    
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
This is a special case, because in theory we could haven Ontologies for all CMODELS, including fedora-system

*/

$current_objects_cmodel=array_diff($models, array('fedora-system:FedoraObject-3.0'), array('biodiversity:biodiversityCModel')); //
//dpm($result);

$o='o';
$s='s';
$query="";
$relationsandcmodels_sparqlmapping_hash=array();//This structure  stores a hash map between Sparql Variables and rels+cmodels form our paths so we don`t have to extract this info from Mulgara.
//@TODO We are having some serious memory problems on Large Graphs. Even with 1000 results, the additional unbounded variables (becasuse of the great idea of making a single big sparql query) are getting
//Tuque still to it's knees, even after switching to xmlreader(why do you store the result inside a variable Mr. Tuque, why?)
//So we will split every path in individual Sparql query and ask a few times for this info. It's slower, but have to test how this works. We can also cache for anonymous users this info.
//Also, on large results, we will filter some depths out, and additionally hide the labels on render, so the response is quicker.
//Just need to define what is "large"


//This is where path finding and graph theory meets genetics. After calculating all posible, directed not cyclic paths using DepthFirst, some of the resulting sequences (partial) sometimes repeat. This means that even
//when the paths are unique as a whole, partial sequences of nodes and named/directed edges may be common. So what i need is to find those common partial paths for one good reason. This way i can just travel them one time and harvest their intermedial values. 
//But for subsequent Sparql queries i don't need to bound their results anymore, i just need the values that follow after that patterns. So i get more optimal responses in terms of memory used (less unbound variables).
//So here it comes http://en.wikipedia.org/wiki/Longest_common_subsequence_problem, LCS implementation. I threath every path item (subject predicate object) as a composite Value and add it to an array. Then i calculate LCS on this structure, and also store
//where this sequence starts in every single path. Then i decide which one i'm going to travel first (getting all intermediate solutions) and on which ones i'm going to travel but only bind to the variables after this solution. Does this makes sense? Yes!
//But, not yet. We will leave this as @TODO

//@TODO 2015. In case we don't have outgoing relations, but we have ingoing ones(eg. we are at a location, this is only related via related_tolocation from an occurrence). 
//We could starting by checking if this object has a relation. If not, check if this object is target of another one. If so, assume the CMODEL to be the one this last one.







$individual_queries=array();
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
    $allcmodels=array_keys($rels);//Need this to allow look ahead viewing during foreach
		foreach($rels as $cmodel=>$rel)
		{
			$querytmp="";	
			$otmp="";
      $otmp_extras="";			
				
			if (array_key_exists(0,$rel))
			{
				$o=$o=='o'?'s':'o';
				$s=$s=='s'?'o':'s';
				if ($cur>0)
					{
					    
				  $relationsandcmodels_sparqlmapping_hash[$sparql_object]=array($cmodel,$current_rel);
					$sparql_object="";	
					$current_rel="";
          $otmp_extras="";
					}
				
				
							
								
					if ($cur==0)
						{
						$otmp="\n {<info:fedora/$pid>";	
            }	
					else
						{	
						if ($cur>1)
							{
    				  $otmp='';
							}
            $otmp.= " optional { ?".$o.$cur."_$paths";
						}
				
          $cur_extras=$cur+1;
          $otmp_extras=".";
          $otmp_extras.="?".$s.$cur_extras."_$paths <fedora-model:label> ?".$s.$cur_extras."_".$paths."_label ;\n\r";
          $nextcmodel=$allcmodels[$cur+1];
          $otmp_extras.="<fedora-model:hasModel> <{$nextcmodel}> ." ; //Lets add also cmodel check. Why? because sometimes relations alone are ambiguos and do generate cycles. But cmodel+relations are acyclic in our implementation
          //@TODO this is throwing an REPO except<fedora-model:state>, runs fine through the risearch web interface...why?
          //$otmp_extras.="?".$s.$cur_extras."_$paths <fedora-model:state> <info:fedora/fedora-system:def/model#Active> .";
					$query.=" $otmp <".$rel[0][0];
					$cur++;
					$query.="> ?$s$cur"."_$paths $otmp_extras";
                 
					$sparql_object=$s.$cur."_$paths";	
				    
					
					$current_rel=$rel;
					
					
				if ($cur==sizeof($rels)-1) 
					{
            
            $query.="}";
              
						if ($cur>1)
							{
              for($i=1;$i<sizeof($rels)-1;$i++)
                {
                $query.="}";
                }  
							}
            $individual_queries[]=$query;
            $query="";
					
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
  
//Now build a filter for those nasty unbound variables that are eatting my PHP memory

}

	$related = array();
    
	$related_sparql_results=array();
  $vlot_settings=array(); //this array holds the node and edges data for the vlot Javscript belonging to islandora_ontologies
	if (!isset($connection))
    {
    $connection = islandora_get_tuque_connection();
    }
  $escaped_pid = str_replace(':', '_', $pid);
	
if (($connection)){
    $current_contentmodel_vlot=preg_match('/^(.*[\/\#])([^\/\#]+)$/',$current_objects_cmodel[0], $m)? $m[2]: $current_objects_cmodel[0]; //Using the current`s object main CMODEL for this solution Pack. It´s an array, so fetch only the first one...should be only one present, but who knows!
		
    $vlot_settings[$pid]=array('relates_to'=>array(),'label'=>$islandora_object->label,'nodetype'=>'node','node_cmodel'=>$current_contentmodel_vlot,'link'=>url("islandora/object/{$pid}",array('absolute'=>TRUE)));//We are constructing and adjacency graph(again) from Sparql results and this is our current object
    if ($pid==$this_object_pid) {//set the current one
    $vlot_settings[$pid]['nodetype']='current_node';
    }
    foreach($individual_queries as $individual_query) {
      $fullquery = <<<EOQ
      SELECT DISTINCT * 
      FROM <#ri>
      WHERE $individual_query
      
EOQ;
    //@TODO xacml filter still missing. 	
    $queryresults=array();
    $queryresults = $connection->repository->ri->sparqlQuery($fullquery);
   

    //@TODO make related object function in islandora_ontologies module and not in this particular one.
    foreach ($queryresults as $queryresult) {//iterate over every computed path
     //store current step in this path as previous object's id
      $previous_object=$pid;
					
      $list=array();
      $level=0;
      $object=null;
      foreach($queryresult as $sparqlvar=>$hit)
      {
						
        if ($hit['type']=='pid')	
        {
          $object_label="";
          $object_link="";
          $level++;
          //We are not longer getting label from Fedora directly. This was causing a big I/O problem on large GRAPHS. So we assume Mulgara is right and the label is returned from there now.
          //If the object does not longer exists, then clicking on it will give trigger the correct "Not found", but handling this here was a nightmare
					$object_label=$queryresult[$sparqlvar.'_label']['value'];
          $object_pid=$hit['value'];
						
          $model_label=$relationsandcmodels_sparqlmapping_hash[$sparqlvar][0];
          $relation=$relationsandcmodels_sparqlmapping_hash[$sparqlvar][1][0][0];	//Aca hay que revisar....pueden ser varios y estoy sacando el primero
						
          //Make CMODEL something people can actually read 
          $model_label=preg_match('/^(.*[\/\#])([^\/\#]+)$/', $model_label, $m)? $m[2]: $model_label;
          //$model_label=preg_match('/^(.*[\:\/])([^\:\/]+)$/', $model_label, $m)? $m[2]: $model_label;
						
          $rel_label=preg_match('/^(.*[\/\#])([^\/\#]+)$/',$relation, $m)? $m[2]: $relation;
						
									
          //Now add this object as adjacent to the previous one, if not already done
											
          $rel_label=preg_match('/^(.*[\:\/])([^\:\/]+)$/', $rel_label, $m)? $m[2]: $rel_label;	
						
            if (isset($previous_object))	
            {
              if (!(isset($vlot_settings[$previous_object]['relates_to'][$object_pid])))
              {
                $vlot_settings[$previous_object]['relates_to'][$object_pid]=array($relation=>$rel_label);
              }
              else
              {
                if (!isset($vlot_settings[$previous_object]['relates_to'][$object_pid][$relation]))
                {
                  $vlot_settings[$previous_object]['relates_to'][$object_pid][$relation]=$rel_label;
                }
              }	
            }
            //Now check if this node is already defined or not. We don´t wan´t to overwrite if it´s already there
            if (!isset($vlot_settings[$object_pid]))
            {
              
              $vlot_settings[$object_pid]=array('relates_to'=>array(),'label'=>$object_label,'node_cmodel'=>$model_label,'nodetype'=>'node','link'=>url("islandora/object/{$object_pid}",array('absolute'=>TRUE)));
              if ($object_pid==$this_object_pid) {//set the current one
              $vlot_settings[$object_pid]['nodetype']='current_node';
              }
            }
            $previous_object=$object_pid; //now set previous as current
          }
      }
    }
  }
}
			
			$variables['related'] = $related;
			if (count($vlot_settings)>1) //only one means the current object (starting object of the graph)
				{
				$vlot_graph_style=islandora_ontologies_vlot_get_info('islandora_red_biodiversidad'); //@see hook_islandora_ontologies_vlot_info and islandora_ontologies_vlot_get_info
				$variables['related_sparql_results']=t('!count Objects in "@label" relations Path',array('!count' => count($vlot_settings),'@label' => $vlot_settings[$this_object_pid]['label']));
				$variables['vlot'] =islandora_ontologies_build_vlot($vlot_graph_style, $vlot_settings, '840','800');
				$variables['vlot']= drupal_render($variables['vlot']);
				}
			else
				{
				$variables['related_sparql_results'] = t('There are no related Darwin Core objects starting from this one.');	
				}	
		
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

