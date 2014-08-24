<?php

/**
 * @file
 * This is the template file for the object page for DwC-A
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
<div class="islandora-object islandora ">
<div><span>Object ID </span><span class="islandora-red-biodiversidad-info"><?php print $variables['islandora_object']->id;?></span></div>	
<dl class="islandora-object-tn">
    <dt>
      <?php if (isset($variables['islandora_thumbnail_url'])): ?>
        <img src="<?php print $variables['islandora_thumbnail_url']; ?>"/></dt>
      <?php endif; ?>
    <dd></dd>
</dl>
<dl class="islandora-object-qr">
    <dt>
      <?php if (isset($variables['QR'])): ?>
        <?php print $variables['QR']; ?>
      <?php endif; ?>
    <dd></dd>
</dl>
  <?php if ($variables['parent_collections']): ?>
    <div class="islandora-red-biodiversidad-content">
      <h2><?php print t('In collections'); ?></h2>
      <ul>
        <?php foreach ($variables['parent_collections'] as $collection): ?>
          <li><?php print l($collection->label, "islandora/object/{$collection->id}"); ?></li>
        <?php endforeach; ?>
      </ul>
    </div>
	<br>
	<hr/>
  <?php endif; ?>
  
 
<div class="islandora-red-biodiversidad islandora-red-biodiversidad-content">
	<h2>DWC-A download available</h2>
	<div class="islandora-metadata-fields islandora-object-fields">
	
  <?php if (isset($variables['dwca-link'])): ?>
    <?php print $variables['dwca-link']; ?>
  <?php endif; ?>
	</div>
</div>
</div>
  
 
<h2>Dublin Core for this object</h2>
 <div class="islandora-default-metadata">
      <?php if (isset($variables['metadata'])): ?>
        <?php print $variables['metadata']; ?>
      <?php endif; ?>
 </div> 


