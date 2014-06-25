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
	    //drupal_set_breadcrumb(islandora_get_breadcrumbs($islandora_object));
		
		
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
	    //$query_processor->solrLimit = 1000;//no limit better for now, leaflet can manage aprox 50.000 cluster points concurrent.
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
		$locations2=array();
		$locations3=array();
		
		
		$j=0;
		 $clustergroup='Locs';
		foreach ($json['response']['docs'] as $key=>$choice) {
			$j++;
			//dpm($choice);
	        if ($j>5){ $clustergroup='Locs2';}
		  if ((isset($choice[$mappings['location']['latlong_lat']])) && (isset($choice[$mappings['location']['latlong_long']]))) {
		  //$coords = explode(",", $choice[$mappings['location']['latlong']][0]); not needed anymore
		   $point=array(
		  'type' => 'point',
		  'lat' => (float) $choice[$mappings['location']['latlong_lat']],
		  'lon' => (float) $choice[$mappings['location']['latlong_long']],
		  'leaflet_id' => 'point-'.$key,
		 // 'flags'=>0x01,
		  'popup' => l($choice['fgs_label_mt'][0], 'islandora/object/'.$choice['PID']),
		  'clusterGroup'=>$clustergroup,
		 );
			  
			  
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
  $module_path = drupal_get_path('module', 'islandora_red_biodiversidad');

  drupal_add_css("$module_path/css/islandora_red_biodiversidad.css");
  $islandora_object = $variables['islandora_object'];
  $object_pid = $islandora_object->id;
  //Let´s get our primary content model
  $title="";
  $variables['eol_render_element']=array();
	 
	
	
	  	
  
  
  
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
			//MAÑANA
			/*
<?php
function example_vertical_tab_render_array() {
  return array(
    'example' => array(
      'fieldset1' => array(
        '#type' => 'fieldset',
        '#title' => t('Test'),
        '#attributes' => array('class' => array('collapsible', 'collapsed')),
        'content' => array(
          '#markup' => t('Test1'),
        ),
      ),
      'fieldset2' => array(
        '#type' => 'fieldset',
        '#title' => t('Test'),
        '#attributes' => array('class' => array('collapsible', 'collapsed')),
        'content' => array(
          '#markup' => t('Test2'),
        ),
      ),
      '#attached' => array(
        'js' => array('misc/collapse.js', 'misc/form.js'),
        'library' => array(array('system', 'drupal.vertical-tabs')),
      ),
      '#prefix' => '<div class="vertical-tabs-panes">',
      '#suffix' => '</div>',
    ),
  );
}
?>
render_arrays			
			
			
			*/		
			if (!empty($title))
			{
			$eoldatahtml=array();
				
				
				
				
			$eoldata=islandora_red_biodiversidad_eol_getdata($title);
			
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
						'#tree' => TRUE,
						'#title'=>t('Bibliographic References'),
						'#attributes' => array('class' => array('collapsible', 'collapsed')),
					    '#group' => 'eol_info',
						
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
						'#tree' => TRUE,
						'#title'=>t('Synonyms'),
						'#attributes' => array('class' => array('collapsible', 'collapsed')),
						 '#group' => 'eol_info',
						
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
						$tmp="</p>".$tmp."</p>";
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
						'#tree' => TRUE,
						'#title'=>t('Vernacular Names'),
						'#attributes' => array('class' => array('collapsible', 'collapsed')),
						 '#group' => 'eol_info',
					    'content' => 
							array(
	         	 				'#markup' => t(''),
	        			),
					);
					
					
					foreach($values as $items)
						{
						
							$tmp_acum[]=$items['vernacularName'].' ('.$items['language'].')';
							
						}
					if(count($tmp_acum)>0)
						{
						$tmp=implode(", ",$tmp_acum);	
						$tmp="</p>".$tmp."</p>";
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
						'#tree' => TRUE,
						'#title'=>t('Overview'),
						'#attributes' => array('class' => array('collapsible')),
						'#group' => 'eol_info',
						
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
			
			
			
			$variables['eol_render_element']= drupal_render($eoldatahtml);
			}
			
			
			
			//$variables['eoldata']=islandora_red_biodiversidad_eol_getdata($query);
 		}
	}
	
	

    $variables['parent_collections'] = islandora_get_parents_from_rels_ext($islandora_object);
    $variables['metadata'] = islandora_retrieve_metadata_markup($islandora_object);
    $variables['description'] = islandora_retrieve_description_markup($islandora_object);
   
	if (isset($islandora_object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['TN'])) {
	      $variables['islandora_thumbnail_url'] = url("islandora/object/{$islandora_object->id}/datastream/TN/view");
	   
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


if (module_exists('islandora_ontologies'))
	{
		
	module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
	$models = $islandora_object->models;
	foreach($models  as $contentmodelpid)
		{
	   islandora_ontologies_get_graph_path($contentmodelpid);
        }
}


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
/*@todo read(and parse) the ontology OWL file directly from the content model*/
  
 
  
  $connection = islandora_get_tuque_connection();
  $escaped_pid = str_replace(':', '_', $pid);
  if ($connection) {
    $query = <<<EOQ
PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
	PREFIX bioscicol: <http://biscicol.org/terms/biscicol.owl#>
SELECT ?object 
FROM <#ri>
WHERE {{
  ?object <fedora-model:hasModel> <info:fedora/biodiversity:biodiversityCModel>. 
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
								$relationlabel=$rel['predicate']['namespace'].":".$rel['predicate']['value'];//
								$relationlabel=$rel['predicate']['value'];
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

