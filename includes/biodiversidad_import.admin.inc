<?php

/**
*	Define a form to import an excel file with GBIF data
* Diego Pino Navarro Marzo 2014
*/
function red_biodiversidad_excelform(array $form, array &$form_state, $object){
    $form_state['islandora']['objects'][0] = $object;
	$form['file'] = array(
	    '#type' => 'file',
	    '#title' => t('Subir un archivo Excel con Formato GBIF'),
		 
	  );
	
	$form['red_biodiversidad_force'] = array(
	  '#type' => 'checkbox',
	  '#title' => t('Forzar sincronizacion')
	);

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Procesar Excel'
	);

	return $form;
}


function red_biodiversidad_excelform_submit(array $form, array &$form_state){
   	module_load_include('inc', 'islandora_red_biodiversidad', 'includes/biodiversidad_processexcel');
    $validators = array(
      'file_validate_extensions' => array('xls', 'xlsx'),
    );
   $file = file_save_upload('file', $validators);  
   if ($file)
    {
	$object = $form_state['islandora']['objects'][0];
    if ($file = file_move($file, 'public://')) {
      // Save the file for use in the submit handler.
      //$form_state['storage']['file'] = $file;
  	  	drupal_set_message(t('The form has been submitted and the excel has been saved, filename: @filename.', array('@filename' => $file->filename)));
     	$id = red_biodiversidad_ingest_specimens_fromexcel($file, $form_state['values']['red_biodiversidad_force'],$object);
	}
	
	}
}

