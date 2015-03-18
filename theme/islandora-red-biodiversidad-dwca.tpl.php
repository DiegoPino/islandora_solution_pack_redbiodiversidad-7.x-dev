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
	<h2>DWC-A download</h2>
	<div class="islandora-metadata-fields islandora-object-fields">
	
  <?php if (isset($variables['dwca-link'])): ?>
    <?php print $variables['dwca-link']; ?>

<?php else:?>
    <?php print t('Download not available yet')?>
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
 
 
 <div class="panel-group" id="datastreams">
   <div class="panel panel-default">
 	  <div class="panel-heading">
 	        <h4 class="panel-title">
 	          <a data-toggle="collapse" data-parent="#datastreams" href="#collapseFive">
 	            <i class="fa fa-download fa-x2"></i>
 				<?php print t('File details'); ?>
 	          </a>
 	        </h4>
 	      </div>
  <div id="collapseFive" class="panel-collapse collapse">
        <div class="panel-body">
 <table class="table">
   <tr>
     <th><?php print t('ID'); ?></th>
     <th><?php print t('Label'); ?></th>
     <th><?php print t('Size'); ?></th>
     <th><?php print t('Mimetype'); ?></th>
     <th><?php print t('Created'); ?></th>
   </tr>
   <?php foreach($datastreams as $key => $value): ?>
   <tr>
       <td><?php if(isset($value['id'])): ?><?php print $value['id']; ?><?php endif; ?></td>
       <td><?php if(isset($value['label_link'])): ?><?php print $value['label_link']; ?><?php endif; ?></td>
       <td><?php if(isset($value['size'])): ?><?php print $value['size']; ?><?php endif; ?></td>
       <td><?php if(isset($value['mimetype'])): ?><?php print $value['mimetype']; ?><?php endif; ?></td>
       <td><?php if(isset($value['created_date'])): ?><?php print $value['created_date']; ?><?php endif; ?></td>
   </tr>
   <?php endforeach; ?>
 </table>
 </div>
 
 


