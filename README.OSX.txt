README.OSX.txt - 2019-01-06 - Building FLTK under macOS
-------------------------------------------------------



 CONTENTS
==========

  1   INTRODUCTION
  2   HOW TO BUILD FLTK USING GCC
    2.1   Prerequisites
    2.2   Downloading and Unpacking
    2.3   Configuring FLTK
    2.4   Building FLTK
    2.5   Testing FLTK
    2.6   Installing FLTK
    2.7   Creating new Projects
  3   HOW TO BUILD FLTK USING XCODE3
    3.1   Prerequisites
    3.2   Downloading and Unpacking
    3.3   Configuring FLTK
    3.4   Building FLTK
    3.5   Testing FLTK
    3.6   Uninstalling previous versions of FLTK
    3.7   Installing FLTK
    3.8   Installing Little Helpers
    3.9   Creating new Projects
  4   HOW TO BUILD FLTK USING XCODE4
    4.1   Prerequisites
    4.2   Downloading and Unpacking
    4.3   Configuring FLTK
    4.4   Building FLTK
    4.5   Testing FLTK
    4.6   Uninstalling previous versions of FLTK
    4.7   Installing FLTK
  5  MAKE AN APPLICATION LAUNCHABLE BY DROPPING FILES ON ITS ICON
  6   DOCUMENT HISTORY


 1  INTRODUCTION
=================

FLTK supports all Mac OS X versions above 10.3 (Panther). See below for how to
build FLTK applications that can run on all (old or recent) Mac OS X versions.

FLTK currently supports the following development environments on the Apple OS X
platform:

    - gcc command line tools
    - Xcode 3.x and higher

CAUTION: gcc command line built libraries and Xcode created Frameworks should
not be mixed!


 2  HOW TO BUILD FLTK USING GCC
================================


 2.1  Prerequisites
--------------------

In order to build FLTK from the command line, you need to install the Xcode
developer environment. It can be downloaded from the Apple Store for free.

After downloading and installing, you need to launch the Terminal. Terminal.app
is located in the "Utilities" folder inside the "Applications" folder. I like
to keep the Terminal in the Dock.


 2.2  Downloading and Unpacking
--------------------------------

Download FLTK from here:

  https://www.fltk.org/software.php

If you are familiar with "git" and like to stay current with your
version, you will find the git access parameters at the bottom of that
page. Unpack FLTK into a convenient location. I like to have everything in my
dev directory:

  cd
  mkdir dev
  cd dev
  mv ~/Downloads/fltk-1.3.xxxx.tar.gz .
  tar xvfz fltk-1.3.xxxx.tar.gz
  cd fltk-1.3.xxxx

If you got FLTK via git then you need one extra step: build the
configure script. Otherwise skip the following part marked ADVANCED:

ADVANCED: (only if you got FLTK via git)
- The preferred procedure is to build the configure script using tools
  called autoconf and automake. If these tools are not both available
  on your system, this post
     https://www.fltk.org/newsgroups.php?s15486+gfltk.coredev+v15499
  details how to get them.

  Then, stay in your FLTK source-code directory and type the following:

  NOCONFIGURE=1 ./autogen.sh

- Alternatively, copy the configure script from the last FLTK weekly snapshot
  to your git source-code directory.

:END_ADVANCED


 2.3  Configuring FLTK
-----------------------


Now configure your FLTK installation: stay in your FLTK source-code directory
and type

  ./configure

CAVEAT: if configure sends this error message :
   configure: error: cannot run /bin/sh ./config.sub
type
  make
and interrupt the building process. Then, run ./configure again.
:END CAVEAT

ADVANCED: type "./configure --help" to get a complete list of optional
configurations parameters. These should be pretty self-explanatory. Some
more details can be found in README.

To create multi-architecture binaries, start "configure" with these flags:
  ./configure  --with-archflags="-arch i386 -arch x86_64"

To create applications that can run under Mac OS X 10.4 and above, use these flags:
  ./configure CXXFLAGS="-mmacosx-version-min=10.4" LDFLAGS="-mmacosx-version-min=10.4"

:END_ADVANCED

The configuration script will check your machine for the required resources
which should all have been part of your Xcode installation. Review the
Configuration Summary, maybe take some notes.


 2.4  Building FLTK
--------------------

Now this is easy. Stay in your FLTK source-code directory and type:

  make

The entire FLTK toolkit including many test programs will be built for you. No
warnings should appear, but "ranlib" may complain about a few modules having no
symbols. This is normal and can safely be ignored.


 2.5  Testing FLTK
-------------------

After a successful build, you can test FLTK's capabilities:

  test/demo


 2.6  Installing FLTK
----------------------

If you did not change any of the configuration settings, FLTK will be installed
in "/usr/local/include" and "/usr/local/lib" by typing

  sudo make install

It is possible to install FLTK without superuser privileges by changing the
installation path to a location within the user account by adding the
"--prefix=PREFIX" parameter to the "./configure" command.


 2.7  Creating new Projects
----------------------------

FLTK provides a neat script named "fltk-config" that can provide all the flags
needed to build FLTK applications using the same flags that were used to build
the library itself. Architecture flags (e.g., -arch i386) used to build the
library, though, are not provided by the fltk-config script. This allows to
build universal libraries and to produce applications of any architecture
from them. Running "fltk-config" without arguments will print a list
of options. The easiest call to compile an FLTK application from a single source
file is:

  fltk-config --compile myProgram.cxx

"fltk-config" and "fluid" will be installed in "/usr/local/bin/" by default. I
recommend that you add it to the command search path.



 3  HOW TO BUILD FLTK USING XCODE3
===================================


 3.1  Prerequisites
--------------------

In order to build FLTK from within Xcode 3, you need to have the Xcode 3
developer environment on your computer. If you don't, go to: 'HOW TO BUILD
FLTK USING XCODE4'


 3.2  Downloading and Unpacking
--------------------------------

Download FLTK from here:

  https://www.fltk.org/software.php

If you are familiar with "git" and like to stay current with your
version, you will find the git access parameters at the bottom of that
page. You can use the SCM system that is built into Xcode.

Unpack FLTK by double-clicking it and copy the new folder into a convenient
location. I have set up a "dev" folder in my home folder for all my projects.


 3.3  Configuring FLTK
-----------------------

Launch Xcode. Open the project file in

  .../fltk-1.3.xxxx/ide/Xcode4/FLTK.xcodeproj

Use the "Project" pulldown menu to change "Active Build Configuration" to
"Release". Change the "Active Architecture"  as desired.


 3.4  Building FLTK
--------------------

Use the "Project" pulldown menu to set the "Active Target" to "Demo". Select
"Build" from the "Build" menu to create all libraries and test applications.

By default, the Xcode4 project builds applications that run under Mac OS X 10.5
and above. To build applications that also run under older Mac OS versions,
select "Edit Project Settings" of the Project menu, then select the Build panel,
and modify the "Mac OS X Deployment Target" item.

All frameworks and apps will be located in "./ide/Xcode4/build/Release/".


 3.5  Testing FLTK
-------------------

Select "Build and Run" from the "Build" menu to run the Demo program. Use "Demo"
to explore all test programs.


 3.6  Uninstalling previous versions of FLTK
---------------------------------------------

Remove FLTK frameworks:

  sudo rm -r /Library/Frameworks/fltk*.framework

Remove Fluid and possibly other utilities:

  sudo rm -r /Developer/Applications/Utilities/FLTK/


 3.7  Installing FLTK
----------------------

When distributing FLTK applications, the FLTK frameworks should be made part of
the application package. For development however, it is very convenient to have
the Release-mode Frameworks in a standard location.

For Xcode project template use, all FLTK frameworks should be copied from
"./ide/Xcode4/build/Release/" to "/Library/Frameworks/". The FLTK header files
for all FLTK frameworks will then be at "/Library/Frameworks/fltk.framework/
Headers/". Add this path to the header search path of your projects.

  sudo rm -f -r /Library/Frameworks/fltk*
  sudo cp -R ide/Xcode4/build/Release/fltk*.framework /Library/Frameworks/

Many FLTK applications will use Fluid, the FLTK User Interface builder, to
generate C++ source code from .fl resource files. Add Fluid to the developer
tools:

  sudo mkdir /Developer/Applications/Utilities/FLTK/
  sudo rm -f -r /Developer/Applications/Utilities/FLTK/fluid.app
  sudo cp -R ide/Xcode4/build/Release/fluid.app /Developer/Applications/Utilities/FLTK/


 3.8  Installing Little Helpers
--------------------------------


- Project Templates:

Project Templates are the quickest way to create a new FLTK application from
within Xcode. The included project builds an FLTK based Cocoa application
written in C++ with support for the Fluid UI designer, image reading, and
OpenGL. Unused FLTK sub-Frameworks can simply be removed from the project.
The template assumes that Release versions of the FLTK frameworks are installed
in /Library/Frameworks as described above.

First, we need to create the Template folder:

  sudo mkdir -p /Library/Application\ Support/Developer/Shared/Xcode/Project\ Templates/

Next, we copy the project template over:

  sudo cp -r ide/Xcode4/Project\ Templates/* /Library/Application\ Support/Developer/Shared/Xcode/Project\ Templates/

After restarting Xcode, the dialog for "File > New Project..." will offer an
FLTK 1.3 user template which is ready to compile.


- Fluid file handling

This section assumes that a Release version of Fluid is installed in
"/Developer/Applications/Utilities/FLTK/" as described above. It will install
a new file type which opens Fluid as an editor for files ending in ".fl".

First, we need to create the spec folder:

  sudo mkdir -p /Library/Application\ Support/Developer/Shared/Xcode/Specifications/

Next, we copy the Fluid specification over:

  sudo cp ide/Xcode4/fluid.pbfilespec /Library/Application\ Support/Developer/Shared/Xcode/Specifications/

Open Xcode preferences and select the File Types tab. Find the
"sourcecode.fluid" entry in "file > text > sourcecode" and set the external
editor to Fluid. When adding ".fl" files, set the File Type in the Info dialog
to "sourcecode.fluid" and Xcode will edit your file in Fluid when
double-clicking.


- More

TODO: Language Definition
TODO: Build Rules


 3.9  Creating new Projects
----------------------------

If the little helpers above were installed, the menu "File > New Project..."
will pop up a dialog that offers a User Template named Fluid. Select it and
follow the instructions.



 4  HOW TO BUILD FLTK USING XCODE4
===================================


 4.1  Prerequisites
--------------------

In order to build FLTK from within Xcode 4, 5, 6, 7, or 8 you need to install
the Xcode developer environment via the Apple App Store that comes with
Lion and up. If you also want the command line version of gcc, you can use
the Download section in the Preferences dialog.


 4.2  Downloading and Unpacking
--------------------------------

Download FLTK from here:

  https://www.fltk.org/software.php

If you are familiar with "git" and like to stay current with your
version, you will find the git access parameters at the bottom of that
page. You can use the SCM system that is built into Xcode.

Unpack FLTK by double-clicking it and copy the new folder into a convenient
location. I have set up a "dev" folder in my home folder for all my projects.


 4.3  Configuring FLTK
-----------------------

Launch Xcode. Open the project file in

  .../fltk-1.3.xxxx/ide/Xcode4/FLTK.xcodeproj

There is nothing else to configure.


 4.4  Building FLTK
--------------------

Use the "Scheme" pulldown menu to change the active target to "Demo" and
"My Mac 32-bit" or "My Mac 64-bit". Select "Build for" -> "Running"Run" from
the "Product" menu to create all libraries and test applications.

By default, the Xcode4 project builds applications that run under Mac OS X 10.5
and above. To build applications that also run under older Mac OS versions,
modify the "OS X Deployment Target" item of the FLTK project build settings.

All frameworks and apps will be located in a private directory. Use
"Window"->"Organizer" to find the full path.


 4.5  Testing FLTK
-------------------

Select "Run" from the "Product" menu to run the Demo program. Use "Demo"
to explore all test programs.


 4.6  Uninstalling previous versions of FLTK
---------------------------------------------

Remove FLTK frameworks:

  sudo rm -r /Library/Frameworks/fltk*.framework

Remove Fluid and possibly other utilities:

  sudo rm -r /Applications/FLTK/


 4.7  Installing FLTK
----------------------

When distributing FLTK applications, the FLTK frameworks should be made part of
the application package. For development however, it is very convenient to have
the Release-mode Frameworks in a standard location.

For Xcode project template use, all FLTK frameworks should be built using
"Build for Archiving" and then copied from
"./(Organizer->Derived Data Path)/Release/" to "/Library/Frameworks/".
The FLTK header files for all FLTK frameworks will then be at
"/Library/Frameworks/fltk.framework/Headers/". Add this path to the header
search path of your projects.

  sudo rm -f -r /Library/Frameworks/fltk*
  sudo cp -R (Organizer->Derived Data Path)/Release/fltk*.framework /Library/Frameworks/

Many FLTK applications will use Fluid, the FLTK User Interface builder, to
generate C++ source code from .fl resource files. Add Fluid to the developer
tools:

  sudo mkdir /Applications/FLTK/
  sudo rm -f -r /Applications/FLTK/fluid.app
  sudo cp -R (Organizer->Derived Data Path)/Release/fluid.app /Applications/FLTK/

    (TODO: 4.8   Installing Little Helpers)
    (TODO: 4.9   Creating new Projects)



  5  MAKE AN APPLICATION LAUNCHABLE BY DROPPING FILES ON ITS ICON
=================================================================
- Prepare an Info.plist file for your application derived from file
ide/Xcode4/plists/editor-Info.plist which allows any file to be dropped
on the application icon.
You can edit this file in Xcode and change
Document types/Item 0/CFBundleTypeExtensions/Item 0
from the current "*" to the desired file extension. Use several items to
declare several extensions.

- Call fl_open_callback() at the beginning of your main() function that sets
what function will be called when a file is dropped on the application icon.

- In Xcode, set the "Info.plist File" build setting of your target application
to the Info.plist file you have prepared.

- Rebuild your application.


 6  DOCUMENT HISTORY
=====================

Oct 29 2010 - matt: removed warnings
Oct 24 2010 - matt: restructured entire document and verified instructions
Dec 19 2010 - Manolo: corrected typos
Dec 29 2010 - Manolo: removed reference to AudioToolbox.framework that's no longer needed
Feb 24 2011 - Manolo: architecture flags are not propagated to the fltk-config script.
Apr 17 2012 - matt: added Xcode4 documentation
Nov 13 2012 - Manolo: added "MAKE AN APPLICATION LAUNCHABLE BY DROPPING FILES ON ITS ICON"
Apr 28 2014 - Manolo: how to build programs that run on various Mac OS X versions
Mar 18 2015 - Manolo: removed uses of the Xcode3 project
Apr 01 2016 - AlbrechtS: corrected typo, formatted most line breaks < 80 columns
