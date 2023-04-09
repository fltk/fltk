
 FLTK Extensions
=================

The extensions directory can be used to add external widgets to the FLTK
Source tree and integrate them into the libraries, documentation and FLUID.

Extensions can be automatically loaded via CMake files.

Widget extensions must be put into their own directory under extensions/widgets. They
Must always be completely self-contained within this directory. The directory
Must contain everything that is needed to add the extension. Removing the directory must
Also completely remove the extension from FLTK.

Extensions are normally used if FLTK is kept integral to building a specific app. 
It is not necessarily useful to install an extended version of FLTK libraries of FLUID.

They must contain at least two files: config.cmake and config.mk . Source and header
Files go into src and FL respectively. Documentation can be added via Doxygen comments
Or via text files in documentation/src/*.dox. FLUID bindings can be added via
A source code module in the fluid directory, and test programs can go into test.

The confg files are included from the respective build system. They can add information
To a predefined number of variables that are later used to build FLTK and FLUID. Additional
Variables may need to be defined.


