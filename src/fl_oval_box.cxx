/*	fl_oval_box.C

	Less-used box types are in seperate files so they are not linked
	in if not used.

*/

#include <FL/Fl.H>
#include <FL/fl_draw.H>

static void fl_oval_flat_box(int x, int y, int w, int h, Fl_Color c) {
  fl_color(c);
  fl_pie(x, y, w, h, 0, 360);
}

static void fl_oval_frame(int x, int y, int w, int h, Fl_Color c) {
  fl_color(c);
  fl_arc(x, y, w, h, 0, 360);
}

static void fl_oval_box(int x, int y, int w, int h, Fl_Color c) {
  fl_oval_flat_box(x,y,w-1,h-1,c);
  fl_oval_frame(x,y,w,h,FL_BLACK);
}

static void fl_oval_shadow_box(int x, int y, int w, int h, Fl_Color c) {
  fl_oval_flat_box(x+3,y+3,w,h,FL_DARK3);
  fl_oval_box(x,y,w,h,c);
}

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);
Fl_Boxtype define_FL_OVAL_BOX() {
  fl_internal_boxtype(_FL_OSHADOW_BOX,fl_oval_shadow_box);
  fl_internal_boxtype(_FL_OVAL_FRAME,fl_oval_frame);
  fl_internal_boxtype(_FL_OFLAT_BOX,fl_oval_flat_box);
  fl_internal_boxtype(_FL_OVAL_BOX,fl_oval_box);
  return _FL_OVAL_BOX;
}
