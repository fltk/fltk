// glut_font.C

// (sort of) emulation of Glut's bitmap drawing functions, using FL's
// font stuff.  Not all the fonts match!

#include <config.h>
#if HAVE_GL

#include <FL/glut.H>
#include <FL/gl.h>

Glut_Bitmap_Font glutBitmap9By15 = {FL_SCREEN, 15};
Glut_Bitmap_Font glutBitmap8By13 = {FL_SCREEN, 13};
Glut_Bitmap_Font glutBitmapTimesRoman10 = {FL_TIMES, 10};
Glut_Bitmap_Font glutBitmapTimesRoman24 = {FL_TIMES, 24};
Glut_Bitmap_Font glutBitmapHelvetica10 = {FL_HELVETICA, 10};
Glut_Bitmap_Font glutBitmapHelvetica12 = {FL_HELVETICA, 12};
Glut_Bitmap_Font glutBitmapHelvetica18 = {FL_HELVETICA, 18};

void glutBitmapCharacter(void *font, int character) {
  gl_font(((Glut_Bitmap_Font *)font)->font,((Glut_Bitmap_Font *)font)->size);
  char a[1]; a[0] = character;
  gl_draw(a,1);
}

int glutBitmapWidth(int font, int character) {
  gl_font(((Glut_Bitmap_Font *)font)->font,((Glut_Bitmap_Font *)font)->size);
  return int(gl_width(character)+.5);
}

#endif

