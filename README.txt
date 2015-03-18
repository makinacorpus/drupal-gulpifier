
Gulpifier module
================
by SebCorbin from Makina Corpus

This module ease the use of Gulp as a frontend automation tool for Drupal. It
aims to provide a clean structure to work with JS and CSS files, as well as
images, sprites and iconfonts.

This README is for interested themers, this is an advanced module for advanced
themers that are looking to bring Gulp to their already existing Drupal
development cycle.


Philosophy
----------

The module provides allow to respect current front-end performance tips:

  * fewer requests for assets (JS and CSS files)
  * minification
  * compression

Overall it is all about network waterfall optimisation to display content under
the 1000ms barrier on mobile. This obviously a goal, and it depends on many
external factors (your server configuration, network latency, use of CDN, ...)


Principles
----------

Drupal is already doing a good job for small sites:

  * CSS standard minification
  * CSS/JS aggregation through bundles
  * GZip compression if enabled on Apache
  * Image resizing

But we lack some manual override to, for example:

  * have a single JS file (not 3 or 4 bundles), there is Advagg module that can
  help with that, but we aim to decide manually which script goes in our single
  bundle
  * have a single CSS file, clean of all core and module work, usually we want
  to start our theming from scratch, without conflicts from modules
  * be able to use LESS or SASS, with sourcemaps for easier debugging
  * a continuous workflow allowing to edit JS/CSS on the fly (file watching)
  * Image sprites
  * Icon font generation
  * basically any tool that Gulp provides


How it works
============

Single JS bundle
----------------

A map.json file is populated on each page visit to construct a potential list
of JS files, that will be later minified and compressed by Gulp.
JS files provided by core, modules, or theme are going to this generated map,
it can be manually modified to blacklist any file that you don't want to go in
the bundle, or to change the order of the files within the bundle.
A good example of JS file to blacklist is token.js, there will never be a time
when your anonymous visitors will access the Token dialog, although it may have
been picked by the discovery process.

This feature is linked to the variable `gulpifier_discover`, that you can
change in _Configuration > Development > Performance_ or via your settings.php
file.


There is also a whitelist of JS files, that Gulpifier won't touch, they won't go
into map.json and will be added on the page as Drupal does otherwise.
A good example for this whitelist is admin_menu.js, it needs to be present when
you are logged in, but there is not point to have in the bundle, nor to put it
in the blacklist.
This whitelist is defined in the .info file of your theme:

    gulpifier_whitelist[js][] = "admin_menu:admin_menu.js"

You can see that there is a special notation "module_name:internal_path" that
can be used, it's more human-readable and doesn't care about subdirectories that
these modules may be in. If you wish, you can also use absolute path, still
relative to the DRUPAL_ROOT:

    gulpifier_whitelist[js][] = "sites/all/modules/admin_menu/admin_menu.js"

As soon as you modify this whitelist, don't forget to flush your theme registry.

This feature is linked to the variable `gulpifier_single_js`, that you can
change in _Configuration > Development > Performance_ or via your settings.php
file.


Single CSS bundle
-----------------

All CSS are by default removed from the front-end, core and module. This allows
you to start fresh from a Framework like bootstrap without worrying about other
files.

There is a whitelist of CSS files, that Gulpifier won't touch, they won't go
into map.json and will be added on the page as Drupal does otherwise.
A good example for this whitelist is jquery.ui theme, it needs to be present when
you are visinting a page with a widget, but there is not point to have in the
bundle, if this widget is only on one page of your site.
This whitelist is defined in the .info file of your theme:

    gulpifier_whitelist[css][] = "admin_menu:admin_menu.js"

You can see that there is a special notation "module_name:internal_path" that
can be used, it's more human-readable and doesn't care about subdirectories that
these modules may be in. If you wish, you can also use absolute path, still
relative to the DRUPAL_ROOT, that also works for base themes:

    gulpifier_whitelist[css][] = "sites/all/themes/rubik/views-admin.rubik.css"

As soon as you modify this whitelist, don't forget to flush your theme registry.

This feature is linked to the variable `gulpifier_single_css`, that you can
change in _Configuration > Development > Performance_ or via your settings.php
file.
