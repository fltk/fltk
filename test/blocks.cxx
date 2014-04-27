//
// "$Id$"
//
// "Block Attack!" scrolling blocks game using the Fast Light Tool Kit (FLTK).
//
// Copyright 2006-2010 by Michael Sweet.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_XPM_Image.H>
#include <FL/Fl_XBM_Image.H>
#include <FL/Fl_Tiled_Image.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Audio headers...
#include <config.h>

#ifndef WIN32
#  include <unistd.h>
#  include <sys/time.h>
#endif // !WIN32

#ifdef HAVE_ALSA_ASOUNDLIB_H
#  define ALSA_PCM_NEW_HW_PARAMS_API
#  include <alsa/asoundlib.h>
#endif // HAVE_ALSA_ASOUNDLIB_H
#ifdef __APPLE__
#  include <CoreAudio/AudioHardware.h>
#endif // __APPLE__
#ifdef WIN32
#  include <mmsystem.h>
#endif // WIN32


#define BLOCK_COLS	20
#define BLOCK_ROWS	10
#define BLOCK_SIZE	32
#define BLOCK_BLAST	100

#include "pixmaps/blast.xpm"
Fl_Pixmap blast_pixmap(blast_xpm);

#include "pixmaps/red.xpm"
Fl_Pixmap red_pixmap(red_xpm);
#include "pixmaps/red_bomb.xpm"
Fl_Pixmap red_bomb_pixmap(red_bomb_xpm);

#include "pixmaps/green.xpm"
Fl_Pixmap green_pixmap(green_xpm);
#include "pixmaps/green_bomb.xpm"
Fl_Pixmap green_bomb_pixmap(green_bomb_xpm);

#include "pixmaps/blue.xpm"
Fl_Pixmap blue_pixmap(blue_xpm);
#include "pixmaps/blue_bomb.xpm"
Fl_Pixmap blue_bomb_pixmap(blue_bomb_xpm);

#include "pixmaps/yellow.xpm"
Fl_Pixmap yellow_pixmap(yellow_xpm);
#include "pixmaps/yellow_bomb.xpm"
Fl_Pixmap yellow_bomb_pixmap(yellow_bomb_xpm);

#include "pixmaps/cyan.xpm"
Fl_Pixmap cyan_pixmap(cyan_xpm);
#include "pixmaps/cyan_bomb.xpm"
Fl_Pixmap cyan_bomb_pixmap(cyan_bomb_xpm);

#include "pixmaps/magenta.xpm"
Fl_Pixmap magenta_pixmap(magenta_xpm);
#include "pixmaps/magenta_bomb.xpm"
Fl_Pixmap magenta_bomb_pixmap(magenta_bomb_xpm);

#include "pixmaps/gray.xpm"
Fl_Pixmap gray_pixmap(gray_xpm);
#include "pixmaps/gray_bomb.xpm"
Fl_Pixmap gray_bomb_pixmap(gray_bomb_xpm);

Fl_Pixmap *normal_pixmaps[] = {
  &red_pixmap,
  &green_pixmap,
  &blue_pixmap,
  &yellow_pixmap,
  &cyan_pixmap,
  &magenta_pixmap,
  &gray_pixmap
};
Fl_Pixmap *bomb_pixmaps[] = {
  &red_bomb_pixmap,
  &green_bomb_pixmap,
  &blue_bomb_pixmap,
  &yellow_bomb_pixmap,
  &cyan_bomb_pixmap,
  &magenta_bomb_pixmap,
  &gray_bomb_pixmap
};

const unsigned char screen_bits[] = {
  0xff, 0x55, 0xff, 0xaa, 0xff, 0x55, 0xff, 0xaa
};
Fl_Bitmap screen_bitmap(screen_bits, 8, 8);
Fl_Tiled_Image screen_tile(&screen_bitmap);


// Sound class...
//
// There are MANY ways to implement sound in a FLTK application.
// The approach we are using here is to conditionally compile OS-
// specific code into the application - CoreAudio for MacOS X, the
// standard Win32 API stuff for Windows, ALSA or X11 for Linux, and
// X11 for all others.  We have to support ALSA on Linux because the
// current Xorg releases no longer support XBell() or the PC speaker.
//
// There are several good cross-platform audio libraries we could also
// use, such as OpenAL, PortAudio, and SDL, however they were not chosen
// for this application because of our limited use of sound.
//
// Many thanks to Ian MacArthur who provided sample code that led to
// the CoreAudio implementation you see here!
class BlockSound {
  // Private, OS-specific data...
#ifdef __APPLE__
  AudioDeviceID device;
#ifndef MAC_OS_X_VERSION_10_5
#define MAC_OS_X_VERSION_10_5 1050
#endif
#  if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
  AudioDeviceIOProcID audio_proc_id;
#  endif
  AudioStreamBasicDescription format;
  short *data;
  int remaining;

  static OSStatus audio_cb(AudioDeviceID device,
			   const AudioTimeStamp *current_time,
			   const AudioBufferList *data_in,
			   const AudioTimeStamp *time_in,
			   AudioBufferList *data_out,
			   const AudioTimeStamp *time_out,
			   void *client_data);
#elif defined(WIN32)
  HWAVEOUT	device;
  HGLOBAL	header_handle;
  LPWAVEHDR	header_ptr;
  HGLOBAL	data_handle;
  LPSTR		data_ptr;

#else
#  ifdef HAVE_ALSA_ASOUNDLIB_H
  snd_pcm_t *handle;
#  endif // HAVE_ALSA_ASOUNDLIB_H
#endif // __APPLE__

  public:

  // Common data...
  static short *sample_data;
  static int sample_size;

  BlockSound();
  ~BlockSound();

  void	play_explosion(float duration);
};

// Sound class globals...
short *BlockSound::sample_data = NULL;
int BlockSound::sample_size = 0;


// Initialize the BlockSound class
BlockSound::BlockSound() {
  sample_size = 0;

#ifdef __APPLE__
  remaining = 0;

  UInt32 size = sizeof(device);

  if (AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
			       &size, (void *)&device) != noErr) return;

  size = sizeof(format);
  if (AudioDeviceGetProperty(device, 0, false, kAudioDevicePropertyStreamFormat,
			     &size, &format) != noErr) return;

  // Set up a format we like...
  format.mSampleRate       = 44100.0;	// 44.1kHz
  format.mChannelsPerFrame = 2;		// stereo

  if (AudioDeviceSetProperty(device, NULL, 0, false,
                             kAudioDevicePropertyStreamFormat,
	                     sizeof(format), &format) != noErr) return;

  // Check we got linear pcm - what to do if we did not ???
  if (format.mFormatID != kAudioFormatLinearPCM) return;

  // Attach the callback and start the device
#  if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
  if (AudioDeviceCreateIOProcID(device, audio_cb, (void *)this, &audio_proc_id) != noErr) return;
  AudioDeviceStart(device, audio_proc_id);
#  else
  if (AudioDeviceAddIOProc(device, audio_cb, (void *)this) != noErr) return;
  AudioDeviceStart(device, audio_cb);
#  endif

  sample_size = (int)format.mSampleRate;

#elif defined(WIN32)
  WAVEFORMATEX	format;

  memset(&format, 0, sizeof(format));
  format.cbSize          = sizeof(format);
  format.wFormatTag      = WAVE_FORMAT_PCM;
  format.nChannels       = 2;
  format.nSamplesPerSec  = 44100;
  format.nAvgBytesPerSec = 44100 * 4;
  format.nBlockAlign     = 4;
  format.wBitsPerSample  = 16;

  data_handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, format.nSamplesPerSec * 4);
  if (!data_handle) return;

  data_ptr = (LPSTR)GlobalLock(data_handle);

  header_handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
  if (!header_handle) return;

  header_ptr = (WAVEHDR *)GlobalLock(header_handle);

  header_ptr->lpData  = data_ptr;
  header_ptr->dwFlags = 0;
  header_ptr->dwLoops = 0;

  if (waveOutOpen(&device, WAVE_MAPPER, &format, 0, 0, WAVE_ALLOWSYNC)
          != MMSYSERR_NOERROR) return;

  sample_size = format.nSamplesPerSec;

#else
#  ifdef HAVE_ALSA_ASOUNDLIB_H
  handle = NULL;

  if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) >= 0) {
    // Initialize PCM sound stuff...
    snd_pcm_hw_params_t *params;

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16);
    snd_pcm_hw_params_set_channels(handle, params, 2);
    unsigned rate = 44100;
    int dir;
    snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);
    snd_pcm_uframes_t period = (int)rate;
    snd_pcm_hw_params_set_period_size_near(handle, params, &period, &dir);

    sample_size = rate;

    if (snd_pcm_hw_params(handle, params) < 0) {
      sample_size = 0;
      snd_pcm_close(handle);
      handle = NULL;
    }
  }
#  endif // HAVE_ALSA_ASOUNDLIB_H
#endif // __APPLE__

  if (sample_size) {
    // Make an explosion sound by passing white noise through a low pass
    // filter with a decreasing frequency...
    sample_data = new short[2 * sample_size];

    short *sample_ptr = sample_data;
    int max_sample = 2 * sample_size - 2;

    *sample_ptr++ = 0;
    *sample_ptr++ = 0;

    for (int j = max_sample; j > 0; j --, sample_ptr ++) {
      float freq = (float)j / (float)max_sample;
      float volume = 32767.0 * (0.5 * sqrt(freq) + 0.5);
      float sample = 0.0001 * ((rand() % 20001) - 10000);

      *sample_ptr = (int)(volume * freq * sample +
                          (1.0 - freq) * sample_ptr[-2]);
    }
  }
}


// Cleanup the BlockSound class
BlockSound::~BlockSound() {
#ifdef __APPLE__
  if (sample_size) {
#  if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
    AudioDeviceStop(device, audio_proc_id);
    AudioDeviceDestroyIOProcID(device, audio_proc_id);
#  else
    AudioDeviceStop(device, audio_cb);
    AudioDeviceRemoveIOProc(device, audio_cb);
#  endif
  }

#elif defined(WIN32)
  if (sample_size) {
    waveOutClose(device);

    GlobalUnlock(header_handle);
    GlobalFree(header_handle);

    GlobalUnlock(data_handle);
    GlobalFree(data_handle);
  }

#else
#  ifdef HAVE_ALSA_ASOUNDLIB_H
  if (handle) {
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
  }
#  endif // HAVE_ALSA_ASOUNDLIB_H
#endif // __APPLE__

  if (sample_size) {
    delete[] sample_data;
  }
}


#ifdef __APPLE__
// Callback function for writing audio data...
OSStatus
BlockSound::audio_cb(AudioDeviceID device,
		     const AudioTimeStamp *current_time,
		     const AudioBufferList *data_in,
		     const AudioTimeStamp *time_in,
		     AudioBufferList *data_out,
		     const AudioTimeStamp *time_out,
		     void *client_data) {
  BlockSound *ss = (BlockSound *)client_data;
  int count;
  float *buffer;

  if (!ss->remaining) return noErr;

  for (count = data_out->mBuffers[0].mDataByteSize / sizeof(float),
          buffer = (float*) data_out->mBuffers[0].mData;
       ss->remaining > 0 && count > 0;
       count --, ss->data ++, ss->remaining --) {
    *buffer++ = *(ss->data) / 32767.0;
  }

  while (count > 0) {
    *buffer++ = 0.0;
    count --;
  }

  return noErr;
}
#endif // __APPLE__


// Play a note for the given amount of time...
void
BlockSound::play_explosion(float duration) {
  Fl::check();

  if (duration <= 0.0)
    return;

#if defined(__APPLE__) || defined(WIN32) || defined(HAVE_ALSA_ASOUNDLIB_H)
  if (duration > 1.0)
    duration = 1.0;

  int samples = (int)(duration * sample_size);
  short *sample_ptr = sample_data + 2 * (sample_size - samples);
#endif // __APPLE__ || WIN32 || HAVE_ALSA_ASOUNDLIB_H

#ifdef __APPLE__
  // Point to the next note...
  data      = sample_ptr;
  remaining = samples * 2;

#elif defined(WIN32)
  if (sample_size) {
    memcpy(data_ptr, sample_ptr, samples * 4);

    header_ptr->dwBufferLength = samples * 4;
    waveOutPrepareHeader(device, header_ptr, sizeof(WAVEHDR));

    waveOutWrite(device, header_ptr, sizeof(WAVEHDR));
  } else Beep(440, (int)(1000.0 * duration));

#elif defined(HAVE_ALSA_ASOUNDLIB_H)
  if (handle) {
    // Use ALSA to play the sound...
    if (snd_pcm_writei(handle, sample_ptr, samples) < 0) {
      snd_pcm_prepare(handle);
      snd_pcm_writei(handle, sample_ptr, samples);
    }
    return;
  }
#endif // __APPLE__
}


class BlockWindow : public Fl_Double_Window
{
  public:

  struct Block
  {
    int		color;
    bool	bomb;
    int		y;
  };

  struct Column
  {
    int		num_blocks;
    Block	blocks[BLOCK_ROWS];
    int		x;
  };

  private:

  Fl_Button	*help_button_,
		*play_button_;

  int		num_columns_;
  Column	columns_[BLOCK_COLS];
  int		count_;
  bool		help_;
  int		high_score_;
  float		interval_;
  int		level_;
  int		num_colors_;
  int		opened_columns_;
  bool		paused_;
  static Fl_Preferences	prefs_;
  int		score_;
  BlockSound	*sound_;
  char		title_[255];
  int		title_y_;

  void		_BlockWindow();
  int		bomb(int color);
  int		click(int col, int row);
  static void	help_cb(Fl_Widget *wi, BlockWindow *bw);
  void		init();
  static void	play_cb(Fl_Widget *wi, BlockWindow *bw);
  static void	timeout_cb(BlockWindow *bw);

  public:

  BlockWindow(int X, int Y, int W, int H, const char *L = 0);
  BlockWindow(int W, int H, const char *L = 0);
  ~BlockWindow();

  void		draw();
  int		handle(int event);
  void		new_game();
  int		score() { return (score_); }
  void          up_level();
};


Fl_Preferences	BlockWindow::prefs_(Fl_Preferences::USER, "fltk.org", "blocks");


int
main(int argc, char *argv[]) {
  Fl::scheme("plastic");
  Fl::visible_focus(0);

  BlockWindow	*bw = new BlockWindow(BLOCK_COLS * BLOCK_SIZE,
                                      BLOCK_ROWS * BLOCK_SIZE + 20,
		                      "Block Attack!");

  bw->show(argc, argv);

  return (Fl::run());
}


// Create a block window at the specified position
BlockWindow::BlockWindow(int X, int Y, int W, int H, const char *L)
    : Fl_Double_Window(X, Y, W, H, L) {
  _BlockWindow();
}


// Create a block window
BlockWindow::BlockWindow(int W, int H, const char *L)
    : Fl_Double_Window(W, H, L) {
  _BlockWindow();
}


// Delete a block window
BlockWindow::~BlockWindow() {
  Fl::remove_timeout((Fl_Timeout_Handler)timeout_cb, (void *)this);
}


// Initialize a block window...
void
BlockWindow::_BlockWindow() {
  init();

  help_button_ = new Fl_Button(0, 0, 20, 20, "?");
  help_button_->callback((Fl_Callback *)help_cb, this);
  help_button_->shortcut('?');

  play_button_ = new Fl_Button(80, (h() - 80) / 2, 80, 80, "@>");
  play_button_->callback((Fl_Callback *)play_cb, this);
  play_button_->labelsize(44);
  play_button_->shortcut(' ');

  sound_ = new BlockSound();

  prefs_.get("high_score", high_score_, 0);

  Fl::add_timeout(0.1, (Fl_Timeout_Handler)timeout_cb, (void *)this);
}


// Bomb all blocks of a given color and return the number of affected blocks
int
BlockWindow::bomb(int color) {
  int		j, k;
  int		count;
  Block		*b;
  Column	*c;


  if (color >= BLOCK_BLAST) return (0);

  for (j = num_columns_, c = columns_, count = 1; j > 0; j --, c ++)
    for (k = c->num_blocks, b = c->blocks; k > 0; k --, b ++)
      if (b->color == color) {
        b->color = -color;
	count ++;
      }

  return (count);
}


// Tag all blocks connected to the clicked block and return the number
// of affected blocks
int
BlockWindow::click(int col, int row) {
  Block		*b;
  Column	*c;
  int		count, color;


  c     = columns_ + col;
  b     = c->blocks + row;
  color = b->color;

  if (color < 0 || color >= BLOCK_BLAST) return (0);

  // Find the bottom block...
  while (row > 0 && b[-1].color == color) {
    row --;
    b --;
  }

  count = 0;

  while (row < c->num_blocks && b->color == color) {
    b->color = -color;

    if (col > 0 && row < c[-1].num_blocks &&
        c[-1].blocks[row].color == color) {
      count += click(col - 1, row);
    }

    if (col < (num_columns_ - 1) && row < c[1].num_blocks &&
        c[1].blocks[row].color == color) {
      count += click(col + 1, row);
    }

    count ++;
    row ++;
    b ++;
  }

  return (count);
}


// Draw the block window...
void
BlockWindow::draw() {
  int		j, k, xx, yy;
  Block		*b;
  Column	*c;


  // Draw the blocks...
  fl_color(FL_BLACK);
  fl_rectf(0, 0, w(), h());

  // Draw the blocks...
  for (j = num_columns_, c = columns_; j > 0; j --, c ++)
    for (k = c->num_blocks, b = c->blocks; k > 0; k --, b ++) {
      xx = w() - c->x;
      yy = h() - BLOCK_SIZE - b->y;

      if (b->color >= BLOCK_BLAST) {
	b->color ++;
        blast_pixmap.draw(xx, yy);
      } else if (b->color < 0) {
        if (b->bomb) bomb_pixmaps[-b->color - 1]->draw(xx, yy);
	else normal_pixmaps[-b->color - 1]->draw(xx, yy);
      } else {
        if (b->bomb) bomb_pixmaps[b->color - 1]->draw(xx, yy);
	else normal_pixmaps[b->color - 1]->draw(xx, yy);
      }
    }

  if (interval_ < 0.0 || paused_) {
    fl_color(FL_BLACK);
    screen_tile.draw(0, 0, w(), h(), 0, 0);
  }

  // Redraw the widgets...
  play_button_->redraw();
  help_button_->redraw();
  draw_children();

  // Draw any paused/game over/new game message...
  if ((paused_ || interval_ < 0.0) && play_button_->w() == 80) {
    const char *s;

    if (help_) {
      s = "Click on adjacent blocks of the same color. Clear all blocks "
          "before they reach the left side.";

      fl_font(FL_HELVETICA_BOLD, 24);
      fl_color(FL_BLACK);
      fl_draw(s, 171, 3, w() - 250, h() - 6,
              (Fl_Align)(FL_ALIGN_WRAP | FL_ALIGN_LEFT));

      fl_color(FL_YELLOW);
      fl_draw(s, 168, 0, w() - 250, h(),
              (Fl_Align)(FL_ALIGN_WRAP | FL_ALIGN_LEFT));
    } else {
      if (interval_ < 0.0) {
#ifdef DEBUG
	// Show sample waveform...
	short *sample_ptr;

	for (i = 0; i < 2; i ++)
	{
	  fl_color(FL_RED + i);
	  fl_begin_line();
	  for (j = 0, sample_ptr = sound_->sample_data + i;
               j < sound_->sample_size;
	       j ++, sample_ptr += 2)
	    fl_vertex(j * w() / sound_->sample_size,
	              *sample_ptr * h() / 4 / 65534 + h() / 2);
	  fl_end_line();
	}
#endif // DEBUG

	if (num_columns_ && (time(NULL) & 7) < 4) s = "Game Over";
	else s = "Block Attack!\nby Michael R Sweet";
      } else s = "Paused";

      fl_font(FL_HELVETICA_BOLD, 32);
      fl_color(FL_BLACK);
      fl_draw(s, 6, 6, w() - 6, h() - 6, FL_ALIGN_CENTER);

      fl_color(FL_YELLOW);
      fl_draw(s, 0, 0, w(), h(), FL_ALIGN_CENTER);
    }
  }

  // Draw the scores and level...
  char s[255];

  sprintf(s, " Score: %d", score_);
  fl_color(FL_WHITE);
  fl_font(FL_HELVETICA, 14);
  fl_draw(s, 40, 0, w() - 40, 20, FL_ALIGN_LEFT);

  sprintf(s, "High Score: %d ", high_score_);
  fl_draw(s, 0, 0, w(), 20, FL_ALIGN_RIGHT);

  if (level_ > 1 || title_y_ <= 0)
  {
    sprintf(s, "Level: %d ", level_);
    fl_draw(s, 0, 0, w(), 20, FL_ALIGN_CENTER);
  }

  if (title_y_ > 0 && interval_ > 0.0)
  {
    int sz = 14 + title_y_ * 86 / h();

    fl_font(FL_HELVETICA_BOLD, sz);
    fl_color(FL_YELLOW);
    fl_draw(title_, 0, title_y_, w(), sz, FL_ALIGN_CENTER);
  }
}


// Handle mouse clicks, etc.
int
BlockWindow::handle(int event) {
  int		j, k, mx, my, count;
  Block		*b;
  Column	*c;


  if (Fl_Double_Window::handle(event)) return (1);
  else if (interval_ < 0.0 || paused_) return (0);

  switch (event) {
    case FL_KEYBOARD:
        if (Fl::event_text()) {
          if (strcmp(Fl::event_text(), "+") == 0)
            up_level();
        }
        break;
    case FL_PUSH :
	mx    = w() - Fl::event_x() + BLOCK_SIZE;
	my    = h() - Fl::event_y();
	count = 0;
	b     = 0;

	for (j = 0, c = columns_; !count && j < num_columns_; j ++, c ++)
	  for (k = 0, b = c->blocks; !count && k < c->num_blocks; k ++, b ++)
	    if (mx >= c->x && mx < (c->x + BLOCK_SIZE) &&
	        my >= b->y && my < (b->y + BLOCK_SIZE)) {
	      if (b->bomb) count = bomb(b->color);
	      else count = click(j, k);

              break;
	    }

        if (count < 2) {
	  for (j = 0, c = columns_; j < num_columns_; j ++, c ++)
	    for (k = 0, b = c->blocks; k < c->num_blocks; k ++, b ++)
	      if (b->color < 0) b->color = -b->color;
	} else {
	  count --;

	  if (b->bomb) {
	    sound_->play_explosion(0.19 + 0.005 * count);

	    interval_ *= 0.995;
	    score_ += count;
	  } else {
	    sound_->play_explosion(0.09 + 0.005 * count);

	    interval_ *= 0.999;
	    score_ += count * count;
	  }

	  if (score_ > high_score_) {
	    high_score_ = score_;
	    prefs_.set("high_score", high_score_);
	  }

	  for (j = 0, c = columns_; j < num_columns_; j ++, c ++)
	    for (k = 0, b = c->blocks; k < c->num_blocks; k ++, b ++)
	      if (b->color < 0) b->color = BLOCK_BLAST;
	}
	return (1);
  }

  return (0);
}


// Toggle the on-line help...
void
BlockWindow::help_cb(Fl_Widget *, BlockWindow *bw) {
  bw->paused_ = bw->help_ = !bw->help_;
  bw->play_button_->label("@>");
  bw->redraw();
}


// Initialize the block window...
void
BlockWindow::init() {
  count_       = 0;
  help_        = false;
  interval_    = -1.0;
  level_       = 1;
  num_colors_  = 3;
  num_columns_ = 0;
  paused_      = false;
  score_       = 0;
  title_[0]    = '\0';
  title_y_     = 0;
}


// Start a new game...
void
BlockWindow::new_game() {
  // Seed the random number generator...
  srand(time(NULL));

  init();

  interval_       = 0.1;
  opened_columns_ = 0;

  strcpy(title_, "Level: 1");
  title_y_ = h();

  redraw();
}


// Play/pause...
void
BlockWindow::play_cb(Fl_Widget *wi, BlockWindow *bw) {
  if (bw->interval_ < 0) bw->new_game();
  else bw->paused_ = !bw->paused_;

  if (bw->paused_) wi->label("@>");
  else {
    wi->label("@-2||");
    bw->help_ = false;
  }
}

void BlockWindow::up_level() {
  interval_ *= 0.95;
  opened_columns_ = 0;
  if (num_colors_ < 7) num_colors_ ++;
  level_ ++;
  sprintf(title_, "Level: %d", level_);
  title_y_ = h();
  Fl::repeat_timeout(interval_, (Fl_Timeout_Handler)timeout_cb, (void *)this);
}

// Animate the game...
void
BlockWindow::timeout_cb(BlockWindow *bw) {
  int		i, j;
  Block		*b;
  Column	*c;
  int		lastx, lasty;


#ifdef DEBUG
  struct timeval curtime;
  static struct timeval lasttime;


  gettimeofday(&curtime, NULL);
  printf("%.3f (%+f - %f)\n",
         curtime.tv_sec + 0.000001 * curtime.tv_usec,
         curtime.tv_sec - lasttime.tv_sec +
	     0.000001 * (curtime.tv_usec - lasttime.tv_usec), bw->interval_);
  lasttime = curtime;
#endif // DEBUG

  // Update blocks that have been destroyed...
  for (i = 0, c = bw->columns_; i < bw->num_columns_; i ++, c ++)
    for (j = 0, b = c->blocks; j < c->num_blocks; j ++, b ++)
      if (b->color > (BLOCK_BLAST + 1)) {
        bw->redraw();

	c->num_blocks --;

	if (j < c->num_blocks) {
	  memmove(b, b + 1, (c->num_blocks - j) * sizeof(Block));
	}

	j --;
	b --;

	if (!c->num_blocks) {
	  bw->num_columns_ --;

	  if (i < bw->num_columns_) {
	    memmove(c, c + 1, (bw->num_columns_ - i) * sizeof(Column));
          }

	  i --;
	  c --;
	  j = c->num_blocks;
	}
      }

  // Let the rest of the blocks fall and/or move...
  for (i = bw->num_columns_, c = bw->columns_, lastx = c->x;
       i > 0;
       i --, c ++) {
    if (c->x > lastx) {
      c->x -= 8;
      bw->redraw();
    }

    lastx = c->x + BLOCK_SIZE;

    if (!bw->paused_ && bw->interval_ > 0.0) {
      bw->redraw();
      c->x ++;
    }

    for (j = c->num_blocks, b = c->blocks, lasty = 0; j > 0; j --, b ++) {
      if (b->y > lasty) {
        bw->redraw();
        b->y -= 8;
      }

      lasty = b->y + BLOCK_SIZE;
    }
  }

  // Slide the title text as needed...
  if (bw->title_y_ > 0) {
    bw->redraw();
    bw->title_y_ -= 5;
  }

  // Play the game...
  if (!bw->paused_ && bw->interval_ > 0.0) {
    bw->count_ --;

    if (bw->count_ <= 0) {
      bw->redraw();
      bw->count_ = BLOCK_SIZE;

      if (bw->num_columns_ == BLOCK_COLS) {
	bw->interval_ = -1.0;
	bw->sound_->play_explosion(0.8);
	bw->play_button_->label("@>");
      } else {
	bw->opened_columns_ ++;

	if (bw->opened_columns_ > (2 * BLOCK_COLS)) {
          bw->up_level();
	}

	c = bw->columns_;

	if (bw->num_columns_) {
          memmove(c + 1, c, bw->num_columns_ * sizeof(Column));
	}

	bw->num_columns_ ++;
	c->x          = 0;
	c->num_blocks = BLOCK_ROWS;

	for (j = 0, b = c->blocks; j < BLOCK_ROWS; j ++, b ++) {
          b->bomb  = bw->num_colors_ > 3 && (rand() & 127) < bw->num_colors_;
          b->color = 1 + (rand() % bw->num_colors_);
	  b->y     = j * (BLOCK_SIZE + 8) + 24;
	}
      }
    }
  }
  else
  {
    bw->count_ --;

    if (bw->count_ <= 0) {
      bw->count_ = 40;
      bw->redraw();
    }
  }

  // Update the play/pause button as needed...
  if ((bw->paused_ || bw->interval_< 0.0) &&
      bw->play_button_->w() < 80) {
    int s = bw->play_button_->w() + 10;

    bw->play_button_->resize(s, (s - 20) * (bw->h() - s) / 120, s, s);
    bw->play_button_->labelsize(s / 2 + 4);
    bw->redraw();
  } else if ((!bw->paused_ && bw->interval_ > 0.0) &&
             bw->play_button_->w() > 20) {
    int s = bw->play_button_->w() - 5;

    bw->play_button_->resize(s, (s - 20) * (bw->h() - s) / 120, s, s);
    bw->play_button_->labelsize(s / 2 + 4);
    bw->redraw();
  }

  if (bw->interval_ > 0.0) {
    Fl::repeat_timeout(bw->interval_, (Fl_Timeout_Handler)timeout_cb,
                       (void *)bw);
  } else {
    Fl::repeat_timeout(0.1, (Fl_Timeout_Handler)timeout_cb,
                       (void *)bw);
  }
}


//
// End of "$Id$".
//
