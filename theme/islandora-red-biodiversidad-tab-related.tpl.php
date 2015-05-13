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
    <div>
      <h2><?php print t('Related Darwin Core Objects Graph'); ?></h2>
      <div>
        <?php if (!empty($variables['related_sparql_results'])):?>
          <p>
            <?php print $variables['related_sparql_results'];?>
          </p>
        <?php endif; ?> 
        <div id="Legend" class="externalLegend"></div>
        <?php if(isset($variables['vlot'])): ?> 
          <?php print $variables['vlot'];?>
        <?php endif; ?>
      </div>
    </div>
  </div>
</div>
