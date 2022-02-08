FLTK EXAMPLE PROGRAMS
---------------------

    This directory contains example FLTK programs that demonstrate
    recommended programming practices and techniques for FLTK application
    programmers. The "*-simple.cxx" files are a good starting point for
    those new to FLTK.

    The programs in this directory are NOT built automatically
    when you build FLTK; you have to manually build them.

    The goals of these example programs:

      o Show good programming style for app programmers to emulate

      o Show simple examples of how to use widgets to new users of FLTK.

      o Show intermediate or advanced examples of techniques often
        misused or hard to document

      o Demonstrate code that are FAQs on the newsgroup forum.
        (such as how to use threads, callbacks, etc)

      o Example code should be short, but not at the expense of clarity.

      o Where possible, examples should emphasize FLTK's simplicity.


NEW SUBMISSIONS: RECOMMENDED PRACTICES

    These programs must follow FLTK coding style as defined in the FLTK
    "CMP" (Configuration Management Plan/Coding Standards).

    Example code should have the above goals in mind.  The best examples
    are those that are as short and clear as possible; terse, but not
    at the expense of clarity.

    To avoid cluttering up the top level directory with ancillary files
    (such as image files or icons), examples that depend on more than
    just a .cxx/.h file pair should have their own subdirectory.

    Data files common to several examples should be located in the
    examples/data directory.

    Ancillary data files should be as small as possible, to keep the
    distribution tar files small. Avoid high resolution images or
    uncompressed images when possible.

    Examples that need large data sets (HD images, etc) should not
    be part of the FLTK distribution; they can be provided as separate
    packages (eg. as articles or external links).

    Some widgets have multiple capabilities that are best demonstrated
    separately. For instance, the table widget can be used as a
    custom data table, or as a spreadsheet, or as a widget container.
    So separate examples for each would be e.g. "table-custom-data.cxx",
    "table-spreadsheet.cxx", "table-widget-container.cxx", etc.

    Example programs should contain comments that help understand the
    concepts shown, but not so verbose as to dwarf the code or make
    the code hard to read. Within code, it's best to use single line
    comments to emphasize code that might be unclear. Let the code
    speak as much as possible.

    Examples programs may be referred to from the documentation
    as good examples on how to do particular programming techniques.


NAMING CONVENTIONS

    Example programs that demonstrate a particular widget should start
    with that widget's name in lowercase, eg. "table.cxx" for Fl_Table.

    Demonstrations of a particular technique should start with
    "howto-xxx.cxx" to avoid naming conflicts with future widgets
    of the same name.

      xxx-simple.cxx          -- Simplest possible example of widget xxx
                                 eg. "table-simple.cxx"

      xxx-<technique>.cxx     -- A particular "technique" using widget xxx
                                 eg. "table-spreadsheet.cxx"

      howto-<technique>.cxx   -- Demonstrate a particular technique,
                                 eg. "howto-threading.cxx"

    Some example programs may depend on multiple files. To avoid
    cluttering up the top level examples directory, such examples will
    have their own subdirectory with the files they depend on localized
    to that directory.

    Example programs should be as small as possible, to keep the
    distribution tar files small.

    Very large examples, or examples that depend on large data sets
    should be submitted as separate articles on the FLTK site, or as
    external links on the FLTK site's 'links' page.


HISTORY

    Previous to FLTK 1.3.0, the fltk/test directory served the dual
    purpose of containing test suites as well as example code.

    But the fltk/test programs started becoming necessarily complex,
    testing for obscure problems, and not necessarily good demos for
    applications programmers.

    The fltk/examples directory was created in FLTK 1.3.0 to separate
    'good programming examples' from the test suite code.


DISCLAIMER

    The examples in this directory are provided 'as-is', without any express
    or implied warranty.  In no event will the authors be held liable for
    any damages arising from the use of this software.


BUGS

    If you find a bug, please report it to the FLTK team. For more
    information on how to do this see this page:

      https://www.fltk.org/bugs.php
