README.Wayland.txt - Wayland platform support for FLTK
------------------------------------------------------


CONTENTS
========

 1   INTRODUCTION

 2   WAYLAND SUPPORT FOR FLTK
   2.1    Configuration
   2.2    Known Limitations

 3   PREPARING PLATFORM-SPECIFIC CODE FOR THE WAYLAND PLATFORM
   3.1    Handling X11-specific source code
   3.2    Handling X11- and Wayland-specific source code in the same app
   3.3    Forcing an app to always use the X11 mechanism

 4   PLATFORM SPECIFIC NOTES
   4.1    Debian and Derivatives (like Ubuntu)
   4.2    Fedora
   4.3    FreeBSD


1 INTRODUCTION
==============

Version 1.4 of the FLTK library introduces support of the public FLTK API on
the Wayland platform. It requires a Wayland-equipped OS, namely Linux or FreeBSD.
Pre-existing platform-independent source code for FLTK 1.3.x should build and
run unchanged with FLTK 1.4 and the Wayland platform.
The code has been tested on Debian, Ubuntu and Fedora with 3 distinct Wayland
compositors: mutter (Gnome's compositor), weston, and KDE.
The code has also been tested under FreeBSD and the sway wayland compositor.
CJK text-input methods, as well as dead and compose keys are supported.


2 WAYLAND SUPPORT FOR FLTK
==========================

On Linux and FreeBSD systems, and provided a Wayland compositor is available at run-time,
it is possible to have your FLTK application do all its windowing through the
Wayland protocol, all its graphics with Cairo or EGL, and all text-drawing with
Pango. If no Wayland compositor is available at run-time, FLTK falls back to
using X11 or OpenGL for its windowing. Cairo and Pango remain used for graphics
and text, respectively.

Environment variable FLTK_BACKEND can be used to control whether Wayland or
X11 is used at run time as follows:
- if FLTK_BACKEND is not defined, Wayland is used when possible, otherwise
  X11 is used;
- if FLTK_BACKEND equals "wayland", the library stops with error if no
  Wayland compositor is available;
- if FLTK_BACKEND equals "x11", the library uses X11 even if a Wayland
  compositor is available;
- if FLTK_BACKEND has another value, the library stops with error.

On pure Wayland systems without the X11 headers and libraries, FLTK can be built
with its Wayland backend only (see below).

 2.1 Configuration
------------------

   2.1.1 Configure-based build can be performed as follows:
Once after "git clone", create the configure file :
  autoconf -f

Prepare build with :
  ./configure --enable-wayland
Add  --disable-x11  to build FLTK for Wayland-only (no x11 backend).

Build with :
  make

   2.1.2 CMake-based build can be performed as follows:
  cmake -S <path-to-source> -B <path-to-build> -DCMAKE_BUILD_TYPE=Release -DOPTION_USE_WAYLAND=1

  cd <path-to-build>; make

The FLTK Wayland platform uses a library called libdecor which handles window decorations
(i.e., titlebars, shade). Libdecor is bundled in the FLTK source code and FLTK uses by default
this form of libdecor. Optionally, OPTION_USE_SYSTEM_LIBDECOR can be turned on to have FLTK
use the system's version of libdecor which is available on recent Linux distributions (e.g.,
Debian Bookworm or more recent in packages libdecor-0-0 and libdecor-0-plugin-1-cairo).

Optionally, OPTION_WAYLAND_ONLY can be turned on to build FLTK for Wayland-only (no x11 backend).

 2.2 Known Limitations
----------------------

* A deliberate design trait of Wayland makes application windows ignorant of their exact
placement on screen. It's possible, though, to position a popup window relatively to
another window. This allows FLTK to properly position menu and tooltip windows. But
Fl_Window::position() has no effect on other top-level windows.

* With Wayland, there is no way to know if a window is currently minimized, nor is there any
way to programmatically unset minimization of a window. Consequently, Fl_Window::show() of
a minimized window has no effect.

* Although the FLTK API to read from and write to the system clipboard is fully functional,
it's currently not possible for an app to be notified of changes to the content of
the system clipboard, that is, Fl::add_clipboard_notify() has no effect.

* With GTK-style window titlebars, narrow windows are silently forced to be wide enough
for the titlebar to display window buttons and a few letters of the title.

* The library should support multi-display configurations in principle, but has not been
tested in that situation.

* Text input methods have been tested without any understanding of the writing systems,
so feedback on this subject would be helpful.


3 PREPARING PLATFORM-SPECIFIC CODE FOR THE WAYLAND PLATFORM
===========================================================

While platform-independent source code prepared for FLTK 1.3 is expected
to be compatible with no change with FLTK 1.4 and the Wayland platform,
platform-specific code may require some attention.

3.1 Handling X11-specific source code
-------------------------------------

If an FLTK 1.4 application contains X11-specific code, execution of this code
in the context of an active Wayland session can produce malfunctions or program crashes.
To ensure that X11-specific code gets called only when an X11 connection is active,
check that function fl_x11_display() returns non-NULL before using any X11-specific
function or variable.

3.2 Handling X11- and Wayland-specific source code in the same app
------------------------------------------------------------------

The recommended way to prepare and use platform-specific code that would contain
both X11-specific and Wayland-specific parts is as follows :

a) Organize platform-specific code as follows :

  #include <FL/platform.H>

  #ifdef __APPLE__
     *** macOS-specific code ***
  #elif defined(_WIN32)
     *** Windows-specific code ***
  #else
      *** X11-specific code ***

      *** Wayland-specific code ***
  #endif

b) Make sure to use distinct names for global variables and functions
in the X11- and the Wayland-specific sections.

c) Check that function fl_x11_display() returns non-NULL before using any X11-specific
function or variable, and that fl_wl_display() returns non-NULL before using any
Wayland-specific function or variable. Make sure that fl_open_display() was called
directly or indirectly before using any such symbol.

3.3 Forcing an FLTK app to always use the X11 backend
-----------------------------------------------------

Alternatively, it's possible to force an FLTK app to use X11 in all
situations by calling function fl_disable_wayland() early in main(), that is,
before fl_open_display() runs. FLTK source code and also platform-specific
code conceived for FLTK 1.3 should run under 1.4 with that single change only.


4 PLATFORM SPECIFIC NOTES
=========================

The following are notes about building FLTK for the Wayland platform
on the various supported Linux distributions/OS.

4.1 Debian and Derivatives (like Ubuntu)
----------------------------------------

Under Debian, the Wayland platform requires version 11 (a.k.a. Bullseye) or more recent.
Under Ubuntu, the Wayland platform is known to work with version 20.04 (focal fossa) or
more recent.

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


4.2 Fedora
----------

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


4.3 FreeBSD
-----------

The Wayland platform is known to work with FreeBSD version 13.1 and the sway compositor.

These packages are necessary to build the FLTK library and the sway compositor:
pkg install git autoconf pkgconf xorg urwfonts gnome glew seatd sway \
                dmenu-wayland dmenu evdev-proto

The FLTK library can be built as follows using either configure or CMake :

1) Using configure

cd <path-to-FLTK-source-tree>
autoconf -f
./configure --enable-localzlib --enable-wayland
make

2) Using CMake

cmake -S <path-to-source> -B <path-to-build> -DOPTION_USE_WAYLAND=1

cd <path-to-build>; make
