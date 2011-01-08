README.Unix.txt - 2010-11-14 - Building FLTK on Unix
-----------------------------------------------------



 CONTENTS
========== 

  1   INTRODUCTION
  2   PREREQUISITES
    2.1   Ubuntu 10
    2.2   Linux Mint 9
    2.3   Fedora 13
    2.4   * http://www2.mandriva.com/
    2.5   * http://www.opensuse.org/en/
    2.6   * http://www.debian.org/
    2.7   * Mandrake?
    2.8   * Sun?
    2.9   * SGI?
    2.10  * HPUX?
  3   HOW TO BUILD FLTK USING GCC
    3.1   Prerequisites
    3.2   Downloading and Unpacking
    3.3   Configuring FLTK
    3.4   Building FLTK
    3.5   Testing FLTK
    3.6   Installing FLTK
    3.7   Creating new Projects
  4   CREATING A NEW PROJECT IN CODE::BLOCKS
  5   DOCUMENT HISTORY

* TODO: we still need to write these chapters



 1  INTRODUCTION
=================

FLTK currently supports the following development environments on vmost Unix 
platforms:

    - gcc command line tools
    - Code::Blocks
    - ...
    
The Symbol font and the Zapf Dingbats font do not work on X11. This is correct
behavior for UTF-8 platforms.



 2  PREREQUISITES
==================


 2.1  Ubuntu 10
----------------

Ubuntu Linux can be downloaded here:

  http://www.ubuntu.com/
  
If you have not done so yet, download and install Ubuntu.

Open a shell and install some software:

  sudo apt-get install g++
  sudo apt-get install gdb
  sudo apt-get install subversion
  sudo apt-get install autoconf
  sudo apt-get install libx11-dev
  sudo apt-get install libglu1-mesa-dev
  
These two are optional, but highly recommended:  
  
  sudo apt-get install libasound2-dev
  sudo apt-get install libxft-dev

If you are planning to use the Code::Blocks IDE, also install this

  sudo apt-get install codeblocks

I like to use subversion to install the latest FLTK-1.3.release:

  svn co http://svn.easysw.com/public/fltk/fltk/branches/branch-1.3/ fltk-1.3
  
To update to the latest version, just go into the fltk-1.3 directory and type

  svn update


 2.2  Linux Mint 9
-------------------

Linux Mint 9 can be downloaded here:

  http://www.linuxmint.com/
  
If you have not done so yet, download and install Linux Mint.

Open a shell and install some software:

  sudo apt-get install g++
  sudo apt-get install gdb
  sudo apt-get install subversion
  sudo apt-get install autoconf
  sudo apt-get install libx11-dev
  sudo apt-get install libglu1-mesa-dev
  
These two are optional, but highly recommended:  
  
  sudo apt-get install libasound2-dev
  sudo apt-get install libxft-dev

If you are planning to use the Code::Blocks IDE, also install this

  sudo apt-get install codeblocks

I like to use subversion to install the latest FLTK-1.3.release:

  svn co http://svn.easysw.com/public/fltk/fltk/branches/branch-1.3/ fltk-1.3
  
To update to the latest version, just go into the fltk-1.3 directory and type

  svn update

FIXME: no FL_SYMBOL font (-*-symbol-*),  font 15 (Zapf-Dingbats)


 2.3  Fedora 13
-------------------

Fedora 13 Linux can be downloaded here:

  http://fedoraproject.org/
  
If you have not done so yet, download and install Fedora.

Open a terminal window and install some software. In Fedora, the default user 
has no permission to call "sudo", so we will change user a few times:

  su root
  yum groupinstall "Development Tools"
  yum groupinstall "X Software Development"

If you are planning to use the Code::Blocks IDE, also install this

  yum install codeblocks.i686  (for 64 bit machines)

Don't forget to leave root status (Ctrl-D) before loading FLTK. To install FLTK 
for every user, you either have to set root user again, or use "visudo" to add 
yourself to the "sudo" list.

I like to use subversion to install the latest FLTK-1.3.release:

  svn co http://svn.easysw.com/public/fltk/fltk/branches/branch-1.3/ fltk-1.3
  
To update to the latest version, just go into the fltk-1.3 directory and type

  svn update

FIXME: no FL_SYMBOL font (-*-symbol-*),  font 15 (Zapf-Dingbats)



 3  HOW TO BUILD FLTK USING GCC
================================


 3.1  Downloading and Unpacking
--------------------------------

Download FLTK from here:

  http://www.fltk.org/software.php

If you are familiar with "subversion" and like to stay current with your 
version, you will find the subversion access parameters at the bottom of that 
page. Unpack FLTK into a convenient location. I like to have everything in my 
dev directory:

  cd
  mkdir dev
  cd dev
  mv ~/Downloads/fltk-1.3.xxxx.tar.gz .
  tar xvfz fltk-1.3.xxxx.tar.gz
  cd fltk-1.3.xxxx
  

 3.2  Configuring FLTK
-----------------------

Stay in your FLTK source-code directory. Type:
  
  autoconf

Now configure your FLTK installation:

  ./configure
  
ADVANCED: type "./configure --help" to get a complete list of optional 
configurations parameters. These should be pretty self-explanatory. Some
more details can be found in README. 
:END_ADVANCED

The configuration script will check your machine for the required resources
which you should have installed as described in the Prerequisites chapter. Review
the Configuration Summary, maybe take some notes.


 3.3  Building FLTK
--------------------

Now this is easy. Stay in your FLTK source-code directory and type: 

  make

The entire FLTK toolkit including many test programs will be built for you. No 
warnings should appear. If some do, please let the FLTK developer team know via
the mailing list or the bug reporting template at www.fltk.org .

Actually, as of Oct 28 2010, quite a bunch of warnings will show, mostly about 
suggested parenthesis. Please ignore them until we can fix them.


 3.4  Testing FLTK
-------------------

After a successful build, you can test FLTK's capabilities:

  test/demo


 3.5  Installing FLTK
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


 3.6  Creating new Projects
----------------------------

FLTK provides a neat script named "fltk-config" that can provide all the flags 
needed to build FLTK applications using the same flags that were used to build
the library itself. Running "fltk-config" without arguments will print a list
options. The easiest call to compile an FLTK application from a single source 
file is: 

  fltk-config --compile myProgram.cxx

"fltk-config" and "fluid" will be installed in "/usr/local/bin/" by default. I
recommend that you add it to the command search path.



 4  CREATING A NEW PROJECT IN CODE::BLOCKS
===========================================

Code::Blocks is a free and popular C++ IDE in the Linux world. It also runs on
OS X and MSWindows. Configured correctly, it can also cross-compile between
these platforms. This chapter focuses on creating a new FLTK project for Linux, 
assuming that FLTK 1.3 was previously built and installed in its default 
location from the command line.

If not done yet, install Code::Blocks as described in the Prerequisites chapter 
above, or download it from their web site. This description is based on 
version 10.05:

  http://www.codeblocks.org/
  
Start Code::Blocks. Select File > New > Project. In the "New from template"
dialog box, click on "FLTK project" and follow the instructions.

The default project support basic fltk. If you would like to add support for
images, OpenGL, GLUT, or Forms, add the corresponding flags --use-images,
--use-gl, --use-glut, and --use-forms respectively.

The flags are located in the "Project Build Options" dialog. To change the 
compiler flags, select your project in the tree view, then select the
"Compiler Settings" tab, then "Other Options" and add the flags to 
`fltk-config --cxxflags` in front of the second "`". 

The linker flags are located in the "Linker Settings" tab under "Other Linker
Options". Add the flags to `fltk-config --ldstaticflags` in front of the 
second "`".



 5  DOCUMENT HISTORY
=====================

Oct 30 2010 - matt: added Code::Blocks chapter
Oct 28 2010 - matt: restructured entire document and verified instructions
Nov 14 2010 - duncan: added install-desktop
