



#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

Fl_Window *win = 0;

int point_test_ix, line_test_ix, rect_test_ix, viewport_test_ix;

void changePageCB(Fl_Widget*, void *ixvp) {
  int ix = (int)ixvp;
  int i = 0, n = win->children();
  for ( ; i<n; i++) 
    win->child(i)->hide();
  if (ix>=n || ix<0) ix = n-1;
  win->child(ix)->show();
}

void newButton(int x, int y, int w, int h, const char *l, int ix, const char *tt) {
  Fl_Button *b = new Fl_Button(x, y, w, h, l);
  b->tooltip(tt);
  b->callback(changePageCB, (void*)ix);
}

void createMenuPage() {
  Fl_Group *page, *g;
  page = new Fl_Group(0, 0, 600, 600);
  g = new Fl_Group(100, 20, 460, 26, "drawing:");
  g->align(FL_ALIGN_LEFT);
  newButton(100+2, 22, 22, 22, "1", point_test_ix, "Testing pixel drawing");
  newButton(125+2, 22, 22, 22, "2", line_test_ix, "Testing fl_line");
  newButton(150+2, 22, 22, 22, "3", rect_test_ix, "Testing fl_rect");
  newButton(175+2, 22, 22, 22, "4", viewport_test_ix, "Testing viewport alignment");
  g->end();
  page->end(); 
}

Fl_Group *beginTestPage(const char *l) {
  int ix = win->children();
  Fl_Group *g = new Fl_Group(0, 0, win->w(), win->h());
  g->box(FL_FLAT_BOX);
  g->hide();
  newButton(20, 20, 20, 20, "M", -1, "Return to main menu");
  newButton(20, 40, 20, 20, "@<", ix-1, "previous test");
  newButton(20, 60, 20, 20, "@>", ix+1, "next test");
  Fl_Box *bx = new Fl_Box(60, 20, win->w()-80, 100, l);
  bx->box(FL_THIN_DOWN_BOX);
  bx->align(FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
  return g;
}

//------- test the point drawing capabilities of this implementation ----------
class PointTest : Fl_Widget {
public: PointTest(int x, int y, int w, int h) : Fl_Widget(x, y, w, h) {}
  void draw() {
    int a = x(), b = y();
    fl_color(FL_BLACK);
    fl_rect(x(), y(), w(), h());
    fl_point(a+10, b+10); fl_point(a+20, b+20); 
    fl_point(a+10, b+20); fl_point(a+20, b+10);
    fl_color(FL_RED); a = x()+70;
    fl_point(a+10, b+10); fl_point(a+20, b+20);
    fl_point(a+10, b+20); fl_point(a+20, b+10);
    fl_color(FL_GREEN); a = x(); b = y()+70;
    fl_point(a+10, b+10); fl_point(a+20, b+20);
    fl_point(a+10, b+20); fl_point(a+20, b+10);
    fl_color(FL_BLUE); a = x()+70;
    fl_point(a+10, b+10); fl_point(a+20, b+20);
    fl_point(a+10, b+20); fl_point(a+20, b+10);
  }
};
void fl_point_test() {
  point_test_ix = win->children();
  Fl_Group *page = beginTestPage(
    "testing the fl_point call\n"
    "You should see four pixels each in black, red, green and blue. "
    "Make sure that pixels are not anti-aliased (blured across multiple pixels)!"
  );
  new PointTest(20, 140, 100, 100);
  page->end();
}

//------- test the line drawing capabilities of this implementation ----------
class LineTest : Fl_Widget {
public: LineTest(int x, int y, int w, int h) : Fl_Widget(x, y, w, h) {}
  void draw() {
    int a = x(), b = y(); fl_color(FL_BLACK); fl_rect(a, b, w(), h());
    // testing fl_xyline(x, y, x1)
    fl_color(FL_RED); fl_point(a+10, b+10); fl_point(a+20, b+10);
    fl_color(FL_BLACK); fl_xyline(a+10, b+10, a+20);
    // testing fl_xyline(x, y, x1, y2);
    fl_color(FL_RED); fl_point(a+10, b+20); fl_point(a+20, b+20);
    fl_point(a+20, b+30);
    fl_color(FL_BLACK); fl_xyline(a+10, b+20, a+20, b+30);
    // testing fl_xyline(x, y, x1, y2, x3);
    fl_color(FL_RED); fl_point(a+10, b+40); fl_point(a+20, b+40);
    fl_point(a+20, b+50); fl_point(a+30, b+50);
    fl_color(FL_BLACK); fl_xyline(a+10, b+40, a+20, b+50, a+30);
    //+++ add testing for the fl_yxline commands!
    // testing fl_loop(x,y, x,y, x,y, x, y)
    fl_color(FL_RED); fl_point(a+60, b+60); fl_point(a+90, b+60);
    fl_point(a+60, b+90); fl_point(a+90, b+90);
    fl_color(FL_BLACK);
    fl_loop(a+60, b+60, a+90, b+60, a+90, b+90, a+60, b+90);
  }
};
void fl_line_test() {
  line_test_ix = win->children();
  Fl_Group *page = beginTestPage(
    "testing the integer based fl_line calls\n"
    "No red pixels should be visible. "
    "If you see bright red pixels, the line drawing alignment is off, "
    "or the last pixel in a line does not get drawn. "
    "If you see dark red pixels, anti-aliasing must be switched off."
  );
  new LineTest(20, 140, 100, 100);
  page->end();
}

//------- test the line drawing capabilities of this implementation ----------
class RectTest : Fl_Widget {
public: RectTest(int x, int y, int w, int h) : Fl_Widget(x, y, w, h) {}
  void draw() {
    int a = x(), b = y(); fl_color(FL_BLACK); fl_rect(a, b, w(), h());
    // testing fl_rect() with positive size
    fl_color(FL_RED);   fl_loop(a+10, b+10, a+40, b+10, a+40, b+40, a+10, b+40);
    fl_color(FL_GREEN); fl_loop(a+ 9, b+ 9, a+41, b+ 9, a+41, b+41, a+ 9, b+41);
    fl_color(FL_GREEN); fl_loop(a+11, b+11, a+39, b+11, a+39, b+39, a+11, b+39);
    fl_color(FL_BLACK); fl_rect(a+10, b+10, 31, 31);
    // testing fl_rect() with positive size
    fl_color(FL_RED);   fl_loop(a+60, b+60, a+90, b+60, a+90, b+90, a+60, b+90);
    fl_color(FL_GREEN); fl_loop(a+59, b+59, a+91, b+59, a+91, b+91, a+59, b+91);
    fl_color(FL_BLACK); fl_rectf(a+60, b+60, 31, 31);
  }
};
void fl_rect_test() {
  rect_test_ix = win->children();
  Fl_Group *page = beginTestPage(
    "testing the fl_rect call\n"
    "No red pixels should be visible. "
    "If you see bright red lines, or if parts of the green frames are hidden, "
    "the rect drawing alignment is off. "
  );
  new RectTest(20, 140, 100, 100);
  page->end();
}

//------- test the line drawing capabilities of this implementation ----------
class ViewportTest : Fl_Widget {
  int pos;
public: ViewportTest(int x, int y, int w, int h, int p) : Fl_Widget(x, y, w, h),
  pos(p) {}
  void draw() {
    if (pos&1) {
      fl_color(FL_RED);   fl_yxline(x()+w(), y(), y()+h());
      fl_color(FL_GREEN); fl_yxline(x()+w()-1, y(), y()+h());
    } else {
      fl_color(FL_RED);   fl_yxline(x()-1, y(), y()+h());
      fl_color(FL_GREEN); fl_yxline(x(), y(), y()+h());
    }
    if (pos&2) {
      fl_color(FL_RED);   fl_xyline(x(), y()+h(), x()+w());
      fl_color(FL_GREEN); fl_xyline(x(), y()+h()-1, x()+w());
    } else {
      fl_color(FL_RED);   fl_xyline(x(), y()-1, x()+w());
      fl_color(FL_GREEN); fl_xyline(x(), y(), x()+w());
    }
    fl_color(FL_BLACK);
    fl_loop(x()+3, y()+3, x()+w()-4, y()+3, x()+w()-4, y()+h()-4, x()+3, y()+h()-4);
  }
};
void fl_viewport_test() {
  viewport_test_ix = win->children();
  Fl_Group *page = beginTestPage(
    "testing viewport alignment\n"
    "Only green lines should be visible. "
    "If red lines are visible in the corners of this window, "
    "your viewport alignment and clipping is off. "
    "If there is a space between the green lines and the window border, "
    "the viewport is off, but some clipping may be working. "
    "Also, your window size may be off to begin with."
  );
  new ViewportTest(0, 0, 20, 20, 0);
  new ViewportTest(page->w()-20, 0, 20, 20, 1);
  new ViewportTest(0, page->h()-20, 20, 20, 2);
  new ViewportTest(page->w()-20,page->h()-20, 20, 20, 3);
  page->end();
}

int main(int argc, char **argv) {
  win = new Fl_Window(600, 600, "Unit Tests for FLTK");
  fl_point_test();
  fl_line_test();
  fl_rect_test();
  fl_viewport_test();
  createMenuPage();
  win->end();
  win->show(argc, argv);
  Fl::run();
}


