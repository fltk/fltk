_README.macOS.md - Building FLTK under Apple macOS_

<a name="contents"></a>
## Contents

* [Contents](#contents)
* [Introduction](#introduction)
* [How to Build FLTK Using _autoconf_ and _make_](#build_autoconf_make)
    * [Prerequisites](#bam_prerequisites)
    * [Downloading FLTK and Unpacking](#bam_download)
    * [Configuring FLTK](#bam_config)
    * [Building FLTK](#bam_build)
    * [Testing FLTK](#bam_test)
    * [Installing FLTK](#bam_install)
    * [Creating new Projects](#bam_new_projects)
3. HOW TO BUILD FLTK USING _CMake_ AND _Xcode_
  1. Prerequisites
  2. Downloading and Unpacking
  3. Configuring FLTK
  4. Building FLTK
  5. Testing FLTK
  6. Uninstalling previous versions of FLTK
  7. Installing FLTK
  8. Installing Little Helpers
  9. Creating new Projects
4. HOW TO BUILD FLTK USING _CMake_ AND _make_
5. MAKE AN APPLICATION LAUNCHABLE BY DROPPING FILES ON ITS ICON
6. DOCUMENT HISTORY

<a name="introduction"></a>
## INTRODUCTION

FLTK supports all macOS versions above 10.3 (Panther). See below for how to
build FLTK applications that can run on all (old or recent) macOS versions.

FLTK 1.4 supports the following build environments on the macOS
platform:

- _autoconf_ and _make_ from the command line
- _cmake_ and _make_ from the command line
- _cmake_ and _Xcode_ 

All environments will generate Unix style static libraries and macOS style app bundles.

<a name="build_autoconf_make"></a>
## How to Build FLTK Using _autoconf_ and _make_

This option is best for users who like to develop their apps without using Apple's Xcode IDE.
Users should be comfortable with using `bash` or `tcsh` in a terminal window. 

<a name="bam_prerequisites"></a>
###  Prerequisites

In order to build FLTK from the command line, you need to install a C++ compiler 
environment, `make` and `autoconf`. _Xcode_ is the easiest way to install all prerequisites,
even if you don't plan to use it as your iDE.

_Xcode_ can be downloaded via the 
[App Store](https://itunes.apple.com/de/app/xcode/id497799835?l=en&mt=12).

After downloading and installing, you need to launch the Terminal. _Terminal.app_
is located in the _Utilities_ folder inside the _Applications_ folder. I like
to keep the Terminal in the Dock for future use (launch Terminal, right-click or control-click
on the Terminal icon that is now in the docking bar, and choose _Options_->_Keep in Dock_).

<a name="bam_download"></a>
### Downloading and Unpacking

FLTK 1.4 is currently (as of Jan. 2019) only available as a source code repository via GitHub.
You will need to clone the repository to check out the source code onto your machine. This
has the great benefit that the source code can be updated simly by telling _git_ to _pull_
the newest release.

Start your terminal. If you have not set up a developer directory yet, I recomment to use
`~/dev` and put all your projects there:

```bash
# make sure we are in the home directory
cd ~
# create our developer directory and go there
mkdir dev
cd dev
```
Now create a copy of the current source code locally:

```bash
git clone https://github.com/fltk/fltk.git fltk-1.4.git
cd fltk-1.4.git
```

<a name="fltk-1.4.git"></a>
### Configuring FLTK

Using you shell in the terminal, make sure that you are in the root directory of your
FLTK source code tree.

If you are configuring fltk for the first time, you need to instruct FLTK to create some 
very basic configuration files. Type:

```bash
NOCONFIGURE=1 ./autogen.sh
```
This script may generate a few error messages which you can sefely ignore.

Now configure your FLTK installation: stay in your FLTK source-code directory
and type

```bash
./configure
```

The configuration script runs a number of tests to find libraries and tools. The configuration
summary should not show any errors. You can now continue to build FLTK.

For the advanced user, there are a few more optinons to the _configure_ script. Type 
`./configure --help` to get a complete list of options. These should be pretty 
self-explanatory. Some more details can be found in 
[online documentation](https://www.fltk.org/doc-1.4/intro.html#intro_unix).

<a name="bam_build"></a>
### Building FLTK

Now this is easy if all the previous steps were successful. Stay in your FLTK source-code 
directory and type:

```bash
make
```

The entire FLTK toolkit including many test programs will be built for you. No
warnings should appear, but "ranlib" may complain about a few modules having no
symbols. This is normal and can safely be ignored.

<a name="bam_test"></a>
### Testing FLTK

After a successful build, you can test FLTK's capabilities by running

```bash
test/demo
```

<a name="bam_install"></a>
### Installing FLTK

If you did not change any of the configuration settings, FLTK will be installed
in `/usr/local/include`, `/usr/local/lib`, and `/usr/local/bin` by typing

```bash
sudo make install
```

It is possible to install FLTK without superuser privileges by changing the
installation path to a location within the user account by adding the
`--prefix=PREFIX` parameter to the `./configure` command.

<a name="bam_new_projects"></a>
### Creating new Projects

FLTK provides a neat script named `fltk-config` that can provide all the flags
needed to build FLTK applications using the same flags that were used to build
the library itself. Running `fltk-config` without arguments will print a list
of options. The easiest call to compile an FLTK application from a single source
file is:

```bash
fltk-config --compile myProgram.cxx
```

`fltk-config` and our user interface designer `fluid` will be installed in 
`/usr/local/bin/` by default. I recommend that you add this directory to the shell
`PATH` variable.



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

If you are familiar with "Git" and like to stay current with your
version, you will find the Git access parameters at the bottom of that
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

By default, the Xcode4 project builds applications that run under macOS 10.5
and above. To build applications that also run under older Mac OS versions,
select "Edit Project Settings" of the Project menu, then select the Build panel,
and modify the "macOS Deployment Target" item.

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

If you are familiar with "Git" and like to stay current with your
version, you will find the Git access parameters at the bottom of that
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

By default, the Xcode4 project builds applications that run under macOS 10.5
and above. To build applications that also run under older Mac OS versions,
modify the "macOS Deployment Target" item of the FLTK project build settings.

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
test/editor-Info.plist which allows any file to be dropped
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
Dec 04 2018 - AlbrechtS: fix typo (lowercase fluid.app) for case sensitive macOS
