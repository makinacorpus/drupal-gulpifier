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

  // @TODO check for npm dependency

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

  $absolute_path = drush_get_context('DRUSH_SELECTED_DRUPAL_ROOT') . '/' . $theme_path;

  // Non-existing gulpfile or confirmation
  if (!file_exists($theme_path . '/gulpfile.js') || drush_confirm("Already existing gulpfile, do you want to rewrite it?")) {
    $css_compiler = drush_choice(array(
      FALSE  => 'None',
      'less' => 'LESS',
      'sass' => 'SASS',
    ), "Do you want to use a CSS compiler?");

    $plugins = array(
      'sprite'     => array(
        'question' => "create an image sprite",
        'package' => 'gulp.spritesmith',
      ),
      'webfont'    => array(
        'question' => "create a custom webfont from SVG files",
        'package' => 'gulp-iconfont',
      ),
      'sourcemaps' => array(
        'question' => "use sourcemaps",
        'package' => 'gulp-sourcemaps',
      ),
      'imagemin'   => array(
        'question' => "compress images",
        'package' => 'gulp-imagemin',
      ),
      'jshint'     => array(
        'question' => "verify the syntax of your files (JSHint)",
        'package' => 'gulp-jshint',
      ),
      'gzip'       => array(
        'question' => "gzip your assets",
        'package' => 'gulp-gzip',
      ),
    );
    $package_install = array(
      'gulp',
      'gulp-rename',
      'gulp-concat',
      'gulp-uglify',
      'gulp-minify-css',
    );
    if ($css_compiler) {
      $package_install[] = 'gulp-' . $css_compiler;
    }
    foreach ($plugins as $name => &$plugin) {
      $plugin['answer'] = drush_confirm("Do you want to " . $plugin['question'] . '?');
      if ($plugin['answer']) {
        $package_install[] = $plugin['package'];
      }
    }

    // Start output buffering
    ob_start();
    // Include the template file
    include 'templates/sample.gulpfile.php';
    // End buffering and return its contents
    $contents = ob_get_clean();
    file_put_contents($theme_path . '/gulpfile.js', $contents);

    $package_json = array(
      'name'    => $theme,
      'version' => '0.1.0',
      'private' => TRUE,
    );
    foreach ($package_install as $package) {
      $package_json['devDependencies'][$package] = 'latest';
    }
    file_put_contents($theme_path . '/package.json', json_encode($package_json, JSON_PRETTY_PRINT) . "\n");

    drush_print();
    drush_print("To install gulp and its plugins, run cd $absolute_path && npm install");
    drush_print("To run gulp, run gulp. To watch files, run gulp watch.");
    drush_print();


    if ($plugins['sprite']['answer']) {
      drush_mkdir($absolute_path . '/images');
      drush_mkdir($absolute_path . '/images/sprite');
    }
  }

  $use_bootstrap = drush_confirm("Do you want to use bootstrap for your project?");
  if ($use_bootstrap) {
    if (!isset($css_compiler)) {
      $css_compiler = drush_choice(array(
        'less' => 'LESS',
        'sass' => 'SASS',
      ), "Which CSS compiler do you want to use?");
    }
    drush_gulpifier_handle_bootstrap($css_compiler, $theme_path, $theme);
  }

  drush_mkdir($absolute_path . '/js');
  drush_file_append_data($absolute_path . '/js/map.json', "{}\n");
  drush_mkdir($absolute_path . '/less');
  drush_mkdir($absolute_path . '/images');
}

/**
 * Prepare bootstrap install
 *
 * @param $css_compiler string CSS Compiler (Less, Sass, or '')
 * @param $theme_path string Theme path
 * @param $name string Theme name
 */
function drush_gulpifier_handle_bootstrap($css_compiler, $theme_path, $name) {
  $makefile = NULL;
  $absolute_theme_path = drush_get_context('DRUSH_SELECTED_DRUPAL_ROOT') . '/' . $theme_path;

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
    drush_file_append_data($absolute_theme_path . '/' . $name . '.info', "

; Include bootstrap from Gulpifier
base theme = bootstrap
");
  }

  if (!file_exists($theme_path . '/bootstrap')) {
    // Download sources
    $url = $css_compiler == 'less' ?
      'https://github.com/twbs/bootstrap/archive/v3.0.2.zip' :
      'https://github.com/twbs/bootstrap-sass/archive/v3.0.2.0.tar.gz';
    drush_print("Downloading boostrap sources from " . $url);
    $filename = drush_download_file($url, drush_tempdir());
    drush_print('filename : '.$filename);
    if (drush_file_is_tarball($filename)) {
      drush_print("Unpacking...");
      $extract_dir = drush_tempdir();
      $result = drush_tarball_extract($filename, $extract_dir, TRUE);
      if (!empty($result)) {
        drush_mkdir($absolute_theme_path . '/bootstrap');
        foreach (array('fonts', 'js', $css_compiler) as $dir) {
          drush_move_dir($extract_dir . '/bootstrap-3.0.2/' . $dir, $absolute_theme_path . '/bootstrap/' . $dir);
        }
        drush_print("Done");
      }
    }
  }

  if (!file_exists($theme_path . '/less')) {
    drush_print("Copying subtheme info...");
    $sub_bootstrap_path = dirname($extension_info['bootstrap']->filename);
    drush_copy_dir($sub_bootstrap_path . '/bootstrap_subtheme/less', $absolute_theme_path . '/less');
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
core = 7.x

stylesheets[all][] = dist/style.min.css
scripts[] = dist/script.min.js
";
  drush_file_append_data($path . '/' . $name . '.info', $info_data);

  return $name;
}
