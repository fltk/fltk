README.Wayland.txt - Wayland platform support for FLTK
------------------------------------------------------


CONTENTS
========

 1   INTRODUCTION

 2   WAYLAND SUPPORT FOR FLTK
   2.1    Configuration
   2.2    Known limitations

 3   PLATFORM SPECIFIC NOTES
   3.1    Debian and Derivatives (like Ubuntu)
   3.2    Fedora


1 INTRODUCTION
==============

Version 1.4 of the FLTK library introduces support of the public FLTK API on
the Wayland platform. It requires a Wayland-equipped OS which means Linux.
Pre-existing platform-independent source code for FLTK 1.3.x should build and
run unchanged with FLTK 1.4 and the Wayland platform.
The code has been tested on Debian, Ubuntu and Fedora with 3 distinct Wayland
compositors: mutter (Gnome's compositor), weston, and KDE.
CJK text-input methods, as well as dead and compose keys are supported.


2 WAYLAND SUPPORT FOR FLTK
==========================

It is possible to have your FLTK application do all its windowing and drawing
through the Wayland protocol on Linux systems. All graphics is done via Cairo or EGL.
All text-drawing is done via Pango.

 2.1 Configuration
---------------

* Configure-based build can be performed as follows:
Once after "git clone", create the configure file :
   autoconf -f

Prepare build with :
   ./configure --enable-wayland [--enable-shared]

Build with :
   make

* CMake-based build can be performed as follows:
cmake -S <path-to-source> -B <path-to-build> -DCMAKE_BUILD_TYPE=Release -DOPTION_USE_WAYLAND=1

cd <path-to-build>; make

The FLTK wayland platform uses a library called libdecor which handles window decorations
(i.e., titlebars, shade). Libdecor is bundled in the FLTK source code and FLTK uses by default
this form of libdecor. Optionally, OPTION_USE_SYSTEM_LIBDECOR can be turned on to have FLTK
use the system's version of libdecor which is available on recent Linux distributions (e.g.,
Debian bookworm or more recent in packages libdecor-0-0 and libdecor-0-plugin-1-cairo).

 2.2 Known limitations
-------------------------------

* A deliberate design trait of Wayland makes application windows ignorant of their exact
placement on screen. It's possible, though, to position a popup window relatively to
another window. This allows FLTK to properly position menu and tooltip windows. But
Fl_Window::position() has no effect on other top-level windows.

* With Wayland, there is no way to know if a window is currently minimized, nor is there any
way to programmatically unset minimization of a window. Consequently, Fl_Window::show() of
a minimized window has no effect.

* It's currently not possible for an app to be notified of changes to the content of
the system clipboard, that is, Fl::add_clipboard_notify() has no effect. The FLTK API to
read from and write to the system clipboard is fully functional, though.

* With GTK-style window titlebars, the minimum width of a window is currently
set at 134 pixels.

* The library should support multi-display configurations in principle, but has not been
tested in that situation.

* Text input methods have been tested without any understanding of the writing systems,
so feedback on this subject would be helpful.

* While platform-independent source code prepared for FLTK 1.3 is expected to be compatible
with FLTK 1.4 and the Wayland platform, X11-specific code will not compile.  In particular,
the common FLTK 1.3 construct :
  #ifdef __APPLE__
     *** macOS-specific code ***
  #elif defined(_WIN32)
     *** Windows-specific code ***
  #else
     *** X11-specific code ***
  #endif
will choke at compile time because it exposes X11-specific code to the non-X11, Wayland
environment. This should be written instead :
  #include <FL/fl_config.h>

  #ifdef __APPLE__
     *** macOS-specific code ***
  #elif defined(_WIN32)
     *** Windows-specific code ***
  #elif defined(FLTK_USE_X11)
     *** X11-specific code ***
  #endif
Moreover, the new FLTK_USE_WAYLAND preprocessor variable is available to bracket
Wayland-specific source code.


3 PLATFORM SPECIFIC NOTES
=========================

The following are notes about building FLTK for the Wayland platform
on the various supported Linux distributions.

    3.1 Debian and Derivatives (like Ubuntu)
    ----------------------------------------
Under Debian, the Wayland platform requires version 11 (a.k.a. Bullseye) or more recent.
Under Ubuntu, the Wayland platform is known to work with version 20.04 (focal fossa) or more recent.

These packages are necessary to build the FLTK library, in addition to those present
in a basic Debian/Ubuntu distribution :
- g++
- gdb
- make
- git
- autoconf
- libglu1-mesa-dev
- libpango1.0-dev
- libwayland-dev
- wayland-protocols
- libdbus-1-dev
- libxkbcommon-dev
- libgtk-3-dev   <== with this, windows get a GTK-style titlebar
- libglew-dev    <== necessary to use OpenGL version 3 or above
- cmake          <== if you plan to build with CMake
- cmake-qt-gui   <== if you plan to use the GUI of CMake

This package is necessary to run FLTK apps under the Gnome-Wayland desktop:
- gnome

These packages allow to run FLTK apps under the KDE/Plasma-Wayland desktop:
- kde-plasma-desktop
- plasma-workspace-wayland


3.2 Fedora

The Wayland platform is known to work with Fedora version 35.

These packages are necessary to build the FLTK library, besides those present
in a Fedora 35 Workstation distribution :
- gcc-c++
- autoconf
- wayland-devel
- wayland-protocols-devel
- cairo-devel
- libxkbcommon-devel
- pango-devel
- dbus-devel
- mesa-libGLU-devel
- gtk3-devel   <== with this, windows get a GTK-style titlebar
- glew-devel   <== necessary to use OpenGL version 3 or above
- cmake        <== if you plan to build with CMake
- cmake-gui    <== if you plan to use the GUI of CMake

Package installation command: sudo yum install <package-name ...>
