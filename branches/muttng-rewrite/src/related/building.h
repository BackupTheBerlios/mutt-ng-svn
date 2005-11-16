/**
 * @page page_building Compiling
 *
 *   <b>NOTE: You need GNU make. Others won't work.</b>
 *
 *   In order to make the build process work at all, you need to create
 *   a file called <code>GNUmakefile.config.mine</code> in the top-level
 *   directory, i.e. in <code>mutt-ng/branches/muttng-rewrite</code>.
 *
 *   There you can set various options. Most important are the
 *   following:
 *
 *     - @c CCSTYLE=(gcc|suncc). Wether to assume use of GNU or Sun
 *       C/C++ compilers. Default: @c gcc.
 *     - @c DEBUG=(0|1). Whether to pass @c -g to the compilers/linkers.
 *       This is useful for running a debuger. This is not the same as
 *       the feature to make mutt-ng print debug messages (which
 *       currently cannot be turned off). Default: @c 0.
 *     - @c UNITPPDIR=/path. Where Unit++ (see
 *       <a href="http://unitpp.sf.net/">http://unitpp.sf.net/</a>) for
 *       running the unit tests is installed. This defaults to
 *       <code>/usr</code>. Default: @c /usr.
 *
 *   From the @c src subdirectory, the following are important targets:
 *
 *     - @c depend. Please always run "make depend" first.
 *     - @c all. Build everything.
 *     - @c test. Build an run all unit tests.
 *     - @c srcdoc. Run doxygen(1) to generate the docs from source. The
 *       output will be placed in the @c doxygen directory. @b NOTE: you
 *       need the graphviz package, too. If not, look through the file
 *       <code>src/Doxyfile</code> and there for a line called
 *       <code>HAVE_DOT</code>. Change the @c YES into @c NO.
 *
 *   If any build process fails, please notify the developers via
 *   <a href="mailto:mutt-ng-devel@lists.berlios.de">mutt-ng-devel@lists.berlios.de</a>.
 *
 */
