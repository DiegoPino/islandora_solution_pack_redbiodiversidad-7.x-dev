<?php

/**
 * @file
 * Class override for grid display
 */

/**
 * Extension of IslandoraSolrResults to create an alternative display type.
 */
class IslandoraSolrResultsCloud extends IslandoraSolrResults {

  /**
   * Renders the Solr results taking external objects in account
   *
   *
   * @see IslandoraSolrResults::displayResults()
   *
   * @param array $solr_results
   *   The processed Solr results from
   *   IslandoraSolrQueryProcessor::islandoraSolrResult.
   *
   * @return string
   *   Rendered Solr results.
   */
 
  public function printResults($solr_results) {
    //@TODO mark external objects as being in other repositories so the user can decide
    $solr_results = islandora_solr_prepare_solr_results($solr_results);
    $object_results = $solr_results['response']['objects'];
    $object_results = islandora_solr_prepare_solr_doc($object_results);
    $elements = array();
    $elements['solr_total'] = $solr_results['response']['numFound'];
    $elements['solr_start'] = $solr_results['response']['start'];

    // Return themed search results.
    return theme('islandora_solr', array('results' => $object_results, 'elements' => $elements));
  }
 
 /* public function printResults($solr_results) {
    $mod_path = drupal_get_path('module', 'islandora_solr_config');
    drupal_add_css("$mod_path/css/islandora_solr_config.theme.css");
    $object_results = $solr_results['response']['objects'];

    $elements = array();
    $elements['solr_total'] = $solr_results['response']['numFound'];
    $elements['solr_start'] = $solr_results['response']['start'];

    return theme('islandora_solr_grid', array(
      'results' => $object_results,
      'elements' => $elements,
    ));
  }*/
}
