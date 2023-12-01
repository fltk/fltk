README.Wayland.txt - Wayland Platform Support for FLTK
------------------------------------------------------


Contents
========

 1   Introduction

 2   Wayland Support for FLTK
   2.1    Configuration
   2.2    Known Limitations

 3   Platform Specific Notes
   3.1    Debian and Derivatives (like Ubuntu, Mint, RaspberryPiOS)
   3.2    Fedora
   3.3    FreeBSD


1 Introduction
==============

Version 1.4 of the FLTK library introduces support of the public FLTK API on
the Wayland platform. It requires a Wayland-equipped OS, namely Linux or FreeBSD.
Pre-existing platform-independent source code for FLTK 1.3.x should build and
run unchanged with FLTK 1.4 and the Wayland platform.
The code has been tested on Debian, Ubuntu, RaspberryPiOS and Fedora with
3 distinct Wayland compositors: Mutter (Gnome's compositor), Weston, and KWin.
The code has also been tested under FreeBSD and the Sway Wayland compositor.
CJK text-input methods, as well as dead and compose keys are supported.


2 Wayland Support for FLTK
==========================

On Linux and FreeBSD systems, the FLTK library is by default configured so FLTK apps
do all their windowing through the Wayland protocol, all their graphics with
Cairo or EGL, and all text-drawing with Pango. If no Wayland compositor is
available at run-time, FLTK apps fall back to using X11 for windowing.
Cairo and Pango remain used for graphics and text, respectively.

Environment variable FLTK_BACKEND can be used to control whether Wayland or
X11 is used at run time as follows:
- if FLTK_BACKEND is not defined, Wayland is used when possible, otherwise
  X11 is used;
- if $FLTK_BACKEND equals "wayland", the library stops with error if no
  Wayland compositor is available;
- if $FLTK_BACKEND equals "x11", the library uses X11 even if a Wayland
  compositor is available;
- if $FLTK_BACKEND has another value, the library stops with error.

Alternatively, it is possible to force a program linked to a Wayland-enabled
FLTK library to use X11 in all situations by putting this declaration somewhere
in the source code :
  FL_EXPORT bool fl_disable_wayland = true;
FLTK source code and also X11-specific source code conceived for FLTK 1.3
should run with a Wayland-enabled, FLTK 1.4 library with that single change only.

On pure Wayland systems without the X11 headers and libraries, FLTK can be built
with its Wayland backend only (see below).

 2.1 Configuration
------------------

On Linux and FreeBSD systems equipped with the adequate software packages
(see section 3 below), the default building procedure produces a Wayland/X11
hybrid library. On systems lacking all or part of Wayland-required packages,
the default building procedure produces a X11-based library.

Use -DOPTION_USE_WAYLAND=OFF with CMake or "configure --disable-wayland" to build
FLTK for the X11 library when the default would build for Wayland.

CMake OPTION_WAYLAND_ONLY or "--disable-x11" configure argument can
be used to produce a Wayland-only library which can be useful, e.g., when
cross-compiling for systems that lack X11 headers and libraries.

The FLTK Wayland platform uses a library called libdecor which handles window decorations
(i.e., titlebars, shade). On very recent Linux distributions (e.g., Debian trixie)
libdecor is available as Linux packages (libdecor-0-dev and libdecor-0-plugin-1-gtk).
FLTK requires version 0.2.0 of these packages or more.
In other situations, FLTK uses a copy of libdecor bundled in the FLTK source code.
FLTK equipped with libdecor supports both the client-side decoration mode (CSD) and the
server-side decoration mode (SSD) as determined by the active Wayland compositor.
Mutter (gnome's Wayland compositor) and Weston use CSD mode, KWin and Sway use SSD mode.
Furthermore, setting environment variable LIBDECOR_FORCE_CSD to 1 will make FLTK use CSD
mode even if the compositor would have selected SSD mode.

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

* Copying data to the clipboard is best done when the app has focus. Any copy operation
performed when the app did not get the focus yet does not change the clipboard. A copy
operation performed when the app has lost the focus is successful only if the type of
the copied data, that is, text or image, is the same as the last data type copied when
the app had the focus.

* Narrow windows with a titlebar are silently forced to be wide enough
for the titlebar to display window buttons and a few letters of the title.

* Text input methods have been tested without any understanding of the writing systems,
so feedback on this subject would be helpful.

* Using OpenGL inside Wayland windows doesn't seem to work on RaspberryPi hardware,
although it works inside X11 windows on the same hardware.


3 Platform Specific Notes
=========================

The following are notes about building FLTK for the Wayland platform
on the various supported Linux distributions/OS.

3.1 Debian and Derivatives (like Ubuntu, Mint, RaspberryPiOS)
-------------------------------------------------------------

Under Debian, the Wayland platform requires version 11 (a.k.a. Bullseye) or more recent.
Under Ubuntu, the Wayland platform requires version 20.04 (focal fossa) or more recent.

These packages are necessary to build the FLTK library, in addition to those listed
in section 2.1 of file README.Unix.txt :
- make
- libpango1.0-dev
- libwayland-dev
- wayland-protocols
- libxkbcommon-dev
- libxinerama-dev
- libdbus-1-dev  <== recommended to query current cursor theme
- libglew-dev    <== necessary to use OpenGL version 3 or above
- cmake          <== if you plan to build with CMake
- cmake-qt-gui   <== if you plan to use the GUI of CMake

These packages allow to run FLTK apps under the Gnome-Wayland desktop:
- gnome-core
- libgtk-3-dev   <== highly recommended, gives windows a Gnome-style titlebar
- libdecor-0-dev and libdecor-0-plugin-1-gtk in versions ≥ 0.2.0
                 <== Recommended if available for the Linux version in use,
                 <== FLTK uses a bundled copy of these packages otherwise.

These packages allow to run FLTK apps under the KDE/Plasma-Wayland desktop:
- kde-plasma-desktop
- plasma-workspace-wayland

Package installation command: sudo apt-get install <package-name ...>


3.2 Fedora
----------

The Wayland platform is known to work with Fedora version 35 or more recent.

These packages are necessary to build the FLTK library, in addition to
package groups listed in section 2.2 of file README.Unix.txt :
- autoconf
- wayland-devel
- wayland-protocols-devel
- cairo-devel
- libxkbcommon-devel
- pango-devel
- mesa-libGLU-devel
- dbus-devel   <== recommended to query current cursor theme
- gtk3-devel   <== highly recommended, gives windows a GTK-style titlebar
- libdecor-0.2.0 <== recommended, present in Fedora Rawhide, not in Fedora 39
- glew-devel   <== necessary to use OpenGL version 3 or above
- cmake        <== if you plan to build with CMake
- cmake-gui    <== if you plan to use the GUI of CMake

Package installation command: sudo yum install <package-name ...>


3.3 FreeBSD
-----------

The Wayland platform is known to work with FreeBSD version 13.1 and the Sway compositor.

These packages are necessary to build the FLTK library and use the Sway compositor:
git autoconf pkgconf xorg urwfonts gnome glew seatd sway dmenu-wayland dmenu evdev-proto

Package installation command: sudo pkg install <package-name ...>
