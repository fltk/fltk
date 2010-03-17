/*
 *  Fl_Gl_Printer.H
 *
 */

#include <FL/Fl_Printer.H>
#include <FL/Fl_Gl_Window.H>

/*
 * @brief To print Fl_Gl_Window's.
 *
 Because Fl_Printer::print_widget() prints only the background of Fl_Gl_Window's,
 this class is to be used to print them.
 */
class Fl_Gl_Printer : public Fl_Printer {
  public:
  /*
   @brief The constructor.
   */
  Fl_Gl_Printer(void) : Fl_Printer() {}

  /*
   @brief Prints an Fl_Gl_Window.
   *
   Under MSWindows, take care to move the print dialog window(s) out of the target OpenGL window(s)
   before closing them.
   @param[in] glw an Fl_Gl_Window to be printed.
   @param[in] delta_x Optional horizontal offset for positioning the window relatively
   to the current origin of graphics functions.
   @param[in] delta_y Same as above, vertically.   
   */
  void print_gl_window(Fl_Gl_Window *glw, int delta_x = 0, int delta_y = 0);
};
