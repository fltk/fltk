README.MSWindows.txt - 2010-10-25 - Building FLTK under Microsoft Windows
-------------------------------------------------------------------------



 CONTENTS
========== 

  1   INTRODUCTION
  2   HOW TO BUILD FLTK USING MinGW
    2.1   Prerequisites
    2.2   Downloading and Unpacking
    2.3   Configuring FLTK
    2.4   Building FLTK
    2.5   Testing FLTK
    2.6   Installing FLTK
    2.7   Creating new Projects
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
  5   DOCUMENT HISTORY


 INTRODUCTION
==============

FLTK currently supports the following development environments on the Microsoft
Windows platform:

    - MinGW gnu command line tools
    - CygWin gnu command line tools
    - VisualStudio 2008
    - VisualStudio 2010
    
CAUTION: Libraries built by any of these environments can not be mixed!


 HOW TO BUILD FLTK USING MinGW
===============================


 Prerequisites
---------------

In order to build FLTK from the command line, you need to install the MinGW
environment from www,mingw.org. The graphical installer " mingw-get-inst" can be
downloaded here for free:

  http://www.mingw.org/wiki/Getting_Started

Launch the installer and follow the instructions. In the "Select Components"
dialog, add "C++ Compiler", "MSYS Basic System", and "MinGW Develoepr Toolkit".
Wait for the installer to finish.

After downloading and installing, you need to launch the MinGW Shell through
the Start menu.


 Downloading and Unpacking
---------------------------

Download FLTK from here:

  http://www.fltk.org/software.php

into your home folder. The default location as seen from MSWindows is similar to

  C:\MinGW\msys\1.0\home\matt\
  
If you are familiar with "subversion" and like to stay current with you version,
you will find the subversion access parameters at the bottom of that page.
Unpack FLTK into a convinient location. I like to have everything in my dev 
directory:

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
configurations parameters. These should be pretty self-explenatory. Some
more details can be found in README. 
:END_ADVANCED

The configuration script will check your machine for the required resources
which should all have been part of your MinGW installation. Review the 
Configuration Summary, maybe take some notes.


 Building FLTK
---------------

Now this is easy. Stay in your FLTK source-code directory and type: 

  make

The entire FLTK toolkit including many test programs will be built for you. No 
warnings should appear.

(actually, as of Oct 25 2010, quite a lot of warnings related to suggested
parentheses and others will appear, this is normal and will be fixed. The linker
will also spit out a lbunch of warnings for every program linked. This needs to
be fixed. Lastly, there is no generator for man pages in MinGW)


 Testing FLTK
--------------

After a successful build, you can test FLTK's capabilities:

  test/demo


 Installing FLTK
-----------------

If you did not change any of the configuration settings, FLTK will be installed 
in "/usr/local/include" and "/usr/local/lib" by typing

  make install
  
It is possible to install FLTK in user space by changing the installation path 
to a location within the user account by adding the "--prefix=PREFIX" parameter
to the "./configure" command.


 Creating new Projects
-----------------------

FLTK provides a neat script named "fltk-config" that can provide all the flags 
needed to build FLTK applications using the same flags that were used to build
the library itself. Running "fltk-config" without arguments will print a list
options. The easiest call to compile an FLTK application from a single source 
file is: 

  fltk-config --compile myProgram.cxx

"fltk-config" and "fluid" will be installed in "/usr/local/bin/" by default. I
recommend that you add it to the command search path.



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

If you are familiar with "subversion" and like to stay current with you version,
you will find the subversion access parameters at the bottom of that page.

Unpack FLTK by using an appropriate unpacker and copy the new folder into a 
convenient location. I have set up a "dev" folder in my home folder for all my 
projects.


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

Select "Start Debugging" form the "Debug" menu or just press F5 to run the Demo
program. Use "Demo" to explore all test programs.


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

I highly discourace using dll's (dynamically linking libraries) on MSWindows
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

You can also include .fl resources: add a new Hedare file to you project, but 
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

If you are familiar with "subversion" and like to stay current with you version,
you will find the subversion access parameters at the bottom of that page.

Unpack FLTK by using an appropriate unpacker and copy the new folder into a 
convenient location. I have set up a "dev" folder in my home folder for all my 
projects.


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

Select "Start Debugging" form the "Debug" menu or just press F5 to run the Demo
program. Use "Demo" to explore all test programs.


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

I highly discourace using dll's (dynamically linking libraries) on MSWindows
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

You can also include .fl resources: add a new Hedare file to you project, but 
let the name end in .fl. Right-click and select "Open with...". Add "fluid.exe"
from the "bin" directory and set it as the default editor.

To automatically compile .fl files, open the Properties editor and cahnge the 
Element Type to Custom Build and click Apply. Now set the
Custom Build Steps to:

  Command Line: fluid.exe -c %(FullPath)
  Description: Compiling Fluid .fl file
  Outputs: $(InputDir)$(InputName).cxx; $(InputDir)$(InputName).h

Now add the generated .cxx file to your project as well. Whenever the .fl file 
is changed, the corresponding .cxx file will be recompiled.


 DOCUMENT HISTORY
==================

Oct 25 2010 - matt: restructured entire document and verified instructions
