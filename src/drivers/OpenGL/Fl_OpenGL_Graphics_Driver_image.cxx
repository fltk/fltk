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
  } else if (rgb->d() == 2) {
    // TODO
  } else if (rgb->d() == 3) {
    internalformat = GL_RGB; format = GL_RGB; type_arg = GL_UNSIGNED_BYTE;
  }
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, internalformat, rgb->data_w(), rgb->data_h(), 0,
               format, type_arg, rgb->array);
  glPopAttrib();
  // restore saved GL parameters
  glPixelStorei(GL_UNPACK_ROW_LENGTH, row_length);
  glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  return texName;
}


static void draw_texture(GLuint texName, int X, int Y, int W, int H, Fl_Color col = FL_WHITE,
                         bool alpha_blend = false) {
  // GL_TRANSFORM_BIT for GL_PROJECTION
  // GL_TEXTURE_BIT for GL_TEXTURE_RECTANGLE_ARB
  glPushAttrib(GL_TRANSFORM_BIT | GL_TEXTURE_BIT);
  //setup matrices
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  uchar RR, GG, BB;
  Fl::get_color(col, RR, GG, BB);
  glColor4ub(RR, GG, BB, 0xff); // the color drawn through the texture seen as a kind of mask

  float winw = Fl_Window::current()->as_gl_window()->pixel_w();
  float winh = Fl_Window::current()->as_gl_window()->pixel_h();

  float R = 2;
  glScalef(R/winw, R/winh, 1.0f);
  glTranslatef(-winw/R, -winh/R, 0.0f);
  float s = Fl_Window::current()->as_gl_window()->pixels_per_unit();
  int HH = s * H, WW = s * W;
  float ox = s * X;
  float oy = winh - s * Y;
  glEnable(GL_TEXTURE_RECTANGLE_ARB);
  if (alpha_blend) {
    glEnable (GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
  }
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texName);
  GLint width, height;
  glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ARB, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ARB, 0, GL_TEXTURE_HEIGHT, &height);
  //write the texture on screen
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f); // draw lower left in world coordinates
  glVertex2f(ox, oy);
  glTexCoord2f(0.0f, (GLfloat)height); // draw upper left in world coordinates
  glVertex2f(ox, oy - HH);
  glTexCoord2f((GLfloat)width, (GLfloat)height); // draw upper right in world coordinates
  glVertex2f(ox + WW, oy - HH);
  glTexCoord2f((GLfloat)width, 0.0f); // draw lower right in world coordinates
  glVertex2f(ox + WW, oy);
  glEnd();
  // reset original matrices
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib(); // GL_TRANSFORM_BIT | GL_TEXTURE_BIT
}


// Argument img here can have depth 3, or 4 to draw color images.
// It can also have depth 1 in 2 situations:
//   1) we draw a depth-1 (gray scale) Fl_RGB_Image (was_bitmap = false)
//   2) we draw an Fl_Bitmap and use a depth-1 Fl_RGB_Image as a drawing tool (was_bitmap = true)
void Fl_OpenGL_Graphics_Driver::draw_rgb134_(Fl_RGB_Image *img,
                                int XP, int YP, int WP, int HP, int cx, int cy, bool was_bitmap) {
  int X, Y, W, H;
  if (start_image(img, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  if (cx || cy || W < img->w() || H < img->h()) { // Partial image drawing.
    int d = img->d();
    int ld = (img->ld() ? img->ld() : d * img->data_w());
    float sx = img->data_w() / float(img->w()), sy = img->data_h() / float(img->h());
    cx *= sx; cy *= sy;
    Fl_RGB_Image *partial_rgb = new Fl_RGB_Image(img->array + cy * ld + cx * d,
                                                 W * sx, H * sy, d, ld);
    partial_rgb->scale(W, H, 0, 1);
    draw_rgb134_(partial_rgb, X, Y, W, H, 0, 0, was_bitmap);
    delete partial_rgb;
    return;
  }
  if (!image_texture_map_) image_texture_map_ = new std::map<Fl_Image*, GLuint>;
  auto iter = image_texture_map_->find(img);
  GLuint texNum;
  if (iter == image_texture_map_->end()) {
    texNum = compute_texture_rectangle(img);
    (*image_texture_map_)[img] = texNum;
  } else texNum = iter->second;
  draw_texture(texNum, X, Y, W, H,
               (was_bitmap ? color() : FL_WHITE), (img->d() == 1 && !was_bitmap));
  color(color()); // reset current color
}


void Fl_OpenGL_Graphics_Driver::draw_rgb(Fl_RGB_Image *img,
                                         int XP, int YP, int WP, int HP, int cx, int cy) {
  // Don't draw an empty image. Drawing of depth-2 images not implemented yet.
  if (!img->array || img->d() == 2) {
    Fl_Graphics_Driver::draw_empty(img, XP, YP);
    return;
  }
  draw_rgb134_(img, XP, YP, WP, HP, cx, cy);
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
  if (cx || cy || W < pxm->w() || H < pxm->h()) { // Partial image drawing.
    Fl_RGB_Image rgb(pxm);
    draw_rgb134_(&rgb, X, Y, W, H, cx, cy);
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
  draw_texture(texNum, X, Y, W, H);
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
  if (cx || cy || W < bm->w() || H < bm->h()) { // Partial image drawing.
    Fl_RGB_Image *rgb = bitmap_to_rgb1(bm);
    draw_rgb134_(rgb, X, Y, W, H, cx, cy, true);
    delete rgb;
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
  draw_texture(texNum, X, Y, W, H, color());
}
