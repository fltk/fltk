// fl_shadow_box.C

// Box drawing code for an obscure box type.
// These box types are in seperate files so they are not linked
// in if not used.

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#define BW 3

static void fl_shadow_frame(int x, int y, int w, int h, Fl_Color c) {
  fl_color(FL_DARK3);
  fl_rectf(x+BW, y+h,  w, BW);
  fl_rectf(x+w,  y+BW, BW,  h);
  fl_color(c);
  fl_rect(x,y,w,h);
}

static void fl_shadow_box(int x, int y, int w, int h, Fl_Color c) {
  fl_color(c);
  fl_rectf(x+1,y+1,w-2,h-2);
  fl_shadow_frame(x,y,w,h,FL_GRAY0);
}

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);
Fl_Boxtype define_FL_SHADOW_BOX() {
  fl_internal_boxtype(_FL_SHADOW_FRAME, fl_shadow_frame);
  fl_internal_boxtype(_FL_SHADOW_BOX, fl_shadow_box);
  return _FL_SHADOW_BOX;
}
