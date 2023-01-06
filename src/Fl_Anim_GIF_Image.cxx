//
// Fl_Anim_GIF_Image class for the Fast Light Tool Kit (FLTK).
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

#include <FL/Fl.H>
#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Graphics_Driver.H>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h> // lround()

#include <FL/Fl_Anim_GIF_Image.H>

/*static*/
bool Fl_GIF_Image::animate = false;


///////////////////////////////////////////////////////////////////////
//  Internal helper classes/structs
///////////////////////////////////////////////////////////////////////

class Fl_Anim_GIF_Image::FrameInfo {
  friend class Fl_Anim_GIF_Image;

  enum Transparency {
    T_NONE = 0xff,
    T_FULL = 0
  };

  struct RGBA_Color {
    uchar r, g, b, alpha;
    RGBA_Color(uchar r = 0, uchar g = 0, uchar b = 0, uchar a = T_NONE) :
      r(r), g(g), b(b), alpha(a) {}
  };

  enum Dispose {
    DISPOSE_UNDEF = 0,
    DISPOSE_NOT = 1,
    DISPOSE_BACKGROUND = 2,
    DISPOSE_PREVIOUS = 3
  };

  struct GifFrame {
    GifFrame() :
      rgb(0),
      scalable(0),
      average_color(FL_BLACK),
      average_weight(-1),
      desaturated(false),
      x(0),
      y(0),
      w(0),
      h(0),
      delay(0),
      dispose(DISPOSE_UNDEF),
      transparent_color_index(-1) {}
    Fl_RGB_Image *rgb;                // full frame image
    Fl_Shared_Image *scalable;        // used for hardware-accelerated scaling
    Fl_Color average_color;           // last average color
    float average_weight;             // last average weight
    bool desaturated;                 // flag if frame is desaturated
    unsigned short x, y, w, h;        // frame original dimensions
    double delay;                     // delay (already converted to ms)
    Dispose dispose;                  // disposal method
    int transparent_color_index;      // needed for dispose()
    RGBA_Color transparent_color;     // needed for dispose()
  };

  FrameInfo(Fl_Anim_GIF_Image *anim) :
    anim(anim),
    valid(false),
    frames_size(0),
    frames(0),
    loop_count(1),
    loop(0),
    background_color_index(-1),
    canvas_w(0),
    canvas_h(0),
    desaturate(false),
    average_color(FL_BLACK),
    average_weight(-1),
    scaling((Fl_RGB_Scaling)0),
    debug_(0),
    optimize_mem(false),
    offscreen(0) {}
  ~FrameInfo();
  void clear();
  void copy(const FrameInfo& fi);
  double convert_delay(int d) const;
  int debug() const { return debug_; }
  bool load(const char *name);
  bool push_back_frame(const GifFrame &frame);
  void resize(int W, int H);
  void scale_frame(int frame);
  void set_frame(int frame);
private:
  Fl_Anim_GIF_Image *anim;          // a pointer to the Image (only needed for name())
  bool valid;                       // flag ig valid data
  int frames_size;                  // number of frames stored in 'frames'
  GifFrame *frames;                 // "vector" for frames
  int loop_count;                   // loop count from file
  int loop;                         // current loop count
  int background_color_index;       // needed for dispose()
  RGBA_Color background_color;      // needed for dispose()
  GifFrame frame;                   // current processed frame
  int canvas_w;                     // width of GIF from header
  int canvas_h;                     // height of GIF from header
  bool desaturate;                  // flag if frames should be desaturated
  Fl_Color average_color;           // color for color_average()
  float average_weight;             // weight for color_average (negative: none)
  Fl_RGB_Scaling scaling;           // saved scaling method for scale_frame()
  int debug_;                       // Flag for debug outputs
  bool optimize_mem;                // Flag to store frames in original dimensions
  uchar *offscreen;                 // internal "offscreen" buffer
private:
private:
  void dispose(int frame_);
  void on_frame_data(Fl_GIF_Image::GIF_FRAME &gf);
  void on_extension_data(Fl_GIF_Image::GIF_FRAME &gf);
  void set_to_background(int frame_);
};


#define LOG(x) if (debug()) printf x
#define DEBUG(x) if (debug() >= 2) printf x
#ifndef LOG
  #define LOG(x)
#endif
#ifndef DEBUG
  #define DEBUG(x)
#endif


//
// helper class FrameInfo implementation
//

Fl_Anim_GIF_Image::FrameInfo::~FrameInfo() {
  clear();
}


void Fl_Anim_GIF_Image::FrameInfo::clear() {
  // release all allocated memory
  while (frames_size-- > 0) {
    if (frames[frames_size].scalable)
      frames[frames_size].scalable->release();
    delete frames[frames_size].rgb;
  }
  delete[] offscreen;
  offscreen = 0;
  free(frames);
  frames = 0;
  frames_size = 0;
}


double Fl_Anim_GIF_Image::FrameInfo::convert_delay(int d) const {
  if (d <= 0)
    d = loop_count != 1 ? 10 : 0;
  return (double)d / 100;
}


void Fl_Anim_GIF_Image::FrameInfo::copy(const FrameInfo& fi) {
  // copy from source
  for (int i = 0; i < fi.frames_size; i++) {
    if (!push_back_frame(fi.frames[i])) {
      break;
    }
    double scale_factor_x = (double)canvas_w / (double)fi.canvas_w;
    double scale_factor_y = (double)canvas_h / (double)fi.canvas_h;
    if (fi.optimize_mem) {
      frames[i].x = lround(fi.frames[i].x * scale_factor_x);
      frames[i].y = lround(fi.frames[i].y * scale_factor_y);
      int new_w = (int)lround(fi.frames[i].w * scale_factor_x);
      int new_h = (int)lround(fi.frames[i].h * scale_factor_y);
      frames[i].w = new_w;
      frames[i].h = new_h;
    }
    // just copy data 1:1 now - scaling will be done adhoc when frame is displayed
    frames[i].rgb = (Fl_RGB_Image *)fi.frames[i].rgb->copy();
    frames[i].scalable = 0;
  }
  optimize_mem = fi.optimize_mem;
  scaling = Fl_Image::RGB_scaling(); // save current scaling mode
  loop_count = fi.loop_count; // .. and the loop_count!
}


void Fl_Anim_GIF_Image::FrameInfo::dispose(int frame) {
  if (frame < 0) {
    return;
  }
  // dispose frame with index 'frame_' to offscreen buffer
  switch (frames[frame].dispose) {
    case DISPOSE_PREVIOUS: {
        // dispose to previous restores to first not DISPOSE_TO_PREVIOUS frame
        int prev(frame);
        while (prev > 0 && frames[prev].dispose == DISPOSE_PREVIOUS)
          prev--;
        if (prev == 0 && frames[prev].dispose == DISPOSE_PREVIOUS) {
          set_to_background(frame);
          return;
        }
        DEBUG(("  dispose frame %d to previous frame %d\n", frame + 1, prev + 1));
        // copy the previous image data..
        uchar *dst = offscreen;
        int px = frames[prev].x;
        int py = frames[prev].y;
        int pw = frames[prev].w;
        int ph = frames[prev].h;
        const char *src = frames[prev].rgb->data()[0];
        if (px == 0 && py == 0 && pw == canvas_w && ph == canvas_h)
          memcpy((char *)dst, (char *)src, canvas_w * canvas_h * 4);
        else {
          if ( px + pw > canvas_w ) pw = canvas_w - px;
          if ( py + ph > canvas_h ) ph = canvas_h - py;
          for (int y = 0; y < ph; y++) {
            memcpy(dst + ( y + py ) * canvas_w * 4 + px, src + y * frames[prev].w * 4, pw * 4);
          }
        }
        break;
      }
    case DISPOSE_BACKGROUND:
      DEBUG(("  dispose frame %d to background\n", frame + 1));
      set_to_background(frame);
      break;

    default: {
        // nothing to do (keep everything as is)
        break;
      }
  }
}


bool Fl_Anim_GIF_Image::FrameInfo::load(const char *name) {
  // decode using FLTK
  valid = false;
  anim->Fl_GIF_Image::load(name, true); // calls on_frame_data() for each frame

  delete[] offscreen;
  offscreen = 0;
  return valid;
}


void Fl_Anim_GIF_Image::FrameInfo::on_extension_data(Fl_GIF_Image::GIF_FRAME &gf) {
  if (!gf.bptr)
     return;
  const uchar *ext = gf.bptr;
  if (memcmp(ext, "NETSCAPE2.0", 11) == 0) {
    const uchar *params = &ext[11];
    loop_count = params[1] | (params[2] << 8);
    DEBUG(("netscape loop count: %u\n", loop_count));
  }
}


void Fl_Anim_GIF_Image::FrameInfo::on_frame_data(Fl_GIF_Image::GIF_FRAME &gf) {
  if (!gf.bptr)
     return;
  int delay = gf.delay;
  if (delay <= 0)
    delay = -(delay + 1);
  LOG(("on_frame_data: frame #%d/%d, %dx%d at %d/%d, delay: %d, bkgd=%d/%d, trans=%d, dispose=%d\n",
        gf.ifrm + 1, -1, gf.w, gf.h, gf.x, gf.y,
        gf.delay, gf.bkgd, gf.clrs, gf.trans, gf.dispose));

  if (!gf.ifrm) {
    // first frame, get width/height
    valid = true; // may be reset later from loading callback
    canvas_w = gf.width;
    canvas_h = gf.height;
    offscreen = new uchar[canvas_w * canvas_h * 4];
    memset(offscreen, 0, canvas_w * canvas_h * 4);
  }

  if (!gf.ifrm) {
    // store background_color AFTER color table is set
    background_color_index = gf.clrs && gf.bkgd < gf.clrs ? gf.bkgd : -1;

    if (background_color_index >= 0) {
        background_color = RGBA_Color(gf.cpal[background_color_index].r,
                                      gf.cpal[background_color_index].g,
                                      gf.cpal[background_color_index].b);
    }
  }

  // process frame
  frame.x = gf.x;
  frame.y = gf.y;
  frame.w = gf.w;
  frame.h = gf.h;
  frame.delay = convert_delay(delay);
  frame.transparent_color_index = gf.trans && gf.trans < gf.clrs ? gf.trans : -1;
  frame.dispose = (Dispose)gf.dispose;
  if (frame.transparent_color_index >= 0) {
    frame.transparent_color = RGBA_Color(gf.cpal[frame.transparent_color_index].r,
                                         gf.cpal[frame.transparent_color_index].g,
                                         gf.cpal[frame.transparent_color_index].b);
  }
  DEBUG(("#%d %d/%d %dx%d delay: %d, dispose: %d transparent_color: %d\n",
    (int)frames_size + 1,
    frame.x, frame.y, frame.w, frame.h,
    gf.delay, gf.dispose, gf.trans));

  // we know now everything we need about the frame..
  dispose(frames_size - 1);

  // copy image data to offscreen
  const uchar *bits = gf.bptr;
  const uchar *endp = offscreen + canvas_w * canvas_h * 4;
  for (int y = frame.y; y < frame.y + frame.h; y++) {
    for (int x = frame.x; x < frame.x + frame.w; x++) {
      uchar c = *bits++;
      if (c == gf.trans)
        continue;
      uchar *buf = offscreen;
      buf += (y * canvas_w * 4 + (x * 4));
      if (buf >= endp)
        continue;
      *buf++ = gf.cpal[c].r;
      *buf++ = gf.cpal[c].g;
      *buf++ = gf.cpal[c].b;
      *buf = T_NONE;
    }
  }

  // create RGB image from offscreen
  if (optimize_mem) {
    uchar *buf = new uchar[frame.w * frame.h * 4];
    uchar *dest = buf;
    for (int y = frame.y; y < frame.y + frame.h; y++) {
      for (int x = frame.x; x < frame.x + frame.w; x++) {
        if (offscreen + y * canvas_w * 4 + x * 4 < endp)
          memcpy(dest, &offscreen[y * canvas_w * 4 + x * 4], 4);
        dest += 4;
      }
    }
    frame.rgb = new Fl_RGB_Image(buf, frame.w, frame.h, 4);
  }
  else {
    uchar *buf = new uchar[canvas_w * canvas_h * 4];
    memcpy(buf, offscreen, canvas_w * canvas_h * 4);
    frame.rgb = new Fl_RGB_Image(buf, canvas_w, canvas_h, 4);
  }
  frame.rgb->alloc_array = 1;

  if (!push_back_frame(frame)) {
    valid = false;
  }
}


bool Fl_Anim_GIF_Image::FrameInfo::push_back_frame(const GifFrame &frame) {
  void *tmp = realloc(frames, sizeof(GifFrame) * (frames_size + 1));
  if (!tmp) {
    return false;
  }
  frames = (GifFrame *)tmp;
  memcpy(&frames[frames_size], &frame, sizeof(GifFrame));
  frames_size++;
  return true;
}


void Fl_Anim_GIF_Image::FrameInfo::resize(int W, int H) {
  double scale_factor_x = (double)W / (double)canvas_w;
  double scale_factor_y = (double)H / (double)canvas_h;
  for (int i=0; i < frames_size; i++) {
    if (optimize_mem) {
      frames[i].x = lround(frames[i].x * scale_factor_x);
      frames[i].y = lround(frames[i].y * scale_factor_y);
      int new_w = (int)lround(frames[i].w * scale_factor_x);
      int new_h = (int)lround(frames[i].h * scale_factor_y);
      frames[i].w = new_w;
      frames[i].h = new_h;
    }
  }
  canvas_w = W;
  canvas_h = H;
}


void Fl_Anim_GIF_Image::FrameInfo::scale_frame(int frame) {
  // Do the actual scaling after a resize if neccessary
  int new_w = optimize_mem ? frames[frame].w : canvas_w;
  int new_h = optimize_mem ? frames[frame].h : canvas_h;
  if (frames[frame].scalable &&
      frames[frame].scalable->w() == new_w &&
      frames[frame].scalable->h() == new_h)
    return;

  Fl_RGB_Scaling old_scaling = Fl_Image::RGB_scaling(); // save current scaling method
  Fl_Image::RGB_scaling(scaling);
  if (!frames[frame].scalable) {
    frames[frame].scalable = Fl_Shared_Image::get(frames[frame].rgb, 0);
  }
  frames[frame].scalable->scale(new_w, new_h, 0, 1);
  Fl_Image::RGB_scaling(old_scaling); // restore scaling method
}


void Fl_Anim_GIF_Image::FrameInfo::set_to_background(int frame) {
  // reset offscreen to background color
  int bg = background_color_index;
  int tp = frame >= 0 ? frames[frame].transparent_color_index : bg;
  DEBUG(("  set_to_background [%d] tp = %d, bg = %d\n", frame, tp, bg));
  RGBA_Color color = background_color;
  if (tp >= 0)
    color = frames[frame].transparent_color;
  if (tp >= 0 && bg >= 0)
    bg = tp;
  color.alpha = tp == bg ? T_FULL : tp < 0 ? T_FULL : T_NONE;
  DEBUG(("  set to color %d/%d/%d alpha=%d\n", color.r, color.g, color.b, color.alpha));
  for (uchar *p = offscreen + canvas_w * canvas_h * 4 - 4; p >= offscreen; p -= 4)
    memcpy(p, &color, 4);
}


void Fl_Anim_GIF_Image::FrameInfo::set_frame(int frame) {
  // scaling pending?
  scale_frame(frame);

  // color average pending?
  if (average_weight >= 0 && average_weight < 1 &&
      ((average_color != frames[frame].average_color) ||
       (average_weight != frames[frame].average_weight))) {
    frames[frame].rgb->color_average(average_color, average_weight);
    frames[frame].average_color = average_color;
    frames[frame].average_weight = average_weight;
  }

  // desaturate pending?
  if (desaturate && !frames[frame].desaturated) {
    frames[frame].rgb->desaturate();
    frames[frame].desaturated = true;
  }
}



///////////////////////////////////////////////////////////////////////
//
// Fl_Anim_GIF_Image
//
// An extension to Fl_GIF_Image.
//
///////////////////////////////////////////////////////////////////////


//
// Fl_Anim_GIF_Image global variables
//

/*static*/
double Fl_Anim_GIF_Image::min_delay = 0.;
/*static*/
bool Fl_Anim_GIF_Image::loop = true;



#include <stdio.h>
#include <stdlib.h>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Group.H>
#include <FL/Fl.H>

//
// class Fl_Anim_GIF_Image implementation
//

Fl_Anim_GIF_Image::Fl_Anim_GIF_Image(const char *name,
                                     Fl_Widget *canvas/* = 0*/,
                                     unsigned short flags/* = 0 */) :
  Fl_GIF_Image(),
  name_(0),
  flags_(flags),
  canvas_(canvas),
  uncache_(false),
  valid_(false),
  frame_(-1),
  speed_(1.),
  fi_(new FrameInfo(this)) {
  fi_->debug_ = ((flags_ & LOG_FLAG) != 0) + 2 * ((flags_ & DEBUG_FLAG) != 0);
  fi_->optimize_mem = (flags_ & OPTIMIZE_MEMORY);
  valid_ = load(name);
  if (canvas_w() && canvas_h()) {
    if (!w() && !h()) {
      w(canvas_w());
      h(canvas_h());
    }
  }
  this->canvas(canvas, flags);
  if (!(flags & DONT_START))
    start();
  else
    frame_ = 0;
}


Fl_Anim_GIF_Image::Fl_Anim_GIF_Image() :
  Fl_GIF_Image(),
  name_(0),
  flags_(0),
  canvas_(0),
  uncache_(false),
  valid_(false),
  frame_(-1),
  speed_(1.),
  fi_(new FrameInfo(this)) {
}


/*virtual*/
Fl_Anim_GIF_Image::~Fl_Anim_GIF_Image() {
  Fl::remove_timeout(cb_animate, this);
  delete fi_;
  free(name_);
}


void Fl_Anim_GIF_Image::canvas(Fl_Widget *canvas, unsigned short flags/* = 0*/) {
  if (canvas_)
    canvas_->image(0);
  canvas_ = canvas;
  if (canvas_ && !(flags & DONT_SET_AS_IMAGE))
    canvas_->image(this); // set animation as image() of canvas
  if (canvas_ && !(flags & DONT_RESIZE_CANVAS))
    canvas_->size(w(), h());
  if (flags_ != flags) {
    flags_ = flags;
    fi_->debug_ = ((flags & LOG_FLAG) != 0) + 2 * ((flags & DEBUG_FLAG) != 0);
  }
  // Note: 'Start' flag is *NOT* used here,
  //       but an already running animation is restarted.
  frame_ = -1;
  if (Fl::has_timeout(cb_animate, this)) {
    Fl::remove_timeout(cb_animate, this);
    next_frame();
  }
  else if ( fi_->frames_size ) {
    set_frame(0);
  }
}


Fl_Widget *Fl_Anim_GIF_Image::canvas() const {
  return canvas_;
}


int Fl_Anim_GIF_Image::canvas_w() const {
  return fi_->canvas_w;
}


int Fl_Anim_GIF_Image::canvas_h() const {
  return fi_->canvas_h;
}


/*static*/
void Fl_Anim_GIF_Image::cb_animate(void *d) {
  Fl_Anim_GIF_Image *b = (Fl_Anim_GIF_Image *)d;
  b->next_frame();
}


void Fl_Anim_GIF_Image::clear_frames() {
  fi_->clear();
  valid_ = false;
}


/*virtual*/
void Fl_Anim_GIF_Image::color_average(Fl_Color c, float i) {
  if (i < 0) {
    // immediate mode
    i = -i;
    for (int f=0; f < frames(); f++) {
      fi_->frames[f].rgb->color_average(c, i);
    }
    return;
  }
  fi_->average_color = c;
  fi_->average_weight = i;
  set_frame();
}


/*virtual*/
Fl_Image *Fl_Anim_GIF_Image::copy(int W, int H) const {
  Fl_Anim_GIF_Image *copied = new Fl_Anim_GIF_Image();
  // copy/resize the base image (Fl_Pixmap)
  // Note: this is not really necessary, if the draw()
  //       method never calls the base class.
  if (fi_->frames_size) {
    Fl_Pixmap *gif = (Fl_Pixmap *)Fl_GIF_Image::copy(W, H);
    copied->Fl_GIF_Image::data(gif->data(), gif->count());
    copied->alloc_data = gif->alloc_data;
    gif->alloc_data = 0;
    delete gif;
  }

  copied->w(W);
  copied->h(H);
  copied->fi_->canvas_w = W;
  copied->fi_->canvas_h = H;
  copied->fi_->copy(*fi_); // copy the meta data

  copied->uncache_ = uncache_; // copy 'inherits' frame uncache status
  copied->valid_ = valid_ && copied->fi_->frames_size == fi_->frames_size;
  copied->scale_frame(); // scale current frame now
  if (copied->valid_ && frame_ >= 0 && !Fl::has_timeout(cb_animate, copied))
    copied->start(); // start if original also was started
  return copied;
}


int Fl_Anim_GIF_Image::debug() const {
  return fi_->debug();
}


double Fl_Anim_GIF_Image::delay(int frame) const {
  if (frame >= 0 && frame < frames())
    return fi_->frames[frame].delay;
  return 0.;
}


void Fl_Anim_GIF_Image::delay(int frame, double delay) {
  if (frame >= 0 && frame < frames())
    fi_->frames[frame].delay = delay;
}


/*virtual*/
void Fl_Anim_GIF_Image::desaturate() {
  fi_->desaturate = true;
  set_frame();
}


/*virtual*/
void Fl_Anim_GIF_Image::draw(int x, int y, int w, int h, int cx/* = 0*/, int cy/* = 0*/) {
  if (this->image()) {
    if (fi_->optimize_mem) {
      int f0 = frame_;
      while (f0 > 0 && !(fi_->frames[f0].x == 0 && fi_->frames[f0].y == 0 &&
                       fi_->frames[f0].w == this->w() && fi_->frames[f0].h == this->h()))
        --f0;
      for (int f = f0; f <= frame_; f++) {
        if (f < frame_ && fi_->frames[f].dispose == FrameInfo::DISPOSE_PREVIOUS) continue;
        if (f < frame_ && fi_->frames[f].dispose == FrameInfo::DISPOSE_BACKGROUND) continue;
        Fl_RGB_Image *rgb = fi_->frames[f].rgb;
        if (rgb) {
          float s = Fl_Graphics_Driver::default_driver().scale();
	       rgb->scale(s*fi_->frames[f].w, s*fi_->frames[f].h, 0, 1);
          rgb->draw(x + s*fi_->frames[f].x, y + s*fi_->frames[f].y, w, h, cx, cy);
        }
      }
    }
    else {
      this->image()->scale(Fl_GIF_Image::w(), Fl_GIF_Image::h(), 0, 1);
      this->image()->draw(x, y, w, h, cx, cy);
    }
  } else {
    // Note: should the base class be called here?
    //       If it is, then the copy() method must also
    //       copy the base image!
    Fl_GIF_Image::draw(x, y, w, h, cx, cy);
  }
}


int Fl_Anim_GIF_Image::frame() const {
  return frame_;
}


void Fl_Anim_GIF_Image::frame(int frame) {
  if (Fl::has_timeout(cb_animate, this)) {
    Fl::warning("Fl_Anim_GIF_Image::frame(%d): not idle!\n", frame);
    return;
  }
  if (frame >= 0 && frame < frames()) {
    set_frame(frame);
  }
  else {
    Fl::warning("Fl_Anim_GIF_Image::frame(%d): out of range!\n", frame);
  }
}


/*static*/
int Fl_Anim_GIF_Image::frame_count(const char *name) {
  Fl_Anim_GIF_Image temp;
  temp.load(name);
  int frames = temp.valid() ? temp.frames() : 0;
  return frames;
}


int Fl_Anim_GIF_Image::frame_x(int frame) const {
  if (frame >= 0 && frame < frames())
    return fi_->frames[frame].x;
  return -1;
}


int Fl_Anim_GIF_Image::frame_y(int frame) const {
  if (frame >= 0 && frame < frames())
    return fi_->frames[frame].y;
  return -1;
}


int Fl_Anim_GIF_Image::frame_w(int frame) const {
  if (frame >= 0 && frame < frames())
    return fi_->frames[frame].w;
  return -1;
}

int Fl_Anim_GIF_Image::frame_h(int frame) const {
  if (frame >= 0 && frame < frames())
    return fi_->frames[frame].h;
  return -1;
}


void Fl_Anim_GIF_Image::frame_uncache(bool uncache) {
  uncache_ = uncache;
}


bool Fl_Anim_GIF_Image::frame_uncache() const {
  return uncache_;
}


int Fl_Anim_GIF_Image::frames() const {
  return fi_->frames_size;
}


Fl_Image *Fl_Anim_GIF_Image::image() const {
  return frame_ >= 0 && frame_ < frames() ? fi_->frames[frame_].rgb : 0;
}


Fl_Image *Fl_Anim_GIF_Image::image(int frame_) const {
  if (frame_ >= 0 && frame_ < frames())
    return fi_->frames[frame_].rgb;
  return 0;
}


bool Fl_Anim_GIF_Image::is_animated() const {
  return valid_ && fi_->frames_size > 1;
}


/*static*/
bool Fl_GIF_Image::is_animated(const char *name) {
  return Fl_Anim_GIF_Image::frame_count(name) > 1;
}


bool Fl_Anim_GIF_Image::load(const char *name) {
  DEBUG(("\nFl_Anim_GIF_Image::load '%s'\n", name));
  clear_frames();
  free(name_);
  name_ = name ? strdup(name) : 0;

  // as load() can be called multiple times
  // we have to replicate the actions of the pixmap destructor here
  uncache();
  if (alloc_data) {
    for (int i = 0; i < count(); i ++) delete[] (char *)data()[i];
    delete[] (char **)data();
  }
  alloc_data = 0;
  w(0);
  h(0);

  if (name_) {
    fi_->load(name);
  }

  frame_ = fi_->frames_size - 1;
  valid_ = fi_->valid;

  if (!valid_) {
    Fl::error("Fl_Anim_GIF_Image: %s has invalid format.\n", name_);
    ld(ERR_FORMAT);
  }
  return valid_;
} // load


const char *Fl_Anim_GIF_Image::name() const {
  return name_;
}


bool Fl_Anim_GIF_Image::next_frame() {
  int frame(frame_);
  frame++;
  if (frame >= fi_->frames_size)  {
    fi_->loop++;
    if (Fl_Anim_GIF_Image::loop && fi_->loop_count > 0 && fi_->loop > fi_->loop_count) {
      DEBUG(("loop count %d reached - stopped!\n", fi_->loop_count));
      stop();
    }
    else
      frame = 0;
  }
  if (frame >= fi_->frames_size)
    return false;
  set_frame(frame);
  double delay = fi_->frames[frame].delay;
  if (min_delay && delay < min_delay) {
    DEBUG(("#%d: correct delay %f => %f\n", frame, delay, min_delay));
    delay = min_delay;
  }
  if (is_animated() && delay > 0 && speed_ > 0) {  // normal GIF has no delay
    delay /= speed_;
    Fl::add_timeout(delay, cb_animate, this);
  }
  return true;
}


/*virtual*/
void Fl_Anim_GIF_Image::on_frame_data(Fl_GIF_Image::GIF_FRAME &gf) {
  fi_->on_frame_data(gf);
}


/*virtual*/
void Fl_Anim_GIF_Image::on_extension_data(Fl_GIF_Image::GIF_FRAME &gf) {
  fi_->on_extension_data(gf);
}


Fl_Anim_GIF_Image& Fl_Anim_GIF_Image::resize(int w, int h) {
  int W(w);
  int H(h);
  if (canvas_ && !W && !H) {
    W = canvas_->w();
    H = canvas_->h();
  }
  if (!W || !H || ((W == this->w() && H == this->h()))) {
    return *this;
  }
  fi_->resize(W, H);
  scale_frame(); // scale current frame now
  this->w(fi_->canvas_w);
  this->h(fi_->canvas_h);
  if (canvas_ && !(flags_ & DONT_RESIZE_CANVAS)) {
    canvas_->size(this->w(), this->h());
  }
  return *this;
}


Fl_Anim_GIF_Image& Fl_Anim_GIF_Image::resize(double scale) {
  return resize((int)lround((double)w() * scale), (int)lround((double)h() * scale));
}


void Fl_Anim_GIF_Image::scale_frame() {
  int i(frame_);
  if (i < 0)
    return;
  fi_->scale_frame(i);
}


void Fl_Anim_GIF_Image::set_frame() {
  int i(frame_);
  if (i < 0)
    return;
  fi_->set_frame(i);
}


void Fl_Anim_GIF_Image::set_frame(int frame) {
  int last_frame = frame_;
  frame_ = frame;
  // NOTE: uncaching decreases performance, but saves a lot of memory
  if (uncache_ && this->image())
    this->image()->uncache();

  fi_->set_frame(frame_);

  if (canvas()) {
    canvas()->parent() &&
      (frame_ == 0 || (last_frame >= 0 && (fi_->frames[last_frame].dispose == FrameInfo::DISPOSE_BACKGROUND  ||
                                           fi_->frames[last_frame].dispose == FrameInfo::DISPOSE_PREVIOUS))) &&
        (canvas()->box() == FL_NO_BOX || (canvas()->align() && !(canvas()->align() & FL_ALIGN_INSIDE)))      ?
      canvas()->parent()->redraw() : canvas()->redraw();
  }
}


double Fl_Anim_GIF_Image::speed() const {
  return speed_;
}


void Fl_Anim_GIF_Image::speed(double speed) {
  speed_ = speed;
}


bool Fl_Anim_GIF_Image::start() {
  Fl::remove_timeout(cb_animate, this);
  if (fi_->frames_size) {
    next_frame();
  }
  return fi_->frames_size != 0;
}


bool Fl_Anim_GIF_Image::stop() {
  Fl::remove_timeout(cb_animate, this);
  return fi_->frames_size != 0;
}


/*virtual*/
void Fl_Anim_GIF_Image::uncache() {
  Fl_GIF_Image::uncache();
  for (int i=0; i < fi_->frames_size; i++) {
    if (fi_->frames[i].rgb) fi_->frames[i].rgb->uncache();
  }
}


bool Fl_Anim_GIF_Image::valid() const {
  return valid_;
}
