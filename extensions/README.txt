
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



This is an idea that I have been following for a while, and now that I finally implemented it, it wasn't complicated at all.

My "extension" system makes it really easy to add widgets and test programs to core FLTK directly from a GitHub archive via CMake. Endusers need to put only a single additional line per extension into their CMake files to download any external widget and add it to FLTK when compiling their apps. For example:
```
# CMakeLists.txt
include (FetchContent)

FetchContent_Declare(FLTK GIT_REPOSITORY https://github.com/fltk/fltk GIT_TAG master)
FetchContent_Populate(Fl_LED_Button https://github.com/matthiaswm/Fl_LED_Button SOURCE fltk/extensions/widgets/Fl_LED_Button)
FetchContent_Populate(Fl_Spaceball https://github.com/matthiaswm/Fl_Spaceball SOURCE fltk/extensions/widgets/Fl_Spaceball)
FetchContent_MakeAvailable(FLTK)

add_executable(main main.cpp)
target_include_directories(main PRIVATE ${fltk_SOURCE_DIR} ${fltk_BINARY_DIR}) # needed for visual studio
target_link_libraries(main PRIVATE fltk) 
```
After calling these, we can `#include <FL/Fl_Spaceball.H>`, for example, or any other FLTK header.

This is all completely transparent and fully automatic once the user launches CMake.


