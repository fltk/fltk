//
// implementation of classes Fl_Surface_Device and Fl_Display_Device for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2024 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#include <config.h>
#include <FL/Fl.H>
#include <FL/Fl_Device.H>
#include <FL/Fl_Graphics_Driver.H>

/* Inheritance diagram.

  +- Fl_Surface_Device: any kind of surface that we can draw onto -> uses an Fl_Graphics_Driver
      |
      +- Fl_Display_Device: some kind of video device (one object per app)
      +- Fl_OpenGL_Display_Device: supports adding FLTK child widgets to an Fl_Gl_Window
      +- Fl_Widget_Surface: any FLTK widget can be drawn to it
          |
          +- Fl_Copy_Surface: draw into the clipboard (in vectorial form if the platform supports it)
          +- Fl_Copy_Surface_Driver: helper class interfacing FLTK with draw-to-clipboard operations
              |
              +- Fl_..._Copy_Surface_Driver: platform-specific implementation of Fl_Copy_Surface_Driver
          +- Fl_Image_Surface: draw into an RGB Image
          +- Fl_Image_Surface_Driver: helper class interfacing FLTK with draw-to-image operations
              |
              +- Fl_..._Image_Surface_Driver: platform-specific implementation of Fl_Image_Surface_Driver
          +- Fl_EPS_File_Surface: draw into an Encapsulated PostScript (.eps) file
          +- Fl_SVG_File_Surface: draw into a Scalable Vector Graphics (.svg) file
          +- Fl_Paged_Device: output to a page-structured surface
              |
              +- Fl_Printer: user can instantiate this to gain access to a printer
              +- Fl_WinAPI_Printer_Driver: Windows-specific helper class interfacing FLTK with print operations
              +- Fl_Cocoa_Printer_Driver: macOS-specific helper class interfacing FLTK with print operations
              +- Fl_PostScript_File_Device: draw into a PostScript file
                  |
                  +- Fl_Posix_Printer_Driver: Fl_Printer uses that under Posix platforms
                  +- Fl_GTK_Printer_Driver: Fl_Printer uses that under Posix+GTK platforms
              +- Fl_PDF_File_Surface: draw into a PDF file
                  +- Fl_PDF_GDI_File_Surface: Windows-specific helper class interfacing FLTK with PDF operations
                  +- Fl_PDF_Pango_File_Surface: Linux/Unix-specific helper class interfacing FLTK with PDF operations
                  +- Fl_PDF_Cocoa_File_Surface: macOS-specific helper class interfacing FLTK with PDF operations


  +- Fl_Graphics_Driver -> directed to an Fl_Surface_Device object
      |
      +- Fl_PostScript_Graphics_Driver: platform-independent graphics driver for PostScript/EPS drawing
      +- Fl_SVG_Graphics_Driver: platform-independent graphics driver for Scalable Vector Graphics drawing
      +- Fl_Quartz_Graphics_Driver: platform-specific graphics driver (MacOS)
          +- Fl_Quartz_Printer_Graphics_Driver: MacOS-specific, for drawing to printers
      +- Fl_Scalable_Graphics_Driver: helper class to support GUI scaling
          +- Fl_Xlib_Graphics_Driver: X11-specific graphics driver
          +- Fl_GDI_Graphics_Driver: Windows-specific graphics driver
              +- Fl_GDIplus_Graphics_Driver: overrides oblique lines, arcs and circles
              +- Fl_GDI_Printer_Graphics_Driver: overrides a few member functions especially for output to printer
      +- Fl_Cairo_Graphics_Driver: full FLTK drawing API based on Cairo and Pango
          +- Fl_Wayland_Graphics_Driver: Wayland-specific graphics driver
          +- Fl_X11_Cairo_Graphics_Driver: used by X11 leg of hybrid Wayland/X11 platform
          +- Fl_PostScript_Graphics_Driver: for PostScript drawing with X11+Pango platform
      +- Fl_OpenGL_Graphics_Driver: draw to an Fl_Gl_Window

*/

/** Make this surface the current drawing surface.
  This surface will receive all future graphics requests.

  Since FLTK 1.4.0 the preferred API to change the current drawing surface
  is Fl_Surface_Device::push_current( ) / Fl_Surface_Device::pop_current().

  \note It is recommended to use this function only as follows :
    - The current drawing surface is the display;
    - make current another surface, e.g., an Fl_Printer or an Fl_Image_Surface object,
      calling set_current() on this object;
    - draw to that surface;
    - make the display current again with Fl_Display_Device::display_device()->set_current();\n
      don't do any other call to set_current() before this one.

  Other scenarios of drawing surface changes should be performed via
  Fl_Surface_Device::push_current() and Fl_Surface_Device::pop_current().
*/
void Fl_Surface_Device::set_current(void)
{
  if (surface_) surface_->end_current();
  fl_graphics_driver = pGraphicsDriver;
  surface_ = this;
  pGraphicsDriver->global_gc();
  driver()->set_current_();
}

Fl_Surface_Device* Fl_Surface_Device::surface_; // the current target surface of graphics operations

/** Is this surface the current drawing surface? */
bool Fl_Surface_Device::is_current() {
  return surface_ == this;
}

Fl_Surface_Device::~Fl_Surface_Device()
{
  if (surface_ == this) surface_ = NULL;
}

/**  A constructor that sets the graphics driver used by the display */
Fl_Display_Device::Fl_Display_Device(Fl_Graphics_Driver *graphics_driver) : Fl_Surface_Device(graphics_driver) {
  this->set_current();
}


/** Returns a pointer to the unique display device */
Fl_Display_Device *Fl_Display_Device::display_device() {
  static Fl_Display_Device *display = new Fl_Display_Device(Fl_Graphics_Driver::newMainGraphicsDriver());
  return display;
}


Fl_Surface_Device *Fl_Surface_Device::default_surface()
{
  return Fl_Display_Device::display_device();
}

static unsigned int surface_stack_height = 0;
static Fl_Surface_Device *surface_stack[16];

/** Pushes \p new_current on top of the stack of current drawing surfaces, and makes it current.
 \p new_current will receive all future graphics requests.

 Any call to push_current() must be matched by a subsequent call to Fl_Surface_Device::pop_current().
 The max height of this stack is 16.
 \version 1.4.0
 */
void Fl_Surface_Device::push_current(Fl_Surface_Device *new_current)
{
  if (surface_stack_height < sizeof(surface_stack)/sizeof(void*)) {
    surface_stack[surface_stack_height++] = surface();
  } else {
    fprintf(stderr, "FLTK Fl_Surface_Device::push_current Stack overflow error\n");
  }
  new_current->set_current();
}

/** Removes the top element from the current drawing surface stack, and makes the new top element current.
 \return A pointer to the new current drawing surface.
 \see Fl_Surface_Device::push_current(Fl_Surface_Device *)
 \version 1.4.0 */
Fl_Surface_Device *Fl_Surface_Device::pop_current()
{
  if (surface_stack_height > 0) surface_stack[--surface_stack_height]->set_current();
  return surface_;
}

Fl_Device_Plugin *Fl_Device_Plugin::opengl_plugin() {
  static Fl_Device_Plugin *pi = NULL;
  if (!pi) {
    Fl_Plugin_Manager pm("fltk:device");
    pi = (Fl_Device_Plugin*)pm.plugin("opengl.device.fltk.org");
  }
  return pi;
}

#if !defined(FL_NO_PRINT_SUPPORT)

#include <FL/Fl_PDF_File_Surface.H>

Fl_PDF_File_Surface::Fl_PDF_File_Surface() {
  platform_surface_ = new_platform_pdf_surface_(&out_filename_);
  driver(platform_surface_->driver());
}


Fl_PDF_File_Surface::~Fl_PDF_File_Surface() {
  delete platform_surface_;
}


/** Localizable text of the "PDF document settings" dialog */
const char * Fl_PDF_File_Surface::format_dialog_title = "PDF document settings";
/** Localizable text of the "PDF document settings" dialog */
const char * Fl_PDF_File_Surface::format_dialog_page_size = "Page Size:";
/** Localizable text of the "PDF document settings" dialog */
const char * Fl_PDF_File_Surface::format_dialog_default = "Set as default";
/** Localizable text of the "PDF document settings" dialog */
const char * Fl_PDF_File_Surface::format_dialog_orientation = "Orientation:";

#endif // !defined(FL_NO_PRINT_SUPPORT)
