 README.Windows.txt - Building FLTK under Microsoft Windows
------------------------------------------------------------


––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
***   CAUTION: This file is outdated. This needs a major rework!   ***
––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––



 CONTENTS
==========

  1 INTRODUCTION

  2 HOW TO BUILD FLTK USING MinGW AND Cygwin

    2.1   The Tools
    2.2   Recommended Command Line Build Environment
    2.3   Prerequisites
    2.4   Downloading and Unpacking
    2.5   Configuring FLTK
    2.6   Building FLTK
    2.7   Testing FLTK
    2.8   Installing FLTK
    2.9   Creating new Projects

  3 HOW TO BUILD FLTK USING MICROSOFT VISUAL STUDIO

    3.1   Prerequisites
    3.2   Downloading and Unpacking
    3.3   Configuring FLTK
    3.4   Building FLTK
    3.5   Testing FLTK
    3.6   Installing FLTK
    3.7   Creating new Projects

  4 FREQUENTLY ASKED QUESTIONS

  5 LINKS

  6 DOCUMENT HISTORY


  1 INTRODUCTION
==================

FLTK 1.3 and later is officially supported on Windows (2000,) 2003,
XP, and later.  Older Windows versions are not officially supported,
but may still work.  The main reason is that the OS version needs
to support UTF-8. FLTK 1.3 is known to work on Windows Vista, Windows 7,
Windows 8/8.1, and Windows 10.

FLTK 1.4 and later versions may require Windows 10 or later.

FLTK currently supports the following development
environments on the Windows platform:

    - Free Microsoft "Visual C++ 2008 Express" or later or "Visual Studio
      Community 2013" or later. The Visual Studio project files must be
      generated using CMake. Visual Studio 2017 includes CMake support:

      "Visual Studio 2017 introduces built-in support for handling CMake
      projects. This makes it a lot simpler to develop C++ projects built
      with CMake without the need to generate VS projects and solutions
      from the command line. This post gives you an overview of the CMake
      support, how to easily get started and stay productive in Visual Studio."

      Citation from:
      https://devblogs.microsoft.com/cppblog/cmake-support-in-visual-studio/

      As of this writing (07/2017) the FLTK team did not yet test and
      verify the functionality of Microsoft's included CMake features.

    - GNU toolsets (Cygwin or MinGW) hosted on Windows.

CAUTION: Libraries built by any one of these environments can not be mixed
with object files from any other environment!


  2 HOW TO BUILD FLTK USING MinGW AND Cygwin
==============================================

This chapter of this document gives a brief overview of
compiling and using FLTK with the Cygwin and MinGW compiler
toolkits.  Both toolkits provide a build environment based
around the GNU C/C++ compiler. Further information is
available from the FLTK website at https://www.fltk.org, such
as this Howto note: https://www.fltk.org/articles.php?L598

The Cygwin build environment supplies a library (the Cygwin
DLL) that is primarily intended to provide a number of
Unix-like POSIX facilities for programs being ported to the
Windows environment (Win32 or WinNT).  Cygwin also supplies
a very Unix-like build environment for Windows, including
the "BASH" Bourne-compatible shell and all of the standard
Unix file utilities (ls, cat, grep, etc.).

Cygwin is developed by Cygnus (now part of RedHat, Inc).
Although provided for free download under the GPL, distributing
programs that require the Cygwin DLL under a license other than
the GPL requires a commercial license for the Cygwin DLL.

Native Windows programs that do not require the Cygwin DLL
(cross-compiled and linked with the MinGW gcc/g++ cross compilers
supplied with Cygwin) may be released under any license freely.

Currently you would have to install mingw64-i686-gcc-g++ for
32-bit Windows applications (despite its name!), and/or
mingw64-x86_64-gcc-g++ for 64-bit applications. You may also
need to install the corresponding '-headers' packages as well.

Currently these tools support gcc 4.5 or newer. The setup for
FLTK is somewhat more complicated because you must configure
this as a cross compiler, but it works well.

The MinGW distribution (Minimalist GNU for Windows) provides
a similar toolset but geared solely towards native Windows
development without the Unix-like POSIX library. The lack of
any libraries under the GPL or any other restrictive license
means that programs built with the MinGW environment may
always be released under any license freely. MinGW also
supplies a Unix-like build environment for Windows,
including MSYS (a Bourne-compatible shell) and the standard
Unix file utilities (ls, cat, grep, etc.)

If you are not familiar with these GNU-like toolkits please
refer to the links section later in this note. In particular,
check out their license conditions carefully before use.


 The Tools
-----------

There are currently four main configurations supported by
FLTK with the GNU tools:

    1. Cygwin: Built using the Cygwin toolset and using the Unix-like
       POSIX compatibility layer provided by the Cygwin DLL.
       License: GPL or non-free commercial license (ask Redhat).
       Note: no longer tested by the FLTK team.

    2. Cygwin using the MinGW cross compiler suite: Built using
       the Cygwin tools but not using the Cygwin DLL.
       License: freely distributable on all Windows systems.
       Note: no longer tested by the FLTK team.

    3. MinGW: Built using the MinGW utilities, compiler and tools. This
       is, in many aspects, analogous to (2.). This is the recommended
       one if you want to build native Windows programs only.
       License: freely distributable on all Windows systems.

    4. MSYS2/Mingw-w64: Built using the MSYS2 utilities, compiler and tools.
       This similar to (3.) but may need some fiddling with the setup if
       you want to build native Windows programs only because the built
       executables *may* depend on some MSYS2 dll's.
       License: freely distributable on all Windows systems.


 Recommended Command Line Build Environment
--------------------------------------------

Our recommendation is to:

    1. Get the current Cygwin toolset.

       This can either produce executables that do or do not
       rely on the Cygwin DLL (check licensing) at your choice.

    2. Get the latest MinGW toolset. It is recommended that you
       also get the MSYS shell and the msysDTK developer toolset.

       This will only produce normal Windows native executables
       without any Unix or POSIX compatibility layer.

    3. Get the latest MSYS2/Mingw-w64 toolset.

    See the links section below for more information.

Either option can generate Windows native executables and option 1 can
provide a Unix-like POSIX portability layer that is reliant on a GPLed library.

See the later sections for detailed information about using
one of these configurations.


 Prerequisites
---------------

In order to build FLTK from the command line, you need to install the MinGW
environment. The graphical installer "mingw-get-inst" can be downloaded for
free.

  NOTE: as of Dec 07, 2024 MinGW development seems to be dormant or dead.
  According to Wikipedia (https://en.wikipedia.org/wiki/MinGW)
  "MinGW migrated to OSDN". See link section below.

Launch the installer and follow the instructions. In the "Select Components"
dialog, add "C++ Compiler", "MSYS Basic System", and "MinGW Developer Toolkit".
Wait for the installer to finish.

After downloading and installing, you need to launch the MinGW Shell through
the Start menu.


 Downloading and Unpacking
---------------------------

Download FLTK from here:

  https://www.fltk.org/software.php

into your home folder. The default location as seen from Windows is similar
to

  C:\MinGW\msys\1.0\home\matt\

If you are familiar with "git" and like to stay current with your
version, you will find the git access parameters at the bottom of
that page. Unpack FLTK into a convenient location. I like to have everything
in my dev directory:

  cd
  mkdir dev
  cd dev
  tar xvzf fltk-1.x.y-source.tar.gz
  cd fltk-1.x.y


 Configuring FLTK
------------------

If you got FLTK via git then you need one extra step. Otherwise skip
over this part. Stay in your FLTK source-code directory and type the
following:

  autoconf

Now configure your FLTK installation:

  ./configure

Hint: Instead of executing `autoconf` and `configure` followed by `make`
to build FLTK (see next section) you can also run `make` directly which
will create and execute the 'configure' script with default parameters
and build FLTK with the default configuration.

ADVANCED: type "./configure --help" to get a complete list of optional
configuration parameters. These should be pretty self-explanatory. Some
more details can be found in README.
:END_ADVANCED

The configuration script will check your machine for the required resources
which should all have been part of your MinGW installation. Review the
Configuration Summary, maybe take some notes.

ADVANCED: some versions of MinGW/Msys are broken and complain about a missing
--enable-auto-import. The solution is to upgrade to the current release. If
that is not possible, you can include the --enable-auto-import flag when
linking:
  ./configure <config flags> LDFLAGS=-Wl,--enable-auto-import
:END_ADVANCED


Known Problems:

  There is a known incompatibility with some Windows git tools that
  may not set the correct line endings for autoconf. If you get strange
  error messages when running ./configure or make, you may need to convert
  configh.in to "Unix line endings" (LF-only). These error messages are
  unspecific, e.g. compilation errors like:

     error: 'U32' does not name a type
     error: 'bmibuffer' was not declared in this scope

  You can fix the line endings with the MinGW/msys tool 'unix2dos' (u2d)
  or with your favorite editor, if it allows to change the line endings,
  then run autoconf and ./configure again.

  We don't know if this issue is still relevant with current Git tools.
  It has been reported when we were still using Subversion (svn).

  For further information see this bug report (regarding svn)

    https://www.fltk.org/newsgroups.php?gfltk.bugs+v:10197


 Building FLTK
---------------

Now this is easy. Stay in your FLTK source-code directory and type:

  make

The entire FLTK toolkit including many test programs will be built for you.
No warnings should appear.


 Testing FLTK
--------------

After a successful build, you can test FLTK's capabilities:

  test/demo


 Installing FLTK
-----------------

If you did not change any of the configuration settings, FLTK will be
installed in "/usr/local/include" and "/usr/local/lib" by typing

  make install

It is possible to install FLTK in user space by changing the installation path
to a location within the user account by adding the "--prefix=PREFIX" parameter
to the "./configure" command.


 Creating new Projects
-----------------------

FLTK provides a neat script named "fltk-config" that can provide all the flags
needed to build FLTK applications using the same flags that were used to build
the library itself. Running "fltk-config" without arguments will print a list
of options. The easiest call to compile an FLTK application from a single
source file is:

  fltk-config --compile myProgram.cxx

"fltk-config" and "fluid" will be installed in "/usr/local/bin/" by default.
We recommend that you add it to the command search path.



  3 HOW TO BUILD FLTK USING MICROSOFT VISUAL STUDIO
=====================================================


 Prerequisites
---------------

In order to build FLTK from within Visual Studio, you need to install the
Visual C++ developer environment from the Microsoft web site. The "Express"
or "Community" edition is free of charge and sufficient to develop FLTK
applications:

  https://visualstudio.microsoft.com/free-developer-offers/

If you intend to use an older (maybe commercial) version you need at least
a version that is supported by the version of CMake you are using to generate
the project files. You should make sure that all available service packs are
installed or building FLTK may fail.

As of Juli 2017 the FLTK team recommend at least Visual Studio 2008 with
current service packs. Visual Studio 2008, 2010, 2013, 2015, and 2017 are
known to work with FLTK 1.4.0 (Git master, as of Juli 2017).

For development of FLTK 1.4 or higher Visual Studio 2019 Community or later
versions are highly recommended.

You may also need to install CMake (cmake-gui) from:

  https://cmake.org/download/

Visual Studio 2017 (and later) has internal CMake support (so you may not
need to install CMake separately).
There is an option to "open a folder" with a CMakeLists.txt file - in our
case the FLTK root folder. You may want to try this.

Note that this has not yet been tested thoroughly by the FLTK team.


 Downloading and Unpacking FLTK
--------------------------------

Download FLTK from here:

  https://www.fltk.org/software.php

If you are familiar with "git" and like to stay current with your version,
you will find the git access parameters at the bottom of that page.

Unpack FLTK by using an appropriate unpacker and copy the new folder into a
convenient location, for instance a "dev" folder in your home folder.


 Configuring FLTK
------------------

Note: Configuration with Visual Studio 2017's internal CMake support is
not yet included in this document. You may try yourself...

Please refer to README.CMake.txt for how to configure FLTK with CMake.

Once you have followed the instructions you should have created a new
build directory with the Visual Studio Solution (project files) for FLTK.

Launch Visual Studio and open the project file (FLTK.sln) or double-click
on FLTK.sln in the Windows Explorer.

Choose "Debug" or "Release" mode from the "Solution Configurations" menu.


 Building FLTK
---------------

Use the context menu of the "demo" project to "Set as StartUp Project". Then
select "Build Solution" from the "Build" menu or press F7 to build all
libraries.


 Testing FLTK
--------------

Select "Start Debugging" from the "Debug" menu or just press F5 to run the
Demo program. Use "Demo" to explore all test programs.


 Installing FLTK
-----------------

********************************************************************************
   The information in this chapter is NO LONGER RECOMMENDED by the FLTK team.
********************************************************************************

The default location for VisualC 2008 libraries and headers is here:

  C:\Program Files\Microsoft Visual Studio 9.0\VC\

It is possible to move the FLTK libraries, headers, and Fluid into the
respective subdirectories, so that they are available for future development
without adding link and include paths to the solution.

  copy the entire FL directory into the include path

  add <build_dir>/FL/fl_config.h

  copy all .lib files from the fltk build directory to the VC lib directory

  copy fluid.exe in the fluid directory to the bin directory

We highly discourage using dll's (dynamically linking libraries) on Windows
because they will require an installation process and likely cause version
conflicts. Use the static .lib libraries instead.


 Creating new Projects
-----------------------

********************************************************************************
   The information in this chapter is NO LONGER RECOMMENDED by the FLTK team.
********************************************************************************

This chapter assumes that libraries and headers were copied into

  C:\Program Files\Microsoft Visual Studio 9.0\VC\

Create a new project of type "General", "Empty Project" and add a simple "C++"
file to it. The FLTK "hello" source code is a good base.

Now open the Project Properties dialog and add "Comctl32.lib" and all the FLTK
libraries that you want to use (at least "fltk.lib") to Additional Dependencies
(Configuration Properties > Linker > Additional Dependencies).

Compile and run your test program with F5.

You can also include .fl resources: add a new Header file to your project, but
let the name end in .fl. Right-click and select "Open with...". Add "fluid.exe"
from the "bin" directory and set it as the default editor.

To automatically compile .fl files, open the Properties editor and set the
Custom Build Steps to:

  Command Line: fluid.exe -c $(InputPath)
  Description: Compiling Fluid .fl file
  Outputs: $(InputDir)$(InputName).cxx; $(InputDir)$(InputName).h

Now add the generated .cxx file to your project as well. Whenever the .fl file
is changed, the corresponding .cxx file will be recompiled.



  4 FREQUENTLY ASKED QUESTIONS
================================


 Why does a console window appear when I run my program?
---------------------------------------------------------

Windows has a flag that determines whether an application
runs in the foreground with a console or in the background
without a console.

If you're using gcc (i.e. MinGW or Cygwin), then use the
linker option "-mwindows" to make your application run in
the background and "-mconsole" to run in the foreground. Use
fltk-config --ldflags to see appropriate linker flags, or use
fltk-config --compile to compile a single source file.

If you're using MS VC++, then you must set the linker option
"/subsystem:windows" to create a "Windows" program (w/o console
window), or set the linker option "/subsystem:console" for a
console program, i.e. with a console window. These options
are set differently in the FLTK project files, depending on
whether you select a "Debug" or "Release" build.

Other compilers and build systems may have different options.

Keep in mind that a windows application cannot send output
to stdout, even if you run it from an existing console
application.
(Note: A special case of this exists if running a MinGW
application from the command line of an MSYS shell, when an
application is able to write to stdout, even if compiled with
"-mwindows".  The same applies to Cygwin.)


 How do I get OpenGL to work?
------------------------------

Both builds should automatically support OpenGL.

The configuration file config.h has a number of settings
which control compile-time compilation.  One such setting is
"HAVE_GL". This may be set to 0 to disable Open GL operation.
Changing the line in config.h to

    #define HAVE_GL 1

will change this to compile and link in OpenGL.



  5 LINKS
===========

The following links may be of use:

  1. Cygwin Homepage:

        https://www.cygwin.com/


  2. MinGW Homepage - see Wikipedia:

        https://en.wikipedia.org/wiki/MinGW (English)
        https://de.wikipedia.org/wiki/MinGW (German, see links)

    --------------------------------------------------------------------
    IMPORTANT: As of Dec 07, 2024 the links below could not be verified:
    --------------------------------------------------------------------

        Repository :  https://osdn.net/projects/mingw/scm/
        Website (1):  https://osdn.net/projects/mingw/
        Website (2):  https://mingw.osdn.io/


  3. MSYS2/Mingw-w64 Wikipedia and Homepage:

        https://en.wikipedia.org/wiki/Mingw-w64
        https://mingw-w64.org/
        https://www.msys2.org/


  4. Check out the FLTK newsgroups at the FLTK homepage:

        https://www.fltk.org/

     Its archival search facilities are EXTREMELY useful
     to check back through previous problems with this
     sort of configuration before posting new questions.


  5. GNU Compiler Collection (GCC) compiler homepage:

        https://gcc.gnu.org/


  6. OpenGL page - for OpenGL and GLUT libs

        https://www.opengl.org/


  7. CMake homepage:

        https://cmake.org/


  Note 1: all links in this document have been checked and verified
    on Dec 07, 2024 except where noted above.

  Note 2: We can't guarantee that these links will be valid any time later.


  6 DOCUMENT HISTORY
======================

  The document history is no longer maintained in this document.
  Please consult the Git history instead.
  Examples:
    git log -- README.Windows.txt
    git log -- README.MSWindows.txt     (previous, renamed version)
    gitk -- README.Windows.txt
