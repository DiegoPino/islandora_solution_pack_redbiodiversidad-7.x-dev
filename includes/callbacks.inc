<?php

/**
 * @file
 * Holds the ajax callbacks for the islandora_entities.
 */

/**
 * A JSON callback that will return the results of a query on entities in Solr.
 *
 * Uses $_get['entities_query'].
 *
 * @param string $type
 *   A string mapped to content model that defines the entity sub-type to
 *   limit the search to.
 */
/*
function islandora_entities_query_entities_callback($type) {
  module_load_include('inc', 'islandora_entities', 'includes/utilities');
  $query_parameters = drupal_get_query_parameters();
  $results = array();

  $type_mappings = array(
    'Tag Place' => ISLANDORA_ENTITIES_PLACE_CONTENT_MODEL,
    'Tag Person' => ISLANDORA_ENTITIES_PERSON_CONTENT_MODEL,
    'Tag Event' => ISLANDORA_ENTITIES_EVENT_CONTENT_MODEL,
    'Tag Organization' => ISLANDORA_ENTITIES_ORGANIZATION_CONTENT_MODEL,
  );

  $solr_results = array();
  if (isset($query_parameters['entities_query'])) {
    $solr_results = islandora_entities_solr_search_entities(
        $type_mappings[$type], $query_parameters['entities_query']
    );
  }

  // Format the results for the CWRCwriter.
  if (isset($solr_results['response']['objects'])) {
    foreach ($solr_results['response']['objects'] as $object_response) {
      $pid = $object_response['PID'];
      $results[] = array(
        'identifier' => $object_response['object_label'],
        // @XXX What's the target attribute? Ask Alan.
        'Object' => l($pid, "islandora/object/$pid", array(
          'attributes' => array('target' => '_blank'),
        )),
      );
    }
  }

  drupal_json_output($results);
}*/

/**
 * Returns array of items to autocomplete forms.
 *
 * @param string $type
 *   Selects eithe Scholar or Department
 *
 * @return array
 *   array of all candidates
 * requires that solr contains and index dw.scientificName and dw.genus from darwincore
 */
function islandora_red_biodiversidad_autocomplete($type) {
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
  $mappings = array(
    'scientificName' => array(
      'cmodel' => 'islandora:sp_red_biodiversidad',
      'title' => 'dw.scientificName',
    ),
    'genus' => array(
      'cmodel' => 'islandora:sp_red_biodiversidad',
      'title' => 'dw.genus',
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
    'fl' => "$biodiversidad_query_field, PID",
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
      $list[$choice[$mappings[$type]['title']]] = $choice[$mappings[$type]['title']];
    }
  }

  drupal_json_output($list);
}
/*Esta función executa ajax commands en la forma de origen poblando datos desde GBIF */



