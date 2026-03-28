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
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl.H>


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
  glPixelStorei(GL_UNPACK_ALIGNMENT, (rgb->d() == 3 ? 1 : 4));
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, (rgb->d() == 3 ? GL_RGB : GL_RGBA8),
               rgb->data_w(), rgb->data_h(), 0,
               (rgb->d() == 3 ? GL_RGB : GL_RGBA),
               (rgb->d() == 3 ? GL_UNSIGNED_BYTE : GL_UNSIGNED_INT_8_8_8_8_REV),
               rgb->array);
  glPopAttrib();
  // restore saved GL parameters
  glPixelStorei(GL_UNPACK_ROW_LENGTH, row_length);
  glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  return texName;
}


void Fl_OpenGL_Graphics_Driver::draw_rgb(Fl_RGB_Image *img,
                                         int XP, int YP, int WP, int HP, int cx, int cy) {
  int X, Y, W, H;
  // Don't draw an empty image. Gray-scale image drawing not implemented yet.
  if (!img->d() || !img->array || img->d() < 3) {
    Fl_Graphics_Driver::draw_empty(img, XP, YP);
    return;
  }
  if (start_image(img, XP, YP, WP, HP, cx, cy, X, Y, W, H)) {
    return;
  }
  Fl_RGB_Image *partial_rgb = NULL;
  if (cx || cy || W < img->w() || H < img->h()) { // Partial image drawing.
    int d = img->d();
    int ld = d * img->data_w();
    float sx = img->data_w() / float(img->w()), sy = img->data_h() / float(img->h());
    cx *= sx; cy *= sy;
    partial_rgb = new Fl_RGB_Image(img->array + cy * ld + cx * d, W * sx, H * sy, d, ld);
    img = partial_rgb;
  }
  GLuint texName = compute_texture_rectangle(img);
  // GL_TRANSFORM_BIT for GL_PROJECTION
  // GL_TEXTURE_BIT for GL_TEXTURE_RECTANGLE_ARB
  glPushAttrib(GL_TRANSFORM_BIT | GL_TEXTURE_BIT);
  //setup matrices
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glColor4ub(0xff, 0xff, 0xff, 0xff); // the color drawn through the texture seen as a kind of mask

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
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texName);
  //write the texture on screen
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f); // draw lower left in world coordinates
  glVertex2f(ox, oy);
  glTexCoord2f(0.0f, (GLfloat)img->data_h()); // draw upper left in world coordinates
  glVertex2f(ox, oy - HH);
  glTexCoord2f((GLfloat)img->data_w(), (GLfloat)img->data_h()); // draw upper right in world coordinates
  glVertex2f(ox + WW, oy - HH);
  glTexCoord2f((GLfloat)img->data_w(), 0.0f); // draw lower right in world coordinates
  glVertex2f(ox + WW, oy);
  glEnd();
  glDeleteTextures(1, &texName);
  color(color()); // reset current color
  // reset original matrices
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib(); // GL_TRANSFORM_BIT | GL_TEXTURE_BIT
  if (partial_rgb) delete partial_rgb;
}


void Fl_OpenGL_Graphics_Driver::draw_pixmap(Fl_Pixmap *img,
                                            int XP, int YP, int WP, int HP, int cx, int cy) {
  Fl_RGB_Image rgb(img);
  draw_rgb(&rgb, XP, YP, WP, HP, cx, cy);
}
