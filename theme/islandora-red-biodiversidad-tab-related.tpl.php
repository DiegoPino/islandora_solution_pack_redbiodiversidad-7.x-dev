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
  		<?php if(isset($variables['vlot'])): ?>
		<h2><?php print t('Related Darwin Core Objects Graph'); ?></h2>
  		<div>
  			<?php print $variables['vlot'];?>
  		</div>
  		<?php endif; ?>
    
      </div>
  </div>
</div>


