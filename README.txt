
Gulpifier module
================
by Makina Corpus

This module ease the use of Gulp as a frontend automation tool for Drupal. It
aims to provide a clean structure to work with JS and CSS files, as well as
images, sprites and iconfonts.

This README is for interested themers, this is an advanced module for advanced
themers that are looking to bring Gulp to their already existing Drupal
development cycle.


Philosophy
----------

The module allows to respect current front-end performance tips:

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

  * have a single JS file (not 3 or 4 bundles, changing along with the pages),
  there is Advagg module that can help with that, but we aim to decide manually
  which scripts go in our single bundle and have this bundle always included,
  and thus cached by the browser
  * have a single CSS file, clean of all core and module work, usually we want
  to start our theming from scratch, without conflicts from modules
  * be able to use LESS or SASS, with sourcemaps for easier debugging
  * a continuous workflow allowing to edit JS/CSS on the fly (file watching)
  * Image sprites
  * Icon font generation
  * basically any tool that Gulp provides


Install
============

Apply this core patch https://www.drupal.org/node/2329453#comment-9470115
This prevent from having segmentation faults when clearing caches (due to
malformed .info files in node_modules)


How it works
============

All settings for gulpifier are relative to a theme, so you can use it on any
theme, may it be front-end or back-end, or multiple themes at once.

Most settings can be set in the interface trhough the theme settings form,
but we recommend to st them in your theme's .info file so that they are tracked
in your VCS. These are always prefixed by `settings[gulpifier_...]`

Firstly, to enable gulpifier for a theme, use `settings[gulpifier_enabled] = 1`
in your theme info file, or trough the checkbox of your theme settings form.


Single JS bundle
----------------

A map.json file is populated on each page visit to construct a potential list
of JS files, that will be later minified and compressed by Gulp.
JS files provided by core, modules, or theme are going to this generated map,
it can be manually modified to blacklist any file that you don't want to go in
the bundle, or to change the order of the files within the bundle.
A good example of JS file to blacklist is token.js, there will rearely be a time
when your anonymous visitors will access the Token dialog, although it may have
been picked by the discovery process. You can set it in the info file with
`settings[gulpifier_discover]`. The path to the map.json file is by default in
`yourtheme/js/map.json` but you can override it in your theme info file with
`settings[gulpifier_map_path]`.

### Whitelist

There is also a whitelist of JS files, that Gulpifier won't process, they won't
go into map.json and will be added on the page as Drupal does otherwise.
A good example for this whitelist is admin_menu.js, it needs to be present when
you are logged in, but there is not point to have it in the bundle, nor to put
it in the blacklist.
This whitelist is defined in the .info file of your theme:

    settings[gulpifier_whitelist][js][] = "admin_menu:admin_menu.js"

You can see that there is a special notation "module_name:internal_path" that
can be used, it's more human-readable and doesn't care about subdirectories that
these modules may be in. If you wish, you can also use absolute path, still
relative to the DRUPAL_ROOT:

    settings[gulpifier_whitelist][js][] = "sites/all/modules/admin_menu/admin_menu.js"

As soon as you modify this whitelist, don't forget to flush your theme registry.

### Blacklist

In case you don't want some JS files either in the map.json nor in any pages,
put them in the js_blacklist as such:

    settings[gulpifier_js_blacklist][] = "toolbar:toolbar.js"


You can activate the single JS bundle in the info file with
`settings[gulpifier_single_js]`.


Single CSS bundle
-----------------

All CSS are by default removed from the front-end, core and modules. This allows
you to start fresh from a Framework like bootstrap without worrying about other
files.

There is a whitelist of CSS files, that Gulpifier won't touch, they won't go
into map.json and will be added on the page as Drupal does otherwise.
A good example for this whitelist is jquery.ui theme, it needs to be present when
you are visinting a page with a widget, but there is not point to have in the
bundle, if this widget is only on one page of your site.
This whitelist is defined in the .info file of your theme:

    settings[gulpifier_whitelist][css][] = "admin_menu:admin_menu.js"

You can see that there is a special notation "module_name:internal_path" that
can be used, it's more human-readable and doesn't care about subdirectories that
these modules may be in. If you wish, you can also use absolute path, still
relative to the DRUPAL_ROOT, that also works for base themes:

    settings[gulpifier_whitelist][css][] = "sites/all/themes/rubik/views-admin.rubik.css"

As soon as you modify this whitelist, don't forget to flush your theme registry.

You can activate the single CSS bundle in the info file with
`settings[gulpifier_single_js]`.


Other settings
--------------

Most of the paths are overridable, e.g.: the map.json, the bundles but you must
change them also in the generated `gulpfile.js`.

### Same scope/group

This setting is available for both JS and CSS and allow all remaining files that
would not be in the bundle to be aggregated in a single remaining file

### No cache buster

Some gulp modules or browser refreshing features won't work with the cache
busting parameter (`?abcdef`) that is added to the assets. It is totally
optional and gulpifier does not require it at all. But you can learn more here:
  - https://hacks.mozilla.org/2014/02/live-editing-sass-and-less-in-the-firefox-developer-tools/
  - http://code.tutsplus.com/tutorials/working-with-less-and-the-chrome-devtools--net-36636

### Discovery killswitch
In production, you may want to disable the JS discovery to prevent
creation/modification of JSON maps, regardless of theme settings. To do so, add
this line to your `settings.php` file:

  $conf['gulpifier_discovery_killswitch'] = TRUE;
