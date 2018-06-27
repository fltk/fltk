#include <FL/fl_draw.H>
#include <stdio.h>
#include <FL/Fl_Device.H>
#include <FL/Fl_Graphics_Driver.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

class SVG_Graphics_Driver : public Fl_Graphics_Driver {
  FILE *out_;
  int width_;
  const char *linecap_;
  uchar red_, green_, blue_;
public:
  SVG_Graphics_Driver(FILE*);
  ~SVG_Graphics_Driver();
  FILE* file() {return out_;}
protected:
  const char *family_;
  const char *bold_;
  const char *style_;
  void rect(int x, int y, int w, int h);
  void rectf(int x, int y, int w, int h);
  void line_style(int style, int width, char *dashes=0);
  void line(int x1, int y1, int x2, int y2);
  void font_(int f, int s);
  void font(int f, int s);
  void draw(const char *str, int n, int x, int y);
  void draw(const char*, int, float, float) ;
  void draw(int, const char*, int, int, int) ;
  void rtl_draw(const char*, int, int, int) {}
  void color(uchar r, uchar g, uchar b);
  void color(Fl_Color c);
  void draw_image(const uchar*, int, int, int, int, int, int) {}
  void draw_image_mono(const uchar*, int, int, int, int, int, int) {}
  void draw_image(void (*)(void*, int, int, int, uchar*), void*, int, int, int, int, int) {}
  void draw_image_mono(void (*)(void*, int, int, int, uchar*), void*, int, int, int, int, int) {}
  void draw(Fl_RGB_Image*, int, int, int, int, int, int) {}
  void draw(Fl_Pixmap*, int, int, int, int, int, int) {}
  void draw(Fl_Bitmap*, int, int, int, int, int, int) {}
  double width(const char*, int) ;
  int height() ;
  int descent() ;
  
  void push_clip(int x, int y, int w, int h) {}
  void pop_clip(){}
  void xyline(int x, int y, int x1){}
  void xyline(int x, int y, int x1, int y2){}
  void yxline(int x, int y, int y1){}
  void yxline(int x, int y, int y1, int x2){}
  virtual void point(int x, int y) {}
  virtual void line(int x, int y, int x1, int y1, int x2, int y2) {}
  virtual void xyline(int x, int y, int x1, int y2, int x3) {}
  virtual void yxline(int x, int y, int y1, int x2, int y3) {}
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2) {}
  virtual void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {}
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2) {}
  virtual void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {}
  virtual int clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) {return 0;}
  virtual int not_clipped(int x, int y, int w, int h) {return 0;}
  virtual void push_no_clip() {}
  virtual void begin_complex_polygon() {}
  virtual void transformed_vertex(double xf, double yf) {}
  virtual void vertex(double x, double y) {}
  virtual void end_points() {}
  virtual void end_line() {}
  virtual void end_loop() {}
  virtual void end_polygon() {}
  virtual void end_complex_polygon() {}
  virtual void gap() {}
  virtual void circle(double x, double y, double r) {}
  virtual void arc(int x, int y, int w, int h, double a1, double a2) {}
  virtual void pie(int x, int y, int w, int h, double a1, double a2) {}
  virtual Fl_Bitmask create_bitmask(int w, int h, const uchar *array) {return 0;}
  virtual void delete_bitmask(Fl_Bitmask bm) {}
};

class SVG_File_Surface : public Fl_Surface_Device {
  int width_, height_;
public:
  SVG_File_Surface(int width, int height, FILE*);
  int width() { return width_; }
  int height() { return height_; }
  ~SVG_File_Surface();
};


SVG_Graphics_Driver::SVG_Graphics_Driver(FILE *f) {
  out_ = f;
  width_ = 1;
  linecap_ = "butt";
  family_ = "";
  bold_ = "";
  style_ = "";
  red_ = green_ = blue_ = 0;
}

SVG_Graphics_Driver::~SVG_Graphics_Driver()
{
}

void SVG_Graphics_Driver::rect(int x, int y, int w, int h) {
  fprintf(out_, "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
	  "fill=\"none\" stroke=\"rgb(%u,%u,%u)\" stroke-width=\"%d\"/>\n", x, y, w, h, red_, green_, blue_, width_);
}

void SVG_Graphics_Driver::rectf(int x, int y, int w, int h) {
  fprintf(out_, "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
	  "fill=\"rgb(%u,%u,%u)\" />\n", x, y, w, h, red_, green_, blue_);
}

void SVG_Graphics_Driver::line(int x1, int y1, int x2, int y2) {
  fprintf(out_, 
	  "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
	  "style=\"stroke:rgb(%u,%u,%u);stroke-width:%d;stroke-linecap:%s\" />\n", 
	  x1,y1,x2,y2, red_, green_, blue_, width_, linecap_);
}

void SVG_Graphics_Driver::font_(int ft, int s) {
  Fl_Graphics_Driver::font(ft, s);
  int famnum = ft/4;
  if (famnum == 0) family_ = "Helvetica";
  else if (famnum == 1) family_ = "Courier";
  else family_ = "Times";
  int modulo = ft % 4;
  int use_bold = modulo == 1 || modulo == 3;
  int use_italic = modulo >= 2;  
  bold_ =  ( use_bold ? " font-weight=\"bold\"" : "" );
  style_ =  ( use_italic ? " font-style=\"italic\"" : "" );
  if (use_italic && famnum != 2) style_ = " font-style=\"oblique\"";
}

void SVG_Graphics_Driver::font(int ft, int s) {
  Fl_Display_Device::display_device()->driver()->font(ft, s);
  font_(ft, s);
}

void SVG_Graphics_Driver::line_style(int style, int width, char *dashes) {
  if (width == 0) width = 1;
  width_ = width;
  if (style & FL_CAP_SQUARE) linecap_ = "square";
  if (style & FL_CAP_ROUND) linecap_ = "round";
  else linecap_ = "butt";
}

void SVG_Graphics_Driver::draw(const char *str, int n, int x, int y) {
  // Caution: Internet Explorer ignores the xml:space="preserve" attribute
  // work-around: replace all spaces by no-break space = U+00A0 = 0xC2-0xA0 (UTF-8) before sending to IE
  fprintf(out_, "<text x=\"%d\" y=\"%d\" font-family=\"%s\"%s%s font-size=\"%d\" "
          "xml:space=\"preserve\" "
          " fill=\"rgb(%u,%u,%u)\" textLength=\"%d\">%.*s</text>\n",x, y, family_, bold_, style_, size(), red_, green_, blue_, (int)width(str, n), n, str);

}

void SVG_Graphics_Driver::draw(const char* str, int n, float fx, float fy) {
  return draw(str, n, (int)fx, (int)fy);
}

void SVG_Graphics_Driver::draw(int angle, const char* str, int n, int x, int y) {
  fprintf(out_, "<g transform=\"translate(%d,%d) rotate(%d)\">", x, y, -angle);
  draw(str, n, 0, 0);
  fputs("</g>\n", out_);
}

void SVG_Graphics_Driver::color(Fl_Color c) {
  Fl_Graphics_Driver::color(c);
  Fl::get_color(c, red_, green_, blue_);
}

void SVG_Graphics_Driver::color(uchar r, uchar g, uchar b) {
  red_ = r;
  green_ = g;
  blue_ = b;
}

double SVG_Graphics_Driver::width(const char* str, int l) {
 return Fl_Display_Device::display_device()->driver()->width(str, l);
}

int SVG_Graphics_Driver::height() {
  return Fl_Display_Device::display_device()->driver()->height();
}

int SVG_Graphics_Driver::descent() {
  return Fl_Display_Device::display_device()->driver()->descent();
}

SVG_File_Surface::SVG_File_Surface(int w, int h, FILE *f) : Fl_Surface_Device(NULL) {
  fprintf(f,
	  "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>\n"
	  "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \n"
	  "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
	  "<svg width=\"%dpx\" height=\"%dpx\" viewBox=\"0 0 %d %d\"\n"
	  "xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n", w, h, w, h);
  width_ = w; height_ = h;
  driver(new SVG_Graphics_Driver(f));
}

SVG_File_Surface::~SVG_File_Surface() {
  SVG_Graphics_Driver *driver = (SVG_Graphics_Driver*)this->driver();
  fputs("</svg>\n", driver->file());
  fflush(driver->file());
  delete driver;
}


int main(int argc, char **argv) {
  Fl_Window *window = new Fl_Window(340,180);
  Fl_Box *box = new Fl_Box(20,40,300,100,"Hello, World!");
  box->box(FL_UP_BOX);
  box->labelfont(FL_BOLD+FL_ITALIC);
  box->labelsize(36);
  box->labeltype(FL_SHADOW_LABEL);
  window->end();
  window->show(argc, argv);
  
  FILE *out = fl_fopen("hello.svg", "w");
  if (out) {
    SVG_File_Surface *svg = new SVG_File_Surface(box->w(), box->h(), out);
    Fl_Surface_Device::push_current(svg);
    fl_color(box->color());
    fl_rectf(0, 0, box->w(), box->h());
    fl_font(box->labelfont(), 36);
    fl_color(box->labelcolor());
    fl_draw(box->label(), 5, 50);
    Fl_Surface_Device::pop_current();
    delete svg;
    fclose(out);
  }

  return Fl::run();
}


