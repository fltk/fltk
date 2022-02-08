#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Wizard.H>

class Panel : public Fl_Group {
public:
  Panel(int X, int Y, int W, int H, Fl_Color C, const char* T)
  : Fl_Group(X, Y,  W, H, T) {
    align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    box(FL_ENGRAVED_BOX);
    labelcolor(C);
    labelsize(20);
    end();
  }
};

class Wizard : public Fl_Wizard {
public:
  Wizard(int X, int Y, int W, int H, const char* T=0)
  : Fl_Wizard(X, Y, W, H, T) {
    p1 = new Panel(X, Y, W, H, FL_RED,     "Panel 1");
    p2 = new Panel(X, Y, W, H, FL_MAGENTA, "Panel 2");
    p3 = new Panel(X, Y, W, H, FL_BLUE,    "Panel 3");
    value(p1);
  }
  void next_panel() {
    Panel* p = (Panel*)value();
    if (p == p3) value(p1); else next();
  }
  void prev_panel() {
    Panel* p = (Panel*)value();
    if (p == p1) value(p3); else prev();
  }
private:
  Panel *p1, *p2, *p3;
};

void next_callback(Fl_Widget *widget, void *data) {
  Wizard *pWizard = (Wizard*)data;
  pWizard->next_panel();
}

void prev_callback(Fl_Widget *widget, void *data) {
  Wizard *pWizard = (Wizard*)data;
  pWizard->prev_panel();
}

int main(int argc, char** argv) {
   Fl_Window window(300, 165, "Fl_Wizard test");
   Wizard wizard(5, 5, 290, 100);
   wizard.end();
   Fl_Group buttons(5, 110, 290, 50);
   buttons.box(FL_ENGRAVED_BOX);
   Fl_Button prev_button( 15, 120, 110, 30, "@< Prev Panel");
   prev_button.callback(prev_callback, (void*)&wizard);
   prev_button.align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_LEFT);
   Fl_Button next_button(175, 120, 110, 30, "Next Panel @>");
   next_button.align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_RIGHT);
   next_button.callback(next_callback, (void*)&wizard);
   buttons.end();
   window.end();
   window.show(argc, argv);
   return Fl::run();
}

