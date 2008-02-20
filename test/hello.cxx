#include "FL/Fl.H"
#include "FL/Fl_Double_Window.H"
#include "FL/Fl_Output.H"
#include "FL/fl_ask.H"

void cb(Fl_Widget*, void*)
{
        fl_alert("Called back");
}
int main()
{
        Fl_Double_Window win(350, 300);
        Fl_Output op(200, 100, 150, 20, "Should Not be Called Back");
        op.value("Click Me");
        op.callback(cb);
        op.when(FL_WHEN_NEVER);
        win.end();
        win.show();
        return Fl::run();
}	 
	

