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

/** \file FL/Fl_Drag_Data.H
 \brief Fl_Drag_Data class.
 */

#ifndef Fl_Drag_Data_H
#define Fl_Drag_Data_H

#include <FL/Fl_Export.H>
#include <FL/Enumerations.H>
#include <stddef.h>
#include <string>
#include <vector>

class Fl_RGB_Image;

/**
 Describes the data offered by a drag and drop operation started with
 Fl::dnd(Fl_Drag_Data*).

 A drag can offer several representations of the same content at once,
 for example an image as bitmap data for image editors and as a file for
 file managers. The drop target picks the representation it understands.

 Any number of files can be added with file() and file_promise(), and both
 can be mixed. A file promise describes a file that does not exist yet:
 the application provides a suggested file name and a callback that writes
 the file. On platforms that support it (macOS, Windows), the callback is
 called when the file is dropped, with the destination chosen by the drop
 target. On other platforms the callback is called by Fl::dnd() to write the
 file into a temporary directory before the drag starts.

 Example: drag an image as a picture and as a PNG file
 \code
 #include <FL/Fl_Drag_Data.H>
 #include <FL/Fl_PNG_Image.H>  // fl_write_png(), requires the fltk_images library

 static int write_png(const char *path, void *img) {
   return fl_write_png(path, (Fl_RGB_Image *)img);
 }

 // in the handle() method of a widget, in response to FL_DRAG:
 Fl_Drag_Data *d = new Fl_Drag_Data();
 d->image(img);
 d->file_promise("Picture.png", "image/png", write_png, img);
 Fl::dnd(d); // Fl::dnd() takes ownership of d
 \endcode

 Example: drag several existing files
 \code
 Fl_Drag_Data *d = new Fl_Drag_Data();
 for (int i = 0; i < n; i++) d->file(paths[i]);
 d->actions(FL_DND_COPY | FL_DND_MOVE);
 d->callback(drag_done_cb, this); // drag_done_cb() deletes the files if action is FL_DND_MOVE
 Fl::dnd(d);
 \endcode

 \par Platform notes
 \li macOS: images are offered as PNG and TIFF data. File promises use
     NSFilePromiseProvider; before macOS 10.12, promised files are written
     before the drag starts.
 \li Windows: images are offered as CF_DIB. If at least one file is promised,
     all files are offered as virtual files (CFSTR_FILEDESCRIPTORW and
     CFSTR_FILECONTENTS), so that the drop target creates them. The drag image
     is shown by the shell's drag image helper.
 \li X11 and Wayland: images are offered as "image/bmp" data. Promised files
     are written into a temporary directory before the drag starts; the files
     are removed when the next drag starts and when the program exits.
     X11 shows no drag image.
 \li X11 programs running under Xwayland: whether they can drag data to
     Wayland programs depends on the compositor. FLTK programs use the Wayland
     backend by default in Wayland sessions.

 \see Fl::dnd(Fl_Drag_Data*)
 \version 1.5.0
 */
class FL_EXPORT Fl_Drag_Data {
public:
  /** Writes a promised file.
   \param[in] path  absolute path of the file to create
   \param[in] user_data  the value given to file_promise()
   \return 0 on success, any other value on error (like fl_write_png())
   */
  typedef int (*File_Writer)(const char *path, void *user_data);

  /** Reports the end of a drag and drop operation.
   \param[in] action  the action performed by the drop target, one of the Fl_Dnd_Action values;
          FL_DND_NONE if the drag was cancelled or refused
   \param[in] user_data  the value given to callback()
   */
  typedef void (*Done_Callback)(int action, void *user_data);

private:
  struct File {
    std::string path;       // path of an existing or written file, empty for unwritten promises
    std::string name;       // file name without directory
    std::string mime_type;  // may be empty
    File_Writer writer;     // NULL for existing files
    void *user_data;
  };
  struct Data {
    std::string mime_type;
    std::string bytes;
  };
  bool has_text_;
  std::string text_;
  std::vector<Data> data_;
  Fl_RGB_Image *image_;
  std::vector<File> files_;
  int actions_;
  Fl_RGB_Image *drag_image_;
  int hot_x_, hot_y_;
  Done_Callback callback_;
  void *callback_data_;
  bool done_;

public:
  Fl_Drag_Data();
  ~Fl_Drag_Data();
  Fl_Drag_Data(const Fl_Drag_Data &) = delete;
  Fl_Drag_Data &operator=(const Fl_Drag_Data &) = delete;

  void text(const char *utf8, int len = -1);
  void data(const char *mime_type, const void *bytes, size_t len);
  void image(const Fl_RGB_Image *img);
  void file(const char *path);
  void file_promise(const char *name, const char *mime_type, File_Writer writer, void *user_data);
  /** Sets the actions the drop target may choose from.
   \param[in] mask  a combination of FL_DND_COPY, FL_DND_MOVE, and FL_DND_LINK;
          the default is FL_DND_COPY
   */
  void actions(int mask) { actions_ = mask; }
  void drag_image(const Fl_RGB_Image *icon, int hot_x, int hot_y);
  /** Sets a callback that reports the action performed when the drag ends.
   The callback is called exactly once, after all promised files requested
   by the drop target have been written.
   */
  void callback(Done_Callback cb, void *user_data = NULL) { callback_ = cb; callback_data_ = user_data; }

  /** \cond DriverDev
   The following methods are used by the platform drivers. */
  bool has_text() const { return has_text_; }
  const std::string &text() const { return text_; }
  int data_count() const { return (int)data_.size(); }
  const char *data_mime_type(int i) const { return data_[i].mime_type.c_str(); }
  const std::string &data_bytes(int i) const { return data_[i].bytes; }
  const Fl_RGB_Image *image() const { return image_; }
  int file_count() const { return (int)files_.size(); }
  bool file_is_promise(int i) const { return files_[i].writer != NULL; }
  // path of an existing or already written file, NULL for unwritten promises
  const char *file_path(int i) const { return files_[i].path.empty() ? NULL : files_[i].path.c_str(); }
  const char *file_name(int i) const { return files_[i].name.c_str(); }
  const char *file_mime_type(int i) const { return files_[i].mime_type.c_str(); }
  int write_file(int i, const char *path);
  int write_promises_to_temp_dir();
  int actions() const { return actions_; }
  const Fl_RGB_Image *drag_image() const { return drag_image_; }
  int drag_image_hot_x() const { return hot_x_; }
  int drag_image_hot_y() const { return hot_y_; }
  void do_callback(int action);
  /** \endcond */
};

#endif // !Fl_Drag_Data_H
