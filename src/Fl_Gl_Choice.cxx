//
// OpenGL visual selection code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#include <config.h>
#if HAVE_GL

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */
#include <FL/Fl.H>
#include "Fl_Gl_Choice.H"
#include <FL/Fl_Gl_Window.H>
#include "Fl_Gl_Window_Driver.H"
#include <FL/gl_draw.H>
#include <stdlib.h>
#ifndef GL_CURRENT_PROGRAM
// from glew.h in Windows, glext.h in Unix, not used by FLTK's macOS platform
#  define GL_CURRENT_PROGRAM 0x8B8D
#endif

typedef void (*glUseProgram_type)(GLint);
static glUseProgram_type glUseProgram_f = NULL;

GLContext *Fl_Gl_Window_Driver::context_list = 0;
int Fl_Gl_Window_Driver::nContext = 0;
static int NContext = 0;

void Fl_Gl_Window_Driver::add_context(GLContext ctx) {
  if (!ctx) return;
  if (nContext==NContext) {
    if (!NContext) NContext = 8;
    NContext *= 2;
    context_list = (GLContext*)realloc(
      context_list, NContext*sizeof(GLContext));
  }
  context_list[nContext++] = ctx;
}

void Fl_Gl_Window_Driver::del_context(GLContext ctx) {
  int i;
  for (i=0; i<nContext; i++) {
    if (context_list[i]==ctx) {
      memmove(context_list+i, context_list+i+1,
        (nContext-i-1) * sizeof(GLContext));
      context_list[--nContext] = 0;
      break;
    }
  }
  if (!nContext) gl_remove_displaylist_fonts();
}


void Fl_Gl_Window_Driver::switch_to_GL1() {
  if (!glUseProgram_f) {
    glUseProgram_f = (glUseProgram_type)GetProcAddress("glUseProgram");
  }
  glGetIntegerv(GL_CURRENT_PROGRAM, &current_prog);
  // Switch from GL3-style to GL1-style drawing;
  // good under Windows, X11 and Wayland; not appropriate under macOS.
  // suggested by https://stackoverflow.com/questions/22293870/mixing-fixed-function-pipeline-and-programmable-pipeline-in-opengl
  if (current_prog) glUseProgram_f(0);
}

void Fl_Gl_Window_Driver::switch_back() {
  if (current_prog) glUseProgram_f((GLuint)current_prog);
}

Fl_Gl_Choice *Fl_Gl_Window_Driver::first;

// this assumes one of the two arguments is zero:
// We keep the list system in Win32 to stay compatible and interpret
// the list later...
Fl_Gl_Choice *Fl_Gl_Window_Driver::find_begin(int m, const int *alistp) {
  Fl_Gl_Choice *g;
  for (g = first; g; g = g->next)
    if (g->mode == m && g->alist == alistp)
      return g;
  return NULL;
}

/**
 \}
 \endcond
 */

#endif // HAVE_GL
