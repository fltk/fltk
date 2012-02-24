README.MSWindows.txt - 2010-10-25 - Building FLTK under Microsoft Windows
-------------------------------------------------------------------------



 CONTENTS
==========

  1   INTRODUCTION
  2   HOW TO BUILD FLTK USING MinGW/Cygwin
    2.1   The Tools
    2.2   Recommended Command Line Build Environment
    2.3   Prerequisites
    2.4   Downloading and Unpacking
    2.5   Configuring FLTK
    2.6   Building FLTK
    2.7   Testing FLTK
    2.8   Installing FLTK
    2.9   Creating new Projects
  3   HOW TO BUILD FLTK USING VISUAL STUDIO 2008
    3.1   Prerequisites
    3.2   Downloading and Unpacking
    3.3   Configuring FLTK
    3.4   Building FLTK
    3.5   Testing FLTK
    3.6   Installing FLTK
    3.7   Creating new Projects
  4   HOW TO BUILD FLTK USING VISUAL STUDIO 2010
    4.1   Prerequisites
    4.2   Downloading and Unpacking
    4.3   Configuring FLTK
    4.4   Building FLTK
    4.5   Testing FLTK
    4.6   Installing FLTK
    4.7   Creating new Projects
  5   FREQUENTLY ASKED QUESTIONS
  7   LINKS
  6   DOCUMENT HISTORY


 INTRODUCTION
==============

FLTK 1.3 and later is officially supported on Windows (2000,) 2003,
XP, and later.  Older Windows versions are not officially supported,
but may still work.  The main reason is that the OS version needs
to support UTF-8. FLTK 1.3 is known to work on Windows 7 and Vista.

FLTK currently supports the following development
environments on the Windows platform:

    - Free Microsoft Visual C++ 2008 Express and Visual
      C++ 2010 Express using the supplied workspace and
      project files. Older and the commercial versions can
      be used as well, if they can open the project files.
      Be sure to get your service packs!

      The project files can be found in the ide/ directory.
      Please read ide/README.IDE for more info about this.

    - GNU toolsets (Cygwin or MinGW) hosted on Windows.

CAUTION: Libraries built by any one of these environments can not be mixed
with object files from any other environment!


 HOW TO BUILD FLTK USING MinGW and Cygwin
==========================================

This chapter of this document gives a brief overview of
compiling and using FLTK with the Cygwin and MinGW compiler
toolkits.  Both toolkits provide a build environment based
around the GNU C/C++ compiler. Further information is
available from the FLTK website at http://www.fltk.org, such
as this Howto note: http://www.fltk.org/articles.php?L598

The Cygwin build environment supplies a library (the Cygwin
DLL) that is primarily intended to provide a number of
Unix-like POSIX facilities for programs being ported to the
Windows environment (Win32 or WinNT).  Cygwin also supplies
a very Unix-like build environment for Windows, including
the "BASH" Bourne-compatible shell and all of the standard
Unix file utilities (ls, cat, grep, etc.).

Cygwin is developed by Cygnus (now part of RedHat, Inc).
Although provided for free download under the GPL,
distributing programs that require the Cygwin DLL under a
license other than the GPL requires a commercial license for
the Cygwin DLL.  Native Windows programs that do not require
the Cygwin DLL (compiled and linked with the "-mno-cygwin"
option) may be released under any license freely.

Note: Since December 2009, there is a new gcc 4.x compiler
that doesn't support the -mno-cygwin option anymore. You
must use the older gcc-3 compiler instead.

An alternative is to install the new (since about Oct. 2010)
mingw cross tools that support newer gcc compilers for building
native Windows applications (like -mno-cygwin above).
Currently you would have to install mingw64-i686-gcc-g++ for
32-bit Windows applications (despite its name!), and/or
mingw64-x86_64-gcc-g++ for 64-bit applications. You may also
need to install the corresponding '-headers' packages as well.
Currently these tools support gcc 4.5.x or newer, but the
setup for FLTK is somewhat more complicated and not yet
completely supported automatically (you may need to edit
some lines in the generated makeinclude file).

The MinGW distribution (Minimalist GNU for Windows) provides
a similar toolset but geared solely towards native Windows
development without the Unix-like POSIX library.  The lack of
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

There are currently three main configurations supported by
FLTK with the GNU tools:

    1. Cygwin: Built using the Cygwin toolset and using the
       Unix-like POSIX compatibility layer provided by the
       Cygwin DLL.

    2. Cygwin using the "-mno-cygwin" option: Built using
       the Cygwin toolset but not using the Cygwin DLL.

    3. MinGW: Built using the MinGW utilities, compiler and
       tools. This is, in many aspects, analogous to the
       Cygwin "-mno-cygwin" option. This is the recommended
       one if you want to build native Windows programs only.


 Recommended Command Line Build Environment
--------------------------------------------

Our recommendation is to:

    1. Get the current Cygwin toolset.

       This can either produce executables that do or do not
       rely on the Cygwin DLL (check licensing) at your
       choice.

    2. Get the latest MinGW toolset. It is recommended that
       you also get the MSYS shell and the msysDTK developer
       toolset.

       This will only produce normal Windows native
       executables without any Unix or POSIX compatibility
       layer.


       See the links section below for more information.

Either option can generate windows-native executables and
option 1 can provide a Unix-like POSIX portability layer that
is reliant on a GPLed library.

See the later sections for detailed information about using
one of these configurations.


 Prerequisites
---------------

In order to build FLTK from the command line, you need to install the MinGW
environment from www.mingw.org. The graphical installer "mingw-get-inst" can
be downloaded here for free:

  http://www.mingw.org/wiki/Getting_Started

Launch the installer and follow the instructions. In the "Select Components"
dialog, add "C++ Compiler", "MSYS Basic System", and "MinGW Developer Toolkit".
Wait for the installer to finish.

After downloading and installing, you need to launch the MinGW Shell through
the Start menu.


 Downloading and Unpacking
---------------------------

Download FLTK from here:

  http://www.fltk.org/software.php

into your home folder. The default location as seen from MSWindows is similar
to

  C:\MinGW\msys\1.0\home\matt\

If you are familiar with "subversion" and like to stay current with your
version, you will find the subversion access parameters at the bottom of
that page. Unpack FLTK into a convenient location. I like to have everything
in my dev directory:

  cd
  mkdir dev
  cd dev
  tar xvfz fltk-1.3.xxxx.tar.gz
  cd fltk-1.3.xxxx


 Configuring FLTK
------------------

Stay in your FLTK source-code directory. Type:

  autoconf

Now configure your FLTK installation:

  ./configure

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


 Building FLTK
---------------

Now this is easy. Stay in your FLTK source-code directory and type:

  make

The entire FLTK toolkit including many test programs will be built for you.
No warnings should appear.

(actually, as of Oct 25 2010, quite a lot of warnings related to suggested
parentheses and others will appear, this is normal and will be fixed. The
linker will also spit out a bunch of warnings for every program linked. This
needs to be fixed. Lastly, there is no generator for man pages in a default
MinGW installation, but you can install man and groff to fix this.)


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
I recommend that you add it to the command search path.



 HOW TO BUILD FLTK USING VISUAL STUDIO 2008
============================================


 Prerequisites
---------------

In order to build FLTK from within VisualStudio 2008, you need to install the
VisualC developer environment from the Microsoft web site. The Express edition
is free of charge and sufficient to develop FLTK applications:

  http://www.microsoft.com/express/Downloads/

You must make sure that at least VisualStudio 2008 Service Pack 1 is installed
or building FLTK on a multicore CPU will be very painful!


 Downloading and Unpacking
---------------------------

Download FLTK from here:

  http://www.fltk.org/software.php

If you are familiar with "subversion" and like to stay current with your
version, you will find the subversion access parameters at the bottom of
that page.

Unpack FLTK by using an appropriate unpacker and copy the new folder into a
convenient location. I have set up a "dev" folder in my home folder for all
my projects.


 Configuring FLTK
------------------

Launch VisualStudio. Open the project file in

  ...\fltk-1.3.xxxx\ide\VisualC2008\fltk.sln

Choose "Debug" or "Release" mode from the "Solution Configurations" menu.


 Building FLTK
---------------

Use the context menu of the "demo" project to "Set as StartUp Project". Then
select "Build Solution" from the "Build" menu or press F7 to build all
libraries.

VisualC 2008 has a bug that messes up building a Solution on multicore CPUs.
Make sure that Visual Studio 2008 Service Pack 1 is installed or, as a
workaround, set the "maximum number of parallel project builds" to 1 (Tools >
Options > Projects and Solutions > Build and Run > maximum number of parallel
project builds). Also, repeating the build command two or three times may
clear unresolved reference errors.


 Testing FLTK
--------------

Select "Start Debugging" from the "Debug" menu or just press F5 to run the
Demo program. Use "Demo" to explore all test programs.


 Installing FLTK
-----------------

The default location for VisualC 2008 libraries and headers is here:

  C:\Program Files\Microsoft Visual Studio 9.0\VC\

It is possible to move the FLTK libraries, headers, and Fluid into the
respective subdirectories, so that they are available for future development
without adding link and include paths to the solution.

  copy the entire FL directory into the include path

  copy all .lib files from the fltk lib directory to the VC lib directory

  copy fluid.exe in the fluid directory to the bin directory

I highly discourage using dll's (dynamically linking libraries) on MSWindows
because they will require an installation process and likely cause version
conflicts. Use the static .lib libraries instead.


 Creating new Projects
-----------------------

This chapter assumes that libraries and headers are copied into

  C:\Program Files\Microsoft Visual Studio 9.0\VC\

Create a new project of type "General", "Empty Project" and add a simple "C++"
file to it. The FLTK "hello" source code is a good base.

Now open the Project Properties dialog and add "Comctl32.lib" and all the FLTK
libraries that you want to use (at least "fltk.lib") to Additional Dependencies
(Configuration Properties > Linker > Additional Dependencies). In the same
dialog, add "WIN32" to the C++ Preprocessor Definitions (Configuration
Properties > C/C++ > Preprocessor > Preprocessor Definitions).

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



 HOW TO BUILD FLTK USING VISUAL STUDIO 2010
============================================


 Prerequisites
---------------

In order to build FLTK from within VisualStudio 2010, you need to install the
VisualC developer environment from the Microsoft web site. The Express edition
is free of charge and sufficient to develop FLTK applications:

  http://www.microsoft.com/express/Downloads/


 Downloading and Unpacking
---------------------------

Download FLTK from here:

  http://www.fltk.org/software.php

If you are familiar with "subversion" and like to stay current with your
version, you will find the subversion access parameters at the bottom of
that page.

Unpack FLTK by using an appropriate unpacker and copy the new folder into a
convenient location. I have set up a "dev" folder in my home folder for all
my projects.


 Configuring FLTK
------------------

Launch VisualStudio. Open the project file in

  .../fltk-1.3.xxxx/ide/VisualC2010/fltk.sln

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

The default location for VisualC 2010 libraries and headers is here:

  C:\Program Files\Microsoft Visual Studio 10.0\VC\

It is possible to move the FLTK libraries, headers, and Fluid into the
respective subdirectories, so that they are available for future development
without adding link and include paths to the solution.

  copy the entire FL directory into the include path

  copy all .lib files from the fltk lib directory to the VC lib directory

  copy fluid.exe in the fluid directory to the bin directory

I highly discourage using dll's (dynamically linking libraries) on MSWindows
because they will require an installation process and likely cause version
conflicts. Use the static .lib libraries instead.


 Creating new Projects
-----------------------

This chapter assumes that libraries and headers are copied into

  C:\Program Files\Microsoft Visual Studio 10.0\VC\

Create a new project of type "General", "Empty Project" and add a simple "C++"
file to it. The FLTK "hello" source code is a good base.

Now open the Project Properties dialog and add "Comctl32.lib" and all the FLTK
libraries that you want to use (at least "fltk.lib") to Additional Dependencies
(Configuration Properties > Linker > Additional Dependencies). In the same
dialog, add "WIN32" to the C++ Preprocessor Definitions (Configuration
Properties > C/C++ > Preprocessor > Preprocessor Definitions).

Compile and run your test program with F5.

You can also include .fl resources: add a new Header file to your project, but
let the name end in .fl. Right-click and select "Open with...". Add "fluid.exe"
from the "bin" directory and set it as the default editor.

To automatically compile .fl files, open the Properties editor and change the
Element Type to Custom Build and click Apply. Now set the
Custom Build Steps to:

  Command Line: fluid.exe -c %(FullPath)
  Description: Compiling Fluid .fl file
  Outputs: $(InputDir)$(InputName).cxx; $(InputDir)$(InputName).h

Now add the generated .cxx file to your project as well. Whenever the .fl file
is changed, the corresponding .cxx file will be recompiled.



 FREQUENTLY ASKED QUESTIONS
============================


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



 LINKS
=======

The following links may be of use:

1. Main Cygwin homepage:

       http://www.cygwin.com/

2. Main Mingw homepage:

       http://www.mingw.org/

   In particular look for the MinGW FAQ at this link for
   a lot of useful Mingw-native development
   documentation.


3. Check out the FLTK newsgroups at the FLTK homepage:

       http://www.fltk.org/

   Its archival search facilities are EXTREMELY useful
   to check back through previous problems with this
   sort of configuration before posting new questions.

4. GNU Compiler Collection (GCC) compiler homepage:

       http://gcc.gnu.org/

5. OpenGL page - for OpenGL and GLUT libs

       http://www.opengl.org/



 DOCUMENT HISTORY
==================

Oct 25 2010 - matt: restructured entire document and verified instructions
Dec 20 2010 - matt: merged with README.win32
Dec 22 2010 - AlbrechtS: added newer Cygwin (cross/mingw-w64) options
Feb 24 2012 - AlbrechtS: clarified console window FAQ
