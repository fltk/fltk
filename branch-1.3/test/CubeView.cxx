//
// "$Id$"
//
// CubeView class implementation for the Fast Light Tool Kit (FLTK).
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

#include "CubeView.h"
#include <math.h>


#if HAVE_GL
CubeView::CubeView(int x,int y,int w,int h,const char *l)
            : Fl_Gl_Window(x,y,w,h,l)
#else
CubeView::CubeView(int x,int y,int w,int h,const char *l)
            : Fl_Box(x,y,w,h,l)
#endif /* HAVE_GL */
{
    vAng = 0.0;
    hAng=0.0;
    size=10.0;
    xshift=0.0;
    yshift=0.0;
    
    /* The cube definition. These are the vertices of a unit cube
     * centered on the origin.*/
    
    boxv0[0] = -0.5; boxv0[1] = -0.5; boxv0[2] = -0.5;
    boxv1[0] =  0.5; boxv1[1] = -0.5; boxv1[2] = -0.5;
    boxv2[0] =  0.5; boxv2[1] =  0.5; boxv2[2] = -0.5;
    boxv3[0] = -0.5; boxv3[1] =  0.5; boxv3[2] = -0.5;
    boxv4[0] = -0.5; boxv4[1] = -0.5; boxv4[2] =  0.5;
    boxv5[0] =  0.5; boxv5[1] = -0.5; boxv5[2] =  0.5;
    boxv6[0] =  0.5; boxv6[1] =  0.5; boxv6[2] =  0.5;
    boxv7[0] = -0.5; boxv7[1] =  0.5; boxv7[2] =  0.5;

#if !HAVE_GL
    label("OpenGL is required for this demo to operate.");
    align(FL_ALIGN_WRAP | FL_ALIGN_INSIDE);
#endif /* !HAVE_GL */
}

#if HAVE_GL
void CubeView::drawCube() {
/* Draw a colored cube */
#define ALPHA 0.5
    glShadeModel(GL_FLAT);

    glBegin(GL_QUADS);
      glColor4f(0.0, 0.0, 1.0, ALPHA);
      glVertex3fv(boxv0);
      glVertex3fv(boxv1);
      glVertex3fv(boxv2);
      glVertex3fv(boxv3);

      glColor4f(1.0, 1.0, 0.0, ALPHA);
      glVertex3fv(boxv0);
      glVertex3fv(boxv4);
      glVertex3fv(boxv5);
      glVertex3fv(boxv1);

      glColor4f(0.0, 1.0, 1.0, ALPHA);
      glVertex3fv(boxv2);
      glVertex3fv(boxv6);
      glVertex3fv(boxv7);
      glVertex3fv(boxv3);

      glColor4f(1.0, 0.0, 0.0, ALPHA);
      glVertex3fv(boxv4);
      glVertex3fv(boxv5);
      glVertex3fv(boxv6);
      glVertex3fv(boxv7);

      glColor4f(1.0, 0.0, 1.0, ALPHA);
      glVertex3fv(boxv0);
      glVertex3fv(boxv3);
      glVertex3fv(boxv7);
      glVertex3fv(boxv4);

      glColor4f(0.0, 1.0, 0.0, ALPHA);
      glVertex3fv(boxv1);
      glVertex3fv(boxv5);
      glVertex3fv(boxv6);
      glVertex3fv(boxv2);
    glEnd();

    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
      glVertex3fv(boxv0);
      glVertex3fv(boxv1);

      glVertex3fv(boxv1);
      glVertex3fv(boxv2);

      glVertex3fv(boxv2);
      glVertex3fv(boxv3);

      glVertex3fv(boxv3);
      glVertex3fv(boxv0);

      glVertex3fv(boxv4);
      glVertex3fv(boxv5);

      glVertex3fv(boxv5);
      glVertex3fv(boxv6);

      glVertex3fv(boxv6);
      glVertex3fv(boxv7);

      glVertex3fv(boxv7);
      glVertex3fv(boxv4);

      glVertex3fv(boxv0);
      glVertex3fv(boxv4);

      glVertex3fv(boxv1);
      glVertex3fv(boxv5);

      glVertex3fv(boxv2);
      glVertex3fv(boxv6);

      glVertex3fv(boxv3);
      glVertex3fv(boxv7);
    glEnd();
}//drawCube

void CubeView::draw() {
    if (!valid()) {
        glLoadIdentity();
        glViewport(0,0,w(),h());
        glOrtho(-10,10,-10,10,-20050,10000);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();

    glTranslatef((GLfloat) xshift, (GLfloat) yshift, 0);
    glRotatef((GLfloat) hAng,0,1,0); glRotatef((GLfloat) vAng,1,0,0);
    glScalef(float(size),float(size),float(size));

    drawCube();
    
    glPopMatrix();
}
#endif /* HAVE_GL */

//
// End of "$Id$".
//
