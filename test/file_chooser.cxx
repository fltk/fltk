/*	Test fl_file_chooser()	*/

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/fl_file_chooser.H>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

Fl_Input *pattern, *current;

void pickfile(Fl_Widget *) {
  const char *p;
  p = fl_file_chooser("Pick a file",pattern->value(),current->value());
  if (p) current->value(p);
}

void thecb(const char *name) {
  printf("Callback '%s'\n",name);
}

int main(int argc, char **argv) {
  Fl_Window window(400,200);
  pattern = new Fl_Input(100,50,280,30,"Pattern:");
  pattern->static_value("*");
  current = new Fl_Input(100,90,280,30,"Current:");
  Fl_Button button(100,120,100,30,"&Choose file");
  button.callback(pickfile);
  window.end();
  window.show(argc, argv);
  fl_file_chooser_callback(thecb);
  return Fl::run();
}
