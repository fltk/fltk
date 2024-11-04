dnl -*- sh -*-
dnl the "configure" script is made from this by running GNU "autoconf"
dnl
dnl Configuration script for the Fast Light Tool Kit (FLTK).
dnl
dnl Copyright 1998-2024 by Bill Spitzak and others.
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
LIBS="${LDFLAGS:=}"


dnl Find common commands...
AC_PROG_CC
AC_PROG_CXX
AC_PATH_TOOL(PKGCONFIG, pkg-config)


dnl Architecture and optimization options...
ARCHFLAGS="${ARCHFLAGS:=}"
AC_ARG_WITH([archflags], AS_HELP_STRING([--with-archflags="flags"], [use custom architecture flags (default=none, macOS values include "-arch arm64", "-arch i386", "-arch ppc", and "-arch x86_64")]), [
    ARCHFLAGS="$withval"
])
AC_SUBST(ARCHFLAGS)

OPTIM="${OPTIM:=}"
AC_ARG_WITH(optim, AS_HELP_STRING([--with-optim="flags"], [use custom optimization flags]), [
    OPTIM="$withval"
])
AC_SUBST(OPTIM)


dnl Other options
AC_ARG_ENABLE([cairo], AS_HELP_STRING([--enable-cairo], [add support for Fl_Cairo_Window]))
AC_ARG_ENABLE([cairoext], AS_HELP_STRING([--enable-cairoext], [use FLTK code instrumentation for Cairo extended use]))

AC_ARG_ENABLE([cp936], AS_HELP_STRING([--enable-cp936], [turn on CP936]))
AS_IF([test x$enable_cp936 = xyes], [
    CFLAGS="$CFLAGS -DCP936"
])

AC_ARG_ENABLE([cygwin], AS_HELP_STRING([--enable-cygwin], [use the Cygwin DLL (default=no)]))

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [turn on debugging]))
AS_IF([test x$enable_debug = xyes], [
    DEBUGFLAG="-g "
], [
    DEBUGFLAG=""
])

AC_ARG_ENABLE([test], AS_HELP_STRING([--disable-test], [build test programs (default=yes)]))
AS_IF([test x$enable_test = xno], [
    TESTDIR=""
], [
    TESTDIR="test"
])
AC_SUBST(TESTDIR)

AC_ARG_ENABLE([forms], AS_HELP_STRING([--disable-forms], [build Forms compatibility library (default=yes)]))

AC_ARG_ENABLE([gl], AS_HELP_STRING([--disable-gl], [turn off OpenGL support]))

AC_ARG_ENABLE([localjpeg], AS_HELP_STRING([--enable-localjpeg], [use local JPEG library (default=auto)]))
AC_ARG_ENABLE([localpng], AS_HELP_STRING([--enable-localpng], [use local PNG library (default=auto)]))
AC_ARG_ENABLE([localzlib], AS_HELP_STRING([--enable-localzlib], [use local ZLIB library (default=auto)]))

AC_ARG_ENABLE([pango], AS_HELP_STRING([--enable-pango], [turn on Pango support]))

AC_ARG_ENABLE([wayland], AS_HELP_STRING([--disable-wayland], [turn off hybrid Wayland/X11 support]))

AC_ARG_ENABLE([usecairo], AS_HELP_STRING([--enable-usecairo], [all drawing to X11 windows uses Cairo]))

AC_ARG_ENABLE([use_std], AS_HELP_STRING([--enable-use_std], [allow FLTK to use std::string etc.]))

AC_ARG_ENABLE([print], AS_HELP_STRING([--disable-print], [turn off print support (X11)]))
AS_IF([test x$enable_print = xno], [
    AC_DEFINE([FL_NO_PRINT_SUPPORT], [Disable X11 print support?])
])

AC_ARG_ENABLE([shared], AS_HELP_STRING([--enable-shared], [turn on shared libraries]))

AC_ARG_ENABLE([svg], AS_HELP_STRING([--disable-svg], [disable SVG support]))
AS_IF([test x$enable_svg != xno], [
    AC_DEFINE([FLTK_USE_SVG], 1, [SVG image support])
])

AC_ARG_ENABLE([threads], AS_HELP_STRING([--disable-threads], [turn off multi-threading support]))

AC_ARG_ENABLE([x11], AS_HELP_STRING([--enable-x11], [use X11 with Cygwin or macOS (default=no)]))

AC_ARG_ENABLE([xcursor], AS_HELP_STRING([--disable-xcursor], [turn off Xcursor support]))

AC_ARG_ENABLE([xfixes], AS_HELP_STRING([--disable-xfixes], [turn off Xfixes support]))

AC_ARG_ENABLE([xft], AS_HELP_STRING([--disable-xft], [turn off Xft support]))

AC_ARG_ENABLE([xinerama], AS_HELP_STRING([--disable-xinerama], [turn off Xinerama support]))

AC_ARG_ENABLE([xrender], AS_HELP_STRING([--disable-xrender], [turn off Xrender support]))

AC_ARG_ENABLE([fluid], AS_HELP_STRING([--disable-fluid], [turn off fluid building]))

AS_CASE([$host_os], [cygwin* | mingw*], [
    AC_ARG_ENABLE([gdiplus], AS_HELP_STRING([--disable-gdiplus], [don't use GDI+ for antialiased graphics]))

    gdiplus_found=no
    AS_IF([test x$enable_gdiplus != xno], [
        AC_CHECK_HEADERS([wtypes.h gdiplus.h], [
            AC_DEFINE([USE_GDIPLUS])
            LIBS="-lgdiplus $LIBS"
            gdiplus_found=yes
        ], [],
    [[#include <wtypes.h>]])
    ])

])

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
        macosversion=$(sw_vers -productVersion | cut -d. -f2)
        macosversion_maj=$(sw_vers -productVersion | cut -d. -f1)
        AS_IF([test $macosversion_maj -ge 11 -o $macosversion -ge 13], [
            CXXFLAGS="$CXXFLAGS -mmacosx-version-min=10.9 -D_LIBCPP_HAS_THREAD_API_PTHREAD"
        ])
    ])
])


dnl Define the libraries and link options we will need.
LINKFLTK="../lib/libfltk.a"
LINKFLTKGL="../lib/libfltk_gl.a"
LINKFLTKIMG="../lib/libfltk_images.a"
GLDEMOS="gldemos"

LIBEXT=".a"
LIBNAME="../lib/libfltk.a"
GLLIBNAME="../lib/libfltk_gl.a"
IMGLIBNAME="../lib/libfltk_images.a"
CAIROLIBNAME="../lib/libfltk_cairo.a"

LIBBASENAME="libfltk.a"
GLLIBBASENAME="libfltk_gl.a"
IMGLIBBASENAME="libfltk_images.a"
CAIROLIBBASENAME="libfltk_cairo.a"

dnl set library names etc. for the optional forms library and set 'build_forms'
dnl to make conditional code independent of the default value of 'enable_forms'
dnl which is 'yes' (enabled) in 1.4.0 (default may be changed later)
AS_IF([test x$enable_forms != xno], [
    build_forms="yes"
    LINKFLTKFORMS="../lib/libfltk_forms.a"
    FLLIBNAME="../lib/libfltk_forms.a"
    FLLIBBASENAME="libfltk_forms.a"
    AC_DEFINE([FLTK_HAVE_FORMS])
], [
    build_forms="no"
    LINKFLTKFORMS=""
    FLLIBNAME=""
    FLLIBBASENAME=""
])


dnl Check for Cairo library unless disabled...
CAIRODIR=""
CAIROFLAGS=""
LINKFLTKCAIRO=""
FLTKCAIROOPTION=""
CAIROLIBS=""

AS_IF([test x$enable_cairoext = xyes], [
    AS_IF([$PKGCONFIG --exists cairo], [
        AC_DEFINE([FLTK_HAVE_CAIROEXT])
        AC_DEFINE([FLTK_HAVE_CAIRO])
        CAIRODIR="cairo"
        CAIROFLAGS="$($PKGCONFIG --cflags cairo)"
        CAIROLIBS="$($PKGCONFIG --libs cairo)"
        CXXFLAGS="$CAIROFLAGS $CXXFLAGS"
        LINKFLTKCAIRO="../lib/libfltk_cairo.a"
        FLTKCAIROOPTION="-L ../cairo -lfltk_cairo$SHAREDSUFFIX"
        LIBS="$CAIROLIBS $LIBS"
        LINKFLTK="$LINKFLTK $LINKFLTKCAIRO"
    ], [
        AC_MSG_ERROR([Cairo requested but not found.])
    ])
], [test x$enable_cairo = xyes], [
    AS_IF([$PKGCONFIG --exists cairo], [
        AC_DEFINE(FLTK_HAVE_CAIRO)
        CAIRODIR="cairo"
        CAIROFLAGS="$($PKGCONFIG --cflags cairo)"
        CAIROLIBS="$($PKGCONFIG --libs cairo)"
        CXXFLAGS="$CAIROFLAGS $CXXFLAGS"
        LINKFLTKCAIRO="../lib/libfltk_cairo.a"
        FLTKCAIROOPTION="-L ../cairo -lfltk_cairo$SHAREDSUFFIX"
    ], [
        AC_MSG_ERROR([Cairo requested but not found.])
    ])
])

AC_SUBST(CAIRODIR)
AC_SUBST(CAIROFLAGS)
AC_SUBST(CAIROLIBS)
AC_SUBST(LINKFLTKCAIRO)
AC_SUBST(FLTKCAIROOPTION)

AC_SUBST(GLDEMOS)
AC_SUBST(GLLIBNAME)
AC_SUBST(IMGLIBNAME)
AC_SUBST(CAIROLIBNAME)
AC_SUBST(LIBEXT)
AC_SUBST(LIBNAME)
AC_SUBST(LINKFLTK)
AC_SUBST(LINKFLTKGL)
AC_SUBST(LINKFLTKIMG)

AC_SUBST(LIBBASENAME)
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
    FLTK_OPTIONS="fltk-options-shared"

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
        DSOCOMMAND="\$(CXX) \$(DSOFLAGS) -Wl,-soname,\$@ \$(LDLIBS) -shared -fPIC $DEBUGFLAG -o"
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
    ], [linux*|*bsd*], [
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
        AC_CACHE_CHECK([if the compiler supports -fvisibility-inlines-hidden], [ac_cv_cxx_fvisibility_inlines], [
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
        AS_IF([test x"$ac_cv_cxx_fvisibility_inlines" = xyes], [
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

    LINKSHARED="-L../src $FLTKCAIROOPTION -lfltk_images$SHAREDSUFFIX -lfltk$SHAREDSUFFIX"
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
    FLTK_OPTIONS="fltk-options"
    LINKSHARED="$LINKFLTKCAIRO ../lib/libfltk_images.a ../lib/libfltk.a"
])

dnl reset FLDSONAME if the Forms compatibility library is disabled (not built):
dnl overwrite the variable because this is easier than adding conditional code above
AS_IF([test x$build_forms = xno], [
    FLDSONAME=""
])

AC_SUBST([FLLIBNAME])
AC_SUBST([FLLIBBASENAME])
AC_SUBST([FLDSONAME])
AC_SUBST([LINKFLTKFORMS])
AC_SUBST([LINKSHARED])


dnl Define the fluid executable used when building the test programs.
dnl In a native build we use the fluid executable created during the build,
dnl in a cross-compilation we use "fluid" which must exist on the build system
dnl and must be in the PATH, i.e. executable as 'fluid' (w/o $EXEEXT).
AS_IF([test "x$fltk_cross_compiling" = xyes], [
    FLUID_BUILD="fluid"
], [
    FLUID_BUILD="../fluid/fluid$EXEEXT"
])

AC_SUBST([DSOCOMMAND])
AC_SUBST([DSOFLAGS])
AC_SUBST([DSOLINK])
AC_SUBST([DSONAME])
AC_SUBST([GLDSONAME])
AC_SUBST([IMGDSONAME])
AC_SUBST([CAIRODSONAME])
AC_SUBST([SHAREDSUFFIX])
AC_SUBST([FLUID])
AC_SUBST([FLUID_BUILD])
AC_SUBST([FLTK_OPTIONS])


dnl Find commands...
AC_PROG_INSTALL
AS_CASE([$host_os],
[osf*], [
    INSTALL="$(pwd)/install-sh -c"
])

AS_IF([test "x$INSTALL" = "x$ac_install_sh"], [
    # Use full path to install-sh script...
    INSTALL="$(pwd)/install-sh -c"
])

AC_PATH_PROGS([NROFF], [nroff groff], [echo])
AC_PATH_PROG([DOXYDOC], [doxygen])

dnl How do we make libraries?
AC_PROG_RANLIB
AC_PATH_TOOL([AR], [ar])

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
], [test $ac_cv_sizeof_long -eq 4], [
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
    ], [osf4], [
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

dnl HP-UX 11.11 does not provide setenv()
AC_CHECK_FUNCS([setenv])


dnl FLTK library uses math library functions...
AC_SEARCH_LIBS([pow], [m])

AC_CHECK_HEADERS([math.h])
AC_CHECK_FUNCS([trunc])


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
AC_CACHE_CHECK([for long long int], [ac_cv_c_long_long], [
    AS_IF([test "$GCC" = yes], [
        ac_cv_c_long_long=yes
    ], [
        AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM([[]],
                [[long long int i;]])],
            [ac_cv_c_long_long=yes],
            [ac_cv_c_long_long=no])
    ])
])

AS_IF([test $ac_cv_c_long_long = yes], [
    AC_DEFINE([HAVE_LONG_LONG])
])


dnl Check for dlopen/dlsym...
AC_SEARCH_LIBS([dlsym], [dl], AC_DEFINE([HAVE_DLSYM]))
AC_CHECK_HEADER([dlfcn.h], AC_DEFINE([HAVE_DLFCN_H]))


dnl Check for audio libraries...
AUDIOLIBS=""

AS_CASE([$host_os], [cygwin* | mingw*], [
    dnl Cygwin environment...
    AUDIOLIBS="-lwinmm"
], [darwin*], [
    AUDIOLIBS="-framework CoreAudio"
], [*], [
    AC_CHECK_HEADER([alsa/asoundlib.h], [
        AC_DEFINE([HAVE_ALSA_ASOUNDLIB_H])
        AUDIOLIBS="-lasound"
    ])
])

AC_SUBST(AUDIOLIBS)


dnl Check for image libraries...
SAVELIBS="$LIBS"
IMAGELIBS=""
STATICIMAGELIBS=""

AC_SUBST([IMAGELIBS])
AC_SUBST([STATICIMAGELIBS])


# Handle the JPEG lib linking mode (use fltk local or system lib)
# If --enable-(resp. --disable-)localjpeg parameter is not set by user
# Then we check the JPEG lib usability, with result in sysjpeglib_ok variable
# Check for System lib use if automatic mode or --disable-localjpeg is requested
sysjpeglib_ok=no
sysjpeginc_ok=no
AS_IF([test x$enable_localjpeg != xyes], [
    AC_CHECK_LIB([jpeg], [jpeg_CreateCompress], [
        AC_CHECK_HEADER([jpeglib.h], [sysjpeginc_ok=yes])
        AS_IF([test x$sysjpeginc_ok = xyes], [
            sysjpeglib_ok=yes
        ])
    ])
])

# Now set the jpeg lib and include flags according to the requested mode and availability
AS_IF([test x$enable_localjpeg = xyes -o x$sysjpeglib_ok = xno], [
    JPEGINC="-I../jpeg"
    JPEG="jpeg"
    IMAGELIBS="-lfltk_jpeg $IMAGELIBS"
    STATICIMAGELIBS="\$libdir/libfltk_jpeg.a $STATICIMAGELIBS"
    AC_DEFINE([HAVE_LIBJPEG])
    # Finally, warn user if system lib was requested but not found
    AS_IF([test x$enable_localjpeg = xno], [
        AC_MSG_WARN([Cannot find system jpeg lib or header: choosing the local lib mode.])
    ])
], [
    JPEGINC=""
    JPEG=""
    IMAGELIBS="-ljpeg $IMAGELIBS"
    STATICIMAGELIBS="-ljpeg $STATICIMAGELIBS"
    AC_DEFINE([HAVE_LIBJPEG])
])


# Handle the ZLIB lib linking mode (use fltk local or system lib)
# If --enable-(resp. --disable-)localzlib parameter is not set by user
# Then we check the ZLIB lib usability, with result in syszlib_ok variable
# Check for System lib use if automatic mode or --disable-localzlib is requested
syszlib_ok=no
syszinc_ok=no
AS_IF([test x$enable_localzlib != xyes], [
    AC_CHECK_LIB([z], [gzgets], [
        AC_CHECK_HEADER([zlib.h], [syszinc_ok=yes])
        AS_IF([test x$syszinc_ok = xyes], [
            syszlib_ok=yes
        ])
    ])
])


# Handle the PNG lib linking mode (use fltk local or system lib)
# If --enable-(resp. --disable-)localpng parameter is not set by user
# Then we check the png lib usability with result in syspng_lib variable

# Now check if system lib is usable, we check Lib AND include availability with inc variant,
# but only, if the builtin lib is not requested
syspnglib_ok=no
syspnginc_ok=no
AS_IF([test x$enable_localpng != xyes -a x$PKGCONFIG != x], [
    AC_MSG_CHECKING([for libpng-1.6.x])
    AS_IF([$PKGCONFIG --exists libpng16], [
        AC_MSG_RESULT([yes])
        AC_DEFINE([HAVE_PNG_H], 1, [Have PNG library?])
        syspnginc_ok=yes
        syspnglib_ok=yes
        PNGINC="$($PKGCONFIG --cflags libpng16)"
        IMAGELIBS="$($PKGCONFIG --libs libpng16) $IMAGELIBS"
        STATICIMAGELIBS="$($PKGCONFIG --libs libpng16) $STATICIMAGELIBS"
    ], [
        AC_MSG_RESULT([no])
    ])
])

AS_IF([test x$enable_localpng != xyes -a $syspnglib_ok = no], [
    AC_CHECK_LIB([png], [png_read_info], [
        AC_CHECK_HEADER([png.h], [
            AC_DEFINE([HAVE_PNG_H])
            syspnginc_ok=yes
        ])
        AC_CHECK_HEADER([libpng/png.h], [
            syspnginc_ok=yes
        ])
        AS_IF([test x$syspnginc_ok = xyes], [
            syspnglib_ok=yes
        ])
    ])
])


# If we use the system zlib, we must also use the system png lib and vice versa.
# If either of them is not available, we fall back to using both local libraries
AS_IF([test x$syspnglib_ok = xyes -a x$syszlib_ok != xyes], [
    syspnglib_ok=no
    enable_localpng=yes
    AC_MSG_WARN([Local z lib selected: overriding png lib to local for compatibility.])
])
AS_IF([test x$syszlib_ok = xyes -a x$syspnglib_ok != xyes], [
    syszlib_ok=no
    enable_localzlib=yes
    AC_MSG_WARN([Local png lib selected: overriding z lib to local for compatibility.])
])


# Now set the Z lib and include flags according to the requested mode and availability
AS_IF([test x$enable_localzlib = xyes -o x$syszlib_ok = xno], [
    ZLIBINC="-I../zlib"
    ZLIB="zlib"
    LIBS="-lfltk_z $LIBS"
    IMAGELIBS="-lfltk_z $IMAGELIBS"
    STATICIMAGELIBS="\$libdir/libfltk_z.a $STATICIMAGELIBS"
    AC_DEFINE([HAVE_LIBZ])
    ac_cv_lib_z_gzgets=no # fc: is still necessary ?
    # Finally, warn user if system lib was requested but not found
    AS_IF([test x$enable_localzlib = xno], [
        AC_MSG_WARN([Cannot find system z lib or header: choosing the local lib mode.])
    ])
], [
    ZLIBINC=""
    ZLIB=""
    LIBS="-lz $LIBS"
    IMAGELIBS="-lz $IMAGELIBS"
    STATICIMAGELIBS="-lz $STATICIMAGELIBS"
    AC_DEFINE([HAVE_LIBZ])
])


# The following is executed if the png lib was not found usable or if local lib is required explicitly
AS_IF([test x$enable_localpng = xyes -o x$syspnglib_ok = xno], [
    PNGINC="-I../png"
    PNG="png"
    IMAGELIBS="-lfltk_png $IMAGELIBS"
    STATICIMAGELIBS="\$libdir/libfltk_png.a $STATICIMAGELIBS"
    AC_DEFINE([HAVE_LIBPNG])
    AC_DEFINE([HAVE_PNG_H])
    AC_DEFINE([HAVE_PNG_GET_VALID])
    AC_DEFINE([HAVE_PNG_SET_TRNS_TO_ALPHA])
    # Finally, warn user if system lib was requested but not found
    AS_IF([test x$enable_localpng = xno], [
        AC_MSG_WARN([Cannot find system png lib or header: choosing the local lib mode.])
    ])
], [
    PNGINC=""
    PNG=""
    IMAGELIBS="-lpng $IMAGELIBS"
    STATICIMAGELIBS="-lpng $STATICIMAGELIBS"
    AC_DEFINE([HAVE_LIBPNG])
    AC_CHECK_LIB([png], [png_get_valid], [
        AC_DEFINE([HAVE_PNG_GET_VALID])
    ])
    AC_CHECK_LIB([png], [png_set_tRNS_to_alpha], [
        AC_DEFINE([HAVE_PNG_SET_TRNS_TO_ALPHA])
    ])
])


AC_SUBST([JPEG])
AC_SUBST([JPEGINC])
AC_SUBST([PNG])
AC_SUBST([PNGINC])
AC_SUBST([HAVE_PNG_H])
AC_SUBST([ZLIB])
AC_SUBST([ZLIBINC])


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
AS_CASE([$host_os], [mingw*], [
    check_pthread=no
], [cygwin*], [
    AS_IF([test "x$enable_cygwin" != xyes], [
        check_pthread=no
    ])
])

AS_IF([test "x$enable_threads" != xno -a x$check_pthread = xyes], [
    AC_CHECK_HEADERS([pthread.h])

    AS_IF([test x$ac_cv_header_pthread_h = xyes], [
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

            AS_IF([test $have_pthread = yes], [
                AC_DEFINE([HAVE_PTHREAD])
                PTHREAD_FLAGS="-D_THREAD_SAFE -D_REENTRANT"

                # Solaris requires -D_POSIX_PTHREAD_SEMANTICS to
                # be POSIX-compliant... :(
                AS_CASE([$host_os], [solaris*], [
                    PTHREAD_FLAGS="$PTHREAD_FLAGS -D_POSIX_PTHREAD_SEMANTICS"
                ])
            ])
        done

        dnl Check if we have PTHREAD_MUTEX_RECURSIVE
        AC_CACHE_CHECK([whether we have PTHREAD_MUTEX_RECURSIVE], ac_cv_pthread_mutex_recursive,[
            AC_LANG_PUSH([C])
            AC_COMPILE_IFELSE(
                [AC_LANG_PROGRAM(
                    [[#include <pthread.h>
                    ]], [[
                        return PTHREAD_MUTEX_RECURSIVE;
                    ]])],
                [ac_cv_pthread_mutex_recursive=yes],
                [ac_cv_pthread_mutex_recursive=no])
            AC_LANG_POP([])
        ])
        AS_IF([test x$ac_cv_pthread_mutex_recursive = xyes], [
            AC_DEFINE([HAVE_PTHREAD_MUTEX_RECURSIVE])
        ])
    ])
])

AC_SUBST([PTHREAD_FLAGS])
AC_SUBST([HAVE_PTHREAD_MUTEX_RECURSIVE])


dnl Define OS-specific stuff...
HLINKS=
OSX_ONLY=:
THREADS=
LIBDECORDIR=""
LIBDECORDBUS=""
FLUIDDIR=""

AC_ARG_WITH([links], AS_HELP_STRING([--with-links], [make header links for common misspellings (default=no)]))

INSTALL_DESKTOP=""
UNINSTALL_DESKTOP=""

AS_IF([test x$enable_fluid != xno], [FLUIDDIR="fluid"])

dnl Option use_std - allow std::string and maybe more
AS_IF([test x$enable_use_std = xyes], [
    AC_DEFINE([FLTK_USE_STD])
    ]
)

dnl Platform specific Processing

AS_CASE([$host_os_gui], [cygwin* | mingw*], [
    dnl Cygwin environment, using windows GDI ...
    # Recent versions of Cygwin are seriously broken and the size
    # checks don't work because the shell puts out \r\n instead of
    # \n.  Here we just force U32 to be defined to "unsigned"...
    AC_DEFINE([U32], [unsigned])
    # We do no longer define WIN32 or _WIN32 (since FLTK 1.4.0)
    # and we don't need to define USE_OPENGL32 (added in svn r6657
    # but never used, see STR #2147)
    # CFLAGS="-mwindows -D_WIN32 -DUSE_OPENGL32 $CFLAGS"
    # CXXFLAGS="-mwindows -D_WIN32 -DUSE_OPENGL32 $CXXFLAGS"
    LDFLAGS="-mwindows $LDFLAGS"
    DSOFLAGS="-mwindows $DSOFLAGS"
    LIBS="$LIBS -lole32 -luuid -lcomctl32 -lws2_32 -lwinspool"

    BUILD="WIN"
    AS_IF([test "x$with_optim" = x], [
        dnl Avoid -Os optimization on Cygwin/MinGW
        with_optim="-O3"
    ])

    AS_IF([test x$enable_gl != xno], [
        AC_CHECK_HEADER([GL/gl.h], [
            AC_DEFINE([HAVE_GL])
            GLLIBS="-lopengl32"
        ])
        AC_CHECK_HEADER([GL/glu.h], [
            AC_DEFINE([HAVE_GL_GLU_H])
            GLLIBS="-lglu32 $GLLIBS"
        ])
    ], [
        LINKFLTKGL=""
        GLLIBNAME=""
        GLDSONAME=""
        GLDEMOS=""
    ])

    AS_IF([test "x$enable_threads" != xno -a x$have_pthread = xyes], [
        AC_DEFINE([HAVE_PTHREAD])
    ])

    THREADS="threads$EXEEXT"

    # Don't make symlinks since Windows is not case sensitive.
    AS_IF([test "x$with_links" != xyes], [
        HLINKS="#"
    ])
], [darwin*], [
    BUILD="OSX"

    # MacOS X uses Cocoa for graphics.
    LIBS="$LIBS -framework Cocoa"
    # Add weak-linked additional frameworks for increasingly high macOS versions
    AC_LANG_PUSH([C])
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM(
            [[#include <AvailabilityMacros.h>
              #if __MAC_OS_X_VERSION_MAX_ALLOWED < 110000
              #error __MAC_OS_X_VERSION_MAX_ALLOWED < 110000
              #endif
            ]], [[
            ]])],
        [LIBS="$LIBS -weak_framework UniformTypeIdentifiers"],
        [])
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM(
            [[#include <AvailabilityMacros.h>
              #if __MAC_OS_X_VERSION_MAX_ALLOWED < 150000
              #error __MAC_OS_X_VERSION_MAX_ALLOWED < 150000
              #endif
            ]], [[
            ]])],
        [LIBS="$LIBS -weak_framework ScreenCaptureKit"],
        [])
    AC_LANG_POP([])

    AS_IF([test x$have_pthread = xyes], [
        AC_DEFINE([HAVE_PTHREAD])
        THREADS="threads$EXEEXT"
    ])

    AS_IF([test x$enable_gl != xno], [
        AC_DEFINE([HAVE_GL])
        AC_DEFINE([HAVE_GL_GLU_H])
        GLLIBS="-framework OpenGL"
    ], [
        LINKFLTKGL=""
        GLLIBNAME=""
        GLDSONAME=""
        GLDEMOS=""
    ])

    # Don't make symlinks because HFS+ is not case sensitive...
    AS_IF([test "x$with_links" != xyes], [
        HLINKS="#"
    ])

    # Some steps are only done for OS X package management
    OSX_ONLY=

    # Install/Uninstall FLUID application
    INSTALL_DESKTOP="install-osx"
    UNINSTALL_DESKTOP="uninstall-osx"
], [*], [
    # All others are UNIX/X11...
    # This includes Cygwin target combined with X11
    AS_IF([test x$have_pthread = xyes], [
        AC_DEFINE([HAVE_PTHREAD])
        THREADS="threads$EXEEXT"
    ])

    AS_IF([test x$enable_wayland != xno], [
      AS_IF([test x$PKGCONFIG = x], [
        dnl pkg-config is not available, issue warning and possibly abort...
        AS_IF([test x$enable_wayland = xyes], [
          AC_MSG_WARN([--enable-wayland: please install pkg-config.])
          AC_MSG_ERROR([Aborting.])
        ],[
          AC_MSG_WARN([pkg-config is not found. Continuing with --disable-wayland])
          enable_wayland="no"
        ])
      ],[
        missing="no"
        AS_IF([$PKGCONFIG --exists 'wayland-client >= 1.18'],[],[missing="yes"])
        AS_IF([$PKGCONFIG --exists 'wayland-protocols >= 1.15'],[],[missing="yes"])
        AS_IF([$PKGCONFIG --exists wayland-cursor],[],[missing="yes"])
        AS_IF([$PKGCONFIG --exists xkbcommon],[],[missing="yes"])
        AS_IF([$PKGCONFIG --exists pangocairo],[],[missing="yes"])
        AS_IF([test x$missing = xyes], [
            AC_MSG_WARN([These packages 'wayland-client>=1.18 wayland-protocols>=1.15 wayland-cursor xkbcommon  pangocairo' are required to build FLTK for wayland.])
            AC_MSG_WARN([At least one of them is missing.])
          AS_IF([test x$enable_wayland = xyes], [
            AC_MSG_ERROR([Building for Wayland is not possible. Aborting.])
          ],[
            AC_MSG_WARN([Continuing with --disable-wayland])
            enable_wayland="no"
          ])
        ])
      ])
    ])

    AS_IF([test x$enable_wayland != xno], [
      dnl Prepare for Wayland...
      dnl Turn option usecairo ON
      AC_DEFINE([FLTK_USE_CAIRO])
      AC_MSG_NOTICE([Turning on the usecairo option])

      BUILD="WAYLAND"
      AC_DEFINE([FLTK_USE_WAYLAND])
      graphics="Wayland"
      AS_IF([test x$enable_x11 != xno], [
       AC_DEFINE([FLTK_USE_X11]) # to build a hybrid Wayland/X11 library
       BUILD="WAYLANDX11"
       graphics="Wayland or X11 with cairo"
       ])
      AS_IF([$PKGCONFIG --exists 'libdecor-0 >= 0.2.0'],
      [
        plugin_dir="$($PKGCONFIG --variable=libdir libdecor-0)/libdecor/plugins-1"
        CFLAGS="$CFLAGS -DUSE_SYSTEM_LIBDECOR"
        CXXFLAGS="$CXXFLAGS -DUSE_SYSTEM_LIBDECOR"
        CFLAGS="$CFLAGS -DLIBDECOR_PLUGIN_DIR=\\\"$plugin_dir\\\" "
        LIBS="$LIBS $($PKGCONFIG --libs libdecor-0)"
      ],
      [
        CFLAGS="$CFLAGS -DUSE_SYSTEM_LIBDECOR=0 -DLIBDECOR_PLUGIN_DIR=\\\"\\\" "
        CXXFLAGS="$CXXFLAGS -DUSE_SYSTEM_LIBDECOR=0"
      ]
      )
      LIBS="$LIBS $($PKGCONFIG --libs wayland-cursor) $($PKGCONFIG --libs wayland-client) $($PKGCONFIG --libs xkbcommon) $($PKGCONFIG --libs pangocairo) "
      AS_IF([test x$enable_x11 != xno], [LIBS="$LIBS $($PKGCONFIG --libs x11)"] )
      LIBS="$LIBS -ldl"
      DSOFLAGS="$LIBS $DSOFLAGS"
      enable_pango=yes
      LIBDECORDIR="libdecor/build"
      AS_IF([$PKGCONFIG --exists dbus-1],
       [LIBDECORDBUS="-DHAS_DBUS $($PKGCONFIG --cflags dbus-1)" LIBS="$LIBS $($PKGCONFIG --libs dbus-1)"]
      )
      LDFLAGS="$LDFLAGS -rdynamic"

      AC_SEARCH_LIBS([dlopen], [dl])

      AS_IF([test x$enable_gl != xno], [
          AS_IF([$PKGCONFIG --exists gl], [
            AS_IF([$PKGCONFIG --exists egl], [
              AS_IF([$PKGCONFIG --exists wayland-egl], [
                AC_DEFINE([HAVE_GL])
                GLLIBS="$($PKGCONFIG --libs wayland-egl) $($PKGCONFIG --libs egl) $($PKGCONFIG --libs gl) $GLLIBS"
          ])])])
          AS_IF([$PKGCONFIG --exists glu], [
             AC_DEFINE([HAVE_GL_GLU_H])
             GLLIBS="$($PKGCONFIG --libs glu) $GLLIBS"
          ])
          AC_CHECK_LIB([GL], [glXGetProcAddressARB], [AC_DEFINE([HAVE_GLXGETPROCADDRESSARB])
             ],, [-lm])
      ], [
          LINKFLTKGL=""
          GLLIBNAME=""
          GLDSONAME=""
          GLDEMOS=""
      ])

      dnl Check for GTK-3 ...
      gtk_found=no
      CFLAGS="$($PKGCONFIG --cflags gtk+-3.0) $CFLAGS"
      AC_CHECK_HEADERS([gtk/gtk.h], [
          CFLAGS="$CFLAGS -DHAVE_GTK"
          LIBS="$LIBS $($PKGCONFIG --libs gtk+-3.0)"
          gtk_found=yes
      ])

      dnl Check for the Pango library ...
      pango_found=no
      CFLAGS="$($PKGCONFIG --cflags pangocairo) $CFLAGS"
      CXXFLAGS="$($PKGCONFIG --cflags pangocairo) $CXXFLAGS"
      LIBS="$LIBS $($PKGCONFIG --libs pangocairo)"

      AC_CHECK_HEADERS([pango/pangocairo.h], [
          AC_DEFINE([USE_PANGO])
          AC_DEFINE([USE_XFT])
          pango_found=yes
      ])

      dnl Early abort if Pango could not be found
      AS_IF([test x$pango_found != xyes], [
          AC_MSG_NOTICE([--enable-wayland: Pango libs and/or headers could not be found.])
          AC_MSG_ERROR([Aborting.])
      ])

      dnl check for Xinerama, Xcursor, Xfixes, Xrender, Xregion.h
      AS_IF([test x$enable_x11 != xno], [
        xinerama_found=no
        AS_IF([$PKGCONFIG --exists xinerama], [
          AC_DEFINE(HAVE_XINERAMA)
          LIBS="$LIBS $($PKGCONFIG --libs xinerama)"
          xinerama_found=yes
        ])

        xcursor_found=no
        AS_IF([$PKGCONFIG --exists xcursor], [
          AC_DEFINE(HAVE_XCURSOR)
          LIBS="$LIBS $($PKGCONFIG --libs xcursor)"
          xcursor_found=yes
        ])

        xfixes_found=no
        AS_IF([$PKGCONFIG --exists xfixes], [
          AC_DEFINE(HAVE_XFIXES)
          LIBS="$LIBS $($PKGCONFIG --libs xfixes)"
          xfixes_found=yes
        ])

        xrender_found=no
        AS_IF([$PKGCONFIG --exists xrender], [
          AC_DEFINE(HAVE_XRENDER)
          LIBS="$LIBS $($PKGCONFIG --libs xrender)"
          xrender_found=yes
        ])

        AC_CHECK_HEADER([X11/Xregion.h], [
          AC_DEFINE([HAVE_X11_XREGION_H])
        ], [], [#include <X11/Xlib.h>])
      ])

    ], [

    dnl Check for X11...
    AC_PATH_XTRA

    AS_IF([test x$no_x = xyes], [
        AC_MSG_ERROR([Configure could not find required X11 libraries, aborting.])
    ])

    AS_IF([test "x$X_PRE_LIBS" != x], [
        AC_MSG_WARN([Ignoring libraries "$X_PRE_LIBS" requested by configure.])
    ])

    LIBS="$LIBS -lX11 $X_EXTRA_LIBS"
    CFLAGS="$CFLAGS $X_CFLAGS"
    CXXFLAGS="$CXXFLAGS $X_CFLAGS"
    AS_CASE([$host_os], [darwin*], [
      DARWIN_LDFLAGS="$LDFLAGS"
    ])
    LDFLAGS="$X_LIBS $LDFLAGS"
    DSOFLAGS="$X_LIBS $DSOFLAGS"
    AC_DEFINE([FLTK_USE_X11])
    BUILD="X11"
    AS_IF([test "x$x_includes" != x], [
        ac_cpp="$ac_cpp -I$x_includes"
    ])

    dnl Check for OpenGL unless disabled...
    GLLIBS=

    AS_IF([test x$enable_gl != xno], [
        AC_SEARCH_LIBS([dlopen], [dl])
        AC_CHECK_HEADER([GL/gl.h], [
            AC_CHECK_LIB([GL], [glXMakeCurrent], [
                AC_DEFINE([HAVE_GL])
                GLLIBS="-lGL"
            ], [
                AC_CHECK_LIB([MesaGL], [glXMakeCurrent], [
                    AC_DEFINE([HAVE_GL])
                    GLLIBS="-lMesaGL"
                ],, [-lm]),
            ], [-lm])
            AC_CHECK_LIB([GL], [glXGetProcAddressARB], [
                AC_DEFINE([HAVE_GLXGETPROCADDRESSARB])
            ],, [-lm])
        ])
        AC_CHECK_HEADER([GL/glu.h], [
            AC_DEFINE([HAVE_GL_GLU_H])
            AS_IF([test x$ac_cv_lib_GL_glXMakeCurrent = xyes], [
                GLLIBS="-lGLU $GLLIBS"
            ])
            AS_IF([test x$ac_cv_lib_MesaGL_glXMakeCurrent = xyes], [
                GLLIBS="-lMesaGLU $GLLIBS"
            ])
        ])

        AS_IF([test x$ac_cv_lib_GL_glXMakeCurrent != xyes -a x$ac_cv_lib_MesaGL_glXMakeCurrent != xyes], [
            LINKFLTKGL=""
            GLLIBNAME=""
            GLDSONAME=""
            GLDEMOS=""
        ])
    ], [
        LINKFLTKGL=""
        GLLIBNAME=""
        GLDSONAME=""
        GLDEMOS=""
    ])

    xinerama_found=no
    AS_IF([test x$enable_xinerama != xno], [
        AC_CHECK_LIB([Xinerama], [XineramaIsActive], [
            AC_DEFINE([HAVE_XINERAMA])
            LIBS="-lXinerama $LIBS"
            xinerama_found=yes
        ])
    ])

    dnl Check for the Xft library unless disabled...
    xft_found=no
    AS_IF([test x$enable_xft != xno -a x$enable_wayland = xno], [
        # Try pkg-config first (freetype2 deprecated freetype-config from some version on)
        FT_FLAGS=""
        AS_IF([test "x$PKGCONFIG" != x], [
            FT_FLAGS="$($PKGCONFIG --cflags xft)"
            AS_IF([test "x$FT_FLAGS" = x], [
                FT_FLAGS="$($PKGCONFIG --cflags freetype2)"
            ])
        ])

        # if that failed, try freetype-config
        AS_IF([test "x$FT_FLAGS" = x], [
            AC_PATH_PROG([FTCONFIG], [freetype-config])
            AS_IF([test "x$FTCONFIG" != x], [
                FT_FLAGS="$($FTCONFIG --cflags)"
            ], [
                # abort if both methods failed
                AC_MSG_NOTICE([please install pkg-config or use 'configure --disable-xft'.])
                AC_MSG_ERROR([Aborting.])
            ])
        ])
        CPPFLAGS="$FT_FLAGS $CPPFLAGS"
        CXXFLAGS="$FT_FLAGS $CXXFLAGS"
        CFLAGS="$FT_FLAGS $CFLAGS"

        AC_CHECK_LIB([fontconfig], [FcPatternCreate])
        AC_CHECK_HEADER([X11/Xft/Xft.h], [
            AC_CHECK_LIB([Xft], [XftDrawCreate], [
                AC_DEFINE([USE_XFT])
                LIBS="-lXft $LIBS"
                BUILD="XFT"
                xft_found=yes
            ])
        ])
    ])

    dnl Issue a warning message if Xft was not found, abort configure
    dnl if Xft was requested explicitly (but not found)
    AS_IF([test x$enable_xft != xno -a x$xft_found != xyes], [
        AC_MSG_WARN([could not find the required Xft headers and/or libraries.])
        AC_MSG_NOTICE([please install Xft headers and libraries or use 'configure --disable-xft'.])
        AS_IF([test x$enable_xft = xyes], [
            AC_MSG_ERROR([Aborting.])
        ])
    ])

    dnl Option usecairo
    AS_IF([test x$enable_usecairo = xyes], [
      enable_pango=yes
      BUILD="CAIRO"
      AC_DEFINE([FLTK_USE_CAIRO])
      AC_MSG_NOTICE([Processing usecairo option])
      ]
    )

    dnl test if Pango is asked but xft was not found
    AS_IF([test x$enable_pango = xyes -a x$xft_found = xno], [
        AC_MSG_WARN([could not find the Xft headers and/or libraries required for Pango.])
        AC_MSG_NOTICE([please install Xft headers and libraries or don't use configure with '--enable-pango'.])
        AC_MSG_ERROR([Aborting.])
    ])

    dnl Check for the Pango library unless disabled...
    pango_found=no
    AS_IF([test x$enable_pango = xyes], [
        AS_IF([test x$PKGCONFIG != x], [
            dnl pkg-config is available, use it...
            dnl AC_MSG_NOTICE([--enable-pango: using pkg-config ...])
            CXXFLAGS="$($PKGCONFIG --cflags pangocairo) $CXXFLAGS"
            AS_CASE([$host_os], [darwin*], [], [*], [
              CXXFLAGS="$($PKGCONFIG --cflags pangoxft) $CXXFLAGS"
            ])
            AS_CASE([$host_os], [darwin*], [
              DARWIN_SAVE_LIBS=$LIBS
              DARWIN_PANGO_LIBS="$($PKGCONFIG --libs pangocairo)"
              LIBS="$($PKGCONFIG --libs pangocairo) $LIBS"
            ], [*], [
              LIBS="$($PKGCONFIG --libs pangocairo --libs pangoxft) $LIBS"
            ])
        ], [
            dnl pkg-config is not available, issue warning and continue...
            AC_MSG_WARN([--enable-pango: please install pkg-config. Continuing anyway.])
        ])

        CPPFLAGS="$CXXFLAGS"
        AS_CASE([$host_os], [darwin*],
          [AC_CHECK_HEADERS([pango/pango.h], [
            AC_CHECK_LIB([pango-1.0], [pango_layout_new], [
              AC_DEFINE([USE_PANGO])
              pango_found=yes
              LIBS=$DARWIN_SAVE_LIBS
              ])
          ])
        ],[*], [
          AC_CHECK_HEADERS([pango/pango.h pango/pangoxft.h], [
            AC_CHECK_LIB([pango-1.0], [pango_layout_new], [
                AC_CHECK_LIB([pangoxft-1.0], [pango_xft_render_layout], [
                    AC_DEFINE([USE_PANGO])
                    pango_found=yes
                ])
            ])
          ])
        ])

        dnl Early abort if --enable-pango was requested but Pango could not be found
        AS_IF([test x$pango_found != xyes], [
            AC_MSG_NOTICE([--enable-pango: Pango libs and/or headers could not be found.])
            AC_MSG_ERROR([Aborting.])
        ])
    ])

    dnl Check for the Xfixes extension unless disabled...
    xfixes_found=no
    AS_IF([test x$enable_xfixes != xno], [
        AC_CHECK_HEADER([X11/extensions/Xfixes.h], [
            AC_CHECK_LIB([Xfixes], [XFixesQueryExtension], [
                AC_DEFINE([HAVE_XFIXES])
                LIBS="-lXfixes $LIBS"
                xfixes_found=yes
            ])
        ], [], [#include <X11/Xlib.h>])
    ])

    dnl Check for the Xcursor library unless disabled...
    xcursor_found=no
    AS_IF([test x$enable_xcursor != xno], [
        AC_CHECK_HEADER([X11/Xcursor/Xcursor.h], [
            AC_CHECK_LIB([Xcursor], [XcursorImageCreate], [
                AC_DEFINE([HAVE_XCURSOR])
                LIBS="-lXcursor $LIBS"
                xcursor_found=yes
            ])
        ], [], [#include <X11/Xlib.h>])
    ])

    dnl Check for the Xrender library unless disabled...
    xrender_found=no
    AS_IF([test x$enable_xrender != xno], [
        AC_CHECK_HEADER([X11/extensions/Xrender.h], [
            AC_CHECK_LIB([Xrender], [XRenderQueryVersion], [
                AC_DEFINE([HAVE_XRENDER])
                LIBS="-lXrender $LIBS"
                xrender_found=yes
            ])
        ], [], [#include <X11/Xlib.h>])
    ])

    AS_CASE([$host_os], [darwin*], [
      AS_IF([test x$pango_found = xyes], [
        #place X_LIBS after homebrew's pango libs
        LIBS="$DARWIN_PANGO_LIBS $X_LIBS $LIBS"
        #remove X_LIBS from LDFLAGS while keeping its initial content
        LDFLAGS="$DARWIN_LDFLAGS"
      ])
    ])

    dnl Check for the X11/Xregion.h header file...
    AC_CHECK_HEADER([X11/Xregion.h], [
        AC_DEFINE([HAVE_X11_XREGION_H])
    ], [], [#include <X11/Xlib.h>])

    ])

    # Make symlinks since UNIX/Linux is case sensitive,
    # but Cygwin in general not.
    AS_CASE([$host_os], [cygwin*], [
        HLINKS="#"
    ])

    # Make symlinks since UNIX/Linux is case sensitive,
    # but only if explicitly configured (default=no)
    AS_IF([test "x$with_links" != xyes], [
        HLINKS="#"
    ])

    # Install/Uninstall FLUID application support files
    INSTALL_DESKTOP="install-linux"
    UNINSTALL_DESKTOP="uninstall-linux"
])

dnl End of platform specific Processing

AC_SUBST([GLDEMOS])
AC_SUBST([GLLIBS])
AC_SUBST([HLINKS])
AC_SUBST([OSX_ONLY])
AC_SUBST([THREADS])

AC_SUBST([FLUIDDIR])

AC_SUBST([LIBDECORDIR])
AC_SUBST([LIBDECORDBUS])

AC_SUBST([INSTALL_DESKTOP])
AC_SUBST([UNINSTALL_DESKTOP])

AC_SUBST([BUILD])

dnl Figure out the appropriate formatted man page extension...
AS_CASE(["$host_os"], [*bsd* | darwin*], [
    # *BSD
    CAT1EXT=0
    CAT3EXT=0
    CAT6EXT=0
], [irix*], [
    # SGI IRIX
    CAT1EXT=z
    CAT3EXT=z
    CAT6EXT=z
], [*], [
    # All others
    CAT1EXT=1
    CAT3EXT=3
    CAT6EXT=6
])

AC_SUBST([CAT1EXT])
AC_SUBST([CAT3EXT])
AC_SUBST([CAT6EXT])

dnl Fix "mandir" variable...
AS_IF([test "$mandir" = "\${prefix}/man" -a "$prefix" = "/usr"], [
    AS_CASE(["$host_os"], [*bsd* | darwin* | linux*], [
        # *BSD, Darwin, and Linux
        mandir="\${prefix}/share/man"
    ], [irix*], [
        # SGI IRIX
        mandir="\${prefix}/share/catman"
    ])
])

dnl Fix "libdir" variable...
AS_IF([test "$prefix" = NONE], [
    prefix=/usr/local
])

AS_IF([test "$exec_prefix" = NONE], [
    exec_prefix="\${prefix}"
])

AS_CASE([$host_os], [irix[1-5]*], [
], [irix*], [
    AS_IF([test "$libdir" = "\${exec_prefix}/lib" -a "$exec_prefix" = "\${prefix}" -a "$prefix" = "/usr"], [
        libdir="/usr/lib32"
    ])
])

dnl Define the command used to update the dependencies (this option
dnl mainly for FLTK core developers - not necessary for users)
MAKEDEPEND="\$(CXX) -M"
AC_SUBST([MAKEDEPEND])

dnl Add warnings to compiler switches:
dnl do this last so messing with switches does not break tests

AS_IF([test -n "$GCC"], [
    # Show all standard warnings + unused variables, conversion errors,
    # and inlining problems when compiling...
    OPTIM="-Wall -Wunused -Wno-format-y2k $OPTIM"

    # The following additional warnings are useful for tracking down problems...
    #OPTIM="-Wshadow -Wconversion $OPTIM"

    # We know that Carbon is deprecated on OS X 10.4. To avoid hundreds of warnings
    # we will temporarily disable 'deprecated' warnings on OS X.
    AS_CASE([$host_os], [darwin[1-7]], [
    ], [darwin*], [
        OPTIM="-Wno-deprecated-declarations $OPTIM"
    ])

    # Set the default compiler optimizations...
    AS_IF([test -z "$DEBUGFLAG"], [
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

        AS_IF([test "x$with_optim" != x], [
            OPTIM="$with_optim $OPTIM"
        ], [
            OPTIM="-Os $OPTIM"
        ])
    ])

    # Generate position-independent code when needed...
    AS_IF([test $PICFLAG = 1], [
        OPTIM="$OPTIM -fPIC"
    ])

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
    AS_CASE([$host_os], [solaris*], [
        AC_MSG_CHECKING([if GCC supports -fpermissive])

        OLDCFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS -fpermissive"
        AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM([[]], [[]])],
            [CXXFLAGS="$CXXFLAGS -fpermissive"
             AC_MSG_RESULT(yes)],
            [CFLAGS="$OLDCFLAGS"
             AC_MSG_RESULT(no)])
    ])
], [
    AS_CASE(["$host_os"], [irix*], [
        # Running some flavor of IRIX; see which version and
        # set things up according...
        AS_IF([test "$uversion" -ge 62], [
            # We are running IRIX 6.2 or higher; uncomment the following
            # lines if you don't have IDO 7.2 or higher:
            #
            #     CXX="CC -n32 -mips3"
            #     CC="cc -n32 -mips3"
            #     LD="ld -n32 -mips3"
            #     MAKEDEPEND="CC -M"

            AS_IF([test "x`grep abi=n32 /etc/compiler.defaults`" = x], [
                AC_MSG_WARN([FOR BEST RESULTS BEFORE COMPILING: setenv SGI_ABI "-n32 -mips3"])
            ])

            OPTIM="-fullwarn $OPTIM"
        ])

        AS_IF([test -z "$DEBUGFLAG"], [
            AS_IF([test "x$with_optim" != x], [
                OPTIM="$with_optim $OPTIM"
            ], [
                OPTIM="-O2 $OPTIM"
                AS_IF([test $uversion -gt 62], [
                    OPTIM="-OPT:Olimit=4000 $OPTIM"
                ])
            ])
        ])
    ], [hpux*], [
        # Running HP-UX; these options should work for the HP compilers.
        AS_IF([test -z "$DEBUGFLAG"], [
            AS_IF([test "x$with_optim" != x], [
                OPTIM="$with_optim $OPTIM"
            ], [
                OPTIM="+O2 $OPTIM"
            ])
        ])

        AS_IF([test $PICFLAG = 1], [
            OPTIM="+z $OPTIM"
        ])

        CXXFLAGS="$CXXFLAGS +W336,501,736,740,749,829"
    ], [OSF1*], [
        # Running Digital/Tru64 UNIX; these options should work for the
        # Digital/Compaq/NewHP compilers.
        AS_IF([test -z "$DEBUGFLAG"], [
            AS_IF([test "x$with_optim" != x], [
                OPTIM="$with_optim $OPTIM"
            ], [
                OPTIM="-O2 $OPTIM"
            ])
        ])
    ], [solaris*], [
        # Solaris
        AS_IF([test -z "$DEBUGFLAG"], [
            AS_IF([test "x$with_optim" != x], [
                OPTIM="$with_optim $OPTIM"
            ], [
                OPTIM="-xO3 $OPTIM"
            ])
        ])

        AS_IF([test $PICFLAG = 1], [
            OPTIM="-KPIC $OPTIM"
        ])
    ], [aix*], [
        AS_IF([test -z "$DEBUGFLAG"], [
            AS_IF([test "x$with_optim" != x], [
                OPTIM="$with_optim $OPTIM"
            ], [
                OPTIM="-O2 $OPTIM"
            ])
        ])

        AC_MSG_WARN([The AIX C and C++ compilers are known not to correctly compile the FLTK library.])
    ], [*], [
        # Running some other operating system; inform the user they
        # should contribute the necessary options via the STR form..
        AC_MSG_WARN([Building FLTK with default compiler optimizations])
        AC_MSG_WARN([Send the FLTK developers your uname and compiler options via https://www.fltk.org/bugs.php])
    ])
])

OPTIM="$DEBUGFLAG $OPTIM"

dnl Take archflags away from CFLAGS (makefiles use ARCHFLAGS explicitly)
AS_CASE([$host_os], [darwin*], [
    AS_IF([test "x$with_archflags" != x], [
        CFLAGS="$(echo $CFLAGS | sed -e 's/$with_archflags//g')"
    ])
])

dnl Define the FLTK documentation directory...
AS_CASE([$host_os], [mingw*], [
    # Determine the path where MSys has /usr installed
    msyspath="$(mount | grep '/usr ' | cut -d ' ' -f -1 | sed -e's,\\,/, g')"

    # Then substitute that in the Windows path instead of /usr
    AC_DEFINE_UNQUOTED([FLTK_DOCDIR], "$msyspath/local/share/doc/fltk")
], [*], [
    AS_IF([test x$prefix = xNONE], [
        AC_DEFINE_UNQUOTED([FLTK_DOCDIR], "/usr/local/share/doc/fltk")
    ], [
        AC_DEFINE_UNQUOTED([FLTK_DOCDIR], "$prefix/share/doc/fltk")
    ])
])

dnl Define the FLTK data directory...
AS_IF([test x$prefix = xNONE], [
    AC_DEFINE_UNQUOTED([FLTK_DATADIR], "/usr/local/share/fltk")
], [
    AC_DEFINE_UNQUOTED([FLTK_DATADIR], "$prefix/share/fltk")
])


dnl Summarize results of configure tests...
echo ""
echo "Configuration Summary"
echo "-------------------------------------------------------------------------"

AS_CASE([$host_os_gui], [cygwin* | mingw*], [
    AS_IF([test x$gdiplus_found = xyes], [graphics="GDI+"], [graphics="GDI"])
], [darwin*], [
    graphics="Quartz"
], [*], [
    AS_IF([test x$enable_wayland = xno], [
        graphics="X11"
    ])
    AS_IF([test x$xft_found = xyes], [
        graphics="$graphics + Xft"
    ])
    AS_IF([test x$xfixes_found = xyes], [
        graphics="$graphics + Xfixes"
    ])
    AS_IF([test x$xinerama_found = xyes], [
        graphics="$graphics + Xinerama"
    ])
    AS_IF([test x$xcursor_found = xyes], [
        graphics="$graphics + Xcursor"
    ])
    AS_IF([test x$xrender_found = xyes], [
        graphics="$graphics + Xrender"
    ])
    AS_IF([test x$enable_usecairo = xyes], [
        graphics="$graphics + Cairo"
    ])
    AS_IF([test x$pango_found = xyes], [
        graphics="$graphics + Pango"
    ])
])

echo "    Directories: prefix=$prefix"
echo "                 bindir=$bindir"
echo "                 datadir=$datadir"
echo "                 datarootdir=$datarootdir"
echo "                 exec_prefix=$exec_prefix"
echo "                 includedir=$includedir"
echo "                 libdir=$libdir"
echo "                 mandir=$mandir"
AS_CASE([$host_os], [mingw*], [
    echo "                 MSys docpath=$msyspath/local/share/doc/fltk"
])

AS_IF([test x$enable_fluid != xno], [
    echo "    Build fluid: YES"
    ],[
    echo "    Build fluid: NO"
    ])

AS_IF([test x$enable_test != xno], [
    echo "    Build tests: YES"
    ],[
    echo "    Build tests: NO"
    ])

AS_IF([test "$fltk_cross_compiling" = "yes"], [
    echo "Cross-compiling: YES"
    echo "          Build: $build -> Host: $host"
], [
    echo "Cross-compiling: NO"
])

echo "       Graphics: $graphics"
dnl
AS_IF([test x$JPEG = x], [
    echo "Image Libraries: JPEG=System"
], [
    echo "Image Libraries: JPEG=Builtin"
])
AS_IF([test x$PNG = x], [
    echo "                 PNG=System"
], [
    echo "                 PNG=Builtin"
])
AS_IF([test x$ZLIB = x], [
    echo "                 ZLIB=System"
], [
    echo "                 ZLIB=Builtin"
])

AS_IF([test x$build_forms = xyes], [
    echo "Forms library:   YES"
], [
    echo "Forms library:   NO"
])

AS_IF([test x$enable_cairo = xyes], [
    echo "                 CAIRO=lib"
])
AS_IF([test x$enable_cairoext = xyes], [
    echo "                 CAIRO=internal_use"
])

AS_IF([test x$enable_largefile != xno], [
    echo "    Large Files: YES"
], [
    echo "    Large Files: NO"
])

AS_IF([test x$GLDEMOS = x], [
    echo "         OpenGL: NO"
], [
    echo "         OpenGL: YES"
])

AS_IF([test x$THREADS = x], [
    echo "        Threads: NO"
], [
    echo "        Threads: YES"
])

AS_IF([test x$enable_use_std != xyes], [
    echo "   Allow std:: : NO"
], [
    echo "   Allow std:: : YES"
])

dnl Set empty BINARY_DIR variable for fltk-config.in (CMake compatibility)
BINARY_DIR=
AC_SUBST([BINARY_DIR])

dnl Write all of the files...
AC_CONFIG_HEADERS([config.h:configh.in])
AC_CONFIG_HEADERS([FL/fl_config.h:fl_config.in])
AC_CONFIG_FILES([makeinclude fltk.list fltk-config fltk.spec FL/Makefile])
AC_OUTPUT

dnl Make sure the fltk-config script is executable...
chmod +x fltk-config

echo ""
echo "-------------------------------------------------------------------------"
echo "  IMPORTANT NOTE:"
echo "-------------------------------------------------------------------------"
echo ""
echo "  FLTK 1.4.x will be the last version(s) of FLTK supporting"
echo "  autotools (configure + provided Makefiles) to build FLTK."
echo "  FLTK 1.5.0 and higher will only support FLTK builds using CMake."
echo ""
echo "  We suggest to explore and use the CMake build system generators"
echo "  for your own FLTK builds as soon as possible. Some new FLTK build"
echo "  options will only be supported by CMake based builds."
echo "  Please see README.CMake.txt for details and instructions."
echo ""
echo "  User projects that use CMake for their own build can benefit"
echo "  substantially if the FLTK library has been built using CMake."
echo ""
echo "-------------------------------------------------------------------------"
echo ""
