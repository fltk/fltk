//
// "$Id$"
//
// Tiny OpenGL v3 demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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

#include <stdarg.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#if defined(__APPLE__)
#  include <OpenGL/gl3.h> // defines OpenGL 3.0+ functions
#else
#  if defined(WIN32)
#    define GLEW_STATIC 1
#  endif
#  include <GL/glew.h>
#endif


void add_output(const char *format, ...);


class SimpleGL3Window : public Fl_Gl_Window {
  GLuint shaderProgram;
  GLuint vertexArrayObject;
  GLuint vertexBuffer;
  GLint positionUniform;
  GLint colourAttribute;
  GLint positionAttribute;
  int gl_version_major;
public:
  SimpleGL3Window(int x, int y, int w, int h) :  Fl_Gl_Window(x, y, w, h) {
    mode(FL_RGB8 | FL_DOUBLE | FL_OPENGL3);
    shaderProgram = 0;
  }
  void draw(void) {
    if (gl_version_major < 3) return;
    if (!shaderProgram) {
      GLuint  vs;
      GLuint  fs;
      int Mslv, mslv; // major and minor version numbers of the shading language
      sscanf((char*)glGetString(GL_SHADING_LANGUAGE_VERSION), "%d.%d", &Mslv, &mslv);
      add_output("Shading Language Version=%d.%d\n",Mslv, mslv);
      const char *vss_format="#version %d%d\n\
      uniform vec2 p;\
      in vec4 position;\
      in vec4 colour;\
      out vec4 colourV;\
      void main (void)\
      {\
      colourV = colour;\
      gl_Position = vec4(p, 0.0, 0.0) + position;\
      }";
      char vss_string[300]; const char *vss = vss_string;
      sprintf(vss_string, vss_format, Mslv, mslv);
      const char *fss_format="#version %d%d\n\
      in vec4 colourV;\
      out vec4 fragColour;\
      void main(void)\
      {\
      fragColour = colourV;\
      }";
      char fss_string[200]; const char *fss = fss_string;
      sprintf(fss_string, fss_format, Mslv, mslv);
      GLint err; GLchar CLOG[1000]; GLsizei length;
      vs = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vs, 1, &vss, NULL);
      glCompileShader(vs);
      glGetShaderiv(vs, GL_COMPILE_STATUS, &err);
      if (err != GL_TRUE) {
	glGetShaderInfoLog(vs, sizeof(CLOG), &length, CLOG); 
	add_output("vs ShaderInfoLog=%s\n",CLOG);
	}
      fs = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fs, 1, &fss, NULL);
      glCompileShader(fs);
      glGetShaderiv(fs, GL_COMPILE_STATUS, &err);
      if (err != GL_TRUE) {
	glGetShaderInfoLog(fs, sizeof(CLOG), &length, CLOG); 
	add_output("fs ShaderInfoLog=%s\n",CLOG);
	}     
      // Attach the shaders
      shaderProgram = glCreateProgram();
      glAttachShader(shaderProgram, vs);
      glAttachShader(shaderProgram, fs);
      glBindFragDataLocation(shaderProgram, 0, "fragColour");
      glLinkProgram(shaderProgram);
      glGetProgramiv(shaderProgram, GL_LINK_STATUS, &err);
      if (err != GL_TRUE) {
        glGetProgramInfoLog(shaderProgram, sizeof(CLOG), &length, CLOG);
        add_output("link log=%s\n", CLOG);
      }
      // Get pointers to uniforms and attributes
      positionUniform = glGetUniformLocation(shaderProgram, "p");
      colourAttribute = glGetAttribLocation(shaderProgram, "colour");
      positionAttribute = glGetAttribLocation(shaderProgram, "position");
      glDeleteShader(vs);
      glDeleteShader(fs);
      // Upload vertices (1st four values in a row) and colours (following four values)
      GLfloat vertexData[]= { -0.5,-0.5,0.0,1.0,   1.0,0.0,0.0,1.0,
        -0.5, 0.5,0.0,1.0,   0.0,1.0,0.0,1.0,
        0.5, 0.5,0.0,1.0,   0.0,0.0,1.0,1.0,
        0.5,-0.5,0.0,1.0,   1.0,1.0,1.0,1.0};
      glGenVertexArrays(1, &vertexArrayObject);
      glBindVertexArray(vertexArrayObject);
      
      glGenBuffers(1, &vertexBuffer);
      glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
      glBufferData(GL_ARRAY_BUFFER, 4*8*sizeof(GLfloat), vertexData, GL_STATIC_DRAW);
      
      glEnableVertexAttribArray((GLuint)positionAttribute);
      glEnableVertexAttribArray((GLuint)colourAttribute  );
      glVertexAttribPointer((GLuint)positionAttribute, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
      glVertexAttribPointer((GLuint)colourAttribute  , 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (char*)0+4*sizeof(GLfloat));
    }
    else if ((!valid())) {
      glViewport(0, 0, pixel_w(), pixel_h());
    }
    glClearColor(0.08, 0.8, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    GLfloat p[]={0,0};
    glUniform2fv(positionUniform, 1, (const GLfloat *)&p);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }
  virtual int handle(int event) {
    static int first = 1;
    if (first && event == FL_SHOW && shown()) {
      first = 0;
      make_current();
#ifndef __APPLE__
      GLenum err = glewInit(); // defines pters to functions of OpenGL V 1.2 and above
      if (err) Fl::warning("glewInit() failed returning %u", err);
      else add_output("Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
      const uchar *glv = glGetString(GL_VERSION);
      add_output("GL_VERSION=%s\n", glv);
      sscanf((const char *)glv, "%d", &gl_version_major);
      if (gl_version_major < 3) add_output("\nThis platform does not support OpenGL V3\n\n");
    }
    
    if (event == FL_PUSH && gl_version_major >= 3) {
      static float factor = 1.1;
      GLfloat data[4];
      glGetBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(GLfloat), data);
      if (data[0] < -0.88 || data[0] > -0.5) factor = 1/factor;
      data[0] *= factor;
      glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(GLfloat), data);
      glGetBufferSubData(GL_ARRAY_BUFFER, 24*sizeof(GLfloat), 4*sizeof(GLfloat), data);
      data[0] *= factor;
      glBufferSubData(GL_ARRAY_BUFFER, 24*sizeof(GLfloat), 4*sizeof(GLfloat), data);
      redraw();
      add_output("push  Fl_Gl_Window::pixels_per_unit()=%.1f\n", pixels_per_unit());
      return 1;
    }
    return Fl_Gl_Window::handle(event);
  }
  void reset(void) { shaderProgram = 0; }
};


void toggle_double(Fl_Widget *wid, void *data) {
  static bool doublebuff = true;
  doublebuff = !doublebuff;
  SimpleGL3Window *glwin = (SimpleGL3Window*)data;
  int flags = glwin->mode();
  if (doublebuff) flags |= FL_DOUBLE; else flags &= ~FL_DOUBLE;
  glwin->mode(flags);
  glwin->reset();
}


Fl_Text_Display *output; // shared between output_win() and add_output()

void output_win(SimpleGL3Window *gl)
{
  output = new Fl_Text_Display(300,0,500, 280);
  Fl_Light_Button *lb = new Fl_Light_Button(300, 280, 500, 20, "Double-Buffered");
  lb->callback(toggle_double);
  lb->user_data(gl);
  lb->value(1);
  output->buffer(new Fl_Text_Buffer());
}


void add_output(const char *format, ...)
{
  va_list args;
  char line_buffer[10000];
  va_start(args, format);
  vsnprintf(line_buffer, sizeof(line_buffer)-1, format, args);
  va_end(args);
  output->buffer()->append(line_buffer);
  output->scroll(10000, 0);
  output->redraw();
}


int main(int argc, char **argv)
{
  Fl::use_high_res_GL(1);
  Fl_Window *topwin = new Fl_Window(800, 300);
  SimpleGL3Window *win = new SimpleGL3Window(0, 0, 300, 300);
  win->end();
  output_win(win);
  topwin->end();
  topwin->resizable(win);
  topwin->label("Click GL panel to reshape");
  topwin->show(argc, argv);
  Fl::run();
}

//
// End of "$Id$".
//

