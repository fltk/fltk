// Fl_Free.H

// Emulation of the Forms "free" widget.  This emulation allows the
// free demo to run, but it is not clear if it is sufficient to make
// porting programs any easier.

#ifndef Fl_Free_H
#define Fl_Free_H

#ifndef Fl_Widget_H
#include "Fl_Widget.H"
#endif

#define FL_NORMAL_FREE		1
#define FL_SLEEPING_FREE	2
#define FL_INPUT_FREE		3
#define FL_CONTINUOUS_FREE	4
#define FL_ALL_FREE		5

typedef int (*FL_HANDLEPTR)(Fl_Widget *, int , float, float, char);

class Fl_Free : public Fl_Widget {
    FL_HANDLEPTR hfunc;
    static void step(void *);
    void draw();
    int handle(int);
public:
    Fl_Free(uchar t,int x,int y,int w,int h,const char *l,FL_HANDLEPTR hdl);
    ~Fl_Free();
};

// old event names for compatability:
#define FL_MOUSE		FL_DRAG
#define FL_DRAW			0
#define FL_STEP			9
#define FL_FREEMEM		12
#define FL_FREEZE		FL_UNMAP
#define FL_THAW			FL_MAP

#endif
