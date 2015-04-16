var gulp = require('gulp'),
  rename = require('gulp-rename'),
  concat = require('gulp-concat'),
  uglify = require('gulp-uglify'),
  cssmin = require('gulp-minify-css'),
<?php if ($plugins['jhint']['answer']): ?>
  jshint = require('gulp-jshint'),
<?php endif; ?>
<?php if ($css_compiler == 'less'): ?>
  less = require('gulp-less'),
<?php endif; ?>
<?php if ($css_compiler == 'sass'): ?>
  sass = require('gulp-sass'),
<?php endif; ?>
<?php if ($plugins['gzip']['answer']): ?>
  gzip = require('gulp-gzip'),
<?php endif; ?>
<?php if ($plugins['imagemin']['answer']): ?>
  imagemin = require('gulp-imagemin'),
  pngquant = require('imagemin-pngquant'),
<?php endif; ?>
<?php if ($plugins['sourcemaps']['answer']): ?>
  sourcemaps = require('gulp-sourcemaps'),
<?php endif; ?>
<?php if ($plugins['webfont']['answer']): ?>
  iconfont = require('gulp-iconfont'),
<?php endif; ?>
<?php if ($plugins['sprite']['answer']): ?>
  spritesmith = require('gulp.spritesmith'),
<?php endif; ?>
  use_sourcemaps = <?php echo $plugins['sourcemaps']['anwer'] ? 'true' : 'false'; ?>;


// Concat and minify JS, reading map.json
gulp.task('js', function () {
  var map = require('./js/map.json'), list = [];
  for (var i in map) {
    if (map.hasOwnProperty(i) && map[i]) {
      // Make relative to drupal path
      list.push('../../../../' + i);
    }
  }
  var pipe = gulp.src(list);
  if (use_sourcemaps) {
    pipe = pipe.pipe(sourcemaps.init());
  }
  pipe = pipe.pipe(concat('script.min.js'))
    .pipe(uglify());
  if (use_sourcemaps) {
    pipe = pipe.pipe(sourcemaps.write('./maps'));
  }
  return pipe.pipe(gulp.dest('./dist/'))
<?php if ($plugins['gzip']['answer']): ?>
    .pipe(gzip())
    .pipe(gulp.dest('./dist/'))
<?php endif; ?>
    .on('error', errorHandler);
});

<?php if ($plugins['jshint']['answer']): ?>
// Verify JS syntax
gulp.task('jshint', function () {
  return gulp.src([
    './js/*.js',
    '!./js/*.min.js',
    '../../modules/**/*.js'
  ])
    .pipe(jshint())
    .pipe(jshint.reporter('default'))
    .pipe(jshint.reporter('fail'))
    .on('error', errorHandler);
});
<?php endif; ?>

<?php if ($css_compiler == 'less'): ?>
// LESS compilation
gulp.task('less', function () {
  var pipe = gulp.src('./less/style.less');
  if (use_sourcemaps) {
    pipe = pipe.pipe(sourcemaps.init());
  }
  pipe = pipe
    .pipe(less())
    .pipe(cssmin())
    .pipe(rename({suffix: '.min'}));
  if (use_sourcemaps) {
    pipe = pipe.pipe(sourcemaps.write('./maps'));
  }
  return pipe
    .pipe(gulp.dest('./dist/'))
<?php if ($plugins['gzip']['answer']): ?>
    .pipe(gzip())
    .pipe(gulp.dest('./dist/'))
<?php endif; ?>
    .on('error', errorHandler);
});
<?php endif; ?>

<?php if ($plugins['imagemin']['answer']): ?>
// Optimisation des images
gulp.task('images', function () {
  return gulp.src(['./img*/*'<?php if($plugins['sprite']['answer']): ?>, './dist*/sprite.png'<?php endif; ?>])
    .pipe(imagemin({
      progressive: true
    }))
    .pipe(gulp.dest('.')).on('error', errorHandler);
});
<?php endif; ?>

<?php if ($plugins['webfont']['answer']): ?>
// Icons
gulp.task('icons', function () {
  return gulp.src('svg/*')
    .pipe(iconfont({
      fontName: 'iconsfont',
      appendCodepoints: true,
      normalize: true
    }))
    .pipe(gulp.dest('./dist/'))
});
<?php endif; ?>

<?php if ($plugins['sprite']['answer']): ?>
gulp.task('sprite', function () {
  var spriteData =
    gulp.src('img/sprite/*.*') // source path of the sprite images
      .pipe(spritesmith({
        imgName: 'sprite.png?' + (new Date).getTime(),
<?php if ($css_compiler): ?>
        cssName: 'sprite.<?php echo $css_compiler; ?>',
<?php endif; ?>
        algorithm: 'binary-tree'
      }));

  spriteData.img.pipe(rename('sprite.png')).pipe(gulp.dest('./dist/')); // output path for the sprite
<?php if ($css_compiler): ?>
  spriteData.css.pipe(gulp.dest('./<?php echo $css_compiler; ?>/')); // output path for the CSS
<?php endif; ?>
});
<?php endif; ?>

gulp.task('default', [
  'jshint',
  'js',
  'sprite',
  'images',
  'less',
  'icons'
]);

gulp.task('watch', function () {
  gulp.watch(['./js/*', '../../modules/**/*.js'], [
    'js',
<?php if ($plugins['jshint']['answer']): ?>
    'jshint'
<?php endif; ?>
  ]);
<?php if ($css_compiler): ?>
  gulp.watch('./<?php echo $css_compiler; ?>/**/*', ['<?php echo $css_compiler; ?>']);
<?php endif; ?>
<?php if ($plugins['sprite']['answer']): ?>
  gulp.watch('./img/*', ['images', 'sprite', 'less']);
<?php endif; ?>
<?php if ($plugins['imagemin']['answer']): ?>
  gulp.watch(['./img/*', '!./img/sprite.png'], ['images']);
<?php endif; ?>
<?php if ($plugins['sprite']['answer']): ?>
  gulp.watch(['./img/*', '!./img/sprite.png'], ['sprite', 'images', 'less']);
<?php endif; ?>
<?php if ($plugins['webfont']['answer']): ?>
  gulp.watch('./svg/*', ['icons']);
<?php endif; ?>
});

// Handle the error
function errorHandler(error) {
  console.log(error.toString());
  this.emit('end');
}
