//
// PDF documentation tool to generate a png image from a Doxygen `@code`
// segment with international characters for the Fast Light Tool Kit (FLTK).
//
// Copyright 2025 by Bill Spitzak and others.
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

//
// Our documentation for the FLTK unicode contains international characters
// to illustrate use of non ASCII characters in the GUI. To generate PDF
// output, Doxygen uses LaTeX which can not easily handle UTF-8 characters in
// beyond Western encoding. This tool generates PNG images from code segments
// containing international characters so that they can be included in the
// PDF documentation instead of the code segments with UTF-8 characters.
//

#include <stdio.h>

#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/filename.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_PNG_Image.H>

#include "../fluid/widgets/Code_Editor.h"
#include "../fluid/widgets/Code_Viewer.h"
#include "../fluid/widgets/Style_Parser.h"

Fl_Window* window = nullptr;
Fl_Group* group = nullptr;
fld::widget::Code_Viewer* code_viewer = nullptr;

void create_window() {
  window = new Fl_Window(1024, 100);
  group = new Fl_Group(0, 0, 1024, 100);
  group->color(0xf7f7ff00);
  group->box(FL_FLAT_BOX);

  code_viewer = new fld::widget::Code_Viewer(5, 5, 1014, 94);
  code_viewer->box(FL_FLAT_BOX);
  code_viewer->color(0xf7f7ff00);
  code_viewer->textsize(30);

  window->resizable(group);
  group->resizable(code_viewer);
}

void save_snapshot(const char* code, const char* filename)
{
//  fprintf(stderr, "\\code\n%s\n\\endcode\n", code);

  code_viewer->buffer()->text(code);
  int n_lines = 1;
  for (const char* s=code; *s; ++s) if (*s == '\n') n_lines++;
  // 300 dpi for 7 inches = 2000 pixels
  window->size(2100, 6 + 34*n_lines );

  // Generate the Image Surface
  Fl_Image_Surface *srfc = new Fl_Image_Surface(window->w(), window->h());

  // Draw the window and its content
  Fl_Image_Surface::push_current(srfc);
  srfc->draw(group, 0, 0);
  fl_rect(0, 0, window->w(), window->h(), 0xccccff00);
  Fl_Image_Surface::pop_current();

//  fprintf(stderr, "  Saving to \"%s\".\n", filename);

  // Write the generated image
  Fl_RGB_Image *img = srfc->image();
  fl_write_png(filename, img);

  // Clean up
  delete img;
  delete srfc;
}

/**
  Main entry point for the PDF documentation helper tool.

  The app scans the input file for the `\\code_international{"filename"}`
  directive, reads the following code segment until
  `\\endcode_international`, and generates a PNG image file with the given
  filename containing the code segment rendered with FLTK's
  code rendering capabilities.

  \param argc Argument count
  \param argv a list of input files with documentation in Doxygen format
  \return Exit code (0 for success, non-zero for failure)
*/
int main(int argc, char *argv[])
{
  int ret = 0;
  char line[1024];
  char cwd[FL_PATH_MAX];

//  fl_getcwd(cwd, FL_PATH_MAX-1);
//  fprintf(stderr, "code_snapshot:\n");
//  fprintf(stderr, "Working directory is \"%s\".\n", cwd);

  create_window();

  for (int i = 1; i < argc; i++) {
    FILE* f = fl_fopen(argv[i], "rb");
    if (!f) {
      fl_getcwd(cwd, FL_PATH_MAX-1);
      fprintf(stderr, "code_snapshot:\nCan't open file \"%s\".\n", argv[i]);
      fprintf(stderr, "Working directory is \"%s\".\n", cwd);
      ret = -1;
      break;
    }

//    fprintf(stderr, "Reading \"%s\".\n", argv[i]);

    std::string code;
    std::string filename;
    bool in_code_block = false;
    for (;;) {
      fgets(line, 1023, f);
      if (feof(f)) break;
      if (in_code_block) {
        if (strstr(line, "\\endcode_international")) {
          if (!code.empty()) {
            code.resize( code.size()-1 );
            save_snapshot(code.c_str(), filename.c_str());
          }
          in_code_block = false;
        } else {
          code += line;
        }
      } else {
        if (strstr(line, "\\code_international")) {
          const char* fn_start = strstr(line, "{\"");
          const char* fn_end = strstr(line, "\"}");
          if (fn_start && fn_end && (fn_end > fn_start)) {
            filename = std::string(fn_start+2, fn_end-fn_start-2);
            in_code_block = true;
          }
        }
      }
    }
    fclose(f);
  }

  delete window;

  return ret;
}
