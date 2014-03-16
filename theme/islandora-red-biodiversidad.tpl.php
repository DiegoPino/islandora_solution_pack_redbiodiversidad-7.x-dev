<?php

/**
 * @file
 * This is the template file for the object page for basic image
 *
 * @TODO: add documentation about file and available variables
 */
?>

<div class="islandora-red-biodiversidad-object islandora" vocab="http://schema.org/" prefix="dcterms: http://purl.org/dc/terms/">
  <div class="islandora-red-biodiversidad-content-wrapper clearfix">
    <?php 
	//print_r($variables);
	if (isset($islandora_object)): ?>
      <div class="islandora-red-biodiversidad-content">
        <?php //print_r($islandora_object); ?>
      </div>
    <?php endif; ?>
  </div>
  <div class="islandora-red-biodiversidad-metadata">
    <?php print $description; ?>
    <?php if ($parent_collections): ?>
      <div>
        <h2><?php print t('In collections'); ?></h2>
        <ul>
          <?php foreach ($parent_collections as $collection): ?>
            <li><?php print l($collection->label, "islandora/object/{$collection->id}"); ?></li>
          <?php endforeach; ?>
        </ul>
      </div>
    <?php endif; ?>
    <?php print $metadata; ?>
  </div>
</div>
