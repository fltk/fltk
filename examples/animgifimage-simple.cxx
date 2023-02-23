//
//  Minimal program for displaying an animated GIF file
//  with the Fl_Anim_GIF_Image class.
//
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Anim_GIF_Image.H>
#include <FL/Fl.H>
#include <stdio.h>

int main(int argc, char *argv[]) {
  Fl_Double_Window win(400, 300, "Fl_Anim_GIF_Image demo");

  // prepare a canvas (widget) for the animation
  Fl_Box canvas(20, 40, win.w()-40, win.h()-80, "Hello from FLTK GIF-animation!");
  canvas.align(FL_ALIGN_TOP|FL_ALIGN_IMAGE_BACKDROP);
  canvas.labelsize(20);

  win.resizable(win);
  win.end();
  win.show(1, argv);

  // Create and load the animated gif as image
  // of the `canvas` widget and start it immediately.
  // We use the `DONT_RESIZE_CANVAS` flag here to tell the
  // animation *not* to change the canvas size (which is the default).
  const char *default_image = "../test/images/fltk_animated.gif";
  Fl_Anim_GIF_Image animgif(/*name_=*/ argv[1] ? argv[1] : default_image,
                          /*canvas_=*/ &canvas,
                           /*flags_=*/ Fl_Anim_GIF_Image::DONT_RESIZE_CANVAS);
  // resize animation to canvas size
  animgif.scale(canvas.w(), canvas.h(), /*can_expand*/1, /*proportional*/1);

  // check if loading succeeded
  printf("%s: ld=%d, valid=%d, frames=%d, size=%dx%d\n",
    animgif.name(), animgif.ld(), animgif.valid(),
    animgif.frames(), animgif.canvas_w(), animgif.canvas_h());
  if (animgif.valid())
    return Fl::run();
}
