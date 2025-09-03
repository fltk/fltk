README.Unix.txt - Building FLTK on Unix/Linux Systems
------------------------------------------------------


 Contents
==========

  1   Introduction

  2   Prerequisites
    2.1   Debian, Ubuntu, Linux Mint, and more ...
    2.2   Fedora
    2.3   NetBSD
    2.4   OpenSuSE [*]
    2.5   SunOS / Solaris
    2.6   SGI [*]
    2.7   HP-UX
    2.8   AIX

  3   How to Build FLTK Using GCC
    3.1   Downloading and Unpacking
    3.2   Configuration and Build Systems
    3.3   Configuring FLTK with autoconf and configure (deprecated)
    3.4   Building FLTK
    3.5   Testing FLTK
    3.6   Installing FLTK
    3.7   Creating new Projects

  4   Creating a new Project in Code::Blocks


[*] TODO: we still need to write these chapters


-------------------------------------------------------------------
  Note: usage of autotools, configure, and the included Makefiles
  to build the FLTK library is deprecated since FLTK 1.4 and will
  be removed in the next minor version (1.5).
  Please consider using CMake instead, see README.CMake.txt.
-------------------------------------------------------------------


 1  Introduction
=================

FLTK currently supports the following development environments on most Unix
and Linux platforms:

    - CMake + the build system of your choice (see README.CMake.txt)
    - gcc command line tools
    - Code::Blocks
    - ...

The Symbol font and the Zapf Dingbats font do not work on X11. This is correct
behavior for UTF-8 platforms.

IMPORTANT:

Please be aware that the following instructions may be outdated because we
can't follow the development of current and future Unix/Linux distributions.
If you find bugs or want to suggest enhancements please let us know.
See https://www.fltk.org/bugs.php for how to do this.



 2  Prerequisites
==================


 2.1  Debian, Ubuntu, Linux Mint, and more ...
-----------------------------------------------

All Linux distributions based on Debian are similar and use the `apt`
package manager.

  - Ubuntu Linux can be downloaded from https://ubuntu.com/download/desktop
  - Debian Linux can be downloaded from https://www.debian.org/
  - Linux Mint   can be downloaded from https://www.linuxmint.com/

If you have not done so yet, download and install the distribution of your choice.

Open a shell and install some development software:

  sudo apt-get install g++
  sudo apt-get install gdb
  sudo apt-get install git
  sudo apt-get install make
  sudo apt-get install cmake      # install CMake...
  sudo apt-get install autoconf   # ...or autoconf (or both)
  sudo apt-get install libx11-dev
  sudo apt-get install libglu1-mesa-dev
  sudo apt-get install libxft-dev
  sudo apt-get install libxcursor-dev

# These packages are optional but recommended:

  sudo apt-get install libasound2-dev
  sudo apt-get install freeglut3-dev
  sudo apt-get install libcairo2-dev
  sudo apt-get install libfontconfig1-dev
  sudo apt-get install libglew-dev
  sudo apt-get install libjpeg-dev
  sudo apt-get install libpng-dev
  sudo apt-get install libpango1.0-dev
  sudo apt-get install libxinerama-dev

If you want to build FLTK for Wayland (the supposed successor of X) you need
some more packages. Please refer to README.Wayland.txt for more information.

If you are planning to use the Code::Blocks IDE, also install this

  sudo apt-get install codeblocks

To install the latest FLTK development (master branch, currently 1.4.x):

  git clone https://github.com/fltk/fltk.git
  cd fltk
  ...

To update to the latest version, just `cd` into the fltk directory and type

  git pull


 2.2  Fedora
-------------

Fedora Linux can be downloaded from https://getfedora.org/

If you have not done so yet, download and install Fedora.

Open a terminal window and install some software. In Fedora, the default user
has no permission to call "sudo", so we will change user a few times:

  su root
  yum groupinstall "Development Tools"
  yum groupinstall "X Software Development"
  yum groupinstall "C Development Tools and Libraries"

If you are planning to use the Code::Blocks IDE, also install this

  yum install codeblocks.i686  (for 64 bit machines)

Don't forget to leave root status (Ctrl-D) before loading FLTK. To install FLTK
for every user, you either have to set root user again, or use "visudo" to add
yourself to the "sudo" list.

To install the latest FLTK development (master branch, currently 1.4.x):

  git clone https://github.com/fltk/fltk.git
  cd fltk
  ...

To update to the latest version, just `cd` into the fltk directory and type

  git pull


 2.3   NetBSD
--------------

NetBSD can be downloaded from https://www.netbsd.org/

If you have not done so yet, download and install NetBSD. Ensure that the
optional distribution sets "comp" (Compiler, header files, development tools)
and x*** (X Window System) are installed.

Now install and configure pkgsrc. The current version can be downloaded here:

  https://www.pkgsrc.org/

To use the current stable version of FLTK, simply install it from pkgsrc:

  cd /usr/pkgsrc/x11/fltk13
  bmake install

For the latest development snapshot, first install the git client from pkgsrc:

  cd /usr/pkgsrc/devel/git
  bmake install

Now fetch the latest FLTK source code:

  git clone https://github.com/fltk/fltk.git

To update to the latest version, just `cd` into the fltk directory and type

  git pull

If you have installed JPEG and PNG libraries from pkgsrc, configure your
environment as follows so that the FLTK configure script can find them:

  export CPPFLAGS="-I/usr/pkg/include"
  export LDFLAGS="-L/usr/pkg/lib"

To install GNU autoconf from pkgsrc:

  cd /usr/pkgsrc/devel/autoconf
  bmake install


 2.4  OpenSuSE
---------------

We still need to write this chapter.

OpenSUSE can be downloaded from https://www.opensuse.org/


 2.5  SunOS / Solaris
----------------------

SunOS is a commercial operating system from Sun Microsystems (in 2009 the
company was sold to Oracle). SunOS is also called Solaris since version 5.
There was an open source derivative called OpenSolaris (based on Solaris 10)
that was cancelled by Oracle. The successor in spirit is called OpenIndiana
and can be downloaded from:

  https://www.openindiana.org/

For FLTK you need at least SunOS 5.7 (aka Solaris 7). This version supports
64 bit machines and POSIX threads. For machines with 64 bit SPARC processors
it is highly recommended to use Sun compilers, the 64 bit code generated by
older versions of GCC for such machines is unusable (expect something between
horribly broken and immediate crash).

Consider using pkgsrc with this operating system. See the NetBSD section if
you want to do so.
If you have GCC and Sun compilers installed, configure your environment like
this to use the Sun compilers and pkgsrc graphics libraries:

  export CC="cc"
  export CXX="CC"
  export CPPFLAGS="-I/usr/pkg/include"
  export LDFLAGS="-L/usr/pkg/lib -R/usr/pkg/lib"

To build a 64 bit FLTK library, add the following flags:

  export CFLAGS="-xarch=v9"
  export CXXFLAGS="-xarch=v9"


 2.6  SGI
----------

We still need to write this chapter.


 2.7  HP-UX
------------

HP-UX is a commercial operating system from HP, no free or open source
derivatives are available.

For FLTK you need at least HP-UX 11.11 and the latest patch bundles should be
installed.

Consider using pkgsrc with this operating system. See the NetBSD section if
you want to do so.


 2.8  AIX
----------

AIX is a commercial operating system from IBM, no free or open source
derivatives are available.

For FLTK you need at least AIX 5L, I have tested version 5.1 patched to ML9.

Consider using pkgsrc with this operating system. See the NetBSD section if
you want to do so.



 3  How to Build FLTK Using GCC
================================


 3.1  Downloading and Unpacking
--------------------------------

The FLTK source code and documentation can be downloaded from:

  https://www.fltk.org/software.php

If you are familiar with "git" and like to stay current with your version,
you will find the git access parameters at the bottom of that page.
Unpack FLTK into a convenient location. I like to have everything in my
dev directory. Change the following instructions to fit your preferences.

  cd
  mkdir dev
  cd dev
  mv ~/Downloads/fltk-1.x.y-source.tar.gz .
  tar xvzf fltk-1.x.y-source.tar.gz
  cd fltk-1.x.y


 3.2  Configuration and Build Systems
--------------------------------------

The following paragraphs describe the "classic" build system with autoconf,
configure, and make that has been used to build FLTK up to version 1.3.x
and can still be used with FLTK 1.4.x.

However, the FLTK team recommend to use CMake which is the preferred build
system generator since FLTK 1.4 used for all platforms (including Windows).
CMake can be used to create the build system of your choice, for instance
Makefiles, Ninja build files, Xcode or Visual Studio IDE projects etc..

-------------------------------------------------------------------
  Note: usage of autotools, configure, and the included Makefiles
  to build the FLTK library is deprecated since FLTK 1.4 and will
  be removed in the next minor version (1.5).
  Please consider using CMake instead, see README.CMake.txt.
-------------------------------------------------------------------

Please see README.CMake.txt for how to build FLTK and your application
programs using CMake. You can stop reading here if you do this.

You can, of course, build FLTK with CMake and your own application(s)
with your existing and well-known build system.

If you like the "classic" build system more, continue reading the
following chapters but please be aware that configure support will
be removed in FLTK 1.5.


 3.3  Configuring FLTK with autoconf and configure (deprecated)
----------------------------------------------------------------

-------------------------------------------------------------------
  Note: usage of autotools, configure, and the included Makefiles
  to build the FLTK library is deprecated since FLTK 1.4 and will
  be removed in the next minor version (1.5).
  Please consider using CMake instead, see README.CMake.txt.
-------------------------------------------------------------------

If you got FLTK via git then you need one extra step. Otherwise skip
over this part. Stay in your FLTK source-code directory and type:

  autoconf

or

  make configure

Both commands create the configure script for you.

Now configure your FLTK installation:

  ./configure

Hint: Instead of executing `autoconf` and `configure` followed by `make`
to build FLTK (see next section) you can also run `make` directly which
will create and execute the 'configure' script with default parameters
and build FLTK with the default configuration.

ADVANCED: type "./configure --help" to get a complete list of optional
configuration parameters. These should be pretty self-explanatory. Some
more details can be found in README.txt.
:END_ADVANCED

The configuration script will check your machine for the required resources
which you should have installed as described in the "Prerequisites" chapter.
Review the "Configuration Summary", maybe take some notes.


 3.4  Building FLTK
--------------------

Now this is easy. Stay in your FLTK source-code directory and type:

  make

The entire FLTK toolkit including many test programs will be built for you. No
warnings should appear. If some do, please let the FLTK developer team know via
the mailing list "fltk.general" or view the bug reporting guidelines at
https://www.fltk.org/bugs.php


 3.5  Testing FLTK
-------------------

After a successful build, you can test FLTK's capabilities:

  test/demo


 3.6  Installing FLTK
----------------------

If you did not change any of the configuration settings, FLTK will be installed
in "/usr/local/include" and "/usr/local/lib" by typing

  sudo make install

If you are using the KDE, GNOME or XFCE desktop environments and want to call
"fluid" from the desktop menu, you will need to install additional files and
icons under "/usr/share" by typing:

  sudo make install-desktop

It is possible to install FLTK without superuser privileges by changing the
installation path to a location within the user account by adding the
"--prefix=PREFIX" parameters to the "./configure" command.

Note: installing FLTK is optional. You can build your own software by using
the FLTK build tree directly. This is recommended if you link your application
statically (which is recommended as well). This is particularly important if you
consider using different FLTK versions on a development system to avoid mixing
FLTK versions when building (a well-known problem) or running FLTK applications
with installed shared libraries of a different FLTK version. OTOH, if you build
shared FLTK libraries you may want to install FLTK, particularly on a production
system.


 3.7  Creating new Projects
----------------------------

FLTK provides a neat script named "fltk-config" that can provide all the flags
needed to build FLTK applications using the same flags that were used to build
the library itself. Running "fltk-config" without arguments will print a list
of options. The easiest call to compile an FLTK application from a single
source file is:

  fltk-config --compile myProgram.cxx

Since version 1.4.0 `fltk-config --compile` can also be used to build a program
from multiple source files. See the official docs in chapter "FLTK Basics" and
section "Compiling Multiple Source Files with 'fltk-config'":
https://www.fltk.org/doc-1.4/basics.html#basics_fltk_config2

"fltk-config" and "fluid" will be installed in "/usr/local/bin/" by default.
We recommend that you add it to the command search path.



 4  Creating a new Project in Code::Blocks
===========================================

Code::Blocks is a free and popular C++ IDE in the Linux world. It also runs on
macOS and Windows. Configured correctly, it can also cross-compile between
these platforms. This chapter focuses on creating a new FLTK project for Linux,
assuming that FLTK was previously built and installed in its default location.

If not done yet, install Code::Blocks as described in the "Prerequisites"
chapter above, or download it from their web site. This description is based
on version 10.05:

  https://www.codeblocks.org/

Start Code::Blocks. Select File > New > Project. In the "New from template"
dialog box, click on "FLTK project" and follow the instructions.

The default project supports basic FLTK. If you would like to add support for
images, OpenGL, GLUT, or Forms, add the corresponding flags --use-images,
--use-gl, --use-glut, and --use-forms respectively.

The flags are located in the "Project Build Options" dialog. To change the
compiler flags, select your project in the tree view, then select the
"Compiler Settings" tab, then "Other Options" and add the flags to
`fltk-config --cxxflags` in front of the second "`".

The linker flags are located in the "Linker Settings" tab under "Other Linker
Options". Add the flags to `fltk-config --ldstaticflags` in front of the
second "`".

Code::Blocks can be set up to use fluid to manage modules.
The following info is from an FLTK user posted on fltk.general 06/17/2013:

"""
    I have these settings in codeblocks on linux:

    Settings ->
      Compiler and debugging settings ->
        Other settings ->
          Advanced options:

    -- Add an extension (in my case "fl")

    -- On command line macro:
                cd $file_dir;  fluid -c $file

    -- Generated files  (to be further compiled):
                $file_dir/$file_name.cxx
                $file_dir/$file_name.h

    Settings -> Environment -> Files extension handling :
                Wildcard : *.fl
                To open file: Launch an external program
                External program: fluid

    With that I can double click on any fluid file I include on a project
    and it opens with fluid. When I press "run" or "build" codeblocks
    calls fluid to generate the c++ files and compiles if needed.
"""
