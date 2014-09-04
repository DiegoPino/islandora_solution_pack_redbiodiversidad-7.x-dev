<?php

/**
 * @file
 * This is the template file for the print object.
 */
?>

  <div>
	  <h3>Object ID: <?php print $variables['islandora_object']->id;?><h2>	
      <h3>Label: <?php print $variables['islandora_object']->label;?><h2>	
	<dl class="islandora-object-qr">
	    <dt>
	      <?php if (isset($variables['QR'])): ?>
	        <?php print $variables['QR']; ?>
	      <?php endif; ?>
	    <dd></dd>
	</dl>
  	<h3>Our Darwin Core Record</h2>
  	<div class="islandora-metadata-fields islandora-object-fields">
	
    <?php if (isset($variables['islandora_dwc_html'])): ?>
      <?php print $variables['islandora_dwc_html']; ?>
    <?php endif; ?>
  	</div>
  </div>

