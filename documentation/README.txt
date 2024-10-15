README.txt (documentation)
---------------------------
Documentation is available in HTML and PDF format built using 'doxygen'
since FLTK 1.3. The documentation must be generated in a separate step
or downloaded from FLTK's software download pages. The documentation can
also be accessed online.

To read the docs after downloading or generating them, open the files:

    documentation/html/index.html
    documentation/fltk.pdf

with your browser or PDF viewer, respectively.


Online Documentation (Latest Release):
--------------------------------------
A documentation version is available online at the FLTK web site,
along with the PDF version of the manual. The docs on the web site
are usually somewhat older (latest release). The active development
version is updated from time to time.

Use this URL to find the current online documentation:

    https://www.fltk.org/documentation.php


Online Documentation (Daily CI Build):
--------------------------------------
The documentation of the development branch 'master' is generated
daily in the CI (Continuous Integration) build on our GitLab mirror.
This is experimental and may be discontinued at any time.

Documentation can be found at these URL's:

    HTML: https://fltk.gitlab.io/fltk/
    PDF:  https://fltk.gitlab.io/fltk/fltk.pdf


Documentation Download:
-----------------------
To download pre-generated docs, go to

    https://www.fltk.org/software.php

and look for the files

    fltk-<version>-docs-html.tar.gz
    fltk-<version>-docs-pdf.tar.gz

Extract the documentation tarballs into the same directory as you
did with the source tarball.


Generation of Documentation (Common Part):
------------------------------------------
To generate the documentation yourself you should have Doxygen
version 1.8 or later. Recommended version as of this writing
(October 2023) is 1.9.8 or later.
Older Doxygen versions than 1.8 may or may not work.

Full documentation generation is supported only on Unix/Linux
platforms that provide a POSIX compatible shell (e.g. 'bash') and
some standard Unix tools. Doxygen alone may be enough to generate
HTML docs but PDF documentation *requires* a POSIX shell and the
mentioned Unix tools.

On the Windows platform building HTML and particularly PDF docs
*may* work if you install and use MinGW, MSYS2, or Cygwin and all
required LaTeX tools but this is not supported and we don't
recommend it.

To generate the PDF version you also need a current version of LaTeX.
To install all required packages on recent Debian and Ubuntu Linux
distributions you can install the package 'doxygen-latex' which
"adds dependencies for all LaTeX packages required to build documents
using the default stylesheet" according to its description.

On other (and older) distributions you may use the packages tetex (older)
or texlive (current as of Ubuntu 18.04) if your distribution provides
them. You may need to install additional fonts and other sub-packages
for the PDF version to be generated correctly.


Generation of Documentation with autoconf (configure) + make:
-------------------------------------------------------------
Use "make html" in the documentation directory to generate the HTML
documentation, or "make pdf" to generate the PDF version. "make docs"
generates both versions in one step (as would do "make clean html pdf").

To read the HTML or PDF docs, see above.


Generation of Documentation with CMake:
---------------------------------------
We assume that you use an out-of-source build as recommended.
Documentation is not generated in a default build. You must set one or
more CMake options using cmake-gui, ccmake, or cmake -DOPTION...

For simplicity we use 'make' in the following description. If you
use another build system (e.g. 'ninja') please replace 'make' with
your preferred build command.

The following two options are predefined ON if you have the
required software packages (doxygen, LaTeX) installed. You can
always leave them ON because the documentation is not built
automatically (it is excluded from the default target "ALL").

  - FLTK_BUILD_HTML_DOCS:BOOL=ON     target: html
  - FLTK_BUILD_PDF_DOCS:BOOL=ON      target: pdf

The following two options default to OFF and can be switched ON.
They are only used when installing the software ('make install')
and the corresponding build options are ON.

  - FLTK_INSTALL_HTML_DOCS:BOOL=OFF
  - FLTK_INSTALL_PDF_DOCS:BOOL=OFF

If you switch one or both of the first two options ON, then the build
targets 'html' and/or 'pdf' are generated, respectively. Target 'docs'
is added to generate both 'html' and 'pdf'. As said above, you need
installed Doxygen and LaTeX software for this to work.

Use 'make html' in the root directory of the build tree to generate the
HTML documentation, or 'make pdf' to generate the PDF version.
'make docs' generates both versions in one step (as would do
'make clean html pdf').

To read the HTML or PDF docs, see above, but use the build directory.

Note: if you enable one or more of the "*INSTALL*" options, then
'make install' will install the docs, but you must still build them
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
post a question in the fltk group fltk.general or post a bug report
or feature request. For more information see this page:

    https://www.fltk.org/bugs.php
