<?php

/**
 * Implements hook_drush_command().
 */
function gulpifier_drush_command() {
  $items['gulpify'] = array(
    'description' => "Initialize a theme to work with gulpifier.",
    'options'     => array(),
    'aliases'     => array('gulpifier'),
    'bootstrap'   => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );
  return $items;
}

function drush_gulpifier_gulpify() {
  $theme = $theme_path = '';

  // First question, do we need to create the theme structure?
  if (drush_confirm("Do you need to create a theme?")) {
    system_rebuild_theme_data(); // just to refresh cache
    $theme = drush_gulpifier_create_structure();
    system_rebuild_theme_data(); // just to refresh cache
    $themes = list_themes(TRUE);
    $theme_path = dirname($themes[$theme]->filename);
  }

  // Second question, get the theme to work on
  if (!$theme) {
    drush_include_engine('drupal', 'environment');
    $theme = drush_choice(drush_get_themes(), 'Which theme do you want to gulpify?', '!key');
    $theme_path = drupal_get_path('theme', $theme);
  }

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
        'package'  => 'gulp.spritesmith',
      ),
      'webfont'    => array(
        'question' => "create a custom webfont from SVG files",
        'package'  => 'gulp-iconfont',
      ),
      'sourcemaps' => array(
        'question' => "use sourcemaps",
        'package'  => 'gulp-sourcemaps',
      ),
      'imagemin'   => array(
        'question' => "compress images",
        'package'  => 'gulp-imagemin',
      ),
      'jshint'     => array(
        'question' => "verify the syntax of your files (JSHint)",
        'package'  => 'gulp-jshint',
      ),
      'gzip'       => array(
        'question' => "gzip your assets",
        'package'  => 'gulp-gzip',
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
    if (!exec('npm --version')) {
      drush_print("You first need to install npm, check out https://nodejs.org/");
    }
    if (!exec('gulp --version')) {
      drush_print("You need to install globally, run:\n\tnpm install -g gulp");
    }
    drush_print("To install gulp and its plugins, run:\n\tcd $absolute_path && npm install");
    drush_print("To run gulp, enter 'gulp'. To watch files, enter 'gulp watch'.");
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

  if (isset($css_compiler)) {
    drush_mkdir($absolute_path . '/' . $css_compiler);
  }

  drush_mkdir($absolute_path . '/js');
  drush_file_append_data($absolute_path . '/js/map.json', "{}\n");
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

  // Handle jQuery update
  drush_gulpifier_handle_jquery_update($css_compiler, $theme_path, $name, $absolute_theme_path, $extension_info);

  if (!isset($extension_info['bootstrap']) && drush_confirm("Bootstrap base theme was not found, do you want to download it?")) {
    // Invoke a new process to download bootstrap.
    drush_invoke_process('@self', 'pm-download', array('bootstrap'), array('--select'), array('interactive' => TRUE));
    $potential_makefiles = drush_scan_directory('.', '@.*\.make$@');
    if (!empty($potential_makefiles) && drush_confirm("Do you want to add it to a makefile?")) {
      $potential_makefiles = array_combine(array_keys($potential_makefiles), array_keys($potential_makefiles));
      $makefile = drush_choice($potential_makefiles);
      if ($makefile) {
        // @TODO specify bootstrap version in the makefile
        drush_file_append_data($makefile, "\n\n; Include bootstrap for Gulpifier\nprojects[] = bootstrap\n");
      }
    }
  }

  // Append some bootstrap properties to the info file
  $themes = list_themes();
  if (empty($themes[$name]->info['base theme'])) {
    drush_file_append_data($absolute_theme_path . '/' . $name . '.info', "\n; Include bootstrap from Gulpifier\nbase theme = bootstrap\nsettings[bootstrap_cdn] = ''\n");
  }

  if (!file_exists($theme_path . '/bootstrap')) {
    // Download sources
    $repo_name = $css_compiler == 'less' ? 'bootstrap' : 'bootstrap-sass';
    $response = drupal_http_request('https://api.github.com/repos/twbs/' . $repo_name . '/releases');
    $response_tags = json_decode($response->data);
    $tags = array();
    foreach ($response_tags as $tag) {
      $tags[$tag->tag_name] = $tag->tag_name;
    }
    $chosen = drush_choice($tags, "Choose a boostrap version");

    $url = '';
    foreach ($response_tags as $tag) {
      if ($tag->tag_name == $chosen) {
        $url = $tag->tarball_url;
      }
    }
    drush_print("Downloading bootstrap sources from " . $url);
    $filename = drush_download_file($url, drush_tempdir());
    if (drush_file_is_tarball($filename)) {
      drush_print("Unpacking...");
      $extract_dir = drush_tempdir();
      $result = drush_tarball_extract($filename, $extract_dir, TRUE);
      if (!empty($result)) {
        drush_mkdir($absolute_theme_path . '/bootstrap');
        $source_dirs = $css_compiler == 'less' ? drupal_map_assoc(array(
          'fonts',
          'js',
          'less',
        )) : array(
          'assets/fonts/bootstrap'              => 'fonts',
          'assets/javascripts/bootstrap'        => 'js',
          'assets/stylesheets/bootstrap'        => 'scss',
          'vendor/assets/fonts/bootstrap'       => 'fonts',
          'vendor/assets/javascripts/bootstrap' => 'js',
          'vendor/assets/stylesheets/bootstrap' => 'scss',
        );
        foreach ($source_dirs as $source => $dest) {
          $dir = $extract_dir . '/' . $result[0] . $source;
          if (is_dir($dir)) {
            drush_move_dir($dir, $absolute_theme_path . '/bootstrap/' . $dest);
          }
        }
        drush_print("Done");
      }
    }
    $extension_info = drush_get_extensions(FALSE);
  }
  else {
    drush_print("Bootstrap sources already exist in  " . $theme_path . '/bootstrap');
  }

  if (!file_exists($theme_path . '/' . $css_compiler)) {
    drush_print("Copying subtheme info...");
    $sub_bootstrap_path = dirname($extension_info['bootstrap']->filename);
    if (is_dir($sub_bootstrap_path . '/bootstrap_subtheme')) {
      drush_copy_dir($sub_bootstrap_path . '/bootstrap_subtheme/' . $css_compiler, $absolute_theme_path . '/' . $css_compiler);
    }
    elseif (is_dir($sub_bootstrap_path . '/starterkits/' . $css_compiler)) {
      drush_copy_dir($sub_bootstrap_path . '/starterkits/less/less', $absolute_theme_path . '/' . $css_compiler);
    }
    drush_print("Please take the time to copy-paste info from starterkit from the boostrap theme.");
  }
}


/**
 * Check if jquery_update is installed
 *
 * @param $css_compiler string CSS Compiler (Less, Sass, or '')
 * @param $theme_path string Theme path
 * @param $name string Theme name
 */
function drush_gulpifier_handle_jquery_update($css_compiler, $theme_path, $name, $absolute_theme_path, $extension_info) {
  if (!isset($extension_info['jquery_update']) && drush_confirm("jQuery Update module was not found, do you want to download it?")) {
    // Invoke a new process to download bootstrap.
    drush_invoke_process('@self', 'pm-download', array('jquery_update'));
    $potential_makefiles = drush_scan_directory('.', '@.*\.make$@');
    if (!empty($potential_makefiles) && drush_confirm("Do you want to add it to a makefile?")) {
      $potential_makefiles = array_combine(array_keys($potential_makefiles), array_keys($potential_makefiles));
      $makefile = drush_choice($potential_makefiles);
      drush_file_append_data($makefile, "\nprojects[] = jquery_update\n");
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
  $machine_names = array_merge(array_keys(module_list()), array_keys(list_themes()));
  do {
    $name = drush_prompt("What will be the system name? (lowercase, only letters and _)");
  } while (!preg_match('@[a-z_]+@', $name) || (in_array($name, $machine_names) && !drush_print("Machine name already taken")));

  $human_name = drush_prompt("What will be the human-readable name?");

  // Path
  $profile = variable_get('install_profile', 'default');
  $paths = array(
    "profiles/$profile/themes/$name" => "Profile (profiles/$profile/themes/$name)",
    "sites/all/themes/$name"         => "All themes (sites/all/themes/$name)",
    "sites/all/themes/custom/$name"  => "Custom themes (sites/all/themes/custom/$name)",
  );
  $path = drush_choice($paths, 'Where do you want it to be created?');

  // Create all that new structure
  drush_mkdir($path);
  $info_data = "name = $human_name
core = 7.x

; Bundle files, don't remove
stylesheets[all][] = dist/style.min.css
scripts[] = dist/script.min.js

; --- Gulpifier settings ---
settings[gulpifier_enabled] = 1

settings[gulpifier_single_js] = 1
settings[gulpifier_js_discovery] = 1

settings[gulpifier_single_css] = 1

; Whitelist
;settings[gulpifier_whitelist][css][] = admin_menu:admin_menu.css
;settings[gulpifier_whitelist][js][] = admin_menu:admin_menu.js
";
  drush_file_append_data($path . '/' . $name . '.info', $info_data);

  return $name;
}
