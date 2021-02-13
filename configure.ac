dnl -*- sh -*-
dnl the "configure" script is made from this by running GNU "autoconf"
dnl
dnl Configuration script for the Fast Light Tool Kit (FLTK).
dnl
dnl Copyright 1998-2021 by Bill Spitzak and others.
dnl
dnl This library is free software. Distribution and use rights are outlined in
dnl the file "COPYING" which should have been included with this file.  If this
dnl file is missing or damaged, see the license at:
dnl
dnl     https://www.fltk.org/COPYING.php
dnl
dnl Please see the following page on how to report bugs and issues:
dnl
dnl      https://www.fltk.org/bugs.php
dnl

dnl We need at least autoconf 2.50...
AC_PREREQ([2.50])


dnl Package name and version
AC_INIT([fltk], [1.4.0], [https://github.com/fltk/fltk/issues], [fltk], [https://www.fltk.org/])

FLTK_VERSION="AC_PACKAGE_VERSION"
FLTK_VERSION_MAJOR=$(echo AC_PACKAGE_VERSION | awk -F. '{print $1}')
FLTK_VERSION_MINOR=$(echo AC_PACKAGE_VERSION | awk -F. '{print $2}')
FLTK_VERSION_PATCH=$(echo AC_PACKAGE_VERSION | awk -F. '{print $3}')

echo "FLTK_VERSION_MAJOR=$FLTK_VERSION_MAJOR"
echo "FLTK_VERSION_MINOR=$FLTK_VERSION_MINOR"
echo "FLTK_VERSION_PATCH=$FLTK_VERSION_PATCH"

FL_DSO_VERSION="${FLTK_VERSION_MAJOR}.${FLTK_VERSION_MINOR}"
FL_ABI_VERSION="${FLTK_VERSION_MAJOR}.${FLTK_VERSION_MINOR}.0"

AC_SUBST(FLTK_VERSION)
AC_SUBST(FLTK_VERSION_MAJOR)
AC_SUBST(FLTK_VERSION_MINOR)
AC_SUBST(FLTK_VERSION_PATCH)
AC_SUBST(FL_DSO_VERSION)
AC_SUBST(FL_ABI_VERSION)


dnl Required file in package...
AC_CONFIG_SRCDIR([src/Fl.cxx])


AC_CANONICAL_HOST


dnl determine whether we're cross-compiling
AS_IF([test "$host" != "$build"], [
    fltk_cross_compiling="yes"
], [
    fltk_cross_compiling="no"
])


dnl Do not automatically add "-g" to compiler options...
dnl This must be _before_ "Find compiler commands..."
CFLAGS="${CFLAGS:=}"
CPPFLAGS="${CPPFLAGS:=}"
CXXFLAGS="${CXXFLAGS:=}"
DSOFLAGS="${DSOFLAGS:=}"
LDFLAGS="${LDFLAGS:=}"


dnl Find common commands...
AC_PROG_CC
AC_PROG_CXX
AC_PATH_TOOL(PKGCONFIG, pkg-config)


dnl Architecture and optimization options...
ARCHFLAGS="${ARCHFLAGS:=}"
AC_ARG_WITH([archflags], AS_HELP_STRING([--with-archflags="flags"], [use custom architecture flags (default=none, macOS values include "-arch arm64e", "-arch i386", "-arch ppc", and "-arch x86_64")]), [
    ARCHFLAGS="$withval"
])
AC_SUBST(ARCHFLAGS)

OPTIM="${OPTIM:=}"
AC_ARG_WITH(optim, AS_HELP_STRING([--with-optim="flags"], [use custom optimization flags]), [
    OPTIM="$withval"
])
AC_SUBST(OPTIM)


dnl Other options
AC_ARG_ENABLE([cairo], AS_HELP_STRING([--enable-cairo], [use Cairo library])) AC_ARG_ENABLE([cairoext], AS_HELP_STRING([--enable-cairoext], [use FLTK code instrumentation for cairo extended use]))

AC_ARG_ENABLE([cp936], AS_HELP_STRING([--enable-cp936], [turn on CP936]))
AS_IF([test x$enable_cp936 = xyes], [
    CFLAGS="$CFLAGS -DCP936"
])

AC_ARG_ENABLE([cygwin], AS_HELP_STRING([--enable-cygwin], [use the Cygwin libraries (default=no)]))

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [turn on debugging]))
AS_IF([test x$enable_debug = xyes], [
    DEBUGFLAG="-g "
], [
    DEBUGFLAG=""
])

AC_ARG_ENABLE([gl], AS_HELP_STRING([--disable-gl], [turn off OpenGL support]))

AC_ARG_ENABLE([localjpeg], AS_HELP_STRING([--enable-localjpeg], [use local JPEG library (default=auto)]))
AC_ARG_ENABLE([localpng], AS_HELP_STRING([--enable-localpng], [use local PNG library (efault=auto)]))
AC_ARG_ENABLE([localzlib], AS_HELP_STRING([--enable-localzlib], [use local ZLIB library (default=auto)]))

AC_ARG_ENABLE([pango], AS_HELP_STRING([--enable-pango], [turn on Pango support]))

AC_ARG_ENABLE([print], AS_HELP_STRING([--disable-print], [turn off print support (X11)]))
AS_IF([test x$enable_print = xno], [
    AC_DEFINE([FL_NO_PRINT_SUPPORT], [Disable X11 print support?])
])

AC_ARG_ENABLE([shared], AS_HELP_STRING([--enable-shared], [turn on shared libraries]))

AC_ARG_ENABLE([svg], AS_HELP_STRING([--disable-svg], [disable SVG support]))
AS_IF([test x$enable_svg != xno], [
    AC_DEFINE([FLTK_USE_SVG], [SVG image support])
])

AC_ARG_ENABLE([threads], AS_HELP_STRING([--disable-threads], [turn off multi-threading support]))

AC_ARG_ENABLE([x11], AS_HELP_STRING([--enable-x11], [use X11 with Cygwin or macOS (default=no)]))

AC_ARG_ENABLE([xcursor], AS_HELP_STRING([--disable-xcursor], [turn off Xcursor support]))

AC_ARG_ENABLE([xdbe], AS_HELP_STRING([--disable-xdbe], [turn off Xdbe support]))

AC_ARG_ENABLE([xfixes], AS_HELP_STRING([--disable-xfixes], [turn off Xfixes support]))

AC_ARG_ENABLE([xft], AS_HELP_STRING([--disable-xft], [turn off Xft support]))

AC_ARG_ENABLE([xinerama], AS_HELP_STRING([--disable-xinerama], [turn off Xinerama support]))

AC_ARG_ENABLE([xrender], AS_HELP_STRING([--disable-xrender], [turn off Xrender support]))

AS_IF([test x$enable_pango = xyes -a x$enable_xft = xno], [
    AC_MSG_ERROR([--disable-xft and --enable-pango are incompatible because Xft is necessary for Pango.])
])


dnl So --with-archflags option is used during "checking size of long"
# TODO: Fix long long test
AS_IF([test "x$with_archflags" != x], [
    CFLAGS="$CFLAGS $with_archflags"
])


dnl FLTK build options to be used in Makefiles (defined in makeinclude)
BUILD=""


dnl OS-specific pre-tests...
dnl host_os_gui equals $host_os unless we target Cygwin or Darwin in combination with X11.
host_os_gui=$host_os
AS_CASE([$host_os], [cygwin*], [
    # Handle Cygwin option *first*, before all other tests.
    AS_IF([test x$enable_cygwin = xyes], [
        # we target Cygwin in combination with X11
	AS_IF([test x$enable_x11 = xyes], [
	    host_os_gui="X11$host_os"
	])
    ])
], [darwin*], [
    AS_IF([test x$enable_x11 = xyes], [
	host_os_gui="X11"
	OPTIM="-U__APPLE__ $OPTIM"
	macosversion=$(sw_vers -productVersion | cut -d. -f2)
	macosversion_maj=$(sw_vers -productVersion | cut -d. -f1)
	AS_IF([test $macosversion_maj -ge 11 -o $macosversion -ge 13], [
	    CXXFLAGS="$CXXFLAGS -mmacosx-version-min=10.9 -D_LIBCPP_HAS_THREAD_API_PTHREAD"
	])
    ])
])


dnl Define the libraries and link options we will need.
LINKFLTK="../lib/libfltk.a"
LINKFLTKFORMS="../lib/libfltk_forms.a"
LINKFLTKGL="../lib/libfltk_gl.a"
LINKFLTKIMG="../lib/libfltk_images.a"
GLDEMOS="gldemos"

LIBEXT=".a"
LIBNAME="../lib/libfltk.a"
FLLIBNAME="../lib/libfltk_forms.a"
GLLIBNAME="../lib/libfltk_gl.a"
IMGLIBNAME="../lib/libfltk_images.a"
CAIROLIBNAME="../lib/libfltk_cairo.a"

LIBBASENAME="libfltk.a"
FLLIBBASENAME="libfltk_forms.a"
GLLIBBASENAME="libfltk_gl.a"
IMGLIBBASENAME="libfltk_images.a"
CAIROLIBBASENAME="libfltk_cairo.a"


dnl Check for Cairo library unless disabled...
CAIRODIR=""
CAIROFLAGS=""
LINKFLTKCAIRO=""
FLTKCAIROOPTION=""
CAIROLIBS=""

AS_IF([test x$enable_cairoext = xyes], [
    AS_IF([$PKGCONFIG --exists cairo], [
	AC_DEFINE([FLTK_USE_CAIRO])
	AC_DEFINE([FLTK_HAVE_CAIRO])
	CAIRODIR="cairo"
	CAIROFLAGS="$($PKGCONFIG --cflags cairo)"
	# TODO: Use pkg-config to get cairo libraries
	CAIROLIBS="-lcairo -lpixman-1"
	CXXFLAGS="$CAIROFLAGS $CXXFLAGS"
	LINKFLTKCAIRO="../lib/libfltk_cairo.a"
	FLTKCAIROOPTION="-L ../cairo -lfltk_cairo$SHAREDSUFFIX"
	LIBS="$CAIROLIBS $LIBS"
	LINKFLTK="$LINKFLTK $LINKFLTKCAIRO"
    ], [
        AC_MSG_ERROR([Cairo requested but not available.])
    ])
], [test x$enable_cairo = xyes], [
    AS_IF([$PKGCONFIG --exists cairo], [
	AC_DEFINE(FLTK_HAVE_CAIRO)
	CAIRODIR="cairo"
	CAIROFLAGS="$($PKGCONFIG --cflags cairo)"
	# TODO: Use pkg-config to get cairo libraries
	CAIROLIBS="-lcairo -lpixman-1"
	CXXFLAGS="$CAIROFLAGS $CXXFLAGS"
	LINKFLTKCAIRO="../lib/libfltk_cairo.a"
	FLTKCAIROOPTION="-L ../cairo -lfltk_cairo$SHAREDSUFFIX"
    ], [
        AC_MSG_ERROR([Cairo requested but not available.])
    ])
])

AC_SUBST(CAIRODIR)
AC_SUBST(CAIROFLAGS)
AC_SUBST(CAIROLIBS)
AC_SUBST(LINKFLTKCAIRO)
AC_SUBST(FLTKCAIROOPTION)

AC_SUBST(FLLIBNAME)
AC_SUBST(GLDEMOS)
AC_SUBST(GLLIBNAME)
AC_SUBST(IMGLIBNAME)
AC_SUBST(CAIROLIBNAME)
AC_SUBST(LIBEXT)
AC_SUBST(LIBNAME)
AC_SUBST(LINKFLTK)
AC_SUBST(LINKFLTKFORMS)
AC_SUBST(LINKFLTKGL)
AC_SUBST(LINKFLTKIMG)

AC_SUBST(LIBBASENAME)
AC_SUBST(FLLIBBASENAME)
AC_SUBST(GLLIBBASENAME)
AC_SUBST(IMGLIBBASENAME)
AC_SUBST(CAIROLIBBASENAME)

AC_ARG_WITH([abiversion], AS_HELP_STRING([--with-abiversion], [Build with FL_ABI_VERSION, e.g. 10304 for FLTK 1.3.4]))
has_abiversion="$with_abiversion"
AS_IF([test "x$has_abiversion" = xyes -o "x$has_abiversion" = xno], [
    has_abiversion=""
])
AS_IF([test "x$has_abiversion" != x], [
    AC_DEFINE_UNQUOTED([FL_ABI_VERSION], [$has_abiversion], [ABI version number])
])


dnl Handle compile-time options...
AS_IF([test "x$enable_shared" = xyes], [
    PICFLAG=1
    SHAREDSUFFIX=""
    FLUID="fluid-shared"

    AS_CASE([$host_os], [darwin*], [
	DSONAME="libfltk.$FL_DSO_VERSION.dylib"
	FLDSONAME="libfltk_forms.$FL_DSO_VERSION.dylib"
	GLDSONAME="libfltk_gl.$FL_DSO_VERSION.dylib"
	IMGDSONAME="libfltk_images.$FL_DSO_VERSION.dylib"
	CAIRODSONAME="libfltk_cairo.$FL_DSO_VERSION.dylib"
	DSOCOMMAND="\$(CXX) \$(ARCHFLAGS) \$(DSOFLAGS) -dynamiclib -lc -o"
    ], [solaris*], [
	DSONAME="libfltk.so.$FL_DSO_VERSION"
	FLDSONAME="libfltk_forms.so.$FL_DSO_VERSION"
	GLDSONAME="libfltk_gl.so.$FL_DSO_VERSION"
	IMGDSONAME="libfltk_images.so.$FL_DSO_VERSION"
	CAIRODSONAME="libfltk_cairo.so.$FL_DSO_VERSION"
	DSOCOMMAND="\$(CXX) \$(DSOFLAGS) -h \$@ \$(LDLIBS) -G $DEBUGFLAG -o"
	AS_IF([test "x$libdir" != "x/usr/lib"], [
	    DSOLINK="-R$libdir"
	])
    ], [hpux*], [
	DSONAME="libfltk.sl.$FL_DSO_VERSION"
	FLDSONAME="libfltk_forms.sl.$FL_DSO_VERSION"
	GLDSONAME="libfltk_gl.sl.$FL_DSO_VERSION"
	IMGDSONAME="libfltk_images.sl.$FL_DSO_VERSION"
	CAIRODSONAME="libfltk_cairo.sl.$FL_DSO_VERSION"
	DSOCOMMAND="ld \$(DSOFLAGS) -b -z +h \$@ $DEBUGFLAG -o"
	AS_IF([test "x$libdir" != "x/usr/lib"], [
	    DSOLINK="-Wl,-rpath,$libdir"
	])
    ], [irix*], [
	DSONAME="libfltk.so.$FL_DSO_VERSION"
	FLDSONAME="libfltk_forms.so.$FL_DSO_VERSION"
	GLDSONAME="libfltk_gl.so.$FL_DSO_VERSION"
	IMGDSONAME="libfltk_images.so.$FL_DSO_VERSION"
	CAIRODSONAME="libfltk_cairo.so.$FL_DSO_VERSION"
	DSOCOMMAND="\$(CXX) \$(DSOFLAGS) -Wl,-soname,\$@,-set_version,sgi1.1 \$(LDLIBS) -shared $DEBUGFLAG -o"
	AS_IF([test "x$libdir" != "x/usr/lib" -a "x$libdir" != "x/usr/lib32" -a "x$libdir" != "x/usr/lib64"], [
	    DSOLINK="-Wl,-rpath,$libdir"
	])
    ], [osf*], [
	DSONAME="libfltk.so.$FL_DSO_VERSION"
	FLDSONAME="libfltk_forms.so.$FL_DSO_VERSION"
	GLDSONAME="libfltk_gl.so.$FL_DSO_VERSION"
	IMGDSONAME="libfltk_images.so.$FL_DSO_VERSION"
	CAIRODSONAME="libfltk_cairo.so.$FL_DSO_VERSION"
	DSOCOMMAND="\$(CXX) \$(DSOFLAGS) -Wl,-soname,\$@ \$(LDLIBS) -shared $DEBUGFLAG -o"
	AS_IF([test "x$libdir" != "x/usr/lib" -a "x$libdir" != "x/usr/lib32"], [
	    DSOLINK="-Wl,-rpath,$libdir"
	])
    ], [linux* | *bsd*], [
	DSONAME="libfltk.so.$FL_DSO_VERSION"
	FLDSONAME="libfltk_forms.so.$FL_DSO_VERSION"
	GLDSONAME="libfltk_gl.so.$FL_DSO_VERSION"
	IMGDSONAME="libfltk_images.so.$FL_DSO_VERSION"
	CAIRODSONAME="libfltk_cairo.so.$FL_DSO_VERSION"
	DSOCOMMAND="\$(CXX) \$(DSOFLAGS) -Wl,-soname,\$@ \$(LDLIBS) -shared -fPIC $DEBUGFLAG -o"

	# See if the compiler supports -fvisibility...
	AC_CACHE_CHECK([if the compiler supports -fvisibility], ac_cv_cxx_fvisibility, [
	    OLDCXXFLAGS="$CXXFLAGS"
	    CXXFLAGS="$CXXFLAGS -fvisibility=hidden"
	    AC_LANG_PUSH([C++])
	    AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([[]], [[]])],
		[ac_cv_cxx_fvisibility=yes],
		[ac_cv_cxx_fvisibility=no])
	    CXXFLAGS="$OLDCXXFLAGS"
	    AC_LANG_POP([])
	])
	AS_IF([test x"$ac_cv_cxx_fvisibility" = xyes], [
	    OPTIM="$OPTIM -fvisibility=hidden"
	])

	# See if the compiler supports -fvisibility-inlines-hidden...
	AC_CACHE_CHECK([if the compiler supports -fvisibility-inlines-hidden], ac_cv_cxx_fvisibility_inlines, [
	    OLDCXXFLAGS="$CXXFLAGS"
	    CXXFLAGS="$CXXFLAGS -fvisibility-inlines-hidden"
	    AC_LANG_PUSH([C++])
	    AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([[]], [[]])],
		[ac_cv_cxx_fvisibility_inlines=yes],
		[ac_cv_cxx_fvisibility_inlines=no])
	    CXXFLAGS="$OLDCXXFLAGS"
	    AC_LANG_POP([])
	])
	AS_IF[test x"$ac_cv_cxx_fvisibility_inlines" = xyes], [
	    CXXFLAGS="$CXXFLAGS -fvisibility-inlines-hidden"
	])

	AS_IF([test "x$libdir" != "x/usr/lib" -a "x$libdir" != "x/usr/lib64"], [
	    DSOLINK="-Wl,-rpath,$libdir"
	])
    ], [aix*], [
	DSONAME="libfltk_s.a"
	FLDSONAME="libfltk_forms_s.a"
	GLDSONAME="libfltk_gl_s.a"
	IMGDSONAME="libfltk_images_s.a"
	CAIRODSONAME="libfltk_cairo_s.a"
	DSOCOMMAND="\$(CXX) \$(DSOFLAGS) -Wl,-bexpall,-bM:SRE,-bnoentry -o"
	SHAREDSUFFIX="_s"
    ], [cygwin* | mingw*], [
	PICFLAG=0
	AS_IF([test x$enable_cygwin != xyes], [
	    DSONAME="mgwfltknox-$FL_DSO_VERSION.dll"
	    FLDSONAME="mgwfltknox_forms-$FL_DSO_VERSION.dll"
	    GLDSONAME="mgwfltknox_gl-$FL_DSO_VERSION.dll"
	    IMGDSONAME="mgwfltknox_images-$FL_DSO_VERSION.dll"
	    CAIRODSONAME="mgwfltknox_cairo-$FL_DSO_VERSION.dll"
	], [test x$enable_x11 = xyes], [
	    DSONAME="cygfltk-$FL_DSO_VERSION.dll"
	    FLDSONAME="cygfltk_forms-$FL_DSO_VERSION.dll"
	    GLDSONAME="cygfltk_gl-$FL_DSO_VERSION.dll"
	    IMGDSONAME="cygfltk_images-$FL_DSO_VERSION.dll"
	    CAIRODSONAME="cygfltk_cairo-$FL_DSO_VERSION.dll"
	], [
	    DSONAME="cygfltknox-$FL_DSO_VERSION.dll"
	    FLDSONAME="cygfltknox_forms-$FL_DSO_VERSION.dll"
	    GLDSONAME="cygfltknox_gl-$FL_DSO_VERSION.dll"
	    IMGDSONAME="cygfltknox_images-$FL_DSO_VERSION.dll"
	    CAIRODSONAME="cygfltknox_cairo-$FL_DSO_VERSION.dll"
	])

	#-----------------------------------------------------------
	# -Wl,--enable-runtime-pseudo-reloc: See str 1585
	# appears to be necessary for older binutils versions < 2.16
	#-----------------------------------------------------------
	LDFLAGS="$LDFLAGS -Wl,--enable-auto-import -Wl,--enable-runtime-pseudo-reloc"
	DSOCOMMAND="\$(CXX) \$(DSOFLAGS) -shared -Wl,--whole-archive -Wl,--export-all-symbols  -Wl,--enable-runtime-pseudo-reloc -Wl,--enable-auto-import -Wl,--enable-auto-image-base -o \$@"
    ], [*], [
	AC_MSG_WARN([Shared libraries may not be supported. Trying -shared option with compiler.])
	DSONAME="libfltk.so.$FL_DSO_VERSION"
	FLDSONAME="libfltk_forms.so.$FL_DSO_VERSION"
	GLDSONAME="libfltk_gl.so.$FL_DSO_VERSION"
	IMGDSONAME="libfltk_images.so.$FL_DSO_VERSION"
	CAIRODSONAME="libfltk_cairo.so.$FL_DSO_VERSION"
	DSOCOMMAND="\$(CXX) \$(DSOFLAGS) -Wl,-soname,\$@ \$(LDLIBS) -shared $DEBUGFLAG -o"
    ])

    LINKSHARED="-L../src $FLTKCAIROOPTION -lfltk_images$SHAREDSUFFIX -lfltk_forms$SHAREDSUFFIX -lfltk$SHAREDSUFFIX"
], [
    DSOCOMMAND="echo"
    DSOLINK=""
    DSONAME=""
    FLDSONAME=""
    GLDSONAME=""
    IMGDSONAME=""
    CAIRODSONAME=""
    PICFLAG=0
    SHAREDSUFFIX=""
    FLUID="fluid"
    LINKSHARED="$LINKFLTKCAIRO ../lib/libfltk_images.a ../lib/libfltk_forms.a ../lib/libfltk.a"
])


dnl Define the fluid executable used when building the test programs.
dnl In a native build we use the fluid executable created during the build,
dnl in a cross-compilation we use `fluid' which must exist on the build system
dnl and must be in the PATH, i.e. executable as 'fluid' (w/o $EXEEXT).
AS_IF([test "x$fltk_cross_compiling" = xyes], [
    FLUID_BUILD="fluid"
]. [
    FLUID_BUILD="../fluid/fluid$EXEEXT"
])

AC_SUBST(DSOCOMMAND)
AC_SUBST(DSOFLAGS)
AC_SUBST(DSOLINK)
AC_SUBST(DSONAME)
AC_SUBST(FLDSONAME)
AC_SUBST(GLDSONAME)
AC_SUBST(IMGDSONAME)
AC_SUBST(CAIRODSONAME)
AC_SUBST(SHAREDSUFFIX)
AC_SUBST(LINKSHARED)
AC_SUBST(FLUID)
AC_SUBST(FLUID_BUILD)


dnl Find commands...
AC_PROG_INSTALL
AS_CASE([$host_os],
[osf*], [
    INSTALL="`pwd`/install-sh -c"
])

AS_IF([test "x$INSTALL" = "x$ac_install_sh"], [
    # Use full path to install-sh script...
    INSTALL="$(pwd)/install-sh -c"
])

AC_PATH_PROGS([NROFF], [nroff groff], [echo])
AC_PATH_PROG(DOXYDOC,doxygen)

dnl How do we make libraries?
AC_PROG_RANLIB
AC_PATH_TOOL(AR, ar)

AS_IF([test "x$AR" = "x:"], [
    AC_MSG_ERROR([Configure could not find the library archiver, aborting.])
])

AS_IF([test "x$RANLIB" != "x:"], [
    LIBCOMMAND="$AR cr"
], [
    LIBCOMMAND="$AR crs"
])

AC_SUBST(LIBCOMMAND)

dnl how to compile (Windows) resource files
dnl this will only be used to compile resources for Windows .exe files
AC_PATH_TOOL(RC,windres)

dnl Architecture checks...
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AS_IF([test $ac_cv_sizeof_short -eq 2], [
    AC_DEFINE([U16], [unsigned short])
])
AS_IF([test $ac_cv_sizeof_int -eq 4], [
    AC_DEFINE([U32], [unsigned])
], [test $ac_cv_sizeof_long -eq 4]. [
    AC_DEFINE([U32],[unsigned long])
])

AS_CASE([$host_os], [darwin*], [
], [*], [
    AC_C_BIGENDIAN

    AS_IF([test $ac_cv_sizeof_int -eq 8], [
	AC_DEFINE([U64], [unsigned])
    ], [test $ac_cv_sizeof_long -eq 8], [
	AC_DEFINE([U64], [unsigned long])
    ])
])

dnl Does the C++ compiler support the bool type?
AC_CACHE_CHECK([whether the compiler recognizes bool as a built-in type], ac_cv_cxx_bool,[
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM([[
	    int f(int  x){return 1;}
	    int f(char x){return 1;}
	    int f(bool x){return 1;}
	  ]], [[
	    bool b = true;
	    return f(b);
	]])],
	[ac_cv_cxx_bool=yes],
	[ac_cv_cxx_bool=no])
    AC_LANG_POP([])
])

AS_IF([test "x$ac_cv_cxx_bool" != xyes], [
    CXXFLAGS="-Dbool=char -Dfalse=0 -Dtrue=1 $CXXFLAGS"
])


dnl Standard headers and functions...
AC_HEADER_DIRENT
AC_CHECK_HEADERS([sys/select.h sys/stdtypes.h])

dnl Do we have the POSIX compatible scandir() prototype?
AC_CACHE_CHECK([whether we have the POSIX compatible scandir() prototype], ac_cv_cxx_scandir_posix,[
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM(
	    [[#include <dirent.h>
	      int func (const char *d, dirent ***list, void *sort) {
		int n = scandir(d, list, 0, (int(*)(const dirent **, const dirent **))sort);
		return n;
	      }
	    ]], [[
	    ]])],
	[ac_cv_cxx_scandir_posix=yes],
	[ac_cv_cxx_scandir_posix=no])
    AC_LANG_POP([])
])

dnl Define both HAVE_SCANDIR... macros, if the POSIX compatible function is
dnl available. Otherwise: check, whether any scandir prototype is available,
dnl but dont use it on SunOS and QNX because of an incompatibility in pre-Y2K
dnl SunOS scandir versions. We assume, though, that the POSIX compatible
dnl version on newer SunOS/Solaris versions works as expected.
AS_IF([test "x$ac_cv_cxx_scandir_posix" = xyes], [
    AC_DEFINE([HAVE_SCANDIR])
    AC_DEFINE([HAVE_SCANDIR_POSIX])
], [
    AC_CHECK_FUNC([scandir], [
        AS_CASE([$host_os], [solaris* | *qnx*], [
	    AC_MSG_WARN([Not using $host_os scandir emulation function.])
	], [*], [
	    AC_DEFINE([HAVE_SCANDIR])
	])
    ])
])

AC_CHECK_FUNC([vsnprintf], [
    AS_CASE([$host_os], [hpux1020], [
	AC_MSG_WARN([Not using built-in vsnprintf function because you are running HP-UX 10.20.])
    ], [osf4]. [
	AC_MSG_WARN([Not using built-in vsnprintf function because you are running Tru64 4.0.])
    ], [*], [
	AC_DEFINE([HAVE_VSNPRINTF])
    ])
])

AC_CHECK_FUNC([snprintf], [
    AS_CASE([$host_os], [hpux1020], [
	AC_MSG_WARN([Not using built-in snprintf function because you are running HP-UX 10.20.])
    ], [osf4], [
	AC_MSG_WARN([Not using built-in snprintf function because you are running Tru64 4.0.])
    ], [*], [
	AC_DEFINE([HAVE_SNPRINTF])
    ])
])

AC_CHECK_HEADERS([strings.h])
AC_CHECK_FUNCS([strcasecmp strlcat strlcpy])

AC_CHECK_HEADERS([locale.h])
AC_CHECK_FUNCS([localeconv])


dnl FLTK library uses math library functions...
AC_SEARCH_LIBS([pow], [m])


dnl Check for largefile support...
AC_SYS_LARGEFILE

LARGEFILE=""
AS_IF([test x$enable_largefile != xno], [
    LARGEFILE="-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE"

    AS_IF([test x$ac_cv_sys_large_files = x1], [
	LARGEFILE="$LARGEFILE -D_LARGE_FILES"
    ])

    AS_IF([test x$ac_cv_sys_file_offset_bits = x64], [
	LARGEFILE="$LARGEFILE -D_FILE_OFFSET_BITS=64"
    ])
])
AC_SUBST(LARGEFILE)


dnl Check for "long long" support...
AC_CACHE_CHECK(for long long int, ac_cv_c_long_long,
        [if test "$GCC" = yes; then
                ac_cv_c_long_long=yes
        else
                AC_COMPILE_IFELSE(
                    [AC_LANG_PROGRAM([[]],
                        [[long long int i;]])],
                    [ac_cv_c_long_long=yes],
                    [ac_cv_c_long_long=no])
        fi])

if test $ac_cv_c_long_long = yes; then
        AC_DEFINE(HAVE_LONG_LONG)
fi

dnl Check for dlopen/dlsym...
AC_SEARCH_LIBS(dlsym, dl, AC_DEFINE(HAVE_DLSYM))
AC_CHECK_HEADER(dlfcn.h, AC_DEFINE(HAVE_DLFCN_H))

dnl Check for audio libraries...
AUDIOLIBS=""

case $host_os in
    cygwin* | mingw*)
        dnl Cygwin environment...
        AUDIOLIBS="-lwinmm"
        ;;

    darwin*)
        AUDIOLIBS="-framework CoreAudio"
        ;;

    *)
        AC_CHECK_HEADER(alsa/asoundlib.h,
            AC_DEFINE(HAVE_ALSA_ASOUNDLIB_H)
            AUDIOLIBS="-lasound")
        ;;
esac

AC_SUBST(AUDIOLIBS)

dnl Check for image libraries...
SAVELIBS="$LIBS"
IMAGELIBS=""
STATICIMAGELIBS=""

AC_SUBST(IMAGELIBS)
AC_SUBST(STATICIMAGELIBS)

# Handle the ZLIB lib linking mode (use fltk local or system lib)
# If --enable-(resp. --disable-)localzlib parameter is not set by user
# Then we check the ZLIB lib usability, with result in syszlib_ok variable
# Check for System lib use if automatic mode or --disable-localzlib is requested
syszlib_ok=no
syszinc_ok=no
if test x$enable_localzlib != xyes; then
    AC_CHECK_LIB(z,gzgets,
        [AC_CHECK_HEADER(zlib.h,
                syszinc_ok=yes)
        if test x$syszinc_ok = xyes; then
            syszlib_ok=yes
        fi])
fi
# Now set the Z lib and include flags according to the requested mode and availability
if test x$enable_localzlib = xyes -o x$syszlib_ok = xno ; then
    ZLIBINC="-I../zlib"
    ZLIB="zlib"
    LIBS="-lfltk_z $LIBS"
    IMAGELIBS="-lfltk_z $IMAGELIBS"
    STATICIMAGELIBS="\$libdir/libfltk_z.a $STATICIMAGELIBS"
    AC_DEFINE(HAVE_LIBZ)
    ac_cv_lib_z_gzgets=no # fc: is still necessary ?
    # Finally, warn user if system lib was requested but not found
    if test x$enable_localzlib = xno; then
        AC_MSG_WARN([Cannot find system z lib or header: choosing the local lib mode.])
    fi
else
    ZLIBINC=""
    ZLIB=""
    LIBS="-lz $LIBS"
    IMAGELIBS="-lz $IMAGELIBS"
    STATICIMAGELIBS="-lz $STATICIMAGELIBS"
    AC_DEFINE(HAVE_LIBZ)
fi

# Handle the PNG lib linking mode (use fltk local or system lib)
# If --enable-(resp. --disable-)localpng parameter is not set by user
# Then we check the png lib usability with result in syspng_lib variable

# Now check if system lib is usable, we check Lib AND include availability with inc variant,
# but only, if the builtin lib is not requested
syspnglib_ok=no
syspnginc_ok=no
if test x$enable_localpng != xyes; then
  AC_CHECK_LIB(png, png_read_info,
    [AC_CHECK_HEADER(png.h,
        AC_DEFINE(HAVE_PNG_H)
        syspnginc_ok=yes)
    AC_CHECK_HEADER(libpng/png.h,
        AC_DEFINE(HAVE_LIBPNG_PNG_H)
        syspnginc_ok=yes)
    if test x$syspnginc_ok = xyes; then
        syspnglib_ok=yes
    fi])
fi

# The following is executed if the lib was not found usable or if local lib is required explicitly
if test x$enable_localpng = xyes -o x$syspnglib_ok = xno ; then
    PNGINC="-I../png"
    PNG="png"
    IMAGELIBS="-lfltk_png $IMAGELIBS"
    STATICIMAGELIBS="\$libdir/libfltk_png.a $STATICIMAGELIBS"
    AC_DEFINE(HAVE_LIBPNG)
    AC_DEFINE(HAVE_PNG_H)
    AC_DEFINE(HAVE_PNG_GET_VALID)
    AC_DEFINE(HAVE_PNG_SET_TRNS_TO_ALPHA)
    # Finally, warn user if system lib was requested but not found
    if test x$enable_localpng = xno; then
        AC_MSG_WARN([Cannot find system png lib or header: choosing the local lib mode.])
    fi
else
    PNGINC=""
    PNG=""
    IMAGELIBS="-lpng $IMAGELIBS"
    STATICIMAGELIBS="-lpng $STATICIMAGELIBS"
    AC_DEFINE(HAVE_LIBPNG)
    AC_CHECK_LIB(png,png_get_valid, AC_DEFINE(HAVE_PNG_GET_VALID))
    AC_CHECK_LIB(png,png_set_tRNS_to_alpha, AC_DEFINE(HAVE_PNG_SET_TRNS_TO_ALPHA))
fi

# Handle the JPEG lib linking mode (use fltk local or system lib)
# If --enable-(resp. --disable-)localjpeg parameter is not set by user
# Then we check the JPEG lib usability, with result in sysjpeglib_ok variable
# Check for System lib use if automatic mode or --disable-localjpeg is requested
sysjpeglib_ok=no
sysjpeginc_ok=no
if test x$enable_localjpeg != xyes; then
    AC_CHECK_LIB(jpeg,jpeg_CreateCompress,
        [AC_CHECK_HEADER(jpeglib.h,
            sysjpeginc_ok=yes)
            if test x$sysjpeginc_ok = xyes; then
                sysjpeglib_ok=yes
            fi])
fi
# Now set the jpeg lib and include flags according to the requested mode and availability
if test x$enable_localjpeg = xyes -o x$sysjpeglib_ok = xno; then
    JPEGINC="-I../jpeg"
    JPEG="jpeg"
    IMAGELIBS="-lfltk_jpeg $IMAGELIBS"
    STATICIMAGELIBS="\$libdir/libfltk_jpeg.a $STATICIMAGELIBS"
    AC_DEFINE(HAVE_LIBJPEG)
    # Finally, warn user if system lib was requested but not found
    if test x$enable_localjpeg = xno; then
        AC_MSG_WARN([Cannot find system jpeg lib or header: choosing the local lib mode.])
    fi
else
    JPEGINC=""
    JPEG=""
    IMAGELIBS="-ljpeg $IMAGELIBS"
    STATICIMAGELIBS="-ljpeg $STATICIMAGELIBS"
    AC_DEFINE(HAVE_LIBJPEG)
fi

AC_SUBST(JPEG)
AC_SUBST(JPEGINC)
AC_SUBST(PNG)
AC_SUBST(PNGINC)
AC_SUBST(ZLIB)
AC_SUBST(ZLIBINC)

# Control the usage of the nanosvg lib and SVG output

dnl Restore original LIBS settings...
LIBS="$SAVELIBS"

dnl See if we need a .exe extension on executables...
AC_EXEEXT

dnl Check for pthreads for multi-threaded apps...
have_pthread=no
PTHREAD_FLAGS=""

dnl Test whether we want to check for pthreads. We must not do it on Windows
dnl unless we run under Cygwin with --enable-cygwin, since we always use
dnl native threads on Windows (even if libpthread is available)
check_pthread=yes
case $host_os in
    mingw*)
        check_pthread=no
        ;;
    cygwin*)
        if test "x$enable_cygwin" != xyes; then
            check_pthread=no
        fi
        ;;
    *)
        ;;
esac

if test "x$enable_threads" != xno -a x$check_pthread = xyes; then
    AC_CHECK_HEADERS(pthread.h)

    if test x$ac_cv_header_pthread_h = xyes; then
        dnl Check various threading options for the platforms we support
        for flag in -lpthreads -lpthread -pthread; do
            AC_MSG_CHECKING([for pthread_create using $flag])
            SAVELIBS="$LIBS"
            LIBS="$flag $LIBS"
            AC_LINK_IFELSE(
                [AC_LANG_PROGRAM(
                    [[#include <pthread.h>]],
                    [[pthread_create(0, 0, 0, 0);]])],
                [have_pthread=yes],
                [LIBS="$SAVELIBS"])
            AC_MSG_RESULT([$have_pthread])

            if test $have_pthread = yes; then
                AC_DEFINE(HAVE_PTHREAD)
                PTHREAD_FLAGS="-D_THREAD_SAFE -D_REENTRANT"

                # Solaris requires -D_POSIX_PTHREAD_SEMANTICS to
                # be POSIX-compliant... :(
                case $host_os in
                    solaris*)
                        PTHREAD_FLAGS="$PTHREAD_FLAGS -D_POSIX_PTHREAD_SEMANTICS"
                        ;;
                esac
                break
            fi
        done
    fi
fi

AC_SUBST(PTHREAD_FLAGS)

dnl Define OS-specific stuff...
HLINKS=
OSX_ONLY=:
THREADS=

AC_ARG_WITH(links, [  --with-links            make header links for common misspellings [[default=no]]])

INSTALL_DESKTOP=""
UNINSTALL_DESKTOP=""

case $host_os_gui in
    cygwin* | mingw*)
        dnl Cygwin environment, using windows GDI ...
        # Recent versions of Cygwin are seriously broken and the size
        # checks don't work because the shell puts out \r\n instead of
        # \n.  Here we just force U32 to be defined to "unsigned"...
        AC_DEFINE(U32,unsigned)
        # We do no longer define WIN32 or _WIN32 (since FLTK 1.4.0)
        # and we don't need to define USE_OPENGL32 (added in svn r6657
        # but never used, see STR #2147)
        # CFLAGS="-mwindows -D_WIN32 -DUSE_OPENGL32 $CFLAGS"
        # CXXFLAGS="-mwindows -D_WIN32 -DUSE_OPENGL32 $CXXFLAGS"
        CFLAGS="-mwindows $CFLAGS"
        CXXFLAGS="-mwindows $CXXFLAGS"
        LDFLAGS="-mwindows $LDFLAGS"
        DSOFLAGS="-mwindows $DSOFLAGS"
        LIBS="$LIBS -lole32 -luuid -lcomctl32 -lws2_32"
        BUILD="WIN"
        if test "x$with_optim" = x; then
            dnl Avoid -Os optimization on Cygwin/MinGW
            with_optim="-O3"
        fi

        if test x$enable_gl != xno; then
            AC_CHECK_HEADER(GL/gl.h,
                AC_DEFINE(HAVE_GL)
                GLLIBS="-lopengl32")
            AC_CHECK_HEADER(GL/glu.h,
                AC_DEFINE(HAVE_GL_GLU_H)
                GLLIBS="-lglu32 $GLLIBS")
        else
            LINKFLTKGL=""
            GLLIBNAME=""
            GLDSONAME=""
            GLDEMOS=""
        fi

        if test "x$enable_threads" != xno; then
            if test x$have_pthread = xyes; then
                AC_DEFINE(HAVE_PTHREAD)
            fi
        fi

        THREADS="threads$EXEEXT"

        # Don't make symlinks since Windows is not case sensitive.
        if test "x$with_links" != xyes; then
                HLINKS="#"
        fi
        ;;

    darwin*)
        AC_DEFINE(__APPLE_QUARTZ__)

        BUILD="OSX"

        # MacOS X uses Cocoa for graphics.
        LIBS="$LIBS -framework Cocoa"

        if test x$have_pthread = xyes; then
            AC_DEFINE(HAVE_PTHREAD)
            THREADS="threads$EXEEXT"
        fi

        if test x$enable_gl != xno; then
            AC_DEFINE(HAVE_GL)
            AC_DEFINE(HAVE_GL_GLU_H)
            GLLIBS="-framework OpenGL"
        else
            LINKFLTKGL=""
            GLLIBNAME=""
            GLDSONAME=""
            GLDEMOS=""
        fi

        # Don't make symlinks because HFS+ is not case sensitive...
        if test "x$with_links" != xyes; then
                HLINKS="#"
        fi

        # Some steps are only done for OS X package management
        OSX_ONLY=

        # Install/Uninstall FLUID application
        INSTALL_DESKTOP="install-osx"
        UNINSTALL_DESKTOP="uninstall-osx"
        ;;

    *)
        # All others are UNIX/X11...
        # This includes Cygwin target combined with X11
        if test x$have_pthread = xyes; then
            AC_DEFINE(HAVE_PTHREAD)
            THREADS="threads$EXEEXT"
        fi

        dnl Check for X11...
        AC_PATH_XTRA

        if test x$no_x = xyes; then
            AC_MSG_ERROR([Configure could not find required X11 libraries, aborting.])
        fi

        if test "x$X_PRE_LIBS" != x; then
            AC_MSG_WARN([Ignoring libraries "$X_PRE_LIBS" requested by configure.])
        fi

        LIBS="$LIBS -lX11 $X_EXTRA_LIBS"
        CFLAGS="$CFLAGS $X_CFLAGS"
        CXXFLAGS="$CXXFLAGS $X_CFLAGS"
        LDFLAGS="$X_LIBS $LDFLAGS"
        DSOFLAGS="$X_LIBS $DSOFLAGS"
        AC_DEFINE(USE_X11)
        BUILD="X11"
        if test "x$x_includes" != x; then
            ac_cpp="$ac_cpp -I$x_includes"
        fi

        dnl Check for OpenGL unless disabled...
        GLLIBS=

        if test x$enable_gl != xno; then
            AC_SEARCH_LIBS(dlopen, dl)
            AC_CHECK_HEADER(GL/gl.h,
                AC_CHECK_LIB(GL, glXMakeCurrent,
                    [AC_DEFINE(HAVE_GL)
                     GLLIBS="-lGL"],
                    AC_CHECK_LIB(MesaGL, glXMakeCurrent,
                        [AC_DEFINE(HAVE_GL)
                         GLLIBS="-lMesaGL"],,
                        [-lm]),
                    [-lm])
                AC_CHECK_LIB(GL, glXGetProcAddressARB,
                    AC_DEFINE(HAVE_GLXGETPROCADDRESSARB),, [-lm])
            )
            AC_CHECK_HEADER(GL/glu.h,
                AC_DEFINE(HAVE_GL_GLU_H)
                if test x$ac_cv_lib_GL_glXMakeCurrent = xyes; then
                    GLLIBS="-lGLU $GLLIBS"
                fi
                if test x$ac_cv_lib_MesaGL_glXMakeCurrent = xyes; then
                    GLLIBS="-lMesaGLU $GLLIBS"
                fi
            )

            if test x$ac_cv_lib_GL_glXMakeCurrent != xyes -a x$ac_cv_lib_MesaGL_glXMakeCurrent != xyes; then
                LINKFLTKGL=""
                GLLIBNAME=""
                GLDSONAME=""
                GLDEMOS=""
            fi
        else
            LINKFLTKGL=""
            GLLIBNAME=""
            GLDSONAME=""
            GLDEMOS=""
        fi

        xinerama_found=no
        if test x$enable_xinerama != xno; then
            AC_CHECK_LIB(Xinerama, XineramaIsActive,
                [AC_DEFINE(HAVE_XINERAMA)
                 LIBS="-lXinerama $LIBS"
                 xinerama_found=yes])
        fi

        dnl Check for the Xft library unless disabled...
        xft_found=no
        if test x$enable_xft != xno; then
            # Try pkg-config first (freetype2 deprecated freetype-config from some version on)
            AC_PATH_PROG(PKGCONFIG, pkg-config)
            FT_FLAGS=""
            if test "x$PKGCONFIG" != x; then
                FT_FLAGS="`$PKGCONFIG --cflags xft`"
                if test "x$FT_FLAGS" = x; then
                    FT_FLAGS="`$PKGCONFIG --cflags freetype2`"
                fi
            fi
            # if that failed, try freetype-config
            if test "x$FT_FLAGS" = x; then
                AC_PATH_PROG(FTCONFIG, freetype-config)
                if test "x$FTCONFIG" != x; then
                    FT_FLAGS="`$FTCONFIG --cflags`"
                else
                    # abort if both methods failed
                    AC_MSG_NOTICE([please install pkg-config or use 'configure --disable-xft'.])
                    AC_MSG_ERROR([Aborting.])
                fi
            fi
            CPPFLAGS="$FT_FLAGS $CPPFLAGS"
            CXXFLAGS="$FT_FLAGS $CXXFLAGS"
            CFLAGS="$FT_FLAGS $CFLAGS"

            AC_CHECK_LIB(fontconfig, FcPatternCreate)
            AC_CHECK_HEADER(X11/Xft/Xft.h,
                    AC_CHECK_LIB(Xft, XftDrawCreate,
                        [AC_DEFINE(USE_XFT)
                         LIBS="-lXft $LIBS"
                         BUILD="XFT"
                         xft_found=yes]))
        fi

        dnl Issue a warning message if Xft was not found, abort configure
        dnl if Xft was requested explicitly (but not found)
        if test x$enable_xft != xno -a x$xft_found != xyes; then
            AC_MSG_WARN([could not find the required Xft headers and/or libraries.])
            AC_MSG_NOTICE([please install Xft headers and libraries or use 'configure --disable-xft'.])
            if test x$enable_xft = xyes; then
                    AC_MSG_ERROR([Aborting.])
            fi
        fi

  dnl test if pango is asked but xft was not found
  if test x$enable_pango = xyes -a x$xft_found = xno; then
      AC_MSG_WARN([could not find the Xft headers and/or libraries required for pango.])
      AC_MSG_NOTICE([please install Xft headers and libraries or don't use configure with '--enable-pango'.])
      AC_MSG_ERROR([Aborting.])
  fi

        dnl Check for the pango library unless disabled...

        pango_found=no
        if test x$enable_pango = xyes; then
          if test x$PKGCONFIG != x; then
            CXXFLAGS="`$PKGCONFIG --cflags pangoxft` $CXXFLAGS"
            LIBS="`$PKGCONFIG --libs pangoxft` $LIBS"
            CXXFLAGS="`$PKGCONFIG --cflags pangocairo` $CXXFLAGS"
            LIBS="`$PKGCONFIG --libs pangocairo` $LIBS"
            case $host_os in
              darwin*)
                dnl make sure LDFLAGS is appropriate for cairo (and pango)
                LDFLAGS="`$PKGCONFIG --libs cairo` $LDFLAGS"
              ;;
           esac
         else
            case $host_os in
                linux*)
                    CXXFLAGS="-I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include $CXXFLAGS"
                    CXXFLAGS="-I/usr/lib/i386-linux-gnu/glib-2.0/include -I/usr/lib/x86_64-linux-gnu/glib-2.0/include $CXXFLAGS"
                  ;;
                freebsd*)
                  CXXFLAGS="-I/usr/local/include/pango-1.0 -I/usr/local/include/glib-2.0 -I/usr/local/lib/glib-2.0/include $CXXFLAGS"
                  ;;
                netbsd*)
                  CXXFLAGS="-I/usr/pkg/include/pango-1.0 -I/usr/pkg/include/glib-2.0 -I/usr/pkg/lib/glib-2.0/include $CXXFLAGS"
                  CXXFLAGS="-I/usr/pkg/include/glib/glib-2.0 $CXXFLAGS"
                  PANGOLIBDIR="-L/usr/pkg/lib"
                  LDFLAGS="-Wl,-rpath,/usr/pkg/lib $LDFLAGS"
                  ;;
            esac
            LIBS="$PANGOLIBDIR -lpango-1.0 -lpangoxft-1.0 -lgobject-2.0 $LIBS"
          fi
                CPPFLAGS="$CXXFLAGS"
                AC_CHECK_HEADERS([pango/pango.h pango/pangoxft.h],
                   AC_CHECK_LIB(pango-1.0, pango_layout_new,
                    AC_CHECK_LIB(pangoxft-1.0, pango_xft_render_layout,
                        AC_DEFINE(USE_PANGO)
                        pango_found=yes)))
                CPPFLAGS=$CXXFLAGS
        fi

        dnl Check for the Xdbe extension unless disabled...

        xdbe_found=no
        if test x$enable_xdbe != xno; then
            AC_CHECK_HEADER(
                [X11/extensions/Xdbe.h],
                [AC_CHECK_LIB(Xext, XdbeQueryExtension,
                    [AC_DEFINE(HAVE_XDBE)
                     LIBS="-lXext $LIBS"
                     xdbe_found=yes])],
                [],
                [#include <X11/Xlib.h>])
        fi

        dnl Check for the Xfixes extension unless disabled...

        xfixes_found=no
        if test x$enable_xfixes != xno; then
            AC_CHECK_HEADER(
                [X11/extensions/Xfixes.h],
                [AC_CHECK_LIB(Xfixes, XFixesQueryExtension,
                    [AC_DEFINE(HAVE_XFIXES)
                     LIBS="-lXfixes $LIBS"
                     xfixes_found=yes])],
                [],
                [#include <X11/Xlib.h>])
        fi

        dnl Check for the Xcursor library unless disabled...

        xcursor_found=no
        if test x$enable_xcursor != xno; then
            AC_CHECK_HEADER(
                [X11/Xcursor/Xcursor.h],
                [AC_CHECK_LIB(Xcursor, XcursorImageCreate,
                    [AC_DEFINE(HAVE_XCURSOR)
                     LIBS="-lXcursor $LIBS"
                     xcursor_found=yes])],
                [],
                [#include <X11/Xlib.h>])
        fi

        dnl Check for the Xrender library unless disabled...

        xrender_found=no
        if test x$enable_xrender != xno; then
            AC_CHECK_HEADER(
                [X11/extensions/Xrender.h],
                [AC_CHECK_LIB(Xrender, XRenderQueryVersion,
                    [AC_DEFINE(HAVE_XRENDER)
                     LIBS="-lXrender $LIBS"
                     xrender_found=yes])],
                [],
                [#include <X11/Xlib.h>])
        fi

        dnl Check for the X11/Xregion.h header file...
        AC_CHECK_HEADER(X11/Xregion.h, AC_DEFINE(HAVE_X11_XREGION_H),,
            [#include <X11/Xlib.h>])

        dnl Check for overlay visuals...
        AC_PATH_PROG(XPROP, xprop)
        AC_CACHE_CHECK(for X overlay visuals, ac_cv_have_overlay,
            if test "x$XPROP" != x; then
                if $XPROP -root 2>/dev/null | grep -c "SERVER_OVERLAY_VISUALS" >/dev/null; then
                    ac_cv_have_overlay=yes
                else
                    ac_cv_have_overlay=no
                fi
            else
                ac_cv_have_overlay=no
            fi)

        if test x$ac_cv_have_overlay = xyes; then
            AC_DEFINE(HAVE_OVERLAY)
        fi

        # Make symlinks since UNIX/Linux is case sensitive,
        # but Cygwin in general not.
        case $host_os in
            cygwin*)
                HLINKS="#"
            ;;
            *)
            ;;
        esac
        # Make symlinks since UNIX/Linux is case sensitive,
        # but only if explicitly configured (default=no)
        if test "x$with_links" != xyes; then
                HLINKS="#"
        fi

        # Install/Uninstall FLUID application support files
        INSTALL_DESKTOP="install-linux"
        UNINSTALL_DESKTOP="uninstall-linux"
        ;;
esac

AC_SUBST(GLDEMOS)
AC_SUBST(GLLIBS)
AC_SUBST(HLINKS)
AC_SUBST(OSX_ONLY)
AC_SUBST(THREADS)

AC_SUBST(INSTALL_DESKTOP)
AC_SUBST(UNINSTALL_DESKTOP)

AC_SUBST(BUILD)

dnl Figure out the appropriate formatted man page extension...
case "$host_os" in
    *bsd* | darwin*)
        # *BSD
        CAT1EXT=0
        CAT3EXT=0
        CAT6EXT=0
        ;;
    irix*)
        # SGI IRIX
        CAT1EXT=z
        CAT3EXT=z
        CAT6EXT=z
        ;;
    *)
        # All others
        CAT1EXT=1
        CAT3EXT=3
        CAT6EXT=6
        ;;
esac

AC_SUBST(CAT1EXT)
AC_SUBST(CAT3EXT)
AC_SUBST(CAT6EXT)

dnl Fix "mandir" variable...
if test "$mandir" = "\${prefix}/man" -a "$prefix" = "/usr"; then
    case "$host_os" in
        *bsd* | darwin* | linux*)
            # *BSD, Darwin, and Linux
            mandir="\${prefix}/share/man"
            ;;
        irix*)
            # SGI IRIX
            mandir="\${prefix}/share/catman"
            ;;
    esac
fi

dnl Fix "libdir" variable...
if test "$prefix" = NONE; then
    prefix=/usr/local
fi

if test "$exec_prefix" = NONE; then
    exec_prefix="\${prefix}"
fi

case $host_os in
    irix[1-5]*)
        ;;
    irix*)
        if test "$libdir" = "\${exec_prefix}/lib" -a "$exec_prefix" = "\${prefix}" -a "$prefix" = "/usr"; then
            libdir="/usr/lib32"
        fi
        ;;
esac

dnl Define the command used to update the dependencies (this option
dnl mainly for FLTK core developers - not necessary for users)
MAKEDEPEND="\$(CXX) -M"
AC_SUBST(MAKEDEPEND)

dnl Add warnings to compiler switches:
dnl do this last so messing with switches does not break tests

if test -n "$GCC"; then
    # Show all standard warnings + unused variables, conversion errors,
    # and inlining problems when compiling...
    OPTIM="-Wall -Wunused -Wno-format-y2k $OPTIM"

    # The following additional warnings are useful for tracking down problems...
    #OPTIM="-Wshadow -Wconversion $OPTIM"

    # We know that Carbon is deprecated on OS X 10.4. To avoid hundreds of warnings
    # we will temporarily disable 'deprecated' warnings on OS X.
    case $host_os in
        darwin[1-7])
            ;;
        darwin*)
            OPTIM="-Wno-deprecated-declarations $OPTIM"
            ;;
    esac

    # Set the default compiler optimizations...
    if test -z "$DEBUGFLAG"; then
        #
        # Note: Can't use -fomit-frame-pointer - prevents tools like
        #       libsafe from working!
        #
        #       Don't use -fforce-mem, -fforce-addr, or -fcaller-saves.
        #       They all seem to make either no difference or enlarge
        #       the code by a few hundred bytes.
        #
        #       "-Os" seems to be the best compromise between speed and
        #       code size.  "-O3" and higher seem to make no effective
        #       difference in the speed of the code, but does bloat the
        #       library 10+%.
        #

        if test "x$with_optim" != x; then
            OPTIM="$with_optim $OPTIM"
        else
            OPTIM="-Os $OPTIM"
        fi
    fi

    # Generate position-independent code when needed...
    if test $PICFLAG = 1; then
        OPTIM="$OPTIM -fPIC"
    fi

    # See if GCC supports -fno-exceptions...
    AC_MSG_CHECKING([if GCC supports -fno-exceptions])
    OLDCFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -fno-exceptions"
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[]], [[]])],
        [OPTIM="$OPTIM -fno-exceptions"
         AC_MSG_RESULT(yes)],
        [AC_MSG_RESULT(no)])
    CFLAGS="$OLDCFLAGS"

    # See if GCC supports -fno-strict-aliasing...
    AC_MSG_CHECKING([if GCC supports -fno-strict-aliasing])
    OLDCFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -fno-strict-aliasing"
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[]], [[]])],
        [OPTIM="$OPTIM -fno-strict-aliasing"
         AC_MSG_RESULT(yes)],
        [AC_MSG_RESULT(no)])
    CFLAGS="$OLDCFLAGS"

dnl Make sure that shared libraries don't have undefined references
    # See if ld supports -no-undefined...
    AC_MSG_CHECKING([if ld supports -no-undefined])
    OLDLDFLAGS="$LDFLAGS"
    LDFLAGS="$LDFLAGS -Wl,-no-undefined"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[]], [[]])],
        [DSOFLAGS="$DSOFLAGS -Wl,-no-undefined"
         AC_MSG_RESULT(yes)],
        [AC_MSG_RESULT(no)])
    LDFLAGS="$OLDLDFLAGS"

    # See if ld supports -Bsymbolic-functions...
    AC_MSG_CHECKING([if ld supports -Bsymbolic-functions])
    OLDLDFLAGS="$LDFLAGS"
    LDFLAGS="$LDFLAGS -Wl,-Bsymbolic-functions"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[]], [[]])],
        [DSOFLAGS="$DSOFLAGS -Wl,-Bsymbolic-functions"
         AC_MSG_RESULT(yes)],
        [AC_MSG_RESULT(no)])
    LDFLAGS="$OLDLDFLAGS"

    # See if toolchain supports a sectioned build...
    AC_MSG_CHECKING([if toolchain supports sections])
    OLDLDFLAGS="$LDFLAGS"
    OLDCFLAGS="$CFLAGS"
    LDFLAGS="$LDFLAGS -Wl,-gc-sections"
    CFLAGS="$CFLAGS -ffunction-sections -fdata-sections"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[]], [[]])],
        [DSOFLAGS="$DSOFLAGS -Wl,-gc-sections"
         OPTIM="$OPTIM -ffunction-sections -fdata-sections"
         AC_MSG_RESULT(yes)],
        [AC_MSG_RESULT(no)])
    LDFLAGS="$OLDLDFLAGS"
    CFLAGS="$OLDCFLAGS"

    # See if we are running Solaris; if so, try the -fpermissive option...
    # This option is required on some versions of Solaris to work around
    # bugs in the X headers up through Solaris 7.
    #
    # Unlike the other compiler/optimization settings, this one is placed
    # in CFLAGS and CXXFLAGS so that fltk-config will provide the option
    # to clients - otherwise client apps will not compile properly...
    case $host_os in
        solaris*)
            AC_MSG_CHECKING([if GCC supports -fpermissive])

            OLDCFLAGS="$CFLAGS"
            CFLAGS="$CFLAGS -fpermissive"
            AC_COMPILE_IFELSE(
                [AC_LANG_PROGRAM([[]], [[]])],
                [CXXFLAGS="$CXXFLAGS -fpermissive"
                 AC_MSG_RESULT(yes)],
                [CFLAGS="$OLDCFLAGS"
                 AC_MSG_RESULT(no)])
            ;;
    esac
else
    case "$host_os" in
        irix*)
            # Running some flavor of IRIX; see which version and
            # set things up according...
            if test "$uversion" -ge 62; then
                # We are running IRIX 6.2 or higher; uncomment the following
                # lines if you don't have IDO 7.2 or higher:
                #
                #     CXX="CC -n32 -mips3"
                #     CC="cc -n32 -mips3"
                #     LD="ld -n32 -mips3"
                #     MAKEDEPEND="CC -M"

                if test "x`grep abi=n32 /etc/compiler.defaults`" = x; then
                        AC_MSG_WARN([FOR BEST RESULTS BEFORE COMPILING: setenv SGI_ABI "-n32 -mips3"])
                fi

                OPTIM="-fullwarn $OPTIM"
            fi
            if test -z "$DEBUGFLAG"; then
                if test "x$with_optim" != x; then
                    OPTIM="$with_optim $OPTIM"
                else
                    OPTIM="-O2 $OPTIM"
                    if test $uversion -gt 62; then
                        OPTIM="-OPT:Olimit=4000 $OPTIM"
                    fi
                fi
            fi
            ;;
        hpux*)
            # Running HP-UX; these options should work for the HP compilers.
            if test -z "$DEBUGFLAG"; then
                if test "x$with_optim" != x; then
                    OPTIM="$with_optim $OPTIM"
                else
                    OPTIM="+O2 $OPTIM"
                fi
            fi

            if test $PICFLAG = 1; then
                OPTIM="+z $OPTIM"
            fi

            CXXFLAGS="$CXXFLAGS +W336,501,736,740,749,829"
            ;;
        OSF1*)
            # Running Digital/Tru64 UNIX; these options should work for the
            # Digital/Compaq/NewHP compilers.
            if test -z "$DEBUGFLAG"; then
                if test "x$with_optim" != x; then
                    OPTIM="$with_optim $OPTIM"
                else
                    OPTIM="-O2 $OPTIM"
                fi
            fi
            ;;
        solaris*)
            # Solaris
            if test -z "$DEBUGFLAG"; then
                if test "x$with_optim" != x; then
                    OPTIM="$with_optim $OPTIM"
                else
                    OPTIM="-xO3 $OPTIM"
                fi
            fi

            if test $PICFLAG = 1; then
                OPTIM="-KPIC $OPTIM"
            fi
            ;;
        aix*)
            if test -z "$DEBUGFLAG"; then
                if test "x$with_optim" != x; then
                    OPTIM="$with_optim $OPTIM"
                else
                    OPTIM="-O2 $OPTIM"
                fi
            fi

            AC_MSG_WARN([The AIX C and C++ compilers are known not to correctly compile the FLTK library.])
            ;;
        *)
            # Running some other operating system; inform the user they
            # should contribute the necessary options via the STR form..
            AC_MSG_WARN([Building FLTK with default compiler optimizations])
            AC_MSG_WARN([Send the FLTK developers your uname and compiler options via https://www.fltk.org/bugs.php])
            ;;
    esac
fi

OPTIM="$DEBUGFLAG $OPTIM"

dnl Take archflags away from CFLAGS (makefiles use ARCHFLAGS explicitly)
case $host_os in
    darwin*)
        if test "x$with_archflags" != x ; then
            CFLAGS="`echo $CFLAGS | sed -e "s/$with_archflags//g"`"
        fi
        ;;
esac

dnl Define the FLTK documentation directory...
case $host_os in
  mingw*)
     # Determine the path where MSys has /usr installed
        msyspath=`mount | grep '\/usr ' | cut -d ' ' -f -1 | sed -e 's/\\\/\// g'`
     # Then substitute that in the Windows path instead of /usr
        AC_DEFINE_UNQUOTED(FLTK_DOCDIR, "$msyspath/local/share/doc/fltk")
    ;;
  *)
    if test x$prefix = xNONE; then
        AC_DEFINE_UNQUOTED(FLTK_DOCDIR, "/usr/local/share/doc/fltk")
    else
        AC_DEFINE_UNQUOTED(FLTK_DOCDIR, "$prefix/share/doc/fltk")
    fi
    ;;
esac

dnl Define the FLTK data directory...
if test x$prefix = xNONE; then
    AC_DEFINE_UNQUOTED(FLTK_DATADIR, "/usr/local/share/fltk")
else
    AC_DEFINE_UNQUOTED(FLTK_DATADIR, "$prefix/share/fltk")
fi

dnl Summarize results of configure tests...
echo ""
echo "Configuration Summary"
echo "-------------------------------------------------------------------------"

case $host_os_gui in
    cygwin* | mingw*)
        graphics="GDI"
        ;;
    darwin*)
        graphics="Quartz"
        ;;
    *)
        graphics="X11"
        if test x$xft_found = xyes; then
            graphics="$graphics + Xft"
        fi
        if test x$xdbe_found = xyes; then
            graphics="$graphics + Xdbe"
        fi
        if test x$xfixes_found = xyes; then
            graphics="$graphics + Xfixes"
        fi
        if test x$xinerama_found = xyes; then
            graphics="$graphics + Xinerama"
        fi
        if test x$xcursor_found = xyes; then
            graphics="$graphics + Xcursor"
        fi
        if test x$xrender_found = xyes; then
            graphics="$graphics + Xrender"
        fi
        if test x$pango_found = xyes; then
            graphics="$graphics + pango"
        fi
        ;;
esac

echo "    Directories: prefix=$prefix"
echo "                 bindir=$bindir"
echo "                 datadir=$datadir"
echo "                 datarootdir=$datarootdir"
echo "                 exec_prefix=$exec_prefix"
echo "                 includedir=$includedir"
echo "                 libdir=$libdir"
echo "                 mandir=$mandir"
case $host_os in
  mingw*)
    echo "                 MSys docpath=$msyspath/local/share/doc/fltk"
  ;;
esac
dnl
echo "Cross-compiling: $fltk_cross_compiling"
if test "$fltk_cross_compiling" = "yes"; then
  echo "          Build: $build -> Host: $host"
fi
dnl
echo "       Graphics: $graphics"
dnl
if test x$JPEG = x; then
    echo "Image Libraries: JPEG=System"
else
    echo "Image Libraries: JPEG=Builtin"
fi
if test x$PNG = x; then
    echo "                 PNG=System"
else
    echo "                 PNG=Builtin"
fi
if test x$ZLIB = x; then
    echo "                 ZLIB=System"
else
    echo "                 ZLIB=Builtin"
fi
if test x$enable_cairo = xyes; then
    echo "                 CAIRO=lib"
fi
if test x$enable_cairoext = xyes; then
    echo "                 CAIRO=internal_use"
fi

if test x$enable_largefile != xno; then
    echo "    Large Files: YES"
else
    echo "    Large Files: NO"
fi

if test x$GLDEMOS = x; then
    echo "         OpenGL: NO"
else
    echo "         OpenGL: YES"
fi

if test x$THREADS = x; then
    echo "        Threads: NO"
else
    echo "        Threads: YES"
fi

dnl Set empty BINARY_DIR variable for fltk-config.in (CMake compatibility)
BINARY_DIR=
AC_SUBST(BINARY_DIR)

dnl Write all of the files...
AC_CONFIG_HEADERS([config.h:configh.in])
AC_CONFIG_HEADERS([FL/abi-version.h:abi-version.in])
AC_CONFIG_FILES([makeinclude fltk.list fltk-config fltk.spec FL/Makefile])
AC_OUTPUT

dnl Make sure the fltk-config script is executable...
chmod +x fltk-config

dnl
dnl End of file.
dnl
