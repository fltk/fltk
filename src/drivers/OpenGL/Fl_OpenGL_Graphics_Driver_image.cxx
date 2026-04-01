//
// Support for drawing images to Fl_Gl_Window for the Fast Light Tool Kit (FLTK).
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

#include "Fl_OpenGL_Graphics_Driver.H"
#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Image.H>
#include <FL/Fl.H>

std::map<Fl_Image*, GLuint> *Fl_OpenGL_Graphics_Driver::image_texture_map_ = NULL;

void Fl_OpenGL_Graphics_Driver::delete_image_texture(Fl_Image *img) {
  if (image_texture_map_) {
    auto iter = image_texture_map_->find(img);
    if (iter != image_texture_map_->end()) {
      glDeleteTextures(1, &iter->second);
      image_texture_map_->erase(iter);
    }
  }
}


void Fl_OpenGL_Graphics_Driver::cache_size(Fl_Image* img, int &width, int &height) {
  width *= pixels_per_unit_; // useful at least for Fl_SVG_Image's
  height *= pixels_per_unit_;
}


#ifndef GL_TEXTURE_RECTANGLE_ARB
#  define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#  define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#endif

static GLuint compute_texture_rectangle(Fl_RGB_Image *rgb)
{
  Fl_RGB_Image *temp_rgb4 = NULL;
  if (rgb->d() == 2) { // convert depth-2 into depth-4 image
    uchar *data = new uchar[rgb->data_w() * rgb->data_h() * 4];
    int ld = rgb->ld() ? rgb->ld() : 2 * rgb->data_w();
    uchar *p = data;
    for (int j = 0; j < rgb->data_h(); j++) {
      const uchar *q = rgb->array + j * ld;
      for (int i = 0; i < rgb->data_w(); i++) {
        *p = *(p+1) = *(p+2) = *q;
        *(p+3) = *(q+1);
        p += 4;
        q += 2;
      }
    }
    temp_rgb4 = new Fl_RGB_Image(data, rgb->data_w(), rgb->data_h(), 4);
    temp_rgb4->alloc_array = 1;
    rgb = temp_rgb4; // and use the depth-4 image to compute texture
  }
  GLuint texName;
  glGenTextures(1, &texName);
  // save GL parameters GL_UNPACK_ROW_LENGTH and GL_UNPACK_ALIGNMENT
  GLint row_length, alignment;
  glGetIntegerv(GL_UNPACK_ROW_LENGTH, &row_length);
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
  // put the bitmap in a texture
  glPushAttrib(GL_TEXTURE_BIT);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texName);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, (rgb->ld() ? rgb->ld() / rgb->d() : rgb->data_w()));
  GLint unpack_alignment = 4;
  if (rgb->d() < 4) {
    int ld = (rgb->ld() ? rgb->ld() : rgb->data_w() * rgb->d());
    if (ld % 4 != 0) unpack_alignment = 1;
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, unpack_alignment);
  GLint internalformat = GL_RGBA8; // for depth-4
  GLenum format = GL_RGBA, type_arg = GL_UNSIGNED_INT_8_8_8_8_REV; // for depth-4
  if (rgb->d() == 1) {
    internalformat = GL_ALPHA8; format = GL_ALPHA; type_arg = GL_UNSIGNED_BYTE;
  } else if (rgb->d() == 3) {
    internalformat = GL_RGB; format = GL_RGB; type_arg = GL_UNSIGNED_BYTE;
  }
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, internalformat, rgb->data_w(), rgb->data_h(), 0,
               format, type_arg, rgb->array);
  glPopAttrib();
  // restore saved GL parameters
  glPixelStorei(GL_UNPACK_ROW_LENGTH, row_length);
  glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  if (temp_rgb4) delete temp_rgb4;
  return texName;
}

// X,Y,W,H = where to draw full or partial texture in FLTK coords
// cx,cy = offset inside full texture in FLTK coords
// sw = img->data_w() / img->w(), sh = img->data_h() / img->h()
static void draw_texture(GLuint texName, int X, int Y, int W, int H,
                         int cx, int cy, float sw, float sh, Fl_Color col = FL_WHITE,
                         bool alpha_blend = false) {
  // GL_TRANSFORM_BIT for GL_PROJECTION
  // GL_TEXTURE_BIT for GL_TEXTURE_RECTANGLE_ARB
  // GL_COLOR_BUFFER_BIT for GL_BLEND and glBlendFunc,
  glPushAttrib(GL_TRANSFORM_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT);
  //setup matrices
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  uchar RR, GG, BB;
  Fl::get_color(col, RR, GG, BB);
  glColor4ub(RR, GG, BB, 0xff); // the color drawn through the texture seen as a kind of mask

  float winw = Fl_Window::current()->as_gl_window()->pixel_w(); // winw,h = pixel size of GL scene
  float winh = Fl_Window::current()->as_gl_window()->pixel_h();

  float R = 2;
  glScalef(R/winw, R/winh, 1.0f);
  glTranslatef(-winw/R, -winh/R, 0.0f);
  float s = Fl_Window::current()->as_gl_window()->pixels_per_unit();
  int HH = s * H, WW = s * W; // WW,HH = size of drawn image in pixels
  float ox = s * X; // ox,oy = lower-left of where to draw image in pixel coords
  float oy = winh - s * Y;
  glEnable(GL_TEXTURE_RECTANGLE_ARB);
  if (alpha_blend) {
    glEnable (GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
  }
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texName);
  GLint width, height; // pixel size of texture
  glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ARB, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ARB, 0, GL_TEXTURE_HEIGHT, &height);
  //write the texture on screen
  // coords inside texture: 0 -> width and 0 -> height
  glBegin(GL_QUADS);
  glTexCoord2i(sw * cx, sh * cy); // lower left
  glVertex2f(ox, oy);
  glTexCoord2i(sw * cx, sh * (cy+H)); // upper left
  glVertex2f(ox, oy - HH);
  glTexCoord2i(sw * (cx+W), sh * (cy+H)); // upper right
  glVertex2f(ox + WW, oy - HH);
  glTexCoord2i(sw * (cx+W), sh * cy); // lower right
  glVertex2f(ox + WW, oy);
  glEnd();
  // reset original matrices
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib(); // GL_TRANSFORM_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT
}


// Argument img here can have depth 1, 2, 3, or 4.
void Fl_OpenGL_Graphics_Driver::draw_rgb(Fl_RGB_Image *img,
                                         int XP, int YP, int WP, int HP, int cx, int cy) {
  // Don't draw an empty image.
  if (!img->array) {
    Fl_Graphics_Driver::draw_empty(img, XP, YP);
    return;
  }
  int X, Y, W, H;
  if (start_image(img, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  if (!image_texture_map_) image_texture_map_ = new std::map<Fl_Image*, GLuint>;
  auto iter = image_texture_map_->find(img);
  GLuint texNum;
  if (iter == image_texture_map_->end()) {
    texNum = compute_texture_rectangle(img);
    (*image_texture_map_)[img] = texNum;
  } else texNum = iter->second;
  draw_texture(texNum, X, Y, W, H, cx, cy,
               float(img->data_w()) / img->w(), float(img->data_h()) / img->h(),
               FL_WHITE, (img->d() == 1));
  color(color()); // reset current color
}


void Fl_OpenGL_Graphics_Driver::draw_pixmap(Fl_Pixmap *pxm,
                                            int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  // Don't draw an empty image.
  if (!pxm->data() || !pxm->w()) {
    Fl_Graphics_Driver::draw_empty(pxm, XP, YP);
    return;
  }
  if (start_image(pxm, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  if (!image_texture_map_) image_texture_map_ = new std::map<Fl_Image*, GLuint>;
  auto iter = image_texture_map_->find(pxm);
  GLuint texNum;
  if (iter == image_texture_map_->end()) {
    Fl_RGB_Image rgb(pxm);
    texNum = compute_texture_rectangle(&rgb);
    (*image_texture_map_)[pxm] = texNum;
  } else texNum = iter->second;
  draw_texture(texNum, X, Y, W, H, cx, cy,
               float(pxm->data_w()) / pxm->w(), float(pxm->data_h()) / pxm->h());
  color(color()); // reset current color
}


static Fl_RGB_Image *bitmap_to_rgb1(Fl_Bitmap *bm) {
  int ld = 4 * ((bm->data_w() + 3) / 4);
  uchar *data = new uchar[ld * bm->data_h()];
  memset(data, 0, bm->data_h() * ld);
  Fl_RGB_Image *rgb = new Fl_RGB_Image(data, bm->data_w(), bm->data_h(), 1, ld);
  rgb->alloc_array = 1;
  int rowBytes = (bm->data_w()+7)>>3 ;
  for (int j = 0; j < bm->data_h(); j++) {
    const uchar *p = bm->array + j*rowBytes;
    for (int i = 0; i < rowBytes; i++) {
      uchar q = *p;
      int last = bm->data_w() - 8*i; if (last > 8) last = 8;
      for (int k=0; k < last; k++) {
        if (q&1) {
          *((uchar*)rgb->array + j*ld + i*8 + k) = 0xff;
        }
        q >>= 1;
      }
      p++;
    }
  }
  return rgb;
}


void Fl_OpenGL_Graphics_Driver::draw_bitmap(Fl_Bitmap *bm,
                                            int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  if (start_image(bm, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  if (!image_texture_map_) image_texture_map_ = new std::map<Fl_Image*, GLuint>;
  GLuint texNum;
  auto iter = image_texture_map_->find(bm);
  if (iter == image_texture_map_->end()) {
    Fl_RGB_Image *rgb = bitmap_to_rgb1(bm);
    texNum = compute_texture_rectangle(rgb);
    (*image_texture_map_)[bm] = texNum;
    delete rgb;
  } else texNum = iter->second;
  draw_texture(texNum, X, Y, W, H, cx, cy,
               float(bm->data_w()) / bm->w(), float(bm->data_h()) / bm->h(), color());
}
