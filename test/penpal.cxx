//
// Penpal pen/stylus/tablet test program for the Fast Light Tool Kit (FLTK).
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

// The Penpal test app is here to test pen/stylus/tablet event distribution
// in the Fl::Pen driver. Our main window has three canvases for drawing.
// The first canvas is a child of the main window. The second canvas is
// inside a group. The third canvas is a subwindow inside the main window.
// A second application window is itself yet another canvas.

// We can test if the events are delivered to the right receiver, if the
// mouse and pen offsets are correct. The pen implementation also reacts
// to pen pressure and angles. If handle() returns 1 when receiving
// Fl::Pen::ENTER, the event handler should not send any mouse events until
// Fl::Pen::LEAVE.

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/platform.H>
#include <FL/fl_draw.H>
#include <FL/names.h>

//
// The canvas interface implements incremental drawing and handles draw events.
// It also implement pressure sensitive drawing with a pen or stylus.
// And it implements an overlay plane that visualizes pen event data.
//
class CanvasInterface {
  Fl_Widget *widget_ { nullptr };
  bool in_window_ { false };
  bool first_draw_ { true };
  Fl_Offscreen offscreen_ { 0 };
  Fl_Color color_ { 1 };
  enum { NONE, HOVER, DRAW, PEN_HOVER, PEN_DRAW } overlay_ { NONE };
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
  void cv_pen_paint();
};

//
// Handle mouse and pen events.
//
int CanvasInterface::cv_handle(int event)
{
  switch (event)
  {
    // Event handling for pen events:
    case Fl::Pen::ENTER: // Return 1 to receive all pen events and suppress mouse events
      // Pen entered the widget area.
      color_++;
      if (color_ > 6) color_ = 1;
      /* fall through */
    case Fl::Pen::HOVER:
      // Pen move over the surface without touching it.
      overlay_ = PEN_HOVER;
      ov_x_ = Fl::event_x();
      ov_y_ = Fl::event_y();
      widget_->redraw();
      return 1;
    case Fl::Pen::TOUCH:
      // Pen tip or eraser just touched the surface.
      /* fall through */
    case Fl::Pen::DRAW:
      // Pen is dragged over the surface, or hovers with a button pressed.
      overlay_ = PEN_DRAW;
      ov_x_ = Fl::event_x();
      ov_y_ = Fl::event_y();
      cv_pen_paint();
      widget_->redraw();
      return 1;
    case Fl::Pen::LIFT:
      // Pen was just lifted from the surface and is now hovering
      return 1;
    case Fl::Pen::LEAVE:
      // The pen left the drawing area.
      overlay_ = NONE;
      widget_->redraw();
      return 1;

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
    case FL_PUSH:
      /* fall through */
    case FL_DRAG:
      overlay_ = DRAW;
      ov_x_ = Fl::event_x();
      ov_y_ = Fl::event_y();
      cv_paint();
      widget_->redraw();
      return 1;
    case FL_RELEASE:
      return 1;
    case FL_LEAVE:
      overlay_ = NONE;
      widget_->redraw();
      return 1;
  }
  return 0;
}

//
// Canvas drawing copies the offscreen bitmap and then draws the overlays.
//
void CanvasInterface::cv_draw()
{
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

  // Preset values for overlay
  int r = 10;
  if (overlay_ == PEN_DRAW)
    r = static_cast<int>(32.0 * Fl::Pen::event_pressure());
  fl_color(FL_BLACK);
  switch (overlay_) {
    case NONE: break;
    case PEN_HOVER:
      fl_color(FL_RED);
      /* fall through */
    case HOVER:
      fl_xyline(ov_x_-10, ov_y_, ov_x_+10);
      fl_yxline(ov_x_, ov_y_-10, ov_y_+10);
      break;
    case PEN_DRAW:
      fl_color(FL_RED);
      /* fall through */
    case DRAW:
      fl_arc(ov_x_-r, ov_y_-r, 2*r, 2*r, 0, 360);
      fl_arc(ov_x_-r/2-40*Fl::Pen::event_tilt_x(),
             ov_y_-r/2-40*Fl::Pen::event_tilt_y(), r, r, 0, 360);
      // printf("%d\n", Fl::Pen::event_pen_id());
      break;
  }
}

//
// Paint a circle with mouse events.
//
void CanvasInterface::cv_paint() {
  if (!offscreen_)
    return;
  int dx = in_window_ ? 0 : widget_->x(), dy = in_window_ ? 0 : widget_->y();
  fl_begin_offscreen(offscreen_);
  fl_draw_circle(Fl::event_x()-dx-12, Fl::event_y()-dy-12, 24, color_);
  fl_end_offscreen();
}

//
// Paint a circle with pen events. If the eraser is touching the surface,
// draw a white circle.
//
void CanvasInterface::cv_pen_paint() {
  if (!offscreen_)
    return;
  int r = static_cast<int>(32.0 * Fl::Pen::event_pressure());
  int dx = in_window_ ? 0 : widget_->x(), dy = in_window_ ? 0 : widget_->y();
  Fl_Color cc = Fl::Pen::event_state(Fl::Pen::State::ERASER_DOWN) ? FL_WHITE : color_;
  fl_begin_offscreen(offscreen_);
  fl_draw_circle(Fl::event_x()-dx-r, Fl::event_y()-dy-r, 2*r, cc);
  fl_end_offscreen();
}


//
// A drawing canvas, based on a minimal widget.
//
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

//
// A drawing canvas based on a window. Can be used as a standalone window
// and also as a subwindow inside another window.
//
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

//
// Main app entry point
//
int main(int argc, char **argv)
{
  // Create our main app window
  auto window = new Fl_Window(100, 100, 640, 220);

  // One testing canvas is just a regular child widget of the window
  auto canvas_widget_0 = new CanvasWidget( 10, 10, 200, 200, "CV1");

  // The second canvas is inside a group
  auto cv1_group = new Fl_Group(215, 5, 210, 210);
  cv1_group->box(FL_FRAME_BOX);
  auto canvas_widget_1 = new CanvasWidget(220, 10, 200, 200, "CV2");
  cv1_group->end();

  // The third canvas is a window inside a window, so we can verify
  // that pen coordinates are calculated correctly.
  auto canvas_widget_2 = new CanvasWindow(430, 10, 200, 200, "CV3");
  canvas_widget_2->end();

  window->end();

  // A fourth canvas is a top level window by itself.
  auto cv_window = new CanvasWindow(100, 380, 200, 200, "CV Window");

  // All canvases subscribe to pen events.
  Fl::Pen::subscribe(canvas_widget_0);
  Fl::Pen::subscribe(canvas_widget_1);
  Fl::Pen::subscribe(canvas_widget_2);
  Fl::Pen::subscribe(cv_window);

  window->show(argc, argv);
  canvas_widget_2->show();
  cv_window->show();

  return Fl::run();
}
