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
      <h3><?php print t('In collections'); ?></h3>
      <ul>
        <?php foreach ($variables['parent_collections'] as $collection): ?>
          <li><?php print l($collection->label, "islandora/object/{$collection->id}"); ?></li>
        <?php endforeach; ?>
      </ul>
    </div>
	<br>
	<hr/>
  <?php endif; ?>
<div class="panel-group" id="accordion">
	
  <?php if ($variables['eol_render_element']): ?>
  <div class="panel panel-default">
	  <div class="panel-heading">
	        <h4 class="panel-title">
	          <a data-toggle="collapse" data-parent="#accordion" href="#collapseOne">
	             <i class="fa fa-book fa-x2"></i>
				<?php print t('According to Enciclopedia of Life'); ?>
	          </a>
	        </h4>
	      </div>
 <div id="collapseOne" class="panel-collapse collapse in">
       <div class="panel-body">
	<?php print($variables['eol_render_element']);	?>
</div>
</div>
  </div>
  <?php endif; ?>

  <?php if (isset($variables['islandora_dwc_html'])): ?>
  <div class="panel panel-default">
	  <div class="panel-heading">
	        <h4 class="panel-title">
	          <a data-toggle="collapse" data-parent="#accordion" href="#collapseTwo">
	            <i class="fa fa-code fa-x2"></i>
				<?php print t('Darwin Core Record for this object'); ?>
	          </a>
	        </h4>
	      </div>
 <div id="collapseTwo" class="panel-collapse collapse">
       <div class="panel-body">
 
    <?php print $variables['islandora_dwc_html']; ?>
 
	</div>
</div>
</div>
 <?php endif; ?>
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
</div>
</div>
</div>

