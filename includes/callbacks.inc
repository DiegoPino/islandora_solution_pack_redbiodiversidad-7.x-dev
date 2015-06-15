<?php

/**
 * @file
 * Holds the ajax callbacks for the islandora_red_biodiversidad.
 */

/**
 * Returns array of items to autocomplete forms.
 *
 * @param string $type
 *   Selects either scientificName or Genus, could be extended to others
 *
 * @return array
 *   array of all candidates
 * requires that solr contains and index dw.scientificName_mt(tokenized field) or dw.genus_mt (tokenized field) from darwincore
 * @see folder sorlconfig/ for our implementation for solr
 */
function islandora_red_biodiversidad_autocomplete($type) {
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
  $mappings = array(
    'scientificName' => array(
      'cmodel' => 'biodiversity:biodiversityCModel',
      'title' => 'dwc.scientificName_mt',
    ),
    'genus' => array(
      'cmodel' => 'biodiversity:biodiversityCModel',
      'title' => 'dwc.genus_mt',
    ),
  );
 
  $results = array();

  $content_model_field = variable_get(
      'islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms'
  );
   
  $biodiversidad_query_field = $mappings[$type]['title'];
  $stub="";
  if (isset($_GET['q'])) {
    $stub = end(explode('/', $_GET['q']));
}
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = format_string(
      '!biodiversidad_query_field:(!biodiversidad_query)', array(
        '!biodiversidad_query_field' => $biodiversidad_query_field,
        '!biodiversidad_query' => $stub.'*',
      )
  );
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 100;
  $query_processor->solrParams = array(
    'fl' => "$biodiversidad_query_field,PID",
    'fq' => format_string(
        '!content_model_field:"info:fedora/!content_model"', array(
          '!content_model_field' => $content_model_field,
          '!content_model' => $mappings[$type]['cmodel'],
        )
    ),
  );
  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  try {
    $results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
    $json = json_decode($results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    watchdog_exception('Islandora Red de Biodiversidad', $e, 'Got an exception while searching ScientificName in Solr for callback.', array(), WATCHDOG_ERROR);
  }
  $list = array();
  
  foreach ($json['response']['docs'] as $choice) {
	
    if (isset($choice[$mappings[$type]['title']])) {
      $list[$choice[$mappings[$type]['title']][0]] = $choice[$mappings[$type]['title']][0];//Solr devuelve un arreglo. Verificar schema.xml
    }
  }
 
  drupal_json_output($list);
}


function islandora_red_biodiversidad_nominatimgeocoding(array $searchparams)
  {        
      $geolocated=array();
      if (count($searchparams)==0) {
        return array();
      }
      $parametro=implode(",", array_map('urlencode',$searchparams));
      //open.mapquestapi.com/nominatim V1 API for geocoding
      $url ="http://open.mapquestapi.com/nominatim/v1/search.php";
      $params = array(
         'format' => 'json',
         'limit' => 3,
         'addressdetails' => 1,
         'osm_type' => 'N',
         );
      
      
    
      $response = drupal_http_request($url . "?q=$parametro&" . drupal_http_build_query($params));
     
        if (!empty($response->error)) {
          $msg_args = array(
            '%url' => $url,
            '@code' => $response->code,
            '%error' => $response->error,
          );
          drupal_set_message(t('Red de Biodiversidad, the HTTP request %url returned the following error (code @code): %error.', $msg_args), 'error');
          return FALSE;
        }
        $data = drupal_json_decode($response->data);
    
      $nodefound=FALSE;
      if (count($data))
        {
          foreach($data as $key=>$found)
          {
           if ($found['osm_type']=='node') 
             {
               //Tomo un osm_type=node
               $geolocated['boundingbox']= $data[$key]['boundingbox'];
               $geolocated['licence']=$data[$key]['licence'];
               $geolocated['lat']= $data[$key]['lat'];
               $geolocated['lon'] = $data[$key]['lon'];
               $geolocated['display_name'] = $data[$key]['display_name'];
               $geolocated['address']= $data[$key]['address'];
               $nodefound=TRUE;
               break;
             } 
          }
          if (!$nodefound)
            {
         
        //Tomo el primero, sea cual sea
        $geolocated['boundingbox']= $data[0]['boundingbox'];
        $geolocated['licence']=$data[0]['licence'];
        $geolocated['lat']= $data[0]['lat'];
        $geolocated['lon'] = $data[0]['lon'];
        $geolocated['display_name'] = $data[0]['display_name'];
        $geolocated['address']= $data[0]['address'];
           }
        }
     
        return $geolocated;
    
  }  

