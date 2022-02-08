README.txt (documentation)
---------------------------
FLTK 1.3 (and higher) documentation is available in HTML and PDF format.
The documentation must be generated in a separate step or downloaded
from FLTK's software download pages. The documentation can also
be accessed online.

To read the docs after downloading or generating them, open the files:

    documentation/html/index.html
    documentation/fltk.pdf

with your browser or PDF viewer, respectively.


Online Documentation:
---------------------
A documentation version is available online at the FLTK web site,
along with the PDF version of the manual. The docs on the web site
are usually somewhat older (latest release). Use this URL to find
the current online documentation:

    https://www.fltk.org/documentation.php


Documentation Download:
-----------------------
To download pre-generated docs, go to

    https://www.fltk.org/software.php

and look for the files

    fltk-<version>-docs-html.tar.gz
    fltk-<version>-docs-pdf.tar.gz

Extract the documentation tarballs into the same directory as you
did with the source tarball.


Generation of Documentation (common part):
------------------------------------------
To generate the documentation yourself, you should have Doxygen
version 1.8 or later (recommended version is 1.8.13 or later).
Older Doxygen versions than 1.8 may or may not work.

To generate the PDF version you also need a current version of LaTeX.
You can use the packages tetex (older) or texlive (current) if your
distribution provides them. You may need to install additional fonts
and other sub-packages for the PDF version to be generated correctly.

Note: packages known to work under Ubuntu 18.04:

  texlive texlive-extra-utils texlive-latex-extra texlive-font-utils


Generation of Documentation with autoconf (configure) + make:
-------------------------------------------------------------
Use "make html" in the documentation directory to generate the HTML
documentation, or "make pdf" to generate the PDF version. "make docs"
generates both versions in one step (as would do "make clean html pdf").

To read the HTML or PDF docs, see above.


Generation of Documentation with CMake + make:
----------------------------------------------
We assume that you use an out-of-source build as recommended.
Documentation is not generated in a default build. You must set one or
more CMake options using cmake-gui, ccmake, or cmake -DOPTION...

These options are predefined to OFF and can be switched ON:

  - OPTION_BUILD_HTML_DOCUMENTATION:BOOL=OFF    target: html
  - OPTION_BUILD_PDF_DOCUMENTATION:BOOL=OFF     target: pdf

  - OPTION_INSTALL_HTML_DOCUMENTATION:BOOL=OFF
  - OPTION_INSTALL_PDF_DOCUMENTATION:BOOL=OFF

If you switch one or both of the first two options ON, then the
targets 'html' and/or 'pdf' are generated, respectively. Target 'docs'
is added to generate both 'html' and 'pdf'. As said above, you need
installed Doxygen and LaTeX software for this to work.

Use "make html" in the root or documentation directory of the build tree
to generate the HTML documentation, or "make pdf" to generate the PDF
version. "make docs" generates both versions in one step (as would do
"make clean html pdf").

To read the HTML or PDF docs, see above, but use the build directory.

Note: if you enable one or more of the "*INSTALL*" options, then
`make install' will install the docs, but you must still build them
manually, because the generation is not included in the standard ('all')
target. This may be changed in the future.


FLTK Developer Snapshots or Git Usage:
--------------------------------------
There is no pre-generated documentation available if you use a current
developer snapshot or git. You must generate the documentation
yourself or access the online documentation.


Bugs and Feature Requests:
--------------------------
If you find any typos, things that are unclear, or would like to
contribute an example, section, or chapter to the FLTK manual, please
post a question in the fltk group fltk.general or post a bug
report or feature request. For more information see this page:

    https://www.fltk.org/bugs.php
