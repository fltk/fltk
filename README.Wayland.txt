README.Wayland.txt - Wayland Platform Support for FLTK
------------------------------------------------------


Contents
========

 1   Introduction

 2   Wayland Support for FLTK
   2.1    Disabling Wayland for Backwards Compatibility
   2.2    Configuration
   2.3    Known Limitations

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

On pure Wayland systems without the X11 headers and libraries, FLTK can be built
with its Wayland backend only (see below).


 2.1 Disabling Wayland for Backwards Compatibility
---------------------------------------------------

Programs using X11 specific functions may need to disable the automatic
detection of Wayland at runtime so they fall back to X11 only.

It is possible to force a program linked to a Wayland-enabled FLTK library
to use X11 in all situations by putting this declaration somewhere in the
source code:

  FL_EXPORT bool fl_disable_wayland = true;

FLTK source code and also X11-specific source code conceived for FLTK 1.3
should run with a Wayland-enabled FLTK 1.4 library with this single change.


Note 1: this may require some linker flags to enable exporting symbols
from *executable* programs which FLTK uses to "read" the global symbol
'fl_disable_wayland'. For GNU `ld` or any GNU compiler this would
be "-rdynamic".


Note 2: When building a user project with CMake 3.4 or higher, i.e. using

  cmake_minimum_required (VERSION 3.4)

or any higher (minimum) CMake version users need to use at least one of
the following techniques:

Option 1: Set target property 'ENABLE_EXPORTS' on all executable
          targets that require to disable the Wayland backend.
          This is the preferred solution because it works per target.

          CMake example:

          set_target_properties(myprog PROPERTIES ENABLE_EXPORTS TRUE)

Option 2: Set CMake policy CMP0065 to 'OLD' (i.e. pre-3.4 behavior)
          This is a quick solution but discouraged because setting
          CMake policies to 'OLD' is "deprecated by definition".
          CMake may issue warnings or ignore this in the future.

          CMake code:

          cmake_policy(SET CMP0065 OLD)

Option 3: Set CMake variable 'CMAKE_ENABLE_EXPORTS' to 'TRUE'.
          Note: use this to be compatible with CMake < 3.27.

Option 4: Set CMake variable 'CMAKE_EXECUTABLE_ENABLE_EXPORTS' to 'TRUE'.
          Note: new in CMake 3.27, ignored in older versions.

Options 3 and 4 can be used as quick solutions like option 2 but these
options affect all targets that are created while the CMake variable is
set. As said above, option 1 should be preferred.

This applies to the FLTK test and demo programs as well, hence we use
option 1 in our build system.


 2.2 Configuration
------------------

On Linux and FreeBSD systems equipped with the adequate software packages
(see section 3 below), the default building procedure produces a Wayland/X11
hybrid library. On systems lacking all or part of Wayland-required packages,
the default building procedure produces a X11-based library.

Use "-D FLTK_BACKEND_WAYLAND=OFF" with CMake or "configure --disable-wayland"
to build FLTK for the X11 library when the default would build for Wayland.

CMake option FLTK_BACKEND_X11=OFF or configure argument "--disable-x11" can
be used to produce a Wayland-only library which can be useful, e.g., when
cross-compiling for systems that lack X11 headers and libraries.

The FLTK Wayland platform uses a library called libdecor which handles window decorations
(i.e., titlebars, shade). On very recent Linux distributions (e.g., Debian trixie)
libdecor is available as Linux packages (libdecor-0-dev and libdecor-0-plugin-1-gtk).
FLTK requires version 0.2.0 or more recent of these packages.
When libdecor is not available or not recent enough, FLTK uses a copy of libdecor
bundled in the FLTK source code.
FLTK equipped with libdecor supports both the client-side decoration mode (CSD) and the
server-side decoration mode (SSD) as determined by the active Wayland compositor.
Mutter (gnome's Wayland compositor) and Weston use CSD mode, KWin and Sway use SSD mode.
Furthermore, setting environment variable LIBDECOR_FORCE_CSD to 1 will make FLTK use CSD
mode even if the compositor would have selected SSD mode.

 2.3 Known Limitations
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

* Text input methods are known to work well for Chinese and Japanese.
Feedback for other writing systems would be helpful.

* Using OpenGL inside Wayland windows doesn't seem to work on RaspberryPi hardware,
although it works inside X11 windows on the same hardware.

* Drag-and-drop initiation from a subwindow doesn't work under the KDE/Plasma desktop.
That is most probably a KWin bug because no such problem occurs with 3 other
Wayland compositors (Mutter, Weston, Sway). A workaround is proposed in issue #997
of the FLTK github repository (https://github.com/fltk/fltk/issues/997).

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
- libxinerama-dev <== except if option FLTK_BACKEND_X11=off is used
- libdbus-1-dev   <== recommended to query current cursor theme
- libglew-dev     <== necessary to use OpenGL version 3 or above
- cmake           <== if you plan to build with CMake
- cmake-qt-gui    <== if you plan to use the GUI of CMake
- libdecor-0-dev  <== recommended if available and if in version ≥ 0.2.0
- libgtk-3-dev    <== highly recommended if libdecor-0-dev is not installed

These packages allow to run FLTK apps under the Gnome-Wayland desktop:
- gnome-core
- libdecor-0-plugin-1-gtk <== install if libdecor-0-dev is installed

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
- dbus-devel     <== recommended to query current cursor theme
- libdecor-devel <== recommended, draws window titlebars
- gtk3-devel     <== highly recommended if libdecor-devel is not installed
- glew-devel     <== necessary to use OpenGL version 3 or above
- cmake          <== if you plan to build with CMake
- cmake-gui      <== if you plan to use the GUI of CMake

Package installation command: sudo yum install <package-name ...>


3.3 FreeBSD
-----------

The Wayland platform is known to work with FreeBSD version 13.1 and the Sway compositor.

These packages are necessary to build the FLTK library and use the Sway compositor:
git autoconf pkgconf xorg urwfonts gnome glew seatd sway dmenu-wayland dmenu evdev-proto

Package installation command: sudo pkg install <package-name ...>
