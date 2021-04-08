//
// Tiny OpenGL v3 + glut demo program for the Fast Light Tool Kit (FLTK).
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

#include <stdio.h>
#if defined(__APPLE__)
#  define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED 1
#  include <OpenGL/gl3.h> // defines OpenGL 3.0+ functions
#else
#  if defined(_WIN32)
#    define GLEW_STATIC 1
#  endif
#  include <GL/glew.h>
#endif
#include <FL/glut.H>


// Globals
// Real programs don't use globals :-D
// Data would normally be read from files
GLfloat vertices[] = {  -1.0f,0.0f,0.0f,
  0.0f,1.0f,0.0f,
  0.0f,0.0f,0.0f };
GLfloat colours[] = {   1.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, 0.0f, 1.0f };
GLfloat vertices2[] = { 0.0f,0.0f,0.0f,
  0.0f,-1.0f,0.0f,
  1.0f,0.0f,0.0f };

// two vertex array objects, one for each object drawn
unsigned int vertexArrayObjID[2];
// three vertex buffer objects in this example
unsigned int vertexBufferObjID[3];


void printShaderInfoLog(GLint shader)
{
  int infoLogLen = 0;
  GLchar *infoLog;

  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
  if (infoLogLen > 0)
  {
    infoLog = new GLchar[infoLogLen];
    // error check for fail to allocate memory omitted
    glGetShaderInfoLog(shader,infoLogLen, NULL, infoLog);
    fprintf(stderr, "InfoLog:\n%s\n", infoLog);
    delete [] infoLog;
  }
}


void init(void)
{
  // Would load objects from file here - but using globals in this example

  // Allocate Vertex Array Objects
  glGenVertexArrays(2, &vertexArrayObjID[0]);
  // Setup first Vertex Array Object
  glBindVertexArray(vertexArrayObjID[0]);
  glGenBuffers(2, vertexBufferObjID);

  // VBO for vertex data
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[0]);
  glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // VBO for colour data
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[1]);
  glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), colours, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);

  // Setup second Vertex Array Object
  glBindVertexArray(vertexArrayObjID[1]);
  glGenBuffers(1, &vertexBufferObjID[2]);

  // VBO for vertex data
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[2]);
  glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), vertices2, GL_STATIC_DRAW);
  glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}


void initShaders(void)
{
  GLuint p, f, v;
  glClearColor (1.0, 1.0, 1.0, 0.0);

  v = glCreateShader(GL_VERTEX_SHADER);
  f = glCreateShader(GL_FRAGMENT_SHADER);

#ifdef __APPLE__
#define SHADING_LANG_VERS "140"
#else
#define SHADING_LANG_VERS "130"
#endif
  // load shaders
  const char *vv = "#version " SHADING_LANG_VERS "\n\
  in  vec3 in_Position;\
  in  vec3 in_Color;\
  out vec3 ex_Color;\
  void main(void)\
  {\
    ex_Color = in_Color;\
    gl_Position = vec4(in_Position, 1.0);\
  }";

  const char *ff = "#version " SHADING_LANG_VERS "\n\
  precision highp float;\
  in  vec3 ex_Color;\
  out vec4 out_Color;\
  void main(void)\
  {\
    out_Color = vec4(ex_Color,1.0);\
  }";

  glShaderSource(v, 1, &vv,NULL);
  glShaderSource(f, 1, &ff,NULL);

  GLint compiled;

  glCompileShader(v);
  glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
  if (!compiled)
  {
    fprintf(stderr, "Vertex shader not compiled.\n");
    printShaderInfoLog(v);
  }

  glCompileShader(f);
  glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);
  if (!compiled)
  {
    fprintf(stderr, "Fragment shader not compiled.\n");
    printShaderInfoLog(f);
  }

  p = glCreateProgram();

  glAttachShader(p,v);
  glAttachShader(p,f);
  glBindAttribLocation(p,0, "in_Position");
  glBindAttribLocation(p,1, "in_Color");

  glLinkProgram(p);
  glGetProgramiv(p, GL_LINK_STATUS, &compiled);
  if (compiled != GL_TRUE) {
    GLchar *infoLog; GLint length;
    glGetProgramiv(p, GL_INFO_LOG_LENGTH, &length);
    infoLog = new GLchar[length];
    glGetProgramInfoLog(p, length, NULL, infoLog);
    fprintf(stderr, "Link log=%s\n", infoLog);
    delete[] infoLog;
  }
  glUseProgram(p);
}


void display(void)
{
  // clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindVertexArray(vertexArrayObjID[0]);       // First VAO
  glDrawArrays(GL_TRIANGLES, 0, 3);     // draw first object

  glBindVertexArray(vertexArrayObjID[1]);               // select second VAO
  glVertexAttrib3f((GLuint)1, 1.0, 0.0, 0.0); // set constant color attribute
  glDrawArrays(GL_TRIANGLES, 0, 3);     // draw second object
 }

const int fullscreen = 0; // TEST (set to 1 to enable fullscreen mode)

int main (int argc, char* argv[])
{
  Fl::use_high_res_GL(true);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | FL_OPENGL3);
  glutInitWindowSize(400, 400);
  glutCreateWindow("Triangle Test");
#ifndef __APPLE__
  GLenum err = glewInit(); // defines pters to functions of OpenGL V 1.2 and above
  if (err) Fl::error("glewInit() failed returning %u", err);
  fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
  int gl_version_major;
  const char *glv = (const char*)glGetString(GL_VERSION);
  fprintf(stderr, "OpenGL version %s supported\n", glv);
  sscanf(glv, "%d", &gl_version_major);
  if (gl_version_major < 3) {
    fprintf(stderr, "\nThis platform does not support OpenGL V3\n\n");
    exit(1);
  }
  initShaders();
  init();
  glutDisplayFunc(display);
  if (fullscreen) Fl::first_window()->fullscreen();
  glutMainLoop();
  return 0;
}
