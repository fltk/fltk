#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <stdio.h>

int main(int argc, char **argv)
{
    Fl_Window *window = new Fl_Window(300,200);
    window->clear_border(); // no border!!
    window->show();

    // wait some time
    Fl::wait(10.);
    window->size(0,0);

    while(true) {
        printf("LOOP ");
        // doesn't wait....
        Fl::wait(100.);
    }

    return 0;
}