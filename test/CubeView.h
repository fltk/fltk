//
// "$Id: CubeView.h,v 1.2 1999/03/04 20:11:49 mike Exp $"
//
// CubeView class definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#ifndef CUBEVIEW_H
#define CUBEVIEW_H 1
#include <config.h>
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#ifdef HAVE_GL
#  include <FL/gl.h>
#  include <GL/glu.h>
#endif /* HAVE_GL */

#include <stdlib.h>

// shorthand to save some bits.
#define v3f(x) glVertex3fv(x)


class CubeView : public Fl_Gl_Window {

public:
    // this value determines the scaling factor used to draw the cube.
    double size;

    CubeView(int x,int y,int w,int h,const char *l=0);

    /* Set the rotation about the vertical (y ) axis.
     *
     * This function is called by the horizontal roller in CubeViewUI and the
     * initialize button in CubeViewUI.
     */
    void v_angle(float angle){vAng=angle;};
    
    // Return the rotation about the vertical (y ) axis.
    float v_angle(){return vAng;};

    /* Set the rotation about the horizontal (x ) axis.
     *
     * This function is called by the vertical roller in CubeViewUI and the
     * initialize button in CubeViewUI.
     */

    void h_angle(float angle){hAng=angle;};

    // the rotation about the horizontal (x ) axis.
    float h_angle(){return hAng;};

    /* Sets the x shift of the cube view camera.
     *
     * This function is called by the slider in CubeViewUI and the
     * initialize button in CubeViewUI.
     */
    void panx(float x){xshift=x;};
    /* Sets the y shift of the cube view camera.
     *
     * This function is called by the slider in CubeViewUI and the
     * initialize button in CubeViewUI.
     */
    void pany(float y){yshift=y;};

    /*The widget class draw() override.
     *
     *The draw() function initialize Gl for another round o f drawing
     * then calls specialized functions for drawing each of the
     * entities displayed in the cube view.
     *
     */
    void draw();    
private:

    /*  Draw the cube boundaries
     *
     *Draw the faces of the cube using the boxv[] vertices, using
     * GL_LINE_LOOP for the faces. The color is \#defined by CUBECOLOR.
     */
    void drawCube();
    
    float vAng,hAng;
    float xshift,yshift;


    float boxv0[3];float boxv1[3];
    float boxv2[3];float boxv3[3];
    float boxv4[3];float boxv5[3];
    float boxv6[3];float boxv7[3];

};
#endif

//
// End of "$Id: CubeView.h,v 1.2 1999/03/04 20:11:49 mike Exp $".
//
