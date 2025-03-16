//
// Fluid Project Templates code for the Fast Light Tool Kit (FLTK).
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

#include "app/templates.h"

#include "Fluid.h"
#include "io/Project_Writer.h"
#include "nodes/factory.h"
#include "nodes/Tree.h"
#include "nodes/Window_Node.h"
#include "panels/template_panel.h"

#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/Fl_PNG_Image.H>
#include "../src/flstring.h"

using namespace fld;
using namespace fld::app;

/**
 Save a design template.
 \todo We should document the concept of templates.
 */
void fld::app::save_template() {
  // Setup the template panel...
  if (!template_panel) make_template_panel();

  template_clear();
  template_browser->add("New Template");
  template_load();

  template_name->show();
  template_name->value("");

  template_instance->hide();

  template_delete->show();
  template_delete->deactivate();

  template_submit->label("Save");
  template_submit->deactivate();

  template_panel->label("Save Template");

  // Show the panel and wait for the user to do something...
  template_panel->show();
  while (template_panel->shown()) Fl::wait();

  // Get the template name, return if it is empty...
  const char *c = template_name->value();
  if (!c || !*c) return;

  // Convert template name to filename_with_underscores
  char savename[FL_PATH_MAX], *saveptr;
  strlcpy(savename, c, sizeof(savename));
  for (saveptr = savename; *saveptr; saveptr ++) {
    if (isspace(*saveptr)) *saveptr = '_';
  }

  // Find the templates directory...
  char filename[FL_PATH_MAX];
  Fluid.preferences.getUserdataPath(filename, sizeof(filename));

  strlcat(filename, "templates", sizeof(filename));
  if (fl_access(filename, 0)) fl_make_path(filename);

  strlcat(filename, "/", sizeof(filename));
  strlcat(filename, savename, sizeof(filename));

  char *ext = filename + strlen(filename);
  if (ext >= (filename + sizeof(filename) - 5)) {
    fl_alert("The template name \"%s\" is too long!", c);
    return;
  }

  // Save the .fl file...
  strcpy(ext, ".fl");

  if (!fl_access(filename, 0)) {
    if (fl_choice("The template \"%s\" already exists.\n"
                  "Do you want to replace it?", "Cancel",
                  "Replace", nullptr, c) == 0) return;
  }

  if (!fld::io::write_file(Fluid.proj, filename)) {
    fl_alert("Error writing %s: %s", filename, strerror(errno));
    return;
  }

#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
  // Get the screenshot, if any...
  Node *t;

  for (t = Fluid.proj.tree.first; t; t = t->next) {
    // Find the first window...
    if (t->is_a(Type::Window)) break;
  }

  if (!t) return;

  // Grab a screenshot...
  Window_Node *wt = (Window_Node *)t;
  uchar *pixels;
  int w, h;

  if ((pixels = wt->read_image(w, h)) == nullptr) return;

  // Save to a PNG file...
  strcpy(ext, ".png");

  errno = 0;
  if (fl_write_png(filename, pixels, w, h, 3) != 0) {
    delete[] pixels;
    fl_alert("Error writing %s: %s", filename, strerror(errno));
    return;
  }

#  if 0 // The original PPM output code...
  strcpy(ext, ".ppm");
  fp = fl_fopen(filename, "wb");
  fprintf(fp, "P6\n%d %d 255\n", w, h);
  fwrite(pixels, w * h, 3, fp);
  fclose(fp);
#  endif // 0

  delete[] pixels;
#endif // HAVE_LIBPNG && HAVE_LIBZ
}

