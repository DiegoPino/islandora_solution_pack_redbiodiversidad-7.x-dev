<?php

/**
 * @file
 * This is the template file for the object page for DwC
 *
 * @TODO: add documentation about file and available variables
 */
?>
<?php
/**
 * @file
 * This is the template file for the object page for Dwc objects.
 */
?>
<div class="islandora-related-objects islandora">
  <div class="islandora-related-content-wrapper clearfix">

    <?php if($related): ?>
      <div>
        <h2><?php print t('Related Darwin Core Objects'); ?></h2>
        <ul>
          <?php foreach ($related as $item): ?>
            <li><?php print $item['relation']; ?>
				<?php print "-> " .$item['model_label']; ?>
				<?php print l( $item['object']->label, "islandora/object/".$item['object']->id.""); ?>
			</li>
          <?php endforeach; ?>
        </ul>
      </div>
    <?php endif; ?>
  </div>
</div>


