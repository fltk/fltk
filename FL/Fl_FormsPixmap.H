/*	Fl_FormsPixmap.H

	Forms compatability widget

*/

#ifndef Fl_FormsPixmap_H
#define Fl_FormsPixmap_H

#include "Fl_Pixmap.H"

class Fl_FormsPixmap : public Fl_Widget {
    Fl_Pixmap *b;
protected:
    void draw();
public:
    Fl_FormsPixmap(Fl_Boxtype, int, int, int, int, const char * = 0);
    void set(/*const*/char * const * bits);
    void Pixmap(Fl_Pixmap *B) {b = B;}
    Fl_Pixmap *Pixmap() const {return b;}
};

#endif
