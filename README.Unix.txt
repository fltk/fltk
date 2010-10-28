README.Unix.txt - 2010-10-28 - Building FLTK on Unix
-----------------------------------------------------



 CONTENTS
========== 

  1   INTRODUCTION
  2   PREREQUISITES
    2.1   Ubuntu 10
    2.2   * http://www.linuxmint.com/about.php
    2.3   * http://fedoraproject.org/
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
  4   * CODE::BLOCKS
  4   DOCUMENT HISTORY


 INTRODUCTION
==============

FLTK currently supports the following development environments on vmost Unix 
platforms:

    - gcc command line tools
    - Code::Blocks
    - ...
    

 PREREQUISITES
===============


 Ubuntu 10
-----------

Ubuntu Linux can be downloaded here:

  www.ubuntu.com
  
If you have not done so yet, download and install Ubuntu.

Open a shell and install some software:

  sudo apt-get install g++
  sudo apt-get install gdb
  sudo apt-get install subversion
  sudo apt-get install autoconf
  sudo apt-get install libx11-dev
  sudo apt-get install libglu1-mesa-dev
  sudo apt-get install libasound2-dev
  sudo apt-get install libxft-dev

If you are planning to use the Code::Blocks IDE, also install this

  sudo apt-get install codeblocks

I like to use subversion to install the latest FLTK-1.3.release:

  svn co http://svn.easysw.com/public/fltk/fltk/branches/branch-1.3/ fltk-1.3
  
To update to the latest version, just go into the fltk-1.3 directory and type

  svn update

FIXME: no FL_SYMBOL font (-*-symbol-*),  font 15 (Zapf-Dingbats)



 HOW TO BUILD FLTK USING GCC
=============================


 Downloading and Unpacking
---------------------------

Download FLTK from here:

  http://www.fltk.org/software.php

If you are familiar with "subversion" and like to stay current with your 
version, you will find the subversion access parameters at the bottom of that 
page. Unpack FLTK into a convinient location. I like to have everything in my 
dev directory:

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
:END_ADVANCED

The configuration script will check your machine for the required resources
which you should have installed as described in the Perequisites chapter. Review
the Configuration Summary, maybe take some notes.


 Building FLTK
---------------

Now this is easy. Stay in your FLTK source-code directory and type: 

  make

The entire FLTK toolkit including many test programs will be built for you. No 
warnings should appear. If some do, please let the FLTK developer team know via
the mailing list or the bug reporting template at www.fltk.org .

Actually, as of Oct 28 2010, quite a bunch of warnigns will show, mostly about 
suggested parenthesis. Please ignore them until we can fix them.


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



 DOCUMENT HISTORY
==================

Oct 28 2010 - matt: restructured entire document and verified instructions
