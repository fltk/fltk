// Fl_Image.C

// Draw a image in a box.

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Image.H>

void Fl_Image::draw(int X, int Y, int W, int H, int cx,int cy) {
  // clip the box down to the size of image, quit if empty:
  if (cx < 0) {W += cx; X -= cx; cx = 0;}
  if (cx+W > w) W = w-cx;
  if (W <= 0) return;
  if (cy < 0) {H += cy; Y -= cy; cy = 0;}
  if (cy+H > h) H = h-cy;
  if (H <= 0) return;
  if (!id) {
    id = (ulong)fl_create_offscreen(w, h);
    fl_begin_offscreen((Fl_Offscreen)id);
    fl_draw_image(array, 0, 0, w, h, d, ld);
    fl_end_offscreen();
  }
  fl_copy_offscreen(X, Y, W, H, (Fl_Offscreen)id, cx, cy);
}

Fl_Image::~Fl_Image() {
  if (id) fl_delete_offscreen((Fl_Offscreen)id);
}

static void image_labeltype(
    const Fl_Label* o, int x, int y, int w, int h, Fl_Align a)
{
  Fl_Image* b = (Fl_Image*)(o->value);
  int cx;
  if (a & FL_ALIGN_LEFT) cx = 0;
  else if (a & FL_ALIGN_RIGHT) cx = b->w-w;
  else cx = (b->w-w)/2;
  int cy;
  if (a & FL_ALIGN_TOP) cy = 0;
  else if (a & FL_ALIGN_BOTTOM) cy = b->h-h;
  else cy = (b->h-h)/2;
  b->draw(x,y,w,h,cx,cy);
}

static void image_measure(const Fl_Label* o, int& w, int& h) {
  Fl_Image* b = (Fl_Image*)(o->value);
  w = b->w;
  h = b->h;
}

void Fl_Image::label(Fl_Widget* o) {
  Fl::set_labeltype(_FL_IMAGE_LABEL, image_labeltype, image_measure);
  o->label(_FL_IMAGE_LABEL, (const char*)this);
}

void Fl_Image::label(Fl_Menu_Item* o) {
  Fl::set_labeltype(_FL_IMAGE_LABEL, image_labeltype, image_measure);
  o->label(_FL_IMAGE_LABEL, (const char*)this);
}
