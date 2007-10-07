#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

int  main(int argc, char ** argv)
{
	Fl_Window *window;
	Fl_Box *box, *box1;

	window = new Fl_Window(300, 180);
	window->color(50);
	box = new Fl_Box(20, 40, 130, 100, "Hello World!");
	box->box(FL_UP_BOX);
	box->labelsize(36);
	box->color(fl_rgb_color(15));
	box->labelfont(FL_BOLD+FL_ITALIC);
	box->labeltype(FL_SHADOW_LABEL);
	box1 = new Fl_Box(150, 40, 130, 100, "Hello World!");
	box1->box(FL_UP_BOX);
	box1->labelsize(36);
	box1->color(15);
	box1->labelfont(FL_BOLD+FL_ITALIC);
	box1->labeltype(FL_SHADOW_LABEL);
	window->end();
	window->resizable(window);
	window->show(argc, argv);
	return(Fl::run());
}

