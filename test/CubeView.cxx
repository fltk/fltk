//
// "$Id: CubeView.cxx,v 1.2 1999/03/04 20:11:48 mike Exp $"
//
// CubeView class implementation for the Fast Light Tool Kit (FLTK).
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

#include "CubeView.h"
#include <math.h>


CubeView::CubeView(int x,int y,int w,int h,const char *l)
            : Fl_Gl_Window(x,y,w,h,l)
{
    vAng = 0.0;
    hAng=0.0;
    size=10.0;
    
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
};

// The color used for the edges of the bounding cube.
#define CUBECOLOR 255,255,255,255

void CubeView::drawCube() {
#ifdef HAVE_GL
/* Draw a colored cube */
    glBegin(GL_LINE_LOOP); glColor4ub(CUBECOLOR); v3f(boxv0); v3f(boxv1); v3f(boxv2); v3f(boxv3); glEnd();
    glBegin(GL_LINE_LOOP); glColor4ub(CUBECOLOR); v3f(boxv5); v3f(boxv4); v3f(boxv7); v3f(boxv6); glEnd();
    glBegin(GL_LINE_LOOP); glColor4ub(CUBECOLOR); v3f(boxv0); v3f(boxv4); v3f(boxv5); v3f(boxv1); glEnd();
    glBegin(GL_LINE_LOOP); glColor4ub(CUBECOLOR); v3f(boxv2); v3f(boxv6); v3f(boxv7); v3f(boxv3); glEnd();
    glBegin(GL_LINE_LOOP); glColor4ub(CUBECOLOR); v3f(boxv0); v3f(boxv3); v3f(boxv7); v3f(boxv4); glEnd();
    glBegin(GL_LINE_LOOP); glColor4ub(CUBECOLOR); v3f(boxv1); v3f(boxv5); v3f(boxv6); v3f(boxv2); glEnd();
#define ALPHA 128
    glBegin(GL_QUADS); glColor4ub(  0,   0, 255, ALPHA); v3f(boxv0); v3f(boxv1); v3f(boxv2); v3f(boxv3); glEnd();
    glBegin(GL_QUADS); glColor4ub(255, 255,   0, ALPHA); v3f(boxv0); v3f(boxv4); v3f(boxv5); v3f(boxv1); glEnd();
    glBegin(GL_QUADS); glColor4ub(  0, 255, 255, ALPHA); v3f(boxv2); v3f(boxv6); v3f(boxv7); v3f(boxv3); glEnd();
    glBegin(GL_QUADS); glColor4ub(255,   0,   0, ALPHA); v3f(boxv4); v3f(boxv5); v3f(boxv6); v3f(boxv7); glEnd();
    glBegin(GL_QUADS); glColor4ub(255,   0, 255, ALPHA); v3f(boxv0); v3f(boxv3); v3f(boxv7); v3f(boxv4); glEnd();
    glBegin(GL_QUADS); glColor4ub(  0, 255,   0, ALPHA); v3f(boxv1); v3f(boxv5); v3f(boxv6); v3f(boxv2); glEnd();
#endif /* HAVE_GL */
};//drawCube

void CubeView::draw() {
#ifdef HAVE_GL
    if (!valid()) {
        glLoadIdentity();
        glViewport(0,0,w(),h());
        glOrtho(-10,10,-10,10,-20000,10000);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();

    glTranslatef(xshift, yshift, 0);
    glRotatef(hAng,0,1,0); glRotatef(vAng,1,0,0);
    glScalef(float(size),float(size),float(size));

    drawCube();
    
    glPopMatrix();
#endif /* HAVE_GL */
};

//
// End of "$Id: CubeView.cxx,v 1.2 1999/03/04 20:11:48 mike Exp $".
//
