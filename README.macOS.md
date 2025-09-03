_README.macOS.md - Building FLTK under Apple macOS_


<a name="contents"></a>
## Contents

* [Contents](#contents)
* [Introduction](#introduction)

* [How to Build FLTK using _CMake_ and _Xcode_](#build_cmake_xcode)
    * [Prerequisites](#bcx_prerequisites)
    * [Downloading FLTK and Unpacking](#bcx_download)
    * [Configuring FLTK](#bcx_config)
    * [Building FLTK](#bcx_build)
    * [Testing FLTK](#bcx_test)
    * [Installing FLTK](#bcx_install)
    * [Creating new Projects](#bcx_new_projects)

* [How to Build FLTK using _CMake_ and _make_](#build_cmake_make)
    * [Prerequisites](#bcm_prerequisites)
    * [Downloading FLTK and Unpacking](#bcm_download)
    * [Configuring FLTK](#bcm_config)
    * [Building FLTK](#bcm_build)
    * [Testing FLTK](#bcm_test)
    * [Installing FLTK](#bcm_install)
    * [Creating new Projects](#bcm_new_projects)

* [How to Build FLTK Using _autoconf_ and _make_](#build_autoconf_make)
    * [Prerequisites](#bam_prerequisites)
    * [Downloading FLTK and Unpacking](#bam_download)
    * [Configuring FLTK](#bam_config)
    * [Building FLTK](#bam_build)
    * [Testing FLTK](#bam_test)
    * [Installing FLTK](#bam_install)
    * [Creating new Projects](#bam_new_projects)

* [Make an Application Launchable by Dropping Files on its Icon](#dropstart)
* [Document History](#doc_history)


<a name="introduction"></a>
## Introduction

FLTK supports macOS version 10.3 Panther and above. At the time of writing (Feb. 2022),
FLTK compiles and runs fine on the most recent macOS 12 Monterey for both Intel and
the new M1 Apple Silicon (Arm) processors.

FLTK 1.4 supports the following build environments on the macOS
platform:

* [_cmake_ and _Xcode_](#build_cmake_xcode), no shell needed
* [_cmake_ and _make_](#build_cmake_make) from the command line
* [_autoconf_ and _make_](#build_autoconf_make) from the command line

All environments will generate Unix style static libraries and macOS style app bundles.


<a name="build_cmake_xcode"></a>
## How to Build FLTK Using _CMake_ and _Xcode_

This option is best for users who like to develop their apps using Apple's Xcode IDE. There
is no need to ever use a command line shell in this configuration.

This option requires an Apple ID and the Administrator password.

<a name="bcx_prerequisites"></a>
###  Prerequisites (CMake, Xcode)

In order to build FLTK, you need to install _CMake_ and _Xcode_.

_Xcode_ is Apple's IDE (Integrated Developer Environment) and can be downloaded via the
[App Store](https://itunes.apple.com/de/app/xcode/id497799835?l=en&mt=12). You will
need an Apple ID and administrator right for this. Installing _Xcode_ needs little to no
user input, but will likely take well over an hour.

"CMake is used to control the software compilation process using simple platform and
compiler independent configuration files, and generate native makefiles and workspaces
that can be used in the compiler environment of your choice."

Please download and install the macOS version of _CMake_ from
[www.cmake.org](https://cmake.org/download/). Download the .dmg file, click it, and when
the Finder window opens, drag the _CMake_ icon into the Applications folder.

<a name="bcx_download"></a>
### Downloading FLTK and Unpacking (CMake, Xcode)

FLTK 1.4 is currently (as of May 2021) available as a source code repository via GitHub.
You will need to clone the repository to check out the source code onto your machine. This
has the great benefit that the source code can be updated later simply by telling _git_ to
_pull_ the newest release.

Weekly snapshots ("tarballs") can be downloaded from https://www.fltk.org/software.php .

If you want to use _Xcode_ to clone the FLTK GitHub repository, you will have to give _Xcode_
access to your GitHub Account in the _Xcode_ preferences. If you don't have a GitHub
account, or don't want to share your credentials with _Xcode_, you can use still the command
line  `git clone https://github.com/fltk/fltk.git fltk-1.4`
to check out the repo.

Start _Xcode_. Select `Source Control >> Clone...` in the main menu.

A dialog box will open with a search field and a list of repositories. Enter `fltk/fltk` in
the search field. A list of matching repositories appears. The first one should be named `fltk`
and be owned by `fltk`. Select it and click _Clone_.

A file chooser appears. Navigate to your home directory. Create a new folder named
`dev`. Enter `fltk-1.4` in the _Save As:_ field and click _Clone_, then _Done_ in the
previous dialog.

The local copy of your repository can be updated by loading it into _Xcode_ and selecting
`Source Control >> Pull...` in the main menu.

<a name="bcx_config"></a>
### Configuring FLTK (CMake, Xcode)

Launch _CMake_ by pressing Command+Spacebar, then type _CMake_ and press return.
_CMake_ should open with a large dialog box.

The first input field is labeled with _Where is the source code:_ . Click on _Browse Source..._
and navigate to your home folder, then `dev`, then `fltk-1.4`. Click _Open_.

The next input field is marked _Where to build the binaries:_. Click _Browse Build..._
and navigate to your home folder,  then `dev`, then `fltk-1.4`, then use _New Folder_
to create a folder named `build`, and inside that, create a folder named `Xcode`. Click _Open_.

The top two input fields should read
```
/Users/your_name/dev/fltk-1.4
```
and
```
/Users/your_name/dev/fltk-1.4/build/Xcode
```

Back in the _CMake_ main window, click _Configure_, select _Xcode_ as the generator and
click _Done_. _CMake_ will now analyse your system and find tools and dependencies. When
done, the upper list field in _CMake_ will show CMAKE and FLTK. Open the FLTK field and
adjust options if you like. Note that the bundled image libraries are built by default.
Further options are available under the CMAKE field.

Finally, click _Generate_ to generate the _Xcode_ IDE file.

You may be wondering why we chose this rather involved way of creating the IDE files instead
of just including the IDEs in the repo. Well, we did for the longest time, but creating new IDEs
for every possible platform is quite involved. IDE file formats change, platforms change, and
FLTK changes as well, and maintenance of the IDEs had become a nightmare. _CMake_ on the
other hand is very smart, knows about platforms and IDEs that we could never support manually,
and usually needs to be run only once. Even when updating the FLTK source code later,
_CMake_ will be smart enough to update the build files as well.

<a name="bcx_build"></a>
### Building FLTK (CMake, Xcode)

Now this is easy if all the previous steps were successful. If you are still in _CMake_, just click
_Open Project_ and _CMake_ will launch _XCode_ for you. If not, just launch _XCode_ and
open `Macintosh HD⁩ ▸ ⁨Users⁩ ▸ your_name⁩ ▸ ⁨dev⁩ ▸ ⁨fltk-1.4⁩ ▸ ⁨build⁩ ▸ ⁨Xcode⁩ ▸ ⁨FLTK.xcodeproj`.

_XCode_ may or may not ask to Autocreate Schemes. Click _Automatically Create Schemes_.

In _Xcode_, set the 'active Scheme' to 'hello' or any other test program and press CMD+R
to compile and run that application.

<a name="bcx_test"></a>
### Testing FLTK (CMake, Xcode)

After a successful build, you can test FLTK's capabilities by choosing 'ALL_BUILD' as the
'active Scheme' and pressing CMD+B to build, then choosing 'demo' and pressing CMD+R to
run the demo program.

Note: compiling 'demo' will not compile any of the other demo programs automatically.
This is intentional so you can build the test and demo programs incrementally.

<a name="bcx_install"></a>
### Installing FLTK (CMake, Xcode)

TODO: choosing the target 'INSTALL' will not work at this point because it requires root
permission.

<a name="bcx_new_projects"></a>
### Creating new Projects (CMake, Xcode)

See README.CMake.txt

<a name="build_cmake_make"></a>
## How to Build FLTK Using _CMake_ and _make_

This option is best for users who like to develop their apps without using Apple's Xcode IDE,
but like the advantages of _CMake_ over _autoconf_. Users should be comfortable with
using `bash` or `tcsh` in a terminal window.

This option requires neither administrator rights, nor an Apple ID.

<a name="bcm_prerequisites"></a>
###  Prerequisites (CMake, make)

In order to build FLTK, you need to install _CMake_ and the _Xcode_ command line tools.

"CMake is used to control the software compilation process using simple platform and
compiler independent configuration files, and generate native makefiles and workspaces
that can be used in the compiler environment of your choice."

Please download and install the macOS version of _CMake_ from
[www.cmake.org](https://cmake.org/download/). Download the .dmg file, click it, and when
the Finder window opens, drag the _CMake_ icon into the Applications folder.

We will be using _CMake_ from the command line. Please launch _CMake_ and choose
`Tools > How to Install for Command Line Use` and follow the instructions
in the dialog box. If you have admin right, you can also use

```bash
sudo ln -s /Applications/CMake.app/Contents/bin/cmake /usr/local/bin/cmake
```

Installing the _Xcode_ command line tools is pretty straight forward. Just enter this
in your shell and follow the dialogs:

```bash
xcode-select --install
```

On older versions of macOS, you will have to install _Xcode_ from the
[App Store](https://itunes.apple.com/de/app/xcode/id497799835?l=en&mt=12)
and then install the command line tools from within _Xcode_.


<a name="bcm_download"></a>
### Downloading and Unpacking (CMake, make)

Downloading FLTK is explained [here](#bam_download).

<a name="bcm_config"></a>
### Configuring FLTK (CMake, make)

Using your shell in the terminal, make sure that you are in the root directory of your
FLTK source code tree.

Create a directory where all FLTK binaries will be built:

```bash
mkdir build
cd build
mkdir Makefile
cd Makefile
```
Now configure your FLTK installation:

```bash
cmake -G "Unix Makefiles" \
    -D CMAKE_BUILD_TYPE=Debug \
    ../..
```

Replace 'Debug' with 'Release' if you want to build a release version.

_CMake_ runs a number of tests to find external headers, libraries, and tools.
The configuration summary should not show any errors. You can now continue to build FLTK.

For the advanced user there are a few more options to the _CMake_ setup. Type
`cmake -L ../..` to get a complete list of options. These should be pretty
self-explanatory. Some more details can be found in
[online documentation](https://www.fltk.org/doc-1.4/intro.html#intro_unix).

<a name="bcm_build"></a>
### Building FLTK (CMake, make)

Now this is easy if all the previous steps were successful. Stay in your `build/Makefiles`
directory and type:

```bash
make
```

The entire FLTK toolkit including many test programs will be built for you. No
warnings should appear, but "ranlib" may complain about a few modules having no
symbols. This is normal and can safely be ignored.

<a name="bcm_test"></a>
### Testing FLTK (CMake, make)

After a successful build, you can test FLTK's capabilities by running

```bash
open bin/test/demo.app
```

<a name="bcm_install"></a>
### Installing FLTK (CMake, make)

If you did not change any of the configuration settings, FLTK will be installed
in `/usr/local/include`, `/usr/local/lib`, and `/usr/local/bin` by typing

```bash
sudo make install
```

It is possible to install FLTK without superuser privileges by changing the
installation path to a location within the user account by adding the
`-D CMAKE_INSTALL_PREFIX=<PREFIX>` parameter to the `cmake` command.

<a name="bcm_new_projects"></a>
### Creating new Projects (CMake, make)

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


<a name="build_autoconf_make"></a>
## How to Build FLTK Using _autoconf_ and _make_

This option is best for users who like to develop their apps without using Apple's Xcode IDE
and prefer minimal dependencies of a _Makefile_ over _CMake_. Users should be comfortable
with using `bash` or `tcsh` in a terminal window.

This option requires administrator rights, but no Apple ID.

<a name="bam_prerequisites"></a>
###  Prerequisites (autoconf, make)

In order to build FLTK from the command line, you need to install a C++ compiler
environment, `make` and `autoconf`. Installing the _Xcode_ command line tools is the easiest
way to get all prerequisites in one simple step.

<a name="xcode_command_line_tools"></a>
Launch _Terminal.app_ by pressing Command+Spacebar and typing `Terminal` and pressing _return_.
I like to keep the Terminal in the Dock for future use (launch Terminal, right-click or control-click
on the Terminal icon that is now in the docking bar, and choose _Options_->_Keep in Dock_).

Installing the _Xcode_ command line tools is pretty straight forward. Just enter this
and follow the dialogs:

```bash
xcode-select --install
```

On older versions of macOS, you will have to install _Xcode_ from the
[App Store](https://itunes.apple.com/de/app/xcode/id497799835?l=en&mt=12)
and then install the command line tools from within _Xcode_.

Apple no longer includes _autoconf_ in the _Xcode_ command line tools. To install
_autoconf_, we first need to install _brew_ by typing this rather cryptic command in the shell
(you will need to type the password of an administrator account):

```bash
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

After a few minutes, we can now build and install all other tools from one simple command:

```bash
brew install autoconf
```

Alternatively, _autoconf_ can be installed without _brew_ as follows :

- Download file _autoconf-latest.tar.gz_ from the <a href=https://ftp.gnu.org/gnu/autoconf/>autoconf ftp site</a>
- Uncompress it to get directory _autoconf-x.xx/_ (where x's indicate autoconf's version number)
- Set this directory as your current directory in the Terminal app and type :

```bash
./configure
make
sudo make install
```


<a name="bam_download"></a>
### Downloading and Unpacking (autoconf, make)

FLTK 1.4 is currently (as of May 2021) available as a source code repository via GitHub.
You will need to clone the repository to check out the source code onto your machine. This
has the great benefit that the source code can be updated later simply by telling _git_ to
_pull_ the newest release.

As an alternative weekly snapshots ("tarballs") can be downloaded from
https://www.fltk.org/software.php .

Start your terminal. If you have not set up a developer directory yet, I recommend to use
`~/dev` and put all your projects there:

```bash
# make sure we are in the home directory
cd ~
# create our developer directory and go there
mkdir dev
cd dev
```
Now create a copy of the source code archive at Github on your local file system:

```bash
git clone https://github.com/fltk/fltk.git fltk-1.4
cd fltk-1.4
```

<a name="bam_config"></a>
### Configuring FLTK (autoconf, make)

Using your shell in the terminal, make sure that you are in the root directory of your
FLTK source code tree.

If you are configuring fltk for the first time, you need to instruct FLTK to create some
very basic configuration files. Type:

```bash
autoconf
```
This creates the configure script.

Now configure your FLTK installation. Stay in your FLTK source-code directory
and type

```bash
./configure
```

The configuration script runs a number of tests to find external headers, libraries, and tools.
The configuration summary should not show any errors. You can now continue to build FLTK.

For the advanced user there are a few more options to the _configure_ script. Type
`./configure --help` to get a complete list of options. These should be pretty
self-explanatory. Some more details can be found in
[online documentation](https://www.fltk.org/doc-1.4/intro.html#intro_unix).

<a name="bam_build"></a>
### Building FLTK (autoconf, make)

Now this is easy if all the previous steps were successful. Stay in your FLTK source-code
directory and type:

```bash
make
```

The entire FLTK toolkit including many test programs will be built for you. No
warnings should appear, but "ranlib" may complain about a few modules having no
symbols. This is normal and can safely be ignored.

<a name="bam_test"></a>
### Testing FLTK (autoconf, make)

After a successful build, you can test FLTK's capabilities by running

```bash
test/demo
```

<a name="bam_install"></a>
### Installing FLTK (autoconf, make)

If you did not change any of the configuration settings, FLTK will be installed
in `/usr/local/include`, `/usr/local/lib`, and `/usr/local/bin` by typing:

```bash
sudo make install
```

It is possible to install FLTK without superuser privileges by changing the
installation path to a location within the user account by adding the
`--prefix=PREFIX` parameter to the `./configure` command.

<a name="bam_new_projects"></a>
### Creating new Projects (autoconf, make)

FLTK provides a neat script named `fltk-config` that can provide all the flags
needed to build FLTK applications using the same flags that were used to build
the library itself. Running `fltk-config` without arguments will print a list
of options. The easiest call to compile an FLTK application from a single source
file is:

```bash
cat << EOF > main.cxx
  #include <FL/Fl.H>
  #include <FL/Fl_Window.H>
  int main(int argc, char **argv) {
    Fl_Window *win = new Fl_Window(600, 400, "Hello, world!");
    win->show(argc, argv);
    return Fl::run();
  }
EOF
fltk-config --compile main.cxx
./main
```

`fltk-config` and our user interface designer `fluid` will be installed in
`/usr/local/bin/` by default. I recommend that you add this directory to the shell
`PATH` variable.


<a name="dropstart"></a>
## Make an Application Launchable by Dropping Files on its Icon

- Prepare an Info.plist file for your application derived from file
_test/mac-resources/editor.plist_ which allows any file to be dropped
on the application icon.
You can edit this file in Xcode and change
CFBundleDocumentTypes/Item 0/CFBundleTypeExtensions/Item 0
from the current "*" to the desired file extension. Use several items to
declare several extensions.

- Call function <a href=https://www.fltk.org/doc-1.4/group__group__macosx.html#ga0702a54934d10f5b72157137cf291296>fl\_open\_callback()</a> at the beginning of your main() function to set
what function will be called when a file is dropped on the application icon.

- In Xcode, set the "Info.plist File" build setting of your target application
to the Info.plist file you have prepared.

- Rebuild your application.

## Links

[GitHub, git, forking, fetching. and pull requests](https://gist.github.com/Chaser324/ce0505fbed06b947d962)

<a name="doc_history"></a>
## DOCUMENT HISTORY

- Oct 29 2010 - matt: removed warnings
- Oct 24 2010 - matt: restructured entire document and verified instructions
- Dec 19 2010 - Manolo: corrected typos
- Dec 29 2010 - Manolo: removed reference to AudioToolbox.framework that's no longer needed
- Feb 24 2011 - Manolo: architecture flags are not propagated to the fltk-config script.
- Apr 17 2012 - matt: added Xcode4 documentation
- Nov 13 2012 - Manolo: added "MAKE AN APPLICATION LAUNCHABLE BY DROPPING FILES ON ITS ICON"
- Mar 18 2015 - Manolo: removed uses of the Xcode3 project
- Apr 01 2016 - Albrecht: corrected typo, formatted most line breaks < 80 columns
- Dec 04 2018 - Albrecht: fix typo (lowercase fluid.app) for case sensitive macOS
- Dec 28 2018 - Matt: complete rework for FLTK 1.4
- Mar 01 2021 - Albrecht: minor updates, macOS Big Sur and Apple Silicon M1 (ARM)
- Feb 23 2022 - Manolo: install autoconf without brew
