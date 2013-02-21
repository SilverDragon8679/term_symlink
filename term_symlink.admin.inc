<?php
/**
* @file
* Add, update, delete settings of Term Symlink module.
*/

function term_symlink_admin_settings() {
  $form = array();
  $term_symlink= variable_get('term_symlink', array());

  if ($term_symlink) {
    $tids= $term_symlink + array_keys($term_symlink);
    $term= taxonomy_term_load_multiple($tids);

    $form["saved_symlinks"]= array(
      '#type' => 'fieldset',
      '#title' => 'Saved term symlinks', 
      '#description' => 'Check items your want to be deleted.'
    );

    $form["saved_symlinks"]['delete'] = array('#type' => 'checkboxes');
    foreach ($term_symlink as $from => $to) {
      $form["saved_symlinks"]['delete']['#options'][$from] =
        "#$from (" . $term[$from]->name . ") points to #$to (" . $term[$to]->name . ")";
    }

    $form["saved_symlinks"]['delete_button']= array('#type' => 'submit', '#value' => 'Unlink');
  }
 
  $form['link']= array(
    '#type' => 'textfield',
    '#title' => t('Symlink term ID'),
    '#default_value' => '',
    '#size' => 30,
    '#maxlength' => 60,
    '#description' => t("A number (tid) of a term which will be a symbolic link for another term. Search by term name available."),
    '#autocomplete_path' => 'admin/term_symlink/autocomplete_callback',
    '#id' => 'link',
  );
  
  $form['target']= array(
    '#type' => 'textfield',
    '#title' => t('Target term ID'),
    '#default_value' => '',
    '#size' => 30,
    '#maxlength' => 60,
    '#description' => t("A number (tid) of a term which will be linked to. Search by term name available."),
    '#autocomplete_path' => 'admin/term_symlink/autocomplete_callback',
    '#id' => 'target',
  );

  $form['#submit']= array('term_symlink_admin_settings_submit');
  $form['#attached']['css'] = array(drupal_get_path('module', 'term_symlink') . '/admin.css');
  return system_settings_form($form);
}


function term_symlink_admin_settings_validate($form, &$form_state) {
  switch ($form_state['values']['op']) {
    case 'Unlink':
      break;
    
    default:
      $link = $form_state['values']['link'];
      $err_cnt= 0;
      if (!is_numeric($link)) {
        form_set_error('link', t('Please enter number.'));
        $err_cnt++;
      }

      $target = $form_state['values']['target'];
      if (!is_numeric($target)) {
        form_set_error('target', t('Please enter number.'));
        $err_cnt++;
      }
      if ($err_cnt) return false;
  
      $err_cnt= 0;

      $term_from = taxonomy_term_load($link);
      $term_to   = taxonomy_term_load($target);

      if (!$term_from) {
        form_set_error('target', t('Term id @tid does not exist', array('@tid' => $link) ) );
        $err_cnt++;
      }

      if (!$term_to) {
        form_set_error('target', t('Term id @tid does not exist', array('@tid' => $target) ) );
        $err_cnt++;
      }
      if ($err_cnt) return false;

      $nodes= taxonomy_select_nodes($link, $pager= FALSE, $limit = 1);
      if ($nodes) {
        form_set_error('target',
          t('Term @name is assigned to some nodes so can not be symlink.',
          array('@name' => $term_from->name))
        );
		return false;
      }

  } // end switch;
  
  return true;
}


function term_symlink_admin_settings_submit($form, $form_state) {
  $term_symlink= variable_get('term_symlink', array());

  if ( 'Unlink' == $form_state['values']['op'] ) {
    if ( $delete=$form_state['values']['delete'] ) {
      foreach ($delete as $key => $val) {
        unset($term_symlink[$val]);
      }
    }
    variable_set('term_symlink', $term_symlink);
    return;
  }

  $term_symlink= variable_get('term_symlink', array());
  $term_symlink[ $form_state['values']['link'] ] = $form_state['values']['target'];
  variable_set('term_symlink', $term_symlink);
}


function term_symlink_autocomplete_callback($string = '') {
  $matches = array();
  if ($string) {
    $result = db_select('taxonomy_term_data')
      ->fields('taxonomy_term_data', array('name', 'tid'))
      ->condition('name', db_like($string) . '%', 'LIKE')
      ->range(0, 12)
      ->execute();
    foreach ($result as $r) {
      $matches[$r->tid] = check_plain($r->name . ' (tid:' . $r->tid . ')');
    }
  }
  drupal_json_output($matches);
}
