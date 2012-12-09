README.Cairo.txt - 2011-12-10 - Cairo rendering support for FLTK
----------------------------------------------------------------



 CONTENTS
==========

 1   INTRODUCTION
 2   CAIRO SUPPORT FOR FLTK 1.3
   2.1   Configuration
   2.2   Currently supported features
   2.3   Future considerations 
 3   PLATFORM SPECIFIC NOTES
   3.1  Linux
   3.2  Windows
   3.3  Mac OSX
 4   DOCUMENT HISTORY



 INTRODUCTION
==============

Cairo is a software library used to provide a vector graphics-based, 
device-independent API for software developers. It is designed to provide 
primitives for 2-dimensional drawing across a number of different 
backends. Cairo is designed to use hardware acceleration when available.


 CAIRO SUPPORT FOR FLTK 1.3
============================

It is now possible to integrate cairo rendering in your fltk application
more easily and transparently.
In 1.3, we provide minimum support for Cairo,
in particular, no "total" cairo rendering layer support is achieved,
as in fltk2.


 Configuration
---------------

All the changes are *inactive* as long as the new configuration
option --enable-cairo is not added to the configure command.
For non configure based platforms/ide, the FLTK_HAVE_CAIRO preprocess
variable has to be defined.
All configure based build files have now this feature integrated,
also vc2005 build files have 2 new build modes "Release Cairo" and 
"Debug Cairo".
Other IDE's will be updated progressively.


 Currently supported features
------------------------------

- Adding a new Fl_Cairo_Window class permitting transparent and easy
integration of a Cairo draw callback without the need to achieve subclassing.

- Adding a Fl::cairo_make_current(Fl_Window*) function only providing
transparently a cairo context to your custom Fl_Window derived class.
This function is intended to be used in your overloaded draw() method.

- Adding an optional cairo autolink context mode support (disabled by default)
  which permits complete & automatic synchronization of OS dependent graphical 
  context and cairo contexts, thus furthering a valid cairo context anytime, 
  in any current window. 
  This feature should be only necessary in the following cases:
  -  Intensive and almost systematic use of cairo contexts in an fltk application
  -  Creation of a new cairo based scheme for fltk ...
  -  Other uses of cairo necessitating the fltk internal instrumentation 
     to automatically making possible the use of a cairo context 
     in any fltk window.

- A new cairo demo that is available in the test subdirectory and has been
   used as a testcase during the multiplatform tests.

For more details, please have a look to the doxygen documentation,
in the Modules section.


 Future considerations
-----------------------

From Bill:
First there is the FLTK_HAVE_CAIRO configuration option. This indicates that
any cairo calls are available. In this case you get something like this:

// static variable holding the last cairo context fltk set:
cairo_t* Fl::cr;

// Make cr draw in this window. This hides the ugly platform-dependent
// part of getting cairo going:
void Fl::cairo_make_current(Fl_Window*)

*** POST 1.3 potential cairo use:
// Set cr to something you made yourself. This lets you reuse functions
// that use cr, and also tells fltk that cr is not one of its own and
// thus cannot be destroyed or reused for a different window:
void Fl::cairo_make_current(cairo_t*)

Second there is the FLTK_USE_CAIRO configuration option. This means that all
drawing is done using Cairo. In this case when a widget draw() method is
called, it is exactly as though cairo_make_current(window) has been done.
*** 

Note that it should be possible to compile so FLTK_HAVE_CAIRO works even
if FLTK_USE_CAIRO does not, and so that turning on FLTK_USE_CAIRO does not
break any programs written for FLTK_HAVE_CAIRO.


 PLATFORM SPECIFIC NOTES
=========================

The folowing are notes about building FLTK with Cairo support
on the various supported operating systems.

    3.1 Linux
    ---------

    From Greg (erco@seriss.com):
    To get FLTK 1.3.x (r9204) to build on Centos 5.5, I found that
    I only needed to install the "cairo-devel" package, ie:
    
        sudo yum install cairo-devel

    ..and then rebuild fltk:

        make distclean
	./configure --enable-cairo
	make

    If you get this error:

        [..]
	Linking cairo_test...
	/usr/bin/ld: cannot find -lpixman-1
	collect2: ld returned 1 exit status
	make[1]: *** [cairo_test] Error 1

    ..remove "-lpixman-1" from fltk's makeinclude file, i.e. change this line:

	-CAIROLIBS      = -lcairo -lpixman-1
	+CAIROLIBS      = -lcairo

    ..then another 'make' should finish the build without errors.
    You should be able to then run the test/cairo_test program.
    
    According to the cairo site, "For Debian and Debian derivatives including
    Ubuntu" you need to install libcairo2-dev, i.e.

	sudo apt-get install libcairo2-dev

    This has been tested and works with Ubuntu 11.10. Note that this also
    installs libpixman-1-dev, so that dependencies on this should be resolved
    as well.


    3.2 Windows
    -----------
    TBD


    3.3 Mac OSX
    -----------
    TBD



 DOCUMENT HISTORY
==================

Dec 20 2010 - matt: restructured document
Dec 09 2011 - greg: Updates for Centos 5.5 builds
Dec 10 2011 - Albrecht: Updates for Ubuntu and Debian, fixed typos.
