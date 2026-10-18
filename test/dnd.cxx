//
// Drag and drop test program for the Fast Light Tool Kit (FLTK).
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

// Drag the boxes in the top row onto the desktop, a file manager, or other
// applications. Drop anything onto the list at the bottom to see what arrives.

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Drag_Data.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/fl_utf8.h>
#include <FL/fl_draw.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

static Fl_Box *status_box = NULL;
static Fl_Choice *request_choice = NULL;
static std::string picture_png; // the picture as PNG data
static Fl_Box *dropped_image_box = NULL;
static Fl_RGB_Image *picture = NULL;
static std::string real_files[2];

static void set_status(const char *fmt, const char *arg) {
  static char buffer[300];
  snprintf(buffer, sizeof(buffer), fmt, arg);
  status_box->label(buffer);
}

static const char *action_name(int action) {
  switch (action) {
    case FL_DND_COPY: return "copied";
    case FL_DND_MOVE: return "moved";
    case FL_DND_LINK: return "linked";
    default: return "cancelled";
  }
}

static void drag_done(int action, void *what) {
  set_status("Drag of %s ended: ", (const char *)what);
  std::string s = status_box->label();
  s += action_name(action);
  status_box->copy_label(s.c_str());
  if (action == FL_DND_MOVE) {
    // the files are gone, create them again for the next test
    for (int i = 0; i < 2; i++) {
      FILE *f = fl_fopen(real_files[i].c_str(), "w");
      if (f) { fprintf(f, "Real file %d\n", i + 1); fclose(f); }
    }
  }
}

static int write_png(const char *path, void *img) {
  return fl_write_png(path, (Fl_RGB_Image *)img);
}

static int write_note(const char *path, void *index) {
  FILE *f = fl_fopen(path, "w");
  if (!f) return -1;
  fprintf(f, "This is note %d, written when it was dropped.\n", (int)(fl_intptr_t)index);
  return fclose(f);
}

// A box that starts a drag and drop operation when dragged
class Drag_Source : public Fl_Box {
  int kind_;
public:
  Drag_Source(int X, int Y, int W, int H, const char *L, int kind)
    : Fl_Box(FL_UP_BOX, X, Y, W, H, L), kind_(kind) {
    align(FL_ALIGN_WRAP | FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM);
  }
  int handle(int event) override {
    switch (event) {
      case FL_PUSH:
        return 1;
      case FL_DRAG:
        if (!Fl::event_is_click()) start_drag();
        return 1;
    }
    return Fl_Box::handle(event);
  }
  void start_drag() {
    Fl_Drag_Data *d = new Fl_Drag_Data();
    switch (kind_) {
      case 0: // an image, as bitmap data, as PNG data, and as a PNG file
        d->image(picture);
        if (!picture_png.empty()) d->data("image/png", picture_png.data(), picture_png.length());
        d->file_promise("FLTK Picture.png", "image/png", write_png, picture);
        d->callback(drag_done, (void *)"the picture");
        break;
      case 1: // three promised files
        for (int i = 1; i <= 3; i++) {
          char name[40];
          snprintf(name, sizeof(name), "Note %d.txt", i);
          d->file_promise(name, "text/plain", write_note, (void *)(fl_intptr_t)i);
        }
        d->callback(drag_done, (void *)"three notes");
        break;
      case 2: // two existing files that may be moved
        for (int i = 0; i < 2; i++) d->file(real_files[i].c_str());
        d->actions(FL_DND_COPY | FL_DND_MOVE);
        d->callback(drag_done, (void *)"two real files");
        break;
      default: // text
        d->text("Hello from FLTK's drag and drop test");
        d->callback(drag_done, (void *)"the text");
        break;
    }
    Fl::dnd(d);
  }
};

// A list that shows everything dropped onto it
class Drop_Target : public Fl_Browser {
  void show_offer() {
    std::string s = "Offered:";
    if (Fl::dnd_contains(Fl::clipboard_files)) s += " files";
    if (Fl::dnd_contains(Fl::clipboard_plain_text)) s += " text";
    if (Fl::dnd_contains(Fl::clipboard_image)) s += " image";
    if (Fl::dnd_contains("image/png")) s += " (image/png)";
    s += ", proposed action: ";
    s += action_name(Fl::event_dnd_action());
    status_box->copy_label(s.c_str());
  }
public:
  Drop_Target(int X, int Y, int W, int H) : Fl_Browser(X, Y, W, H) {
    add("@iDrop text, files, or images here");
  }
  int handle(int event) override {
    switch (event) {
      case FL_DND_ENTER:
        show_offer();
        switch (request_choice->value()) {
          case 1: Fl::dnd_request(Fl::clipboard_image); break;
          case 2: Fl::dnd_request("image/png"); break;
          case 3: Fl::dnd_request(Fl::clipboard_plain_text); break;
          default: break;
        }
        return 1;
      case FL_DND_DRAG:
        return 1;
      case FL_DND_RELEASE:
        show_offer();
        return 1;
      case FL_PASTE: {
        clear();
        char line[200];
        const char *type = Fl::event_clipboard_type();
        snprintf(line, sizeof(line), "@bType: %s, %d bytes, %s",
                 type ? type : "(none)", Fl::event_length(), action_name(Fl::event_dnd_action()));
        add(line);
        if (type == Fl::clipboard_image) {
          Fl_RGB_Image *img = (Fl_RGB_Image *)Fl::event_clipboard();
          snprintf(line, sizeof(line), "@.Image of %d x %d pixels, depth %d",
                   img->data_w(), img->data_h(), img->d());
          add(line);
          Fl_Image *old = dropped_image_box->image();
          img->scale(dropped_image_box->w() - 4, dropped_image_box->h() - 4, 1, 1);
          dropped_image_box->image(img); // takes ownership of the image, see below
          delete old;
          dropped_image_box->redraw();
          return 1; // we own the image now
        }
        if (type && !strcmp(type, "image/png")) { // raw data
          const unsigned char *p = (const unsigned char *)Fl::event_text();
          std::string hex = "@.Data starts with:";
          for (int i = 0; i < Fl::event_length() && i < 8; i++) {
            char b[8];
            snprintf(b, sizeof(b), " %02X", p[i]);
            hex += b;
          }
          add(hex.c_str());
          return 1;
        }
        if (type == Fl::clipboard_files) {
          for (int i = 0; i < Fl::dnd_file_count(); i++) {
            snprintf(line, sizeof(line), "File %d: ", i + 1);
            add((std::string("@.") + line + Fl::dnd_file(i)).c_str());
          }
          return 1;
        }
        std::string text(Fl::event_text() ? Fl::event_text() : "", Fl::event_length());
        size_t start = 0;
        while (start <= text.length()) {
          size_t end = text.find('\n', start);
          if (end == std::string::npos) end = text.length();
          add(("@." + text.substr(start, end - start)).c_str());
          start = end + 1;
        }
        return 1;
      }
    }
    return Fl_Browser::handle(event);
  }
};

static Fl_RGB_Image *make_picture(int W, int H) {
  uchar *data = new uchar[W * H * 4];
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      uchar *p = data + (y * W + x) * 4;
      int dx = x - W / 2, dy = y - H / 2;
      bool inside = (dx * dx + dy * dy) < (H * H) / 5;
      p[0] = inside ? 230 : (uchar)(255 * x / W);
      p[1] = inside ? 200 : (uchar)(255 * y / H);
      p[2] = inside ? 40 : 160;
      p[3] = (x < 4 || y < 4 || x >= W - 4 || y >= H - 4) ? 0 : 255; // transparent border
    }
  }
  Fl_RGB_Image *img = new Fl_RGB_Image(data, W, H, 4);
  img->alloc_array = 1;
  return img;
}

int main(int argc, char **argv) {
  fl_register_images(); // X11 and Wayland need this to receive dropped PNG images
  const char *tmp = fl_getenv("TMPDIR");
  if (!tmp || !*tmp) tmp = fl_getenv("TEMP");
  if (!tmp || !*tmp) tmp = "/tmp";
  for (int i = 0; i < 2; i++) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/fltk dnd test file %d.txt", tmp, i + 1);
    real_files[i] = path;
    FILE *f = fl_fopen(path, "w");
    if (f) { fprintf(f, "Real file %d\n", i + 1); fclose(f); }
  }
  picture = make_picture(160, 120);
  { // get the picture as PNG data
    std::string png_path = std::string(tmp) + "/fltk dnd test picture.png";
    if (fl_write_png(png_path.c_str(), picture) == 0) {
      FILE *f = fl_fopen(png_path.c_str(), "rb");
      if (f) {
        char buffer[4096];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0) picture_png.append(buffer, n);
        fclose(f);
      }
      fl_unlink(png_path.c_str());
    }
  }

  Fl_Double_Window win(560, 450, "Drag and Drop Test");
  Drag_Source *pic = new Drag_Source(10, 10, 125, 130, "Picture (image + PNG file)", 0);
  Fl_RGB_Image *thumb = (Fl_RGB_Image *)picture->copy();
  thumb->scale(100, 75);
  pic->image(thumb);
  new Drag_Source(145, 10, 125, 130, "Three promised files", 1);
  new Drag_Source(280, 10, 125, 130, "Two real files (copy or move)", 2);
  new Drag_Source(415, 10, 135, 130, "Text", 3);
  request_choice = new Fl_Choice(130, 145, 180, 25, "When dropped, ask for:");
  request_choice->add("default (files or text)|image|image/png (raw data)|text");
  request_choice->value(0);
  Drop_Target *target = new Drop_Target(10, 175, 420, 230);
  // the image is drawn inside the box, so the caption is a separate widget
  dropped_image_box = new Fl_Box(FL_DOWN_BOX, 440, 175, 110, 110, NULL);
  new Fl_Box(440, 285, 110, 20, "dropped image");
  status_box = new Fl_Box(10, 415, 540, 25, "Drag one of the boxes above");
  status_box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  win.resizable(target);
  win.end();
  win.show(argc, argv);
  return Fl::run();
}
