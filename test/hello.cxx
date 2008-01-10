#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <iostream>

//typedef Fl_Window Window;
typedef Fl_Double_Window Window;

class Win : public Window {
public:
  Win(int w, int h, char const* label = 0)
    : Window(w, h, label) {;}

  int handle(int event)
  {
    int r = Window::handle(event);
      switch(event) {
        case FL_PUSH: std::cout << "PUSH\n"; break;
        case FL_RELEASE: std::cout << "RELEASE\n"; break;
        case FL_KEYDOWN:
          if(Fl::event_key() == FL_Escape) {
            Fl::release(); hide();
          }
          break;
    }
    return r;
  }
};

int main()
{
  Fl_Double_Window first(300, 300, "First Window");
  first.end();
  Win second(200, 200, "Second Window");
  second.end();
  first.show();
  second.show();
  Fl::grab(&second);
  return Fl::run();
}
