//
// Hello, World! program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

// The Penpal test app is here to test pen/stylus/tablet event distribution
// in the Fl::Pen driver. Our main window has three canvases for drawing.
// The first canvas is a child of the main window. The second canvas is
// inside a group. The third canvas is a subwindow inside the main window.
// A second application window is itself yet another canvas.

// We can test if the events are delivered to the right receiver, if the
// mouse and pen offsets are correct. The pen implementation will also react
// to pen pressure and angles.

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>
#include <FL/names.h>

class CanvasInterface {
  Fl_Widget *widget_ { nullptr };
  bool in_window_ { false };
  bool first_draw_ { true };
  Fl_Offscreen offscreen_ { 0 };
  Fl_Color color_ { 1 };
  enum { NONE, HOVER, DRAW } overlay_ { NONE };
  int ov_x_ { 0 };
  int ov_y_ { 0 };
public:
  CanvasInterface(Fl_Widget *w) : widget_(w) { }
  CanvasInterface(Fl_Window *w) : widget_(w), in_window_(true) { }
  ~CanvasInterface() {
    if (offscreen_) fl_delete_offscreen(offscreen_);
  }
  int cv_handle(int event);
  void cv_draw();
  void cv_paint();
};

int CanvasInterface::cv_handle(int event) {
  switch (event) {
    // Event handling for mouse events:
    case FL_ENTER:
      color_++;
      if (color_ > 6) color_ = 1;
      /* fall through */
    case FL_MOVE:
      overlay_ = HOVER;
      ov_x_ = Fl::event_x();
      ov_y_ = Fl::event_y();
      widget_->redraw();
      return 1;
    case FL_PUSH: /* fall through */
    case FL_DRAG:
      overlay_ = DRAW;
      ov_x_ = Fl::event_x();
      ov_y_ = Fl::event_y();
      cv_paint();
      widget_->redraw();
      return 1;
    case FL_RELEASE: return 1;
    case FL_LEAVE: overlay_ = NONE; widget_->redraw(); return 1;

  }
  return 0;
}

void CanvasInterface::cv_draw() {
  if (first_draw_) {
    first_draw_ = false;
    offscreen_ = fl_create_offscreen(widget_->w(), widget_->h());
    fl_begin_offscreen(offscreen_);
    fl_color(FL_WHITE);
    fl_rectf(0, 0, widget_->w(), widget_->h());
    fl_end_offscreen();
  }
  int dx = in_window_ ? 0 : widget_->x(), dy = in_window_ ? 0 : widget_->y();
  fl_copy_offscreen(dx, dy, widget_->w(), widget_->h(), offscreen_, 0, 0);
  switch (overlay_) {
    case NONE: break;
    case HOVER:
      fl_color(FL_BLACK);
      fl_xyline(ov_x_-10, ov_y_, ov_x_+10);
      fl_yxline(ov_x_, ov_y_-10, ov_y_+10);
      break;
    case DRAW:
      fl_arc(ov_x_-10, ov_y_-10, 20, 20, 0, 360);
      break;
      break;
  }
}

void CanvasInterface::cv_paint() {
  if (!offscreen_)
    return;
  int dx = in_window_ ? 0 : widget_->x(), dy = in_window_ ? 0 : widget_->y();
  fl_begin_offscreen(offscreen_);
  fl_draw_circle(Fl::event_x()-dx-12, Fl::event_y()-dy-12, 24, color_);
  fl_end_offscreen();
}


class CanvasWidget : public Fl_Widget, CanvasInterface {
public:
  CanvasWidget(int x, int y, int w, int h, const char *l=nullptr)
  : Fl_Widget(x, y, w, h, l), CanvasInterface(this) { }
  ~CanvasWidget() override { }
  int handle(int event) override {
    auto ret = cv_handle(event);
    return ret ? ret : Fl_Widget::handle(event);
  }
  void draw() override { return cv_draw(); }
};

class CanvasWindow : public Fl_Window, CanvasInterface {
public:
  CanvasWindow(int x, int y, int w, int h, const char *l=nullptr)
  : Fl_Window(x, y, w, h, l), CanvasInterface(this) { }
  ~CanvasWindow() override { }
  int handle(int event) override {
    auto ret = cv_handle(event);
    return ret ? ret : Fl_Window::handle(event);
  }
  void draw() override { return cv_draw(); }
};


int main(int argc, char **argv) {
  Fl::Pen::State s = Fl::Pen::State::BUTTON0 | Fl::Pen::State::BUTTON2;

  auto window = new Fl_Window(100, 100, 640, 220);

  auto canvas_widget_0 = new CanvasWidget( 10, 10, 200, 200, "CV1");
  auto cv1_group = new Fl_Group(215, 5, 210, 210);
  cv1_group->box(FL_FRAME_BOX);
  auto canvas_widget_1 = new CanvasWidget(220, 10, 200, 200, "CV2");
  cv1_group->end();
  auto canvas_widget_2 = new CanvasWindow(430, 10, 200, 200, "CV3");
  canvas_widget_2->end();

  window->end();

  auto cv_window = new CanvasWindow(100, 380, 200, 200);

  Fl::Pen::subscribe(canvas_widget_0);
  Fl::Pen::subscribe(canvas_widget_1);
  Fl::Pen::subscribe(canvas_widget_2);
  Fl::Pen::subscribe(cv_window);

  window->show(argc, argv);
  canvas_widget_2->show();
  cv_window->show();

  return Fl::run();
}
