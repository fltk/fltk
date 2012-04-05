//
// "$Id$"
//
// CubeView class definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#ifndef CUBEVIEW_H
#define CUBEVIEW_H 1
#include <config.h>
#include <FL/Fl.H>
#if HAVE_GL
#  include <FL/Fl_Gl_Window.H>
#  include <FL/gl.h>
#else
#  include <FL/Fl_Box.H>
#endif /* HAVE_GL */

#include <stdlib.h>

#if HAVE_GL
class CubeView : public Fl_Gl_Window {
#else
class CubeView : public Fl_Box {
#endif /* HAVE_GL */

public:
    // this value determines the scaling factor used to draw the cube.
    double size;

    CubeView(int x,int y,int w,int h,const char *l=0);

    /* Set the rotation about the vertical (y ) axis.
     *
     * This function is called by the horizontal roller in CubeViewUI and the
     * initialize button in CubeViewUI.
     */
    void v_angle(double angle){vAng=angle;}
    
    // Return the rotation about the vertical (y ) axis.
    double v_angle() const {return vAng;}

    /* Set the rotation about the horizontal (x ) axis.
     *
     * This function is called by the vertical roller in CubeViewUI and the
     * initialize button in CubeViewUI.
     */

    void h_angle(double angle){hAng=angle;}

    // the rotation about the horizontal (x ) axis.
    double h_angle() const {return hAng;}

    /* Sets the x shift of the cube view camera.
     *
     * This function is called by the slider in CubeViewUI and the
     * initialize button in CubeViewUI.
     */
    void panx(double x){xshift=x;}
    /* Sets the y shift of the cube view camera.
     *
     * This function is called by the slider in CubeViewUI and the
     * initialize button in CubeViewUI.
     */
    void pany(double y){yshift=y;}

#if HAVE_GL
    /*The widget class draw() override.
     *
     *The draw() function initialize Gl for another round o f drawing
     * then calls specialized functions for drawing each of the
     * entities displayed in the cube view.
     *
     */
    void draw();    
#endif /* HAVE_GL */
private:

    /*  Draw the cube boundaries
     *
     *Draw the faces of the cube using the boxv[] vertices, using
     * GL_LINE_LOOP for the faces. The color is \#defined by CUBECOLOR.
     */
#if HAVE_GL
    void drawCube();
#else
    void drawCube() { }
#endif /* HAVE_GL */
    
    double vAng,hAng;
    double xshift,yshift;


    float boxv0[3];float boxv1[3];
    float boxv2[3];float boxv3[3];
    float boxv4[3];float boxv5[3];
    float boxv6[3];float boxv7[3];

};
#endif

//
// End of "$Id$".
//
