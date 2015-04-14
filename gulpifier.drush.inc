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

  $theme_path = drupal_get_path('theme', $theme);

  // Non-existing gulpfile or confirmation
  if (!file_exists($theme_path . '/gulpfile.js') || drush_confirm("Already existing gulpfile, do you want to rewrite it?")) {
    $css_compiler = drush_choice(array(
      FALSE  => 'None',
      'less' => 'LESS',
      'sass' => 'SASS',
    ), "Do you want to use a CSS compiler?");
    $use_sprite = drush_confirm("Do you want to create an image sprite?");
    $use_webfont = drush_confirm("Do you want to create a custom webfont from SVG files?");
    $use_sourcemaps = drush_confirm("Do you want to use sourcemaps?");
    $use_imagemin = drush_confirm("Do you want to compress images?");
    $use_jshint = drush_confirm("Do you want to verify the syntax of your files (JSHint)?");
    $use_gzip = drush_confirm("Do you want to gzip your assets?");

    // Start output buffering
    ob_start();
    // Include the template file
    include 'sample.gulpfile.php';
    // End buffering and return its contents
    $contents = ob_get_clean();
    $fp = fopen($theme_path . '/gulpfile.js', "w");
    fwrite($fp, $contents);
    fclose($fp);

  }
  $use_bootstrap = drush_confirm("Do you want to use bootstrap for your project?");
  if ($use_bootstrap) {
    if (!isset($css_compiler)) {
      $css_compiler = drush_choice(array(
        'less' => 'LESS',
        'sass' => 'SASS',
      ), "Which CSS compiler do you want to use?");
    }
    drush_gulpifier_handle_bootstrap($css_compiler, $theme_path);
  }
}

/**
 * @param $css_compiler
 */
function drush_gulpifier_handle_bootstrap($css_compiler, $theme_path) {
  $makefile = NULL;
  $extension_info = drush_get_extensions(FALSE);
  if (!isset($extension_info['bootstrap']) && drush_confirm("Bootstrap base theme was not found, do you want to download it?")) {
    // Invoke a new process to download bootstrap.
    $result = drush_invoke_process('@self', 'pm-download', array('bootstrap'));
    drush_print($result);
    $potential_makefiles = drush_scan_directory('.', '@.*\.make$@');
    if (!empty($potential_makefiles) && drush_confirm("Do you want to add it to a makefile?")) {
      $makefile = drush_choice($potential_makefiles);
      drush_file_append_data($makefile, "

; Include bootstrap for Gulpifier
projects[] = bootstrap
");
    }
  }
  if (!file_exists($theme_path . '/bootstrap')) {
    // Download sources
    $url = $css_compiler == 'less' ?
      'https://github.com/twbs/bootstrap/archive/v3.0.2.zip' :
      'https://github.com/twbs/bootstrap-sass/archive/v3.0.2.tar.gz';
    drush_print("Downloading boostrap sources from " . $url);
    $filename = drush_download_file($url, drush_tempdir());
    if (drush_file_is_tarball($filename)) {
      drush_print("Unpacking...");
      $extract_dir = drush_tempdir();
      $result = drush_tarball_extract($filename, $extract_dir, TRUE);
      if (!empty($result)) {
        $absolute_theme_path = drush_get_context('DRUSH_SELECTED_DRUPAL_ROOT') . '/' . $theme_path;
        drush_mkdir($absolute_theme_path . '/bootstrap');
        foreach (array('fonts', 'js', $css_compiler) as $dir) {
          drush_move_dir($extract_dir . '/bootstrap-3.0.2/' . $dir, $absolute_theme_path . '/bootstrap/' . $dir);
        }
        drush_print("Done");
      }
    }
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
  do {
    $name = drush_prompt("What will be the system name? (lowercase, only letters and _)");
  } while (!preg_match('@[a-z_]+@', $name));

  $human_name = drush_prompt("What will be the human-readable name?");

  // Path
  $profile = variable_get('install_profile', 'default');
  $paths = array(
    "profiles/$profile/themes/$name" => "Profile (profiles/$profile/themes/$name)",
    "sites/all/themes/$name" => "All themes (sites/all/themes/$name)",
    "sites/all/themes/custom/$name" => "Custom themes (sites/all/themes/custom/$name)",
  );
  $path = drush_choice($paths, 'Where do you want it to be created?');

  // Create all that new structure
  drush_mkdir($path);
  $info_data = "name = $human_name
core = 7.x";
  drush_file_append_data($path . '/' . $name . '.info', $info_data);

  return $name;
}
