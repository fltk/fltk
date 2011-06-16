
#include "main.h"
#include "ui.h"

#include <FL/FL.h>

int main(int argc, char **argv)
{
  Fl_Window *win = make_window();
  win->show(argc, argv);
  return Fl::run();
}
