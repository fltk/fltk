//
// FLUID template support for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2020 by Bill Spitzak and others.
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

// generated by Fast Light User Interface Designer (fluid) version 1.0500

#include "template_panel.h"
#include "Fluid.h"
#include "tools/filename.h"
#include <FL/Fl_Shared_Image.H>
#include <FL/fl_ask.H>
#include <FL/fl_string_functions.h>
#include "../src/flstring.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <zlib.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <io.h>
#else
#include <unistd.h>
#endif // _WIN32 && !__CYGWIN__

Fl_Double_Window *template_panel=(Fl_Double_Window *)0;

static void cb_template_panel(Fl_Double_Window*, void*) {
  Fl_Shared_Image *img = (Fl_Shared_Image *)template_preview->image();
  if (img) img->release();
  template_preview->image(0);

  template_browser->deselect();
  template_name->value("");
  template_instance->value("");
  template_panel->hide();
}

Fl_Browser *template_browser=(Fl_Browser *)0;

static void cb_template_browser(Fl_Browser*, void*) {
  if (Fl::event_clicks()) {
    template_panel->hide();
    return;
  }
  Fl_Shared_Image *img = (Fl_Shared_Image *)template_preview->image();
  if (img) img->release();
  template_preview->image(0);
  template_preview->redraw();

  int item = template_browser->value();

  if (item <= 1) template_instance->deactivate();
  else template_instance->activate();

  if (item < 1) {
    template_submit->deactivate();
    template_delete->deactivate();
    return;
  }

  template_submit->activate();

  const char *flfile = (const char *)template_browser->data(item);
  if (!flfile) {
    template_delete->deactivate();
    return;
  }

  template_name->value(template_browser->text(item));

  template_delete->activate();

  char pngfile[1024], *ext;

  strlcpy(pngfile, flfile, sizeof(pngfile));
  if ((ext = strrchr(pngfile, '.')) == nullptr) return;
  strcpy(ext, ".png");

  img = Fl_Shared_Image::get(pngfile);

  if (img) {
    template_preview->image(img);
    template_preview->redraw();
  }
}

Fl_Box *template_preview=(Fl_Box *)0;

Fl_Input *template_name=(Fl_Input *)0;

static void cb_template_name(Fl_Input*, void*) {
  if (strlen(template_name->value())) {
    template_submit->activate();
    if (Fl::event_key() == FL_Enter) template_panel->hide();
  } else template_submit->deactivate();
}

Fl_Input *template_instance=(Fl_Input *)0;

Fl_Button *template_delete=(Fl_Button *)0;

static void cb_Cancel(Fl_Button*, void*) {
  Fl_Shared_Image *img = (Fl_Shared_Image *)template_preview->image();
  if (img) img->release();
  template_preview->image(0);

  template_browser->deselect();
  template_name->value("");
  template_instance->value("");
  template_panel->hide();
}

Fl_Return_Button *template_submit=(Fl_Return_Button *)0;

static void cb_template_submit(Fl_Return_Button*, void*) {
  Fl_Shared_Image *img = (Fl_Shared_Image *)template_preview->image();
  if (img) img->release();
  template_preview->image(0);

  template_panel->hide();
}

Fl_Double_Window* make_template_panel() {
  { template_panel = new Fl_Double_Window(460, 355, "New/Save Template");
    template_panel->callback((Fl_Callback*)cb_template_panel);
    { template_browser = new Fl_Browser(10, 28, 180, 250, "Available Templates:");
      template_browser->type(2);
      template_browser->labelfont(1);
      template_browser->callback((Fl_Callback*)cb_template_browser);
      template_browser->align(Fl_Align(FL_ALIGN_TOP_LEFT));
      template_browser->when(FL_WHEN_CHANGED | FL_WHEN_NOT_CHANGED);
    } // Fl_Browser* template_browser
    { template_preview = new Fl_Box(200, 28, 250, 250);
      template_preview->box(FL_THIN_DOWN_BOX);
      template_preview->align(Fl_Align(FL_ALIGN_CLIP|FL_ALIGN_INSIDE));
      Fl_Group::current()->resizable(template_preview);
    } // Fl_Box* template_preview
    { template_name = new Fl_Input(198, 288, 252, 25, "Template Name:");
      template_name->labelfont(1);
      template_name->textfont(4);
      template_name->callback((Fl_Callback*)cb_template_name);
      template_name->when(FL_WHEN_CHANGED | FL_WHEN_NOT_CHANGED);
    } // Fl_Input* template_name
    { template_instance = new Fl_Input(198, 288, 252, 25, "Instance Name:");
      template_instance->labelfont(1);
      template_instance->textfont(4);
      template_instance->hide();
    } // Fl_Input* template_instance
    { Fl_Group* o = new Fl_Group(10, 323, 440, 25);
      { template_delete = new Fl_Button(10, 323, 143, 25, "Delete Template");
        template_delete->callback((Fl_Callback*)template_delete_cb);
      } // Fl_Button* template_delete
      { Fl_Box* o = new Fl_Box(153, 323, 126, 25);
        Fl_Group::current()->resizable(o);
      } // Fl_Box* o
      { Fl_Button* o = new Fl_Button(289, 323, 72, 25, "Cancel");
        o->callback((Fl_Callback*)cb_Cancel);
      } // Fl_Button* o
      { template_submit = new Fl_Return_Button(371, 323, 79, 25, "Save");
        template_submit->callback((Fl_Callback*)cb_template_submit);
      } // Fl_Return_Button* template_submit
      o->end();
    } // Fl_Group* o
    template_panel->set_modal();
    template_panel->end();
  } // Fl_Double_Window* template_panel
  return template_panel;
}

void template_clear() {
  int i;
  void *filename;

  for (i = 1; i <= template_browser->size(); i ++) {
    if ((filename = template_browser->data(i)) != nullptr) free(filename);
  }

  template_browser->deselect();
  template_browser->clear();
}

void template_delete_cb(Fl_Button *, void *) {
  int item = template_browser->value();
  if (item < 1) return;

  const char *name = template_browser->text(item);
  const char *flfile = (const char *)template_browser->data(item);
  if (!flfile) return;

  if (!fl_choice("Are you sure you want to delete the template \"%s\"?",
                 "Cancel", "Delete", 0, name)) return;

  if (fl_unlink(flfile)) {
    fl_alert("Unable to delete template \"%s\":\n%s", name, strerror(errno));
    return;
  }

  char pngfile[1024], *ext;
  strlcpy(pngfile, flfile, sizeof(pngfile));
  if ((ext = strrchr(pngfile, '.')) != nullptr) {
    strcpy(ext, ".png");
    fl_unlink(pngfile);
  }

  template_browser->remove(item);
  template_browser->do_callback();
}

static int tmpl_FLTK_License_fl_size = 614;
static unsigned char tmpl_FLTK_License_fl[397] = /* data compressed and inlined from ../templates/FLTK_License.fl */
{120,156,133,82,77,79,27,49,16,189,239,175,120,130,11,72,237,110,130,90,169,
112,130,134,82,69,160,20,137,244,208,19,242,174,103,215,35,28,123,101,123,89,
210,40,255,157,241,146,168,199,250,100,143,223,215,140,125,10,173,146,66,203,
150,208,250,128,100,8,119,54,189,224,119,164,128,165,75,20,90,213,16,110,41,114,
231,164,116,214,218,129,245,121,241,74,33,178,119,152,151,179,47,179,121,97,72,
105,10,207,78,109,8,187,210,236,139,198,107,58,30,155,183,183,92,216,108,200,37,
236,170,170,168,42,92,47,87,79,235,155,213,226,199,245,63,95,21,19,30,184,51,9,
107,239,45,238,57,225,236,238,97,125,127,94,22,31,164,133,239,183,97,2,204,47,
47,191,125,190,152,93,124,69,189,197,119,182,22,79,61,167,191,234,5,202,105,
120,145,11,241,200,90,27,142,176,92,7,21,182,144,109,27,136,16,125,155,70,21,
168,196,45,199,20,184,30,82,110,39,179,135,72,152,92,34,4,0,63,36,203,142,52,
216,101,181,156,116,26,215,201,226,215,227,159,229,234,231,9,70,195,141,65,52,
126,176,26,70,189,18,106,34,39,248,198,14,90,136,35,39,35,188,108,45,196,18,88,
182,211,49,203,77,82,114,179,225,24,217,117,144,89,104,181,81,29,233,79,136,146,
51,219,89,110,200,73,40,149,174,14,45,229,101,82,234,227,85,85,141,227,88,182,
242,100,165,15,93,117,200,84,246,166,63,64,31,45,41,225,30,181,90,111,173,31,
179,83,47,38,144,150,141,31,145,60,2,245,62,36,212,67,23,167,41,72,158,129,226,
127,13,51,254,232,182,199,142,221,115,244,67,144,31,35,187,143,79,81,236,139,
119,230,100,201,193};

static int tmpl_1of7GUIs_fl_size = 763;
static unsigned char tmpl_1of7GUIs_fl[486] = /* data compressed and inlined from ../templates/1of7GUIs.fl */
{120,156,109,82,203,138,219,64,16,188,207,87,52,228,178,102,177,45,25,59,
187,142,73,14,121,56,187,36,224,92,76,142,102,36,181,164,206,142,102,196,60,252,
88,33,216,223,200,61,127,146,63,201,151,164,37,69,176,9,97,4,163,26,122,170,
170,171,231,5,100,210,75,200,73,33,228,198,130,47,17,182,202,63,192,222,161,133,
123,237,209,230,50,69,120,143,142,10,205,71,87,185,10,148,77,4,28,209,58,50,26,
226,89,180,140,34,1,37,202,12,237,65,203,10,161,153,149,173,72,77,134,35,76,207,
231,86,64,106,170,10,181,135,70,196,96,114,184,249,184,191,119,66,244,27,156,
164,131,4,81,67,106,81,122,204,128,177,4,87,147,158,154,60,239,202,59,107,149,
116,236,232,215,211,119,39,24,58,114,240,206,84,181,180,228,216,9,215,236,146,
111,152,250,233,206,18,235,116,36,58,131,109,208,169,103,167,82,137,47,214,20,
86,86,21,233,162,239,150,149,185,179,35,42,83,247,198,146,11,124,8,5,155,248,
68,142,229,125,167,41,238,66,37,245,180,211,9,126,204,68,246,140,80,88,19,234,
209,219,103,164,68,211,163,216,107,234,179,241,63,127,120,184,147,90,27,134,64,
26,22,81,188,156,9,81,122,95,187,87,243,249,77,17,200,205,10,242,101,72,102,
100,6,60,23,45,52,14,21,119,193,246,73,31,156,9,150,243,215,198,31,24,13,33,139,
86,136,177,43,104,248,130,169,81,119,23,197,86,29,190,146,206,204,169,63,22,74,
38,168,56,162,208,121,134,190,234,124,57,149,208,44,111,95,194,98,189,128,120,
189,132,213,170,5,127,169,121,198,38,36,252,12,44,231,250,40,187,191,35,57,226,
125,100,222,5,207,17,240,20,123,186,195,137,178,2,187,97,14,148,241,10,248,187,
141,96,177,24,102,31,65,243,119,233,244,205,81,170,128,87,209,100,211,114,11,
204,248,54,120,255,167,133,231,94,69,42,149,74,100,250,0,13,241,84,8,94,195,191,
76,52,80,77,54,130,174,175,55,226,255,66,212,9,13,238,214,235,103,238,134,245,
27,226,34,7,2};

void template_install(const char *path, const char *name, const uchar *inSrc, int inSrcLen, int inDstLen) {
  char filename[FL_PATH_MAX];
    strcpy(filename, path);
    strcat(filename, name);
    FILE *f = fopen(filename, "wb");
    if (!f) return;
    uLong dstLen = inDstLen;
    Bytef *dst = (Bytef*)::malloc(inDstLen);
    if (uncompress(dst, &dstLen, (Bytef*)inSrc, (uLong)inSrcLen) != Z_OK) { /* error */ }
    if (fwrite(dst, dstLen, 1, f) <= 0) { /* error */ }
    fclose(f);
}

void template_load() {
  int i;
  char name[1024], filename[1400], path[1024], *ptr;
  struct dirent **files;
  int num_files;

  Fluid.preferences.getUserdataPath(path, sizeof(path));
  strlcat(path, "templates", sizeof(path));
  fl_make_path(path);

  int sample_templates_generated = 0;
  Fluid.preferences.get("sample_templates_generated", sample_templates_generated, 0);

  if (sample_templates_generated < 2) {
    strcpy(filename, path);
    strcat(filename, "/FLTK_License.fl");
    FILE *f = fopen(filename, "wb");
    if (f) {
      fputs(
  "# data file for the Fltk User Interface Designer (fluid)\nversion 1.0400\nheader_name {.h}\n"
  "code_name {.cxx}\ncomment {//\n// @INSTANCE@ for the Fast Light Tool Kit (FLT"
  "K).\n//\n// Copyright 1998-2023 by Bill Spitzak and others.\n//\n// This library is free sof"
  "tware. Distribution and use rights are outlined in\n// the file \"COPYING\" which should have "
  "been included with this file.  If this\n// file is missing or damaged, see the license at:\n"
  "//\n//     https://www.fltk.org/COPYING.php\n//\n// Please see the following page on how to report "
  "bugs and issues:\n//\n//     https://www.fltk.org/bugs.php\n//\n} {selected in_source in_head"
  "er\n}\n", f);
      fclose(f);
    }

    template_install(path, "/FLTK_License.fl", tmpl_FLTK_License_fl, sizeof(tmpl_FLTK_License_fl), tmpl_FLTK_License_fl_size);
    template_install(path, "/1of7GUIs.fl", tmpl_1of7GUIs_fl, sizeof(tmpl_1of7GUIs_fl), tmpl_1of7GUIs_fl_size);
    sample_templates_generated = 2;
    Fluid.preferences.set("sample_templates_generated", sample_templates_generated);
    Fluid.preferences.flush();
  }

  num_files = fl_filename_list(path, &files);

  for (i = 0; i < num_files; i ++) {
    if (fl_filename_match(files[i]->d_name, "*.fl")) {
      // Format the name as the filename with "_" replaced with " "
      // and without the trailing ".fl"...
      strlcpy(name, files[i]->d_name, sizeof(name));
      *strstr(name, ".fl") = '\0';

      for (ptr = name; *ptr; ptr ++) {
        if (*ptr == '_') *ptr = ' ';
      }

      // Add the template to the browser...
      snprintf(filename, sizeof(filename), "%s/%s", path, files[i]->d_name);
      template_browser->add(name, fl_strdup(filename));
    }

    free(files[i]);
  }

  if (num_files > 0) free(files);
}
