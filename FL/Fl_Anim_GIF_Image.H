//
// Fl_Anim_GIF_Image class header for the Fast Light Tool Kit (FLTK).
//
// Copyright 2016-2022 by Christian Grabner <wcout@gmx.net>.
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

#ifndef Fl_Anim_Gif_Image_H
#define Fl_Anim_Gif_Image_H

// forward declarations
class Fl_Image;
class Fl_Widget;

#include <FL/Fl_GIF_Image.H>

/**
 The Fl_Anim_GIF_Image class supports loading, caching,
 and drawing of animated Compuserve GIF<SUP>SM</SUP> images.
 The class loads all images contained in the file and animates
 them by cycling through them as defined by the delay times in
 the image file.

 You must supply an FLTK widget as "container" in order to see
 the animation by specifying it in the constructor or later
 using the canvas() method.
 */
class FL_EXPORT Fl_Anim_GIF_Image : public Fl_GIF_Image {

  class FrameInfo; // internal helper class

public:

  /**
   When opening an Fl_Anim_GIF_Image there are some options
   that can be passed in a 'flags' value.
   */
  enum Flags {
    /**
     This flag indicates to the loader that it should not start
     the animation immediately after successful load, which is
     the default.
     It can be started later using the start() method.
     */
    DONT_START = 1,
    /**
     This flag indicates to the loader that it should not
     resize the canvas widget of the animation to the dimensions
     of the animation, which is the default.
     Needed for special use cases.
     */
    DONT_RESIZE_CANVAS = 2,
    /**
     This flag indicates to the loader that it should not
     set the animation as image() member of the canvas widget,
     which is the default.
     Needed for special use cases.
     */
    DONT_SET_AS_IMAGE = 4,
    /**
     Often frames change just a small area of the animation canvas.
     This flag indicates to the loader to try using less memory
     by storing frame data not as canvas-sized images but use the
     sizes defined in the GIF file.
     The drawbacks are higher cpu usage during playback and maybe
     minor artefacts when resized.
     */
    OPTIMIZE_MEMORY = 8,
    /**
     This flag can be used to print informations about the
     decoding process to the console.
     */
    LOG_FLAG = 64,
    /**
     This flag can be used to print even more informations about
     the decoding process to the console.
     */
    DEBUG_FLAG = 128
  };
  /**
   The constructor creates an new animated gif object from
   the given file.
   Optionally it applies the canvas() method after successful load.
   If 'DONT_START' is not specified in the 'flags' parameter it calls start()
   after successful load.
   */
  Fl_Anim_GIF_Image(const char *name, Fl_Widget *canvas = 0, unsigned short flags = 0);
  Fl_Anim_GIF_Image();
  ~Fl_Anim_GIF_Image() FL_OVERRIDE;
  /**
   The canvas() method sets or gets the current widget, that
   is used to display the frame images.
   The _flags_ parameter specifies whether the canvas widget
   is resized to the animation dimensions and/or its image()
   method will be used to set the current frame image
   during animation.
   */
  void canvas(Fl_Widget *canvas, unsigned short flags = 0);
  Fl_Widget *canvas() const;
  /**
   Return the width and height of the animation canvas as
   specified in the GIF file header
   */
  int canvas_w() const;
  /**
   Return the width and height of the animation canvas as
   specified in the GIF file header
   */
  int canvas_h() const;
  /**
   The color_average() method applies the specified color_average
   to all frames of the animation.
   */
  void color_average(Fl_Color c, float i) FL_OVERRIDE;
  /**
   The virtual copy() method makes a copy of the animated image
   and resizes all of its frame images to W x H using
   the current resize method.
   */
  Fl_Image *copy(int W, int H) const FL_OVERRIDE;
  int debug() const;
  /**
   The desaturate() method applies desaturate() to all frames
   of the animation.
   */
  void desaturate() FL_OVERRIDE;
  void draw(int x, int y, int w, int h, int cx = 0, int cy = 0) FL_OVERRIDE;
  /**
   Return the delay of frame 'frame_' `[0-frames() -1]` in seconds
   */
  double delay(int frame_) const;
  /**
   Set the delay of frame 'frame_' `[0-frames() -1]` in seconds
   */
  void delay(int frame, double delay);
  /**
   Return the number of frames.
   */
  int frames() const;
  /**
   Set the current frame in the range index `[0-frames() -1]`
   */
  void frame(int frame);
  /**
   Return the current frame in the range index `[0-frames() -1]`
   or -1 if the image has no frames.
   */
  int frame() const;
  /**
   Return the current frame image.
   */
  Fl_Image *image() const;
  /**
   Return the frame image of frame 'frame_'
   */
  Fl_Image *image(int frame) const;
  /**
   The is_animated() method is just a convenience method for
   testing the valid flag and the frame count beeing greater 1.
   */
  bool is_animated() const;
  /**
   The static frame_count() method is just a convenience method for
   getting the number of images (frames) stored in a GIF file.

   As this count is not readily available in the GIF header, the
   whole GIF file has be parsed (which is done here by using a
   temporary Fl_Anim_GIF_Image object for simplicity).
   So this call may be slow with large files.
   */
  static int frame_count(const char *name);
  /**
   Use frame_uncache() to set or forbid frame image uncaching.
   If frame uncaching is set, frame images are not offscreen cached
   for re-use and will be re-created every time they are displayed.
   This saves a lot of memory on the expense of cpu usage and
   should be carefully considered. Per default frame caching will
   be done.
   */
  void frame_uncache(bool uncache);
  /**
   Return the active frame_uncache() setting.
   */
  bool frame_uncache() const;
  /**
   The load() method is either used from the constructor to load
   the image from the given file, or to re-load an existing
   animation from another file.
   */
  bool load(const char *name);
  /**
   The loop flag can be used to (dis-)allow loop count.
   If set (which is the default), the animation will be
   stopped after the number of repeats specified in the
   GIF file (typically this count is set to 'forever' anyway).
   If cleared the animation will always be 'forever',
   regardless of what is specified in the GIF file.
   */
  static bool loop;
  /**
   The min_delay value can be used to set a minimum value
   for the frame delay for playback. This is to prevent
   CPU hogs caused by images with very low delay rates.
   This is a global value for all Fl_Anim_GIF_Image objects.
   */
  static double min_delay;
  /**
   Return the name of the played file as specified in the constructor.
   */
  const char *name() const;
  /**
   Return if the animation is currently running or stopped.
   */
  bool playing() const { return valid() && Fl::has_timeout(cb_animate, (void *)this); }
  /**
   The start() method (re-)starts the playing of the frames.
   */
  bool start();
  /**
   The stop() method stops the playing of the frames.
   */
  bool stop();
  /**
   The resize() method resizes the image to the
   specified size, replacing the current image.
   */
  Fl_Anim_GIF_Image& resize(int w, int h);
  Fl_Anim_GIF_Image& resize(double scale);
  /**
   The speed() method changes the playing speed
   to 'speed' x original speed. E.g. to play at half
   speed call it with 0.5, for double speed with 2.
   */
  void speed(double speed);
  double speed() const;
  /**
    Uncache all cached image data now. Re-implemented from Fl_Pixmap.
  */
  void uncache() FL_OVERRIDE;
  /**
   The valid() method returns if the class has
   successfully loaded and the image has at least
   one frame.
   */
  bool valid() const;
  /**
   Return the frame position of frame 'frame'
   Usefull only if loaded with 'optimize_mem' and
   the animation also has size optimized frames.
   */
  int frame_x(int frame) const;
  int frame_y(int frame) const;
  /**
   Return the frame dimensions of frame 'frame'.
   Usefull only if loaded with 'optimize_mem' and
   the animation also has size optimized frames.
   */
  int frame_w(int frame) const;
  int frame_h(int frame) const;

protected:

  bool next_frame();
  void clear_frames();
  void set_frame(int frame);

  static void cb_animate(void *d);
  void scale_frame();
  void set_frame();
  void on_frame_data(Fl_GIF_Image::GIF_FRAME &f) FL_OVERRIDE;
  void on_extension_data(Fl_GIF_Image::GIF_FRAME &f) FL_OVERRIDE;

private:

  char *name_;
  unsigned short flags_;
  Fl_Widget *canvas_;
  bool uncache_;
  bool valid_;
  int frame_; // current frame
  double speed_;
  FrameInfo *fi_;
};

#endif // Fl_Anim_Gif_Image_H
