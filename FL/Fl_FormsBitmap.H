/*	Fl_FormsBitmap.H

	Forms compatability widget

*/

#ifndef Fl_FormsBitmap_H
#define Fl_FormsBitmap_H

#include "Fl_Bitmap.H"

class Fl_FormsBitmap : public Fl_Widget {
    Fl_Bitmap *b;
protected:
    void draw();
public:
    Fl_FormsBitmap(Fl_Boxtype, int, int, int, int, const char * = 0);
    void set(int W, int H, const uchar *bits);
    void bitmap(Fl_Bitmap *B) {b = B;}
    Fl_Bitmap *bitmap() const {return b;}
};

#endif
