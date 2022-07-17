
# FLTK - Fast Light Tool Kit (FLTK) 
 The Fast Light Tool Kit ("FLTK", pronounced "fulltick") is across-platform C++ GUI toolkit for UNIX®/Linux® (X11 orWayland), Microsoft® Windows®, and macOS®. FLTK provides modern GUI functionality without the bloat and supports 3D graphics via OpenGL® and its built-in GLUT emulation.

## STATE OF DEVELOPMENT

-   The [`master`](https://github.com/fltk/fltk/tree/master) branch contains work in progress for the next major version FLTK 1.4. As such it's considered unstable, but any testing and feedback is highly appreciated.


## DOWNLOAD

-   You can get the latest official release on [`FLTK's website`](https://www.fltk.org/software.php).
-   You can also get the source code of the current development version from the [`Git repository`](https://github.com/fltk/fltk).

if you are using MSYS64 just type  `mingw-w64-x86_64-fltk` for X64 version 

    For more information see README.txt:
    https://github.com/fltk/fltk/blob/master/README.txt
    
 ## LICENSING

 FLTK comes with complete free source code. FLTK is availableunder the terms of the GNU Library General Public License.Contrary to popular belief, it can be used in commercial software! (Even Bill Gates could use it.)
 
 ## ON-LINE DOCUMENTATION

 The documentation in HTML and PDF forms can be created by Doxygen from the source files. HTML and PDF versions of thisdocumentation is also available from the FLTK web site at:[`https://www.fltk.org/documentation.php`](https://www.fltk.org/documentation.php)

## BUILDING
  ## On Window
There are two ways to build FLTK under Microsoft Windows. The first is to use CMake to create the Visual C++ project files in your favorite development directory, then build FLTK with Visual Studio. The second method is to use a GNU-based development tool. To build with the Cygwin or MinGW tools, use the supplied configure script as specified in the UNIX section above: sh configure ...options...
See README.Windows.md and README.CMake.md for more info

## AUTHOR
It was originally developed by Mr. Bill Spitzak and is currently maintained by a small group of developers across the world with a central repository on GitHub.




![Build](https://github.com/fltk/fltk/actions/workflows/build.yml/badge.svg)
