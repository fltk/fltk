
Utility programs used to build the FLTK library.

Contents:

  cmap.cxx              generate the built-in colormap (src/fl_cmap.h)

  code_snapshot.cxx     PDF documentation tool to generate a png image from a
                        Doxygen `@code` segment with international characters

Build System:

  The util directory includes CMake support. Utilities are built
  automatically during the build process when needed. Some utilities
  are only built when specific features are enabled:

  - The colormap generator `bin/cmap` is only built manually if needed
    to generate a new colormap (src/fl_cmap.h).
    See instructions in util/cmap.cxx.

  - The PDF documentation helper is only built when FLTK_BUILD_PDF_DOCS=ON.

  To add a new utility, edit util/CMakeLists.txt and follow the existing
  patterns for conditional building and target configuration.
