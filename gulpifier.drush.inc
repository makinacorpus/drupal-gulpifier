<?php

/**
 * Implements hook_drush_command().
 */
function gulpifier_drush_command() {
  $items['gulpify'] = array(
    'description' => "Initialize a theme to work with gulpifier.",
    'options'     => array(
      'spreads' => array(
        'description'   => 'Comma delimited list of spreads.',
        'example-value' => 'mayonnaise,mustard',
      ),
    ),
    'aliases'     => array('gulpifier'),
    'bootstrap'   => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );
  return $items;
}

function drush_gulpifier_gulpify() {
  $theme = '';

  // First question, do we need to create the theme structure?
  if (drush_confirm("Do you need to create a theme?")) {
    $theme = drush_gulpifier_create_structure();
  }

  // Second question, get the theme to work on
  if (!$theme) {
    drush_include_engine('drupal', 'environment');
    $theme = drush_choice(drush_get_themes(), 'Which theme do you want to gulpify?', '!key');
  }
}

/**
 * Create the basic structure of a theme
 *
 * Asks for a name and path to put it
 *
 * @return string name
 */
function drush_gulpifier_create_structure() {
  $name = drush_prompt("What will be the system name? (lowercase, only letters and _)");

  $human_name = drush_prompt("What will be the human-readable name?");

  // Path
  $profile = variable_get('install_profile', 'default');
  $paths = array(
    "profiles/$profile/themes/$name" => "Profile (profiles/$profile/themes/$name)",
    'sites/all/themes/$theme_name' => "All themes (sites/all/themes/$name)",
    'sites/all/themes/custom/$theme_name' => "Custom themes (sites/all/themes/custom/$name)",
  );
  $path = drush_choice($paths, 'Where do you want it to be created?');

  // Create all that new structure
  drush_mkdir($path);
  $info_data = "name = $human_name
core = 7.x";
  drush_file_append_data($path . '/' . $name . '.info', $info_data);

  return $name;
}
