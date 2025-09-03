//
// CubeView class definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

#ifndef CUBEVIEW_H
#define CUBEVIEW_H 1

// Note: The CubeView demo program can only be built if
//  (1) '<config.h>' can be included (FLTK build) *or*
//  (2) 'HAVE_GL' has been defined prior to including this header.

// Define 'HAVE_GL=1' on the compiler commandline to build
// this program w/o 'config.h' (needs FLTK lib with GL).

#ifndef HAVE_GL
#include <config.h>
#endif

// Note to editor: the following code can and should be copied
// to the fluid tutorial in 'documentation/src/fluid.dox'
// *without* '#if HAVE_GL' preprocessor statements, leaving
// only those parts where the condition is true.

// [\code in documentation/src/fluid.dox]
#include <FL/Fl.H>
#if HAVE_GL
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#else
#include <FL/Fl_Box.H>
#endif /* HAVE_GL */

#if HAVE_GL
class CubeView : public Fl_Gl_Window {
#else
class CubeView : public Fl_Box {
#endif /* HAVE_GL */

public:
  CubeView(int x, int y, int w, int h, const char *l = 0);

  // This value determines the scaling factor used to draw the cube.
  double size;

  /* Set the rotation about the vertical (y) axis.
   *
   * This function is called by the horizontal roller in
   * CubeViewUI and the initialize button in CubeViewUI.
   */
  void v_angle(double angle) { vAng = angle; }

  // Return the rotation about the vertical (y) axis.
  double v_angle() const { return vAng; }

  /* Set the rotation about the horizontal (x) axis.
   *
   * This function is called by the vertical roller in
   * CubeViewUI and the initialize button in CubeViewUI.
   */

  void h_angle(double angle) { hAng = angle; }

  // The rotation about the horizontal (x) axis.
  double h_angle() const { return hAng; }

  /* Sets the x shift of the cube view camera.
   *
   * This function is called by the slider in CubeViewUI
   * and the initialize button in CubeViewUI.
   */
  void panx(double x) { xshift = x; }

  /* Sets the y shift of the cube view camera.
   *
   * This function is called by the slider in CubeViewUI
   * and the initialize button in CubeViewUI.
   */
  void pany(double y) { yshift = y; }

#if HAVE_GL
  /* The widget class draw() FL_OVERRIDE.
   *
   * The draw() function initializes Gl for another round of
   * drawing, then calls specialized functions for drawing each
   * of the entities displayed in the cube view.
   */
  void draw() FL_OVERRIDE;
#endif /* HAVE_GL */

private:
  /*  Draw the cube boundaries.
   *
   * Draw the faces of the cube using the boxv[] vertices,
   * using GL_LINE_LOOP for the faces.
   */
#if HAVE_GL
  void drawCube();
#else
  void drawCube() {}
#endif /* HAVE_GL */

  double vAng, hAng;
  double xshift, yshift;

  float boxv0[3]; float boxv1[3];
  float boxv2[3]; float boxv3[3];
  float boxv4[3]; float boxv5[3];
  float boxv6[3]; float boxv7[3];
};

// [\endcode in documentation/src/fluid.dox]

#endif // CUBEVIEW_H
