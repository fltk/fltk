README.OSX.txt - 2010-10-23 - Building FLTK under Apple OS X
------------------------------------------------------------



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
  4   DOCUMENT HISTORY


 INTRODUCTION
==============

FLTK currently supports the following development environments on the Apple OS X
platform:

    - gcc command line tools
    - Xcode 3.x
    
CAUTION: gcc command line built libraries and Xcode created Frameworks should
not be mixed!    


 HOW TO BUILD FLTK USING GCC
=============================


 Prerequisites
---------------

In order to build FLTK from the command line, you need to install the Xcode
developer environment from the Apple Inc. web site. The developer environment
can be downloaded from the Mac Dev Center for free:

  http://developer.apple.com/technologies/xcode.html
  
After downloading and installing, you need to launch the Terminal. Terminal.app
is located in the "Utilities" folder inside the "Applications" folder. I like to
keep the Terminal in the Dock.


 Downloading and Unpacking
---------------------------

Download FLTK from here:

  http://www.fltk.org/software.php

If you are familiar with "subversion" and like to stay current with you version,
you will find the subversion access parameters at the bottom of that page.
Unpack FLTK into a convinient location. I like to have everything in my dev 
directory:

  cd
  mkdir dev
  cd dev
  mv ~/Downloads/fltk-1.3.xxxx.tar.gz .
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

To create Universal Binaries, start "configure" with these flags:
  ./configure  --with-archflags="-arch i386 -arch ppc -arch x86_64"
:END_ADVANCED

The configuration script will check your machine for the required resources
which should all have been part of your Xcode installation. Review the 
Configuration Summary, maybe take some notes.


 Building FLTK
---------------

Now this is easy. Stay in your FLTK source-code directory and type: 

  make

The entire FLTK toolkit including many test programs will be built for you. No 
warnings should appear, but "ranlib" will complain about a few modules having no
symbols. This is normal and can safely be ignored.

(actually, as of Oct 23 2010, a handful of warnings related to string literals
may appear, this is normal and will be fixed)


 Testing FLTK
--------------

After a successful build, you can test FLTK's capabilities:

  test/demo


 Installing FLTK
-----------------

If you did not change any of the configuration settings, FLTK will be installed 
in "/usr/local/include" and "/usr/local/lib" by typing

  sudo make install
  
It is possible to install FLTK without superuser previleges by changing the 
installation path to a location within the user account by adding the 
"--prefix=PREFIX" parameters to the "./configure" command.


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



 HOW TO BUILD FLTK USING XCODE3
================================


 Prerequisites
---------------

In order to build FLTK from within Xcode, you need to install the Xcode
developer environment from the Apple Inc. web site. The developer environment
can be downloaded from the Mac Dev Center for free:

  http://developer.apple.com/technologies/xcode.html
  

 Downloading and Unpacking
---------------------------

Download FLTK from here:

  http://www.fltk.org/software.php

If you are familiar with "subversion" and like to stay current with you version,
you will find the subversion access parameters at the bottom of that page.
Unpack FLTK by double-clicking it and copy the new folder into a convenient
location. I have set up a "dev" folder in my home folder for all my projects.


 Configuring FLTK
------------------

Launch Xcode. Open the project file in 

  .../fltk-1.3.xxxx/ide/Xcode3/FLTK.xcodeproj

Use the "Project" pulldown menu to select the "Active Architecture" and
"Active Build Configuration".
  

 Building FLTK
---------------

Use the "Project" pulldown menu to set the "Active Target" to "Demo". Select
"Build" form the "Build" menu to create all libraries and test applications.

All frameworks and apps will be loacted in "./ide/Xcode3/build/Debug/" or 
"./ide/Xcode3/build/Release/".


 Testing FLTK
--------------

Use the "Project" pulldown menu to set the "Active Target" to "Demo". Select
"Build and Run" form the "Build" menu to run the Demo program. Use Demo to 
explore all test programs.


 Uninstalling previous versions of FLTK
----------------------------------------

Remove FLTK frameworks:

  sudo rm -r /Library/Frameworks/fltk*.framework
  
Remove Fluid and other possibly utilities:

  sudo rm -r /Developer/Applications/Utilities/
  

 Installing FLTK
-----------------

All FLTK frameworks should be copied from "./ide/Xcode3/build/Debug/" or 
"./ide/Xcode3/build/Release/" to "/Library/Frameworks/". All required FLTK
header files will be at "/Library/Frameworks/fltk.framework/Headers/" which
should be added to the header search path.

  sudo cp -R ide/Xcode3/build/Debug/fltk*.framework /Library/Frameworks/

Many FLTK applications will use Fluid, the FLTK User Interface builder, to 
generate C++ source code from .fl resource files. Add Fluid to the developer 
tools:

  sudo mkdir /Developer/Applications/Utilities/FLTK/
  sudo cp -R ide/Xcode3/build/Debug/Fluid.app /Developer/Applications/Utilities/FLTK/


 Installing Little Helpers
---------------------------

FIXME: 

Xcode can learn how to create FLTK projects for you. To add FLTK projects as a 
new project template for Xcode 3, copy "./ide/Xcode3/Project Templates/" to
"~/Library/Application Support/Developer/Shared/Xcode/" and restart Xcode.

Projects that are generated using this template expect fltk 1.3 as it is 
installed using the command line. Also, you will need to add the 
"AudioToolbox.framework" manually which is needed to create warning beeps.

Xcode can also handle the .fl file extension to edit and compile Fluid resource
files for you. The template project contains custom build rules for .fl files.

Choose Xcode > Preferences to open Xcode's preferences window. There is a 
toolbar at the top of the window with many buttons. Click the File Types button. 

Create the file "Library/Application Support/Developer/Shared/Xcode/
Specifications/fluid.pbfilespec" and add the following lines:
  
  {
    Identifier = sourcecode.fluid;
    BasedOn = sourcecode;
    Extensions = (fl);
  }

END OF FIXME.


 Creating new Projects
-----------------------

If the little helpers above were installed, the menu "File > New Project..."
will pop up a dialog that offers a User Template named Fluid. Select it and
follow the instructions. You will need to add the "AudioToolbox.framework" 
manually which is needed to create warning beeps.



 DOCUMENT HISTORY
==================

Oct 24 2010 - matt: restructured entire document and verified instructions
