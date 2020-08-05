//
// Implementation of classes Fl_SVG_Graphics_Driver and Fl_SVG_File_Surface in the Fast Light Tool Kit (FLTK).
//
// Copyright 2020 by Bill Spitzak and others.
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

// Complete implementation to draw into an SVG file using the standard FLTK drawing API.

#include <config.h>
#include <FL/Fl_SVG_File_Surface.H>
#if FLTK_USE_SVG
#include <FL/fl_draw.H>
#include <stdio.h>
#include <FL/math.h>
#include <FL/Fl_Widget_Surface.H>
#include <FL/Fl_Graphics_Driver.H>
#include <FL/Fl.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Bitmap.H>
#include <FL/fl_string.h>

extern "C" {
#if defined(HAVE_LIBPNG)
#  ifdef HAVE_PNG_H
#    include <png.h>
#  else
#    include <libpng/png.h>
#  endif // HAVE_PNG_H
#endif // HAVE_LIBPNG

#ifdef HAVE_LIBJPEG
#  include <jpeglib.h>
#endif // HAVE_LIBJPEG
}

class Fl_SVG_Graphics_Driver : public Fl_Graphics_Driver {
  FILE *out_;
  int width_;
  int line_style_;
  const char *linecap_;
  const char *linejoin_;
  uchar red_, green_, blue_;
  char *dasharray_; // the dash array as SVG needs it
  char *user_dash_array_; // the dash array as FLTK needs it
  int p_size;
  typedef struct { float x; float y; } XPOINT;
  XPOINT *p;
  class Clip {
  public:
    int x, y, w, h; // the clip rectangle
    char Id[12]; // "none" or SVG Id of this clip rectangle
    Clip *prev; // previous in pile of clips
  };
  Clip * clip_; // top of pile of clips
  int clip_count_; // to generate distinct SVG clip Ids
  char *last_rgb_name_; // NULL or SVG Id of last defined RGB image
  const char *family_;
  const char *bold_;
  const char *style_;
public:
  Fl_SVG_Graphics_Driver(FILE*);
  ~Fl_SVG_Graphics_Driver();
  FILE* file() {return out_;}
protected:
  void rect(int x, int y, int w, int h);
  void rectf(int x, int y, int w, int h);
  void compute_dasharray(float s, char *dashes=0);
  void line_style(int style, int width, char *dashes=0);
  void line(int x1, int y1, int x2, int y2);
  void font_(int f, int s);
  void font(int f, int s);
  void draw(const char *str, int n, int x, int y);
  void draw(const char*, int, float, float) ;
  void draw(int, const char*, int, int, int) ;
  void rtl_draw(const char *str, int n, int x, int y);
  void color(uchar r, uchar g, uchar b);
  void color(Fl_Color c);
  double width(const char*, int) ;
  void text_extents(const char*, int n, int& dx, int& dy, int& w, int& h);
  int height() ;
  int descent() ;
  void draw_rgb(Fl_RGB_Image *rgb, int XP, int YP, int WP, int HP, int cx, int cy);
  void define_rgb_png(Fl_RGB_Image *rgb, const char *name, int x, int y);
  void define_rgb_jpeg(Fl_RGB_Image *rgb, const char *name, int x, int y);
  void draw_pixmap(Fl_Pixmap *pxm,int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_bitmap(Fl_Bitmap *bm,int XP, int YP, int WP, int HP, int cx, int cy);
  void draw_image(const uchar* buf, int x, int y, int w, int h, int d, int l);
  void draw_image(Fl_Draw_Image_Cb cb, void* data, int x, int y, int w, int h, int d);
  void draw_image_mono(const uchar* buf, int x, int y, int w, int h, int d, int l);
  void draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int x, int y, int w, int h, int d);
  void push_clip(int x, int y, int w, int h);
  void push_no_clip();
  void pop_clip();
  int clip_box(int x, int y, int w, int h, int& X, int& Y, int& W, int& H);
  int not_clipped(int x, int y, int w, int h);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2);
  void polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);
  void loop(int x0, int y0, int x1, int y1, int x2, int y2);
  void point(int x, int y);
  void transformed_vertex0(float x, float y);
  void transformed_vertex(double xf, double yf);
  void vertex(double x,double y);
  void end_points();
  void end_line();
  void fixloop();
  void end_loop();
  void end_polygon();
  void begin_complex_polygon();
  void gap();
  void end_complex_polygon();
  void circle(double x, double y,double r);
  void arc(int x,int y,int w,int h,double a1,double a2);
  void pie(int x,int y,int w,int h,double a1,double a2);
  void arc_pie(char AorP, int x, int y, int w, int h, double a1, double a2);
};

Fl_SVG_Graphics_Driver::Fl_SVG_Graphics_Driver(FILE *f) {
  out_ = f;
  width_ = 1;
  line_style_ = 0;
  linecap_ = "butt";
  linejoin_ = "miter";
  family_ = "";
  bold_ = "";
  style_ = "";
  red_ = green_ = blue_ = 0;
  clip_count_ = 0;
  clip_ = NULL;
  user_dash_array_ = 0;
  dasharray_ = fl_strdup("none");
  p_size = 0;
  p = NULL;
  last_rgb_name_ = NULL;
}

Fl_SVG_Graphics_Driver::~Fl_SVG_Graphics_Driver()
{
  if (user_dash_array_) free(user_dash_array_);
  if (dasharray_) free(dasharray_);
  while (clip_){
    Clip * c= clip_;
    clip_= clip_->prev;
    delete c;
  }
  if (last_rgb_name_) free(last_rgb_name_);
}

void Fl_SVG_Graphics_Driver::rect(int x, int y, int w, int h) {
  fprintf(out_, "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
          "fill=\"none\" stroke=\"rgb(%u,%u,%u)\" stroke-width=\"%d\" stroke-dasharray=\"%s\""
          " stroke-linecap=\"%s\" stroke-linejoin=\"%s\"/>\n", x, y, w-1, h-1, red_, green_, blue_, width_, dasharray_, linecap_, linejoin_);
}

void Fl_SVG_Graphics_Driver::rectf(int x, int y, int w, int h) {
  fprintf(out_, "<rect x=\"%.3f\" y=\"%.3f\" width=\"%d\" height=\"%d\" "
          "fill=\"rgb(%u,%u,%u)\" />\n", x-.5, y-.5, w, h, red_, green_, blue_);
}

void Fl_SVG_Graphics_Driver::point(int x, int y) {
  rectf(x,y,1,1);
}

void Fl_SVG_Graphics_Driver::line(int x1, int y1, int x2, int y2) {
  fprintf(out_,
          "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
          "style=\"stroke:rgb(%u,%u,%u);stroke-width:%d;stroke-linecap:%s;stroke-linejoin:%s;stroke-dasharray:%s\" />\n",
          x1,y1,x2,y2, red_, green_, blue_, width_, linecap_, linejoin_, dasharray_);
}

void Fl_SVG_Graphics_Driver::font_(int ft, int s) {
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

void Fl_SVG_Graphics_Driver::font(int ft, int s) {
  Fl_Display_Device::display_device()->driver()->font(ft, s);
  font_(ft, s);
}

void Fl_SVG_Graphics_Driver::compute_dasharray(float s, char *dashes) {
  if (user_dash_array_ && user_dash_array_ != dashes) {free(user_dash_array_); user_dash_array_ = NULL;}
  if (dashes && *dashes) {
    if (dasharray_) free(dasharray_);
    dasharray_ = (char*)calloc(10*strlen(dashes) + 1, 1);
    for (char *p = dashes; *p; p++) {
      sprintf(dasharray_+strlen(dasharray_), "%.3f,", (*p)/s);
    }
    dasharray_[strlen(dasharray_) - 1] = 0;
    if (user_dash_array_ != dashes) user_dash_array_ = fl_strdup(dashes);
    return;
  }
  int dash_part = line_style_ & 0xFF;
  if (dash_part == FL_SOLID)  {
    if (strcmp(dasharray_, "none")) {
      free(dasharray_);
      dasharray_ = fl_strdup("none");
    }
  } else {
    int cap_part = (line_style_ & 0xF00);
    bool is_flat = (cap_part == FL_CAP_FLAT || cap_part == 0);
    float dot = (is_flat ? width_/s : width_*0.6/s);
    float gap = (is_flat ? width_/s : width_*1.5/s);
    float big = (is_flat ? 3*width_/s : width_*2.5/s);
    if (dasharray_) free(dasharray_);
    dasharray_ = (char*)malloc(61);
    if (dash_part == FL_DOT) sprintf(dasharray_, "%.3f,%.3f", dot, gap);
    else if (dash_part == FL_DASH) sprintf(dasharray_, "%.3f,%.3f", big, gap);
    else if (dash_part == FL_DASHDOT) sprintf(dasharray_, "%.3f,%.3f,%.3f,%.3f", big, gap, dot, gap);
    else sprintf(dasharray_, "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f", big, gap, dot, gap, dot, gap);
  }
}

void Fl_SVG_Graphics_Driver::line_style(int style, int width, char *dashes) {
  line_style_ = style;
  if (width == 0) width = 1;
  width_ = width;
  int cap_part = style & 0xF00;
  if (cap_part == FL_CAP_SQUARE) linecap_ = "square";
  else if (cap_part == FL_CAP_ROUND) linecap_ = "round";
  else linecap_ = "butt";
  int join_part = style & 0xF000;
  if (join_part == FL_JOIN_BEVEL) linejoin_ = "bevel";
  else if (join_part == FL_JOIN_MITER) linejoin_ = "miter";
  else if (join_part == FL_JOIN_ROUND) linejoin_ = "round";
  else linejoin_ = "miter";
  compute_dasharray(1., dashes);
}

void Fl_SVG_Graphics_Driver::draw(const char *str, int n, int x, int y) {
  // Caution: Internet Explorer ignores the xml:space="preserve" attribute
  // work-around: replace all spaces by no-break space = U+00A0 = 0xC2-0xA0 (UTF-8) before sending to IE
  fprintf(out_, "<text x=\"%d\" y=\"%d\" font-family=\"%s\"%s%s font-size=\"%d\" "
          "xml:space=\"preserve\" "
          " fill=\"rgb(%u,%u,%u)\" textLength=\"%d\">", x, y, family_, bold_, style_, size(), red_, green_, blue_, (int)width(str, n));
  for (int i = 0; i < n; i++) {
    if (str[i] == '&') fputs("&amp;", out_);
    else if (str[i] == '<') fputs("&lt;", out_);
    else if (str[i] == '>') fputs("&gt;", out_);
    else fputc(str[i], out_);
  }
  fputs("</text>\n", out_);
}

void Fl_SVG_Graphics_Driver::draw(const char* str, int n, float fx, float fy) {
  return draw(str, n, (int)fx, (int)fy);
}

void Fl_SVG_Graphics_Driver::draw(int angle, const char* str, int n, int x, int y) {
  fprintf(out_, "<g transform=\"translate(%d,%d) rotate(%d)\">", x, y, -angle);
  draw(str, n, 0, 0);
  fputs("</g>\n", out_);
}

void Fl_SVG_Graphics_Driver::rtl_draw(const char *str, int n, int x, int y) {
  int w = (int)width(str, n);
  draw(str, n, x - w, y);
}

void Fl_SVG_Graphics_Driver::color(Fl_Color c) {
  Fl_Graphics_Driver::color(c);
  Fl::get_color(c, red_, green_, blue_);
}

void Fl_SVG_Graphics_Driver::color(uchar r, uchar g, uchar b) {
  red_ = r;
  green_ = g;
  blue_ = b;
}

double Fl_SVG_Graphics_Driver::width(const char* str, int l) {
 return Fl_Display_Device::display_device()->driver()->width(str, l);
}

void Fl_SVG_Graphics_Driver::text_extents(const char *c, int n, int& dx, int& dy, int& w, int& h) {
  Fl::first_window()->make_current(); // to get a valid drawing gc
  Fl_Display_Device::display_device()->driver()->text_extents(c, n, dx, dy, w, h);
}

int Fl_SVG_Graphics_Driver::height() {
  return Fl_Display_Device::display_device()->driver()->height();
}

int Fl_SVG_Graphics_Driver::descent() {
  return Fl_Display_Device::display_device()->driver()->descent();
}

Fl_SVG_File_Surface::Fl_SVG_File_Surface(int w, int h, FILE *f) : Fl_Widget_Surface(new Fl_SVG_Graphics_Driver(f)) {
  Fl_Window *win = Fl::first_window();
  float s = (win ? Fl::screen_scale(win->screen_num()) : 1);
  int sw = w * s, sh = h * s;
  fprintf(f,
          "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>\n"
          "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \n"
          "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
          "<svg width=\"%dpx\" height=\"%dpx\" viewBox=\"0 0 %d %d\"\n"
          "xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n", sw, sh, sw, sh);
  width_ = w; height_ = h;
  fprintf(f, "<g transform=\"scale(%f)\">\n", s);
  fputs("<g transform=\"translate(0,0)\">\n", f);
}

Fl_SVG_File_Surface::~Fl_SVG_File_Surface() {
  Fl_SVG_Graphics_Driver *driver = (Fl_SVG_Graphics_Driver*)this->driver();
  if (driver) {
    fputs("</g></g></svg>\n", driver->file());
    fflush(driver->file());
    delete driver;
  }
}

FILE *Fl_SVG_File_Surface::file() {
  Fl_SVG_Graphics_Driver *driver = (Fl_SVG_Graphics_Driver*)this->driver();
  return driver->file();
}

int Fl_SVG_File_Surface::close() {
  Fl_SVG_Graphics_Driver *driver = (Fl_SVG_Graphics_Driver*)this->driver();
  fputs("</g></g></svg>\n", driver->file());
  int retval = fclose(driver->file());
  delete driver;
  this->driver(NULL);
  return retval;
}

void Fl_SVG_File_Surface::translate(int x, int y) {
  Fl_SVG_Graphics_Driver *driver = (Fl_SVG_Graphics_Driver*)this->driver();
  fprintf(driver->file(), "<g transform=\"translate(%d,%d) \">\n", x, y);
}

void Fl_SVG_File_Surface::untranslate() {
  Fl_SVG_Graphics_Driver *driver = (Fl_SVG_Graphics_Driver*)this->driver();
  fputs("</g>\n", driver->file());
}

void Fl_SVG_File_Surface::origin(int x, int y) {
  Fl_SVG_Graphics_Driver *driver = (Fl_SVG_Graphics_Driver*)this->driver();
  fprintf(driver->file(), "</g><g transform=\"translate(%d,%d) \">\n", x, y);
  Fl_Widget_Surface::origin(x, y);
}

int Fl_SVG_File_Surface::printable_rect(int *w, int *h) {
  *w = width_;
  *h = height_;
  return 0;
}

struct svg_base64_t { // holds data useful to perform base64-encoding of a stream of bytes
  FILE *svg; // where base64-encoded data is output
  int lline; // follows length of current line in svg file
  uchar buff[3]; // holds up to 3 bytes that still need encoding
  int lbuf; // # of valid bytes in buff
};

// Performs base64 encoding of up to 3 bytes.
// To be called successively with 3 consecutive bytes (l=3),
// and possibly with l=1 or l=2 only at the end of the byte stream.
// Always writes 4 printable characters to the output FILE.
static void to_base64(uchar *p, int l, svg_base64_t *svg_base64) {
  static char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  uchar B0 = *p++;
  uchar B1 = (l == 1 ? 0 : *p++);
  uchar B2 = (l <= 2 ? 0 : *p);
  fputc(base64_table[ B0 >> 2 ], svg_base64->svg);
  fputc(base64_table[ ((B0 & 0x3) << 4) + (B1 >> 4) ], svg_base64->svg);
  fputc( (l == 1 ? '=' : base64_table[ ((B1 & 0xF) << 2) + (B2 >> 6) ]), svg_base64->svg );
  fputc( (l < 3 ? '=' : base64_table[ B2 & 0x3F ]), svg_base64->svg );
  svg_base64->lline += 4;
  if (svg_base64->lline >= 80) {
    fputc('\n', svg_base64->svg);
    svg_base64->lline = 0;
  }
}

// Writes to the svg file, in base64-encoded form, a block of length bytes.
// 1 or 2 bytes may remain unprocessed after return.
// Returns the number of remaining unprocessed bytes.
static size_t write_by_3(uchar *data, size_t length, svg_base64_t *svg_base64) {
  while (length >= 3) {
    to_base64(data, 3, svg_base64);
    data += 3;
    length -= 3;
  }
  return length;
}

#ifdef HAVE_LIBPNG

// processes length bytes of the png stream under construction
static void user_write_data(png_structp png_ptr, png_bytep data, png_size_t length) {
  svg_base64_t *svg_base64_data = (svg_base64_t*)png_get_io_ptr(png_ptr);
  if (svg_base64_data->lbuf == 1 && length >= 2) {
    svg_base64_data->buff[1] = *data++; length--;
    svg_base64_data->buff[2] = *data++; length--;
    write_by_3(svg_base64_data->buff, 3, svg_base64_data);
  }  else if (svg_base64_data->lbuf == 2 && length >= 1) {
    svg_base64_data->buff[2] = *data++; length--;
    write_by_3(svg_base64_data->buff, 3, svg_base64_data);
  }
  size_t new_l = length;
  if (length >= 3) {
    new_l = write_by_3(data, length, svg_base64_data);
  }
  svg_base64_data->lbuf = new_l;
  if (new_l) {
    memcpy(svg_base64_data->buff, data + length - new_l, new_l);
  }
}

// processes last bytes to be base64 encoded
static void user_flush_data(png_structp png_ptr) {
  svg_base64_t *svg_base64_data = (svg_base64_t*)png_get_io_ptr(png_ptr);
  if (svg_base64_data->lbuf) to_base64(svg_base64_data->buff, svg_base64_data->lbuf, svg_base64_data);
}

/* How to define first the image data and next use it, possibly several times:
<defs><image id="myimage"  width="64" height="64" href="data:image/png;base64,
iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAAnElEQVR4nO3UsQ3EIADAwEQgsf+S7ECB
/sdwkfMEru7de/+eDzfvvfVD2hxj1A9pc61VP6TNc079kMYABjCAAfVDGgMYwAAG1A9pDGAAAxhQP6Qx
gAEMYED9kMYABjCAAfVDGgMYwAAG1A9pDGAAAxhQP6QxgAEMYED9kMYABjCAAfVDGgMYwAAG1A9pDGAA
AxhQP6QxgAEM+LYBf9sdYcTRmp6pAAAAAElFTkSuQmCCAAAAAElFTkSuQmCC"/>
</defs>
<use href="#myimage" x="xxx" y="yyy"/>
<use href="#myimage" x="xxx2" y="yyy2"/>
*/
/* Specify image data and draw it in one go:
 <image x="xxx" y="yyy"  width="64" height="64" href="data:image/png;base64,
 iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAAnElEQVR4nO3UsQ3EIADAwEQgsf+S7ECB
 /sdwkfMEru7de/+eDzfvvfVD2hxj1A9pc61VP6TNc079kMYABjCAAfVDGgMYwAAG1A9pDGAAAxhQP6Qx
 gAEMYED9kMYABjCAAfVDGgMYwAAG1A9pDGAAAxhQP6QxgAEMYED9kMYABjCAAfVDGgMYwAAG1A9pDGAA
 AxhQP6QxgAEM+LYBf9sdYcTRmp6pAAAAAElFTkSuQmCCAAAAAElFTkSuQmCC"/>
 */

void Fl_SVG_Graphics_Driver::define_rgb_png(Fl_RGB_Image *rgb, const char *name, int x, int y) {
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) return;
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    return;
  }
  if (name) {
    if (last_rgb_name_) free(last_rgb_name_);
    last_rgb_name_ = fl_strdup(name);
  }
  float f = rgb->data_w() > rgb->data_h() ? float(rgb->w()) / rgb->data_w(): float(rgb->h()) / rgb->data_h();
  if (name) fprintf(out_, "<defs><image id=\"%s\" ", name);
  else fprintf(out_, "<image x=\"%d\" y=\"%d\" ", x, y);
  fprintf(out_, "width=\"%f\" height=\"%f\" href=\"data:image/png;base64,\n", f*rgb->data_w(), f*rgb->data_h());
  // Transforms the image into a stream of bytes in PNG format,
  // base64-encode this byte stream, and outputs the result to the svg FILE.
  svg_base64_t svg_base64_data;
  svg_base64_data.svg = out_;
  svg_base64_data.lline = 0;
  svg_base64_data.lbuf = 0;
  // user_write_data is a function repetitively called by libpng which receives blocks of bytes.
  png_set_write_fn(png_ptr, &svg_base64_data, user_write_data, user_flush_data);
  int color_type;
  switch (rgb->d()) {
    case 1:
      color_type = PNG_COLOR_TYPE_GRAY;
      break;
    case 2:
      color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
      break;
    case 3:
      color_type = PNG_COLOR_TYPE_RGB;
      break;
    case 4:
    default:
      color_type = PNG_COLOR_TYPE_RGB_ALPHA;
  }
  png_set_IHDR(png_ptr, info_ptr, rgb->data_w(), rgb->data_h(), 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  const uchar **row_pointers = new const uchar*[rgb->data_h()];
  int ld = rgb->ld() ? rgb->ld() : rgb->d() * rgb->data_w();
  for (int i=0; i < rgb->data_h(); i++) row_pointers[i] = (rgb->array + i*ld);
  png_set_rows(png_ptr, info_ptr, (png_bytepp)row_pointers);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  png_write_end(png_ptr, NULL);
  user_flush_data(png_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  delete[] row_pointers;
  if (name) fputs("\"/></defs>\n", out_);
  else fputs("\"/>\n", out_);
}

#endif // HAVE_LIBPNG

#ifdef HAVE_LIBJPEG

struct jpeg_client_data_struct {
  JOCTET JPEG_BUFFER[50000];
  size_t size;
  svg_base64_t base64_data;
};

static void init_destination(jpeg_compress_struct *cinfo) {
  jpeg_client_data_struct *client_data = (jpeg_client_data_struct*)(cinfo->client_data);
  cinfo->dest->next_output_byte = client_data->JPEG_BUFFER;
  cinfo->dest->free_in_buffer = client_data->size;
}

static size_t process_jpeg_chunk(jpeg_compress_struct *cinfo, size_t length) {
  jpeg_client_data_struct *client_data = (jpeg_client_data_struct*)(cinfo->client_data);
  JOCTET *data = client_data->JPEG_BUFFER;
  size_t new_l = length;
  if (length >= 3) {
    new_l = write_by_3(data, length, &client_data->base64_data);
    if (new_l) memmove(client_data->JPEG_BUFFER, data + length - new_l, new_l);
  }
  cinfo->dest->next_output_byte = client_data->JPEG_BUFFER + new_l;
  cinfo->dest->free_in_buffer = client_data->size - new_l;
  return new_l;
}

static boolean empty_output_buffer(jpeg_compress_struct *cinfo) {
  jpeg_client_data_struct *client_data = (jpeg_client_data_struct*)(cinfo->client_data);
  process_jpeg_chunk(cinfo, client_data->size);
  return TRUE;
}

static void term_destination(jpeg_compress_struct *cinfo) {
  jpeg_client_data_struct *client_data = (jpeg_client_data_struct*)(cinfo->client_data);
  size_t new_l = process_jpeg_chunk(cinfo, client_data->size - cinfo->dest->free_in_buffer);
  if (new_l) {
    to_base64(client_data->JPEG_BUFFER, new_l, &client_data->base64_data);
  }
}

void Fl_SVG_Graphics_Driver::define_rgb_jpeg(Fl_RGB_Image *rgb, const char *name, int x, int y) {
  if (name) {
    if (last_rgb_name_) free(last_rgb_name_);
    last_rgb_name_ = fl_strdup(name);
  }
  float f = rgb->data_w() > rgb->data_h() ? float(rgb->w()) / rgb->data_w(): float(rgb->h()) / rgb->data_h();
  if (name) fprintf(out_, "<defs><image id=\"%s\" ", name);
  else fprintf(out_, "<image x=\"%d\" y=\"%d\" ", x, y);
  fprintf(out_, "width=\"%f\" height=\"%f\" href=\"data:image/jpeg;base64,\n", f*rgb->data_w(), f*rgb->data_h());
  // Transforms the image into a stream of bytes in JPEG format,
  // base64-encode this byte stream, and outputs the result to the svg FILE.
  jpeg_compress_struct cinfo;
  jpeg_error_mgr jerr;
  jpeg_client_data_struct jpeg_client_data;
  jpeg_client_data.size = sizeof(jpeg_client_data.JPEG_BUFFER);
  cinfo.client_data = &jpeg_client_data;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_destination_mgr jpeg_mgr;
  jpeg_mgr.init_destination = init_destination;
  jpeg_mgr.empty_output_buffer = empty_output_buffer;
  jpeg_mgr.term_destination = term_destination;
  cinfo.dest = &jpeg_mgr;
  cinfo.image_width = rgb->data_w();
  cinfo.image_height = rgb->data_h();
  cinfo.input_components = rgb->d();  // 1 or 3
  cinfo.in_color_space = rgb->d() == 3 ? JCS_RGB : JCS_GRAYSCALE;
  jpeg_set_defaults(&cinfo);
  jpeg_client_data.base64_data.svg = out_;
  jpeg_client_data.base64_data.lline = 0;
  jpeg_client_data.base64_data.lbuf = 0;
  jpeg_start_compress(&cinfo, TRUE);
  int ld = rgb->ld() ? rgb->ld() : rgb->data_w() * rgb->d();
  JSAMPROW row_pointer[1];
  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = (uchar*)rgb->array + ld*cinfo.next_scanline;
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  if (name) fputs("\"/></defs>\n", out_);
  else fputs("\"/>\n", out_);
}
#endif // HAVE_LIBJPEG

void Fl_SVG_Graphics_Driver::draw_rgb(Fl_RGB_Image *rgb, int XP, int YP, int WP, int HP, int cx, int cy) {
#if defined(HAVE_LIBPNG)
  char name[24];
  bool need_clip = (cx || cy || WP != rgb->w() || HP != rgb->h());
  void *p = (void*)*Fl_Graphics_Driver::id(rgb);
  if (p) sprintf(name, "FLrgb%p", p); else name[0] = 0;
  if (!p || !last_rgb_name_ || strcmp(name, last_rgb_name_) != 0) {
    if (*name==0 && need_clip) push_clip(XP, YP, WP, HP);
#if defined(HAVE_LIBJPEG)
    if (rgb->d() == 3 || rgb->d() == 1) define_rgb_jpeg(rgb, *name ? name : NULL, XP-cx, YP-cy);
    else
#endif // HAVE_LIBJPEG
      define_rgb_png(rgb, *name ? name : NULL, XP-cx, YP-cy);
    if (*name==0 && need_clip) pop_clip();
  }
  if (*name) {
    if (need_clip) push_clip(XP, YP, WP, HP);
    fprintf(out_, "<use href=\"#%s\" x=\"%d\" y=\"%d\"/>\n", last_rgb_name_, XP-cx, YP-cy);
    if (need_clip) pop_clip();
  }
#endif // HAVE_LIBPNG
}

void Fl_SVG_Graphics_Driver::draw_pixmap(Fl_Pixmap *pxm, int XP, int YP, int WP, int HP, int cx, int cy) {
#if defined(HAVE_LIBPNG)
  char name[24];
  bool need_clip = (cx || cy || WP != pxm->w() || HP != pxm->h());
  void *p = (void*)*Fl_Graphics_Driver::id(pxm);
  if (p) sprintf(name, "FLpx%p", p); else name[0] = 0;
  if (!p || !last_rgb_name_ || strcmp(name, last_rgb_name_) != 0) {
    Fl_RGB_Image *rgb = new Fl_RGB_Image(pxm);
    if (*name==0 && need_clip) push_clip(XP, YP, WP, HP);
    define_rgb_png(rgb, *name ? name : NULL, XP-cx, YP-cy);
    if (*name==0 && need_clip) pop_clip();
    delete rgb;
  }
  if (*name) {
    if (need_clip) push_clip(XP, YP, WP, HP);
    fprintf(out_, "<use href=\"#%s\" x=\"%d\" y=\"%d\"/>\n", last_rgb_name_, XP-cx, YP-cy);
    if (need_clip) pop_clip();
  }
#endif // HAVE_LIBPNG
}

void Fl_SVG_Graphics_Driver::draw_bitmap(Fl_Bitmap *bm, int XP, int YP, int WP, int HP, int cx, int cy) {
#if defined(HAVE_LIBPNG)
  char name[45];
  bool need_clip = (cx || cy || WP != bm->w() || HP != bm->h());
  void *p = (void*)*Fl_Graphics_Driver::id(bm);
  if (p) sprintf(name, "FLbm%p%X", p, fl_color()); else name[0] = 0;
  if (!p || !last_rgb_name_ || strcmp(name, last_rgb_name_) != 0) {
    uchar R, G, B;
    Fl::get_color(fl_color(), R, G, B);
    uchar *data = new uchar[bm->data_w() * bm->data_h() * 4];
    memset(data, 0, bm->data_w() * bm->data_h() * 4);
    Fl_RGB_Image *rgb = new Fl_RGB_Image(data, bm->data_w(), bm->data_h(), 4);
    rgb->alloc_array = 1;
    int rowBytes = (bm->data_w()+7)>>3 ;
    for (int j = 0; j < bm->data_h(); j++) {
      const uchar *p = bm->array + j*rowBytes;
      for (int i = 0; i < rowBytes; i++) {
        uchar q = *p;
        int last = bm->data_w() - 8*i; if (last > 8) last = 8;
        for (int k=0; k < last; k++) {
          if (q&1) {
            uchar *r = (uchar*)rgb->array + j*bm->data_w()*4 + i*8*4 + k*4;
            *r++ = R; *r++ = G; *r++ = B; *r = ~0;
          }
          q >>= 1;
        }
        p++;
      }
    }
    if (*name==0 && need_clip) push_clip(XP, YP, WP, HP);
    define_rgb_png(rgb, *name ? name : NULL, XP-cx, YP-cy);
    if (*name==0 && need_clip) pop_clip();
    delete rgb;
  }
  if (*name) {
    if (need_clip) push_clip(XP, YP, WP, HP);
    fprintf(out_, "<use href=\"#%s\" x=\"%d\" y=\"%d\"/>\n", last_rgb_name_, XP-cx, YP-cy);
    if (need_clip) pop_clip();
  }
#endif // HAVE_LIBPNG
}

void Fl_SVG_Graphics_Driver::draw_image(const uchar* buf, int x, int y, int w, int h, int d, int l) {
  if (d < 0) {
    fprintf(out_, "<g transform=\"translate(%d,%d) scale(-1,1)\">\n", x, y);
    x = -w; y = 0; buf -= (w-1)*abs(d);
  }
  if (l < 0) {
    fprintf(out_, "<g transform=\"translate(%d,%d) scale(1,-1)\">\n", x, y);
    x = 0; y = -h; buf -= (h-1)*abs(l);
  }
  Fl_RGB_Image *rgb = new Fl_RGB_Image(buf, w, h, abs(d), abs(l));
  rgb->draw(x, y);
  delete rgb;
  if (d < 0) fprintf(out_, "</g>\n");
  if (l < 0) fprintf(out_, "</g>\n");
}

void Fl_SVG_Graphics_Driver::draw_image(Fl_Draw_Image_Cb cb, void* data, int x, int y, int w, int h, int d) {
  uchar *buf = new uchar[w*h*d];
  for (int j = 0; j < h; j++) {
    cb(data, 0, j, w, buf + j*w*d);
  }
  draw_image(buf, x, y, w, h, d, 0);
  delete [] buf;
}

struct mono_image_data {
  const uchar *buf;
  int d;
  int l;
};

static void mono_image_cb(mono_image_data* data, int x, int y, int w, uchar* buf) {
  for (int i = 0; i < w; i++)
    *buf++ = *(data->buf + y*data->l + (x++)*data->d);
}

void Fl_SVG_Graphics_Driver::draw_image_mono(const uchar* buf, int x, int y, int w, int h, int d, int l) {
  mono_image_data data;
  data.buf = buf; data.d = d; data.l = (l?l:w*d);
  draw_image((Fl_Draw_Image_Cb)mono_image_cb, (void*)&data, x, y, w, h, 1);
}

void Fl_SVG_Graphics_Driver::draw_image_mono(Fl_Draw_Image_Cb cb, void* data, int x, int y, int w, int h, int d) {
  uchar *buf = new uchar[w*h*d];
  for (int j = 0; j < h; j++) {
    cb(data, 0, j, w, buf + j*w*d);
  }
  draw_image_mono(buf, x, y, w, h, d, 0);
  delete[] buf;
}

void Fl_SVG_Graphics_Driver::push_clip(int x, int y, int w, int h) {
  Clip * c=new Clip();
  clip_box(x,y,w,h,c->x,c->y,c->w,c->h);
  c->prev=clip_;
  sprintf(c->Id, "FLclip%d", clip_count_++);
  clip_=c;
  fprintf(out_, "<clipPath id=\"%s\"><rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\"/></clipPath><g clip-path=\"url(#%s)\">\n",
          c->Id, clip_->x , clip_->y , clip_->w, clip_->h, c->Id);
}

void Fl_SVG_Graphics_Driver::push_no_clip() {
  Clip * c=clip_;
  while (c) {
    fprintf(out_, "</g>");
    c = c->prev;
  }
  c=new Clip();
  c->prev=clip_;
  strcpy(c->Id, "none"); // mark of no_clip
  clip_=c;
  fprintf(out_, "<g clip-path=\"none\">\n");
}

void Fl_SVG_Graphics_Driver::pop_clip() {
  Clip *c;
  bool was_no_clip = clip_ && (strcmp(clip_->Id, "none") == 0);
  fprintf(out_, "</g>");
  if (clip_) {
    c = clip_;
    clip_ = clip_->prev;
    delete c;
  }
  if (was_no_clip) {
    Clip *next = NULL;
    c=clip_;
    while (c) {
      Clip *c2 = new Clip(*c);
      c2->prev = next;
      next = c2;
      c = c->prev;
    }
    while (next) {
      fprintf(out_, "<g clip-path=\"url(#%s)\">", next->Id);
      c = next->prev;
      delete next;
      next = c;
    }
  }
  fprintf(out_, "\n");
}

int Fl_SVG_Graphics_Driver::clip_box(int x, int y, int w, int h, int& X, int& Y, int& W, int& H) {
  if (!clip_) {
    X = x; Y = y; W = w; H = h;
    return 0;
  }
  if (clip_->w < 0) {
    X = x; Y = y; W = w; H = h;
    return 1;
  }
  int ret = 0;
  if (x > (X=clip_->x)) {X=x; ret=1;}
  if (y > (Y=clip_->y)) {Y=y; ret=1;}
  if ((x+w) < (clip_->x+clip_->w)) {
    W=x+w-X;
    ret=1;
  }else
    W = clip_->x + clip_->w - X;
  if(W<0){
    W=0;
    return 1;
  }
  if ((y+h) < (clip_->y+clip_->h)) {
    H=y+h-Y;
    ret=1;
  }else
    H = clip_->y + clip_->h - Y;
  if(H<0){
    W=0;
    H=0;
    return 1;
  }
  return ret;
}

int Fl_SVG_Graphics_Driver::not_clipped(int x, int y, int w, int h) {
  if (!clip_) return 1;
  if (clip_->w < 0) return 1;
  int X = 0, Y = 0, W = 0, H = 0;
  clip_box(x, y, w, h, X, Y, W, H);
  if (W) return 1;
  return 0;
}

void Fl_SVG_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2) {
  fprintf(out_, "<path d=\"M %d %d L %d %d L %d %d z\" fill=\"rgb(%u,%u,%u)\" />\n",
          x0, y0, x1, y1, x2, y2, red_, green_, blue_);
}

void Fl_SVG_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  fprintf(out_, "<path d=\"M %d %d L %d %d L %d %d L %d %d z\" fill=\"rgb(%u,%u,%u)\" />\n",
          x0, y0, x1, y1, x2, y2, x3, y3, red_, green_, blue_);
}

void Fl_SVG_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  fprintf(out_, "<path d=\"M %d %d L %d %d L %d %d L %d %d z\" fill=\"none\" stroke=\"rgb(%u,%u,%u)\" "
          "stroke-width=\"%d\" stroke-linejoin=\"%s\" stroke-linecap=\"%s\" stroke-dasharray=\"%s\"/>\n",
          x0, y0, x1, y1, x2, y2, x3, y3, red_, green_, blue_, width_, linejoin_, linecap_, dasharray_);
}

void Fl_SVG_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2) {
  fprintf(out_, "<path d=\"M %d %d L %d %d L %d %d z\" fill=\"none\" stroke=\"rgb(%u,%u,%u)\" "
          "stroke-width=\"%d\" stroke-linejoin=\"%s\" stroke-linecap=\"%s\" stroke-dasharray=\"%s\"/>\n",
          x0, y0, x1, y1, x2, y2, red_, green_, blue_, width_, linejoin_, linecap_, dasharray_);
}

void Fl_SVG_Graphics_Driver::transformed_vertex0(float x, float y) {
  if (!n || x != p[n-1].x || y != p[n-1].y) {
    if (n >= p_size) {
      p_size = p ? 2*p_size : 16;
      p = (XPOINT*)realloc((void*)p, p_size*sizeof(*p));
    }
   p[n].x = x;
   p[n].y = y;
   n++;
   }
}

void Fl_SVG_Graphics_Driver::transformed_vertex(double xf, double yf) {
  transformed_vertex0(float(xf), float(yf));
}

void Fl_SVG_Graphics_Driver::vertex(double x,double y) {
  transformed_vertex0(float(x*m.a + y*m.c + m.x), float(x*m.b + y*m.d + m.y));
}

void Fl_SVG_Graphics_Driver::end_points() {
  for (int i=0; i<n; i++) {
    fprintf(out_, "<path d=\"M %f %f L %f %f\" fill=\"none\" stroke=\"rgb(%u,%u,%u)\" stroke-width=\"%d\" />\n",
        p[i].x, p[i].y, p[i].x, p[i].y, red_, green_, blue_, width_);
  }
}

void Fl_SVG_Graphics_Driver::end_line() {
  if (n < 2) {
    end_points();
    return;
  }
  if (n<=1) return;
  fprintf(out_, "<path d=\"M %f %f", p[0].x, p[0].y);
  for (int i=1; i<n; i++)
    fprintf(out_, " L %f %f", p[i].x, p[i].y);
  fprintf(out_, "\" fill=\"none\" stroke=\"rgb(%u,%u,%u)\" stroke-width=\"%d\" stroke-dasharray=\"%s\" stroke-linecap=\"%s\" stroke-linejoin=\"%s\" />\n",
          red_, green_, blue_, width_, dasharray_, linecap_, linejoin_);
}

void Fl_SVG_Graphics_Driver::fixloop() {  // remove equal points from closed path
  while (n>2 && p[n-1].x == p[0].x && p[n-1].y == p[0].y) n--;
}

void Fl_SVG_Graphics_Driver::end_loop() {
  fixloop();
  if (n>2) transformed_vertex((float)p[0].x, (float)p[0].y);
  end_line();
}

void Fl_SVG_Graphics_Driver::end_polygon() {
  fixloop();
  if (n < 3) {
    end_line();
    return;
  }
  if (n<=1) return;
  fprintf(out_, "<path d=\"M %f %f", p[0].x, p[0].y);
  for (int i=1; i<n; i++)
    fprintf(out_, " L %f %f", p[i].x, p[i].y);
  fprintf(out_, " z\" fill=\"rgb(%u,%u,%u)\" />\n", red_, green_, blue_);
}

void Fl_SVG_Graphics_Driver::circle(double x, double y, double r) {
  double xt = transform_x(x,y);
  double yt = transform_y(x,y);
  double rx = r * (m.c ? sqrt(m.a*m.a+m.c*m.c) : fabs(m.a));
  double ry = r * (m.b ? sqrt(m.b*m.b+m.d*m.d) : fabs(m.d));
  int llx = (int)rint(xt-rx);
  int w = (int)rint(xt+rx)-llx;
  int lly = (int)rint(yt-ry);
  int h = (int)rint(yt+ry)-lly;
  fprintf(out_, "<circle cx=\"%g\" cy=\"%g\" r=\"%g\"", xt, yt, (w+h)*0.25f);
  if (what == POLYGON)
    fprintf(out_, " fill");
  else
    fprintf(out_, " fill=\"none\" stroke-width=\"%d\" stroke-dasharray=\"%s\" stroke-linecap=\"%s\" stroke", width_, dasharray_,linecap_);
  fprintf(out_, "=\"rgb(%u,%u,%u)\" />\n", red_, green_, blue_);
}

void Fl_SVG_Graphics_Driver::begin_complex_polygon() {
  begin_polygon();
  gap_ = 0;
}

void Fl_SVG_Graphics_Driver::gap() {
  while (n>gap_+2 && p[n-1].x == p[gap_].x && p[n-1].y == p[gap_].y) n--;
  if (n > gap_+2) {
    transformed_vertex((float)p[gap_].x, (float)p[gap_].y);
    gap_ = n;
  } else {
    n = gap_;
  }
}

void Fl_SVG_Graphics_Driver::end_complex_polygon() {
  gap();
  if (n < 3) {
    end_line();
    return;
  }
  if (n<=1) return;
  fprintf(out_, "<path d=\"M %f %f", p[0].x, p[0].y);
  for (int i=1; i<n; i++)
    fprintf(out_, " L %f %f", p[i].x, p[i].y);
  fprintf(out_, " z\" fill=\"rgb(%u,%u,%u)\" />\n", red_, green_, blue_);
}

void Fl_SVG_Graphics_Driver::arc(int x, int y, int w, int h, double a1, double a2) {
  arc_pie('A', x, y, w, h, a1, a2);
}

void Fl_SVG_Graphics_Driver::pie(int x, int y, int w, int h, double a1, double a2) {
  arc_pie('P', x, y, w, h, a1, a2);
}

void Fl_SVG_Graphics_Driver::arc_pie(char AorP, int x, int y, int w, int h, double a1, double a2) {
  // This implementation was constructed as follows:
  // - follow Fl_Quartz_Graphics_Driver::arc(int x,...).
  // which applies a translation, a scaling, and then calls
  //     CGContextAddArc(gc_, 0, 0, 0.5, a1, a2, 1);
  // to draw an arc of a circle given its center, its radius, and starting and ending angles
  // - consider https://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
  // which gives the equations that transform the center parameterization used by
  // CGContextAddArc() to the endpoint parameterization used by the "elliptical arc curve" command of SVG (A).
  if (w <= 0 || h <= 0) return;
  bool full = fabs(a1-a2) == 360; // case of full circle/disk
  a1 = (-a1)/180.0f*M_PI; a2 = (-a2)/180.0f*M_PI;
  float cx = x + 0.5f*w /*- 0.5f*/, cy = y + 0.5f*h - 0.5f;
  double r = (w!=h ? 0.5 : (w+h)*0.25f-0.5f);
  float stroke_width = width_;
  float sx, sy;
  if (w != h) {
    sx = w-1; sy = h-1;
    stroke_width /= ((sx+sy)/2);
  } else {
    sx = sy = 2*r;
    stroke_width /= sx;
  }
  fprintf(out_, "<g transform=\"translate(%f,%f) scale(%f,%f)\">\n", cx, cy, sx, sy);
  if (AorP == 'A') compute_dasharray((sx+sy)/2, user_dash_array_);
  if (full) {
    fprintf(out_, "<circle cx=\"0\" cy=\"0\" r=\"0.5\" style=\"fill");
    if (AorP == 'A')
      fprintf(out_, ":none;stroke-width:%f;stroke-linecap:%s;stroke-dasharray:%s;stroke", stroke_width, linecap_, dasharray_);
  } else {
    double x1 = 0.5*cos(a1), y1 = 0.5 * sin(a1);
    double x2 = 0.5*cos(a2), y2 = 0.5 * sin(a2);
    int fA = fabs(a2-a1) > M_PI ? 1 : 0;
    if (AorP == 'A')
      fprintf(out_, "<path d=\"M %f,%f A 0.5,0.5 0 %d,0 %f,%f\" "
              "style=\"fill:none;stroke-width:%f;stroke-linecap:%s;stroke-dasharray:%s;stroke",
              x1, y1, fA, x2, y2, stroke_width, linecap_, dasharray_);
    else
      fprintf(out_, "<path d=\"M 0,0 L %f,%f A 0.5,0.5 0 %d,0 %f,%f z\" style=\"fill",
              x1, y1, fA, x2, y2);
  }
  fprintf(out_, ":rgb(%u,%u,%u)\"/>\n</g>\n", red_, green_, blue_);
  if (AorP == 'A') compute_dasharray(1., user_dash_array_);
}

#else

Fl_SVG_File_Surface::Fl_SVG_File_Surface(int w, int h, FILE *f) : Fl_Widget_Surface(NULL) {
  width_ = height_ = 0;
}
Fl_SVG_File_Surface::~Fl_SVG_File_Surface() {}
int Fl_SVG_File_Surface::close() {return 0;}
FILE *Fl_SVG_File_Surface::file() {return NULL;}
void Fl_SVG_File_Surface::origin(int x, int y) {}
void Fl_SVG_File_Surface::translate(int x, int y) {}
void Fl_SVG_File_Surface::untranslate() {}
int Fl_SVG_File_Surface::printable_rect(int *w, int *h) {return 0;}

#endif // FLTK_USE_SVG
