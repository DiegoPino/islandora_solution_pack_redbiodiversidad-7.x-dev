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


