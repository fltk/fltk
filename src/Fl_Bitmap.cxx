/*	Fl_Bitmap.C

	Draw a bitmap in a box.

*/

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Bitmap.H>

void Fl_Bitmap::draw(int XP, int YP, int WP, int HP, int cx, int cy) {
  // account for current clip region (faster on Irix):
  int X,Y,W,H; fl_clip_box(XP,YP,WP,HP,X,Y,W,H);
  cx += X-XP; cy += Y-YP;
  // clip the box down to the size of image, quit if empty:
  if (cx < 0) {W += cx; X -= cx; cx = 0;}
  if (cx+W > w) W = w-cx;
  if (W <= 0) return;
  if (cy < 0) {H += cy; Y -= cy; cy = 0;}
  if (cy+H > h) H = h-cy;
  if (H <= 0) return;
#ifdef WIN32
  if (!id) {
    // we need to pad the lines out to words & swap the bits
    // in each byte.
    int w1 = (w+7)/8;
    int w2 = ((w+15)/16)*2;
    uchar* newarray = new uchar[w2*h];
    const uchar* src = array;
    uchar* dest = newarray;
    for (int y=0; y < h; y++) {
      for (int n = 0; n < w1; n++) {
	*dest++ =
	  ((*src&0x01) << 7) +
	  ((*src&0x02) << 5) +
	  ((*src&0x04) << 3) +
	  ((*src&0x08) << 1) +
	  ((*src&0x10) >> 1) +
	  ((*src&0x20) >> 3) +
	  ((*src&0x40) >> 5) +
	  ((*src&0x80) >> 7);
	src++;
      }
      dest += w2-w1;
    }
    id = (ulong)CreateBitmap(w, h, 1, 1, newarray);
    array = newarray; // keep the pointer so I can free it later
  }
  HDC tempdc = CreateCompatibleDC(fl_gc);
  SelectObject(tempdc, (HGDIOBJ)id);
  SelectObject(fl_gc, fl_brush());
  // secret bitblt code found in old MSWindows reference manual:
  BitBlt(fl_gc, X, Y, W, H, tempdc, cx, cy, 0xE20746L);
  DeleteDC(tempdc);
#else
  if (!id) id = XCreateBitmapFromData(fl_display, fl_window,
				      (const char*)array, (w+7)&-8, h);
  XSetStipple(fl_display, fl_gc, id);
  int ox = X-cx; if (ox < 0) ox += w;
  int oy = Y-cy; if (oy < 0) oy += h;
  XSetTSOrigin(fl_display, fl_gc, ox, oy);
  XSetFillStyle(fl_display, fl_gc, FillStippled);
  XFillRectangle(fl_display, fl_window, fl_gc, X, Y, W, H);
  XSetFillStyle(fl_display, fl_gc, FillSolid);
#endif
}

Fl_Bitmap::~Fl_Bitmap() {
#ifdef WIN32
  if (id) {
    DeleteObject((HGDIOBJ)id);
    delete[] (uchar*)array;
  }
#else
  if (id) fl_delete_offscreen((Fl_Offscreen)id);
#endif
}

static void bitmap_labeltype(
    const Fl_Label* o, int x, int y, int w, int h, Fl_Align a)
{
  Fl_Bitmap* b = (Fl_Bitmap*)(o->value);
  int cx;
  if (a & FL_ALIGN_LEFT) cx = 0;
  else if (a & FL_ALIGN_RIGHT) cx = b->w-w;
  else cx = (b->w-w)/2;
  int cy;
  if (a & FL_ALIGN_TOP) cy = 0;
  else if (a & FL_ALIGN_BOTTOM) cy = b->h-h;
  else cy = (b->h-h)/2;
  fl_color((Fl_Color)o->color);
  b->draw(x,y,w,h,cx,cy);
}

static void bitmap_measure(const Fl_Label* o, int& w, int& h) {
  Fl_Bitmap* b = (Fl_Bitmap*)(o->value);
  w = b->w;
  h = b->h;
}

void Fl_Bitmap::label(Fl_Widget* o) {
  Fl::set_labeltype(_FL_BITMAP_LABEL, bitmap_labeltype, bitmap_measure);
  o->label(_FL_BITMAP_LABEL, (const char*)this);
}

void Fl_Bitmap::label(Fl_Menu_Item* o) {
  Fl::set_labeltype(_FL_BITMAP_LABEL, bitmap_labeltype, bitmap_measure);
  o->label(_FL_BITMAP_LABEL, (const char*)this);
}
