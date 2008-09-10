/* Drag and Drop test demo. */
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

static Fl_Window *win_a;
static Fl_Window *win_b;

class Sender : public Fl_Box {
public:
    Sender(int x,int y,int w,int h) : Fl_Box(x,y,w,h) { }

    int handle(int event) {
        int ret = Fl_Box::handle(event);
        switch ( event ) {
            case FL_PUSH:
                Fl::copy("message",7,0);
                Fl::dnd();
                return(1);
        }
        return(ret);
    }
};

class Receiver : public Fl_Box {
public:
    Receiver(int x,int y,int w,int h) : Fl_Box(x,y,w,h) { }

    int handle(int event) {
        int ret = Fl_Box::handle(event);
        switch ( event ) {
            case FL_DND_ENTER:
            case FL_DND_DRAG:
            case FL_DND_RELEASE:
                return(1);
            case FL_PASTE:
                label(Fl::event_text());
                fprintf(stderr, "PASTE: %s\n", Fl::event_text());
				fflush(stderr);
                return(1);
        }
        return(ret);
    }
};
//
// Demonstrate DND (drag+drop) from red sender to green receiver
//

static Sender *Tx;
static Receiver *Rx;

int main(int argc, char **argv) {
	win_a = new Fl_Window(40, 40, 200,100);
	win_a->begin();
	Tx = new Sender(5,5,90,90);
	  Tx->box(FL_UP_BOX);
	  Tx->label("Drag from here");
	  Tx->align(FL_ALIGN_WRAP);
	  Tx->color(FL_RED);
	win_a->end();
	win_a->show(argc, argv);

	win_b = new Fl_Window (350, 40, 200,100,"Receiver");
	win_b->begin();
	Rx = new Receiver(105,5,90,90);
	  Rx->box(FL_FLAT_BOX);
	  Rx->label("to here");
	  Rx->color(FL_GREEN);
	win_b->end();
	win_b->show();

	return Fl::run();
}
/* End of File */
