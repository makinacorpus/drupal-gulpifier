{
  "name": "<?php echo $theme; ?>",
  "version": "0.1.0",
  "private": true,
  "devDependencies": {
    "gulp": "^3.8.6",
    "gulp-concat": "^2.3.3",
<?php if ($use_gzip): ?>
    "gulp-gzip": "0.0.8",
<?php endif; ?>
<?php if ($use_webfont): ?>
    "gulp-iconfont": "^1.0.0",
<?php endif; ?>
<?php if ($use_imagemin): ?>
    "gulp-imagemin": "^2.2.1",
    "imagemin-pngquant": "^4.0.0"
<?php endif; ?>
<?php if ($use_jhint): ?>
    "gulp-jshint": "^1.7.1",
<?php endif; ?>
<?php if ($css_compiler == 'less'): ?>
    "gulp-less": "^1.3.1",
<?php endif; ?>
    "gulp-minify-css": "^0.4.3",
    "gulp-rename": "^1.2.0",
<?php if ($use_sourcemaps): ?>
    "gulp-sourcemaps": "^1.3.0",
<?php endif; ?>
    "gulp-uglify": "^0.3.1",
<?php if ($use_sprite): ?>
    "gulp.spritesmith": "^2.5.1",
<?php endif; ?>
  }
}
