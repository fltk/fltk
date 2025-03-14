//
// Command Line Arguments Handling code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include "app/args.h"

#include "Fluid.h"

#include <FL/Fl.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>

using namespace fld;
using namespace fld::app;

/**
 Load args from command line into variables.

 \param[in] argc number of arguments in the list
 \param[in] argv pointer to an array of arguments
 \return 0 if the args were handled successfully, -1 if there was an error
    and the usage message was shown.
 */
int Args::load(int argc,char **argv) {
  int i = 1;
  Fl::args_to_utf8(argc, argv); // for MSYS2/MinGW
  if (   (Fl::args(argc,argv,i,arg_cb) == 0)     // unsupported argument found
      || (Fluid.batch_mode && (i != argc-1))        // .fl filename missing
      || (!Fluid.batch_mode && (i < argc-1))        // more than one filename found
      || (argv[i] && (argv[i][0] == '-'))) {  // unknown option
    static const char *msg =
    "usage: %s <switches> name.fl\n"
    " -u : update .fl file and exit (may be combined with '-c' or '-cs')\n"
    " -c : write .cxx and .h and exit\n"
    " -cs : write .cxx and .h and strings and exit\n"
    " -o <name> : .cxx output filename, or extension if <name> starts with '.'\n"
    " -h <name> : .h output filename, or extension if <name> starts with '.'\n"
    " --help : brief usage information\n"
    " --version, -v : print fluid version number\n"
    " -d : enable internal debugging\n";
    const char *app_name = nullptr;
    if ( (argc > 0) && argv[0] && argv[0][0] )
      app_name = fl_filename_name(argv[0]);
    if ( !app_name || !app_name[0])
      app_name = "fluid";
#ifdef _MSC_VER
    // TODO: if this is fluid-cmd, use stderr and not fl_message
    fl_message(msg, app_name);
#else
    fprintf(stderr, msg, app_name);
#endif
    return -1;
  }
  return i;
}


int Args::arg_cb(int argc, char** argv, int& i) {
  return Fluid.args.arg(argc, argv, i);
}


/**
 Handle command line arguments.
 \param[in] argc number of arguments in the list
 \param[in] argv pointer to an array of arguments
 \param[inout] i current argument index
 \return number of arguments used; if 0, the argument is not supported
 */
int Args::arg(int argc, char** argv, int& i) {
  if (argv[i][0] != '-')
    return 0;
  if (argv[i][1] == 'd' && !argv[i][2]) {
    Fluid.debug_external_editor=1;
    i++; return 1;
  }
  if (argv[i][1] == 'u' && !argv[i][2]) {
    update_file++;
    Fluid.batch_mode++;
    i++; return 1;
  }
  if (argv[i][1] == 'c' && !argv[i][2]) {
    compile_file++;
    Fluid.batch_mode++;
    i++; return 1;
  }
  if ((strcmp(argv[i], "-v")==0) || (strcmp(argv[i], "--version")==0)) {
    show_version = 1;
    i++; return 1;
  }
  if (argv[i][1] == 'c' && argv[i][2] == 's' && !argv[i][3]) {
    compile_file++;
    compile_strings++;
    Fluid.batch_mode++;
    i++; return 1;
  }
  if (argv[i][1] == 'o' && !argv[i][2] && i+1 < argc) {
    code_filename = argv[i+1];
    Fluid.batch_mode++;
    i += 2; return 2;
  }
#ifndef NDEBUG
  if ((i+1 < argc) && (strcmp(argv[i], "--autodoc") == 0)) {
    autodoc_path = argv[i+1];
    i += 2; return 2;
  }
#endif
  if (strcmp(argv[i], "--help")==0) {
    return 0;
  }
  if (argv[i][1] == 'h' && !argv[i][2]) {
    if ( (i+1 < argc) && (argv[i+1][0] != '-') ) {
      header_filename = argv[i+1];
      Fluid.batch_mode++;
      i += 2;
      return 2;
    } else {
      // a lone "-h" without a filename will output the help string
      return 0;
    }
  }
  return 0;
}

