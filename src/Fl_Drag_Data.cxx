//
// Drag and drop data for the Fast Light Tool Kit (FLTK).
//
// Copyright 2026 by Bill Spitzak and others.
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

#include <FL/Fl_Drag_Data.H>
#include <FL/Fl_Image.H>
#include <FL/fl_utf8.h>
#include <FL/filename.H>
#include "Fl_Screen_Driver.H"
#include "flstring.h"
#include <stdio.h>
#include <stdlib.h>


// Temporary files written by write_promises_to_temp_dir() for the most recent drag.
// They are removed when the next drag writes its files, and when the program exits.
static std::vector<std::string> temp_files;
static std::string temp_dir;

static void remove_temp_files() {
  for (size_t i = 0; i < temp_files.size(); i++) fl_unlink(temp_files[i].c_str());
  temp_files.clear();
  if (!temp_dir.empty()) fl_rmdir(temp_dir.c_str());
  temp_dir.clear();
}


// Returns a private copy of img with the full resolution of its pixel data
static Fl_RGB_Image *copy_image(const Fl_RGB_Image *img) {
  if (!img || img->w() <= 0 || img->h() <= 0) return NULL;
  Fl_RGB_Image *c = (Fl_RGB_Image *)img->copy(img->data_w(), img->data_h());
  c->normalize(); // rasterize Fl_SVG_Image
  c->scale(img->w(), img->h(), 0, 1);
  return c;
}


/** Creates an empty drag data object. */
Fl_Drag_Data::Fl_Drag_Data() :
  has_text_(false),
  image_(NULL),
  actions_(FL_DND_COPY),
  drag_image_(NULL),
  hot_x_(0), hot_y_(0),
  callback_(NULL),
  callback_data_(NULL),
  done_(false)
{
}


/** Deletes the drag data object.
 There is no need to call this: Fl::dnd(Fl_Drag_Data*) takes ownership of the object.
 */
Fl_Drag_Data::~Fl_Drag_Data() {
  delete image_;
  delete drag_image_;
}


/** Offers UTF-8 encoded text.
 \param[in] utf8 the text
 \param[in] len the number of bytes in \p utf8, or -1 if \p utf8 is nul-terminated
 */
void Fl_Drag_Data::text(const char *utf8, int len) {
  if (!utf8) { has_text_ = false; text_.clear(); return; }
  if (len < 0) len = (int)strlen(utf8);
  text_.assign(utf8, len);
  has_text_ = true;
}


/** Offers arbitrary data of a given MIME type, for example "image/png" or "application/pdf".
 The data is copied. This can be called several times for different MIME types.
 */
void Fl_Drag_Data::data(const char *mime_type, const void *bytes, size_t len) {
  if (!mime_type || !bytes) return;
  Data d;
  d.mime_type = mime_type;
  d.bytes.assign((const char *)bytes, len);
  data_.push_back(d);
}


/** Offers an image as bitmap data in the platform's native image formats.
 The image is copied, so the caller keeps ownership of \p img.
 This does not create a file when dropped onto a file manager; use
 file_promise() for that.
 */
void Fl_Drag_Data::image(const Fl_RGB_Image *img) {
  delete image_;
  image_ = copy_image(img);
}


/** Offers an existing file.
 Call this once per file to drag several files.
 \param[in] path absolute path of the file
 */
void Fl_Drag_Data::file(const char *path) {
  if (!path || !*path) return;
  File f;
  f.path = path;
  f.name = fl_filename_name(path);
  f.writer = NULL;
  f.user_data = NULL;
  files_.push_back(f);
}


/** Offers a file that is created only when needed.
 Call this once per file to drag several files.
 \param[in] name the suggested file name, without directory, for example "Picture.png"
 \param[in] mime_type the MIME type of the file, for example "image/png", or NULL;
            used by some platforms to describe the file to the drop target
 \param[in] writer a callback that writes the file to the given path
 \param[in] user_data passed to \p writer
 \see File_Writer
 */
void Fl_Drag_Data::file_promise(const char *name, const char *mime_type,
                                File_Writer writer, void *user_data) {
  if (!name || !*name || !writer) return;
  File f;
  f.name = fl_filename_name(name);
  if (mime_type) f.mime_type = mime_type;
  f.writer = writer;
  f.user_data = user_data;
  files_.push_back(f);
}


/** Sets the image shown under the mouse pointer during the drag.
 The image is copied. Without a drag image, the platform's default
 representation is used.
 \param[in] icon the image
 \param[in] hot_x, hot_y position of the mouse pointer in the image
 */
void Fl_Drag_Data::drag_image(const Fl_RGB_Image *icon, int hot_x, int hot_y) {
  delete drag_image_;
  drag_image_ = copy_image(icon);
  hot_x_ = hot_x;
  hot_y_ = hot_y;
}


/** \cond DriverDev */

/* Write file i to path, using its writer for a promise or copying an existing file.
 Returns 0 on success.
 */
int Fl_Drag_Data::write_file(int i, const char *path) {
  File &f = files_[i];
  if (f.writer) return f.writer(path, f.user_data);
  FILE *in = fl_fopen(f.path.c_str(), "rb");
  if (!in) return -1;
  FILE *out = fl_fopen(path, "wb");
  if (!out) { fclose(in); return -1; }
  char buffer[16384];
  size_t n;
  int ret = 0;
  while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
    if (fwrite(buffer, 1, n, out) != n) { ret = -1; break; }
  }
  if (ferror(in)) ret = -1;
  fclose(in);
  if (fclose(out)) ret = -1;
  return ret;
}


/* For platforms that don't support file promises: writes all promised files
 into a new temporary directory and records their paths, so that they can be
 dragged as existing files. Files of the previous drag are removed first.
 Returns 0 if all files were written.
 */
int Fl_Drag_Data::write_promises_to_temp_dir() {
  bool have_promises = false;
  for (size_t i = 0; i < files_.size(); i++) {
    if (files_[i].writer && files_[i].path.empty()) have_promises = true;
  }
  if (!have_promises) return 0;
  static bool registered = false;
  if (!registered) { atexit(remove_temp_files); registered = true; }
  remove_temp_files();

  temp_dir = Fl_Screen_Driver::dnd_make_temp_dir("fltk-dnd");
  if (temp_dir.empty()) return -1;

  int ret = 0;
  std::vector<std::string> used_names;
  for (size_t i = 0; i < files_.size(); i++) {
    File &f = files_[i];
    if (!f.writer || !f.path.empty()) continue;
    // make the name unique in the directory: "name.ext", "name 2.ext", ...
    std::string name = f.name;
    const char *ext = fl_filename_ext(f.name.c_str());
    std::string base(f.name.c_str(), ext - f.name.c_str());
    for (int n = 2; ; n++) {
      bool used = false;
      for (size_t j = 0; j < used_names.size(); j++) if (used_names[j] == name) used = true;
      if (!used) break;
      char num[20];
      snprintf(num, sizeof(num), " %d", n);
      name = base + num + ext;
    }
    used_names.push_back(name);
    std::string path = temp_dir + "/" + name;
    if (f.writer(path.c_str(), f.user_data) == 0) {
      f.path = path;
      temp_files.push_back(path);
    } else {
      fl_unlink(path.c_str());
      ret = -1;
    }
  }
  return ret;
}


/* Called by the platform driver when the drag ends. Calls the callback once. */
void Fl_Drag_Data::do_callback(int action) {
  if (done_) return;
  done_ = true;
  if (callback_) callback_(action, callback_data_);
}

/** \endcond */
