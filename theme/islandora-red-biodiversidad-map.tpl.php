<?php
/**
 *
 * @see islandora_leaflet, islandora_leaflet_markercluster and our own implementation in /js
 */
?>
<?php print $results; ?>

<?php if ($variables['wasgeocoded']):?>
     
      <p>
      <?php print $variables['geodecoded'];?>
      </p>
<p>Nominatim Search Courtesy of <a href="http://www.mapquest.com/" target="_blank">MapQuest</a> <img src="http://developer.mapquest.com/content/osm/mq_logo.png"></p>
<?php endif; ?> 
