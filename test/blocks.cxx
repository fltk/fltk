//
// "$Id$"
//
// "Block Attack!" scrolling blocks game using the Fast Light Tool Kit (FLTK).
//
// Copyright 2006 by Michael Sweet.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_XPM_Image.H>
#include <FL/fl_draw.H>
#include <FL/x.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

// Audio headers...
#include <config.h>

#ifndef WIN32
#  include <unistd.h>
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

Fl_Pixmap *normal_pixmaps[] =
{
  &red_pixmap,
  &green_pixmap,
  &blue_pixmap,
  &yellow_pixmap,
  &cyan_pixmap,
  &magenta_pixmap,
  &gray_pixmap
};
Fl_Pixmap *bomb_pixmaps[] =
{
  &red_bomb_pixmap,
  &green_bomb_pixmap,
  &blue_bomb_pixmap,
  &yellow_bomb_pixmap,
  &cyan_bomb_pixmap,
  &magenta_bomb_pixmap,
  &gray_bomb_pixmap
};


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

  // Attach the callback
  if (AudioDeviceAddIOProc(device, audio_cb, (void *)this) != noErr) return;

  // Start the device...
  AudioDeviceStart(device, audio_cb);

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

  data_handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, format.nSamplesPerSec / 5);
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
    AudioDeviceStop(device, audio_cb);
    AudioDeviceRemoveIOProc(device, audio_cb);
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
void BlockSound::play_explosion(float duration) {
  Fl::check();

  if (duration <= 0.0)
    return;

  if (duration > 1.0)
    duration = 1.0;

  int samples = (int)(duration * sample_size);
  short *sample_ptr = sample_data + 2 * (sample_size - samples);

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

  int		num_columns_;
  Column	columns_[BLOCK_COLS];
  int		count_;
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

  int		bomb(int color);
  int		click(int col, int row);
  void		init();
  static void	timeout_cb(BlockWindow *bw);

  public:

  BlockWindow(int X, int Y, int W, int H, const char *L = 0);
  BlockWindow(int W, int H, const char *L = 0);
  ~BlockWindow();

  void		draw();
  int		handle(int event);
  void		new_game();
  int		score() { return (score_); }
};


Fl_Preferences	BlockWindow::prefs_(Fl_Preferences::USER, "fltk.org", "blocks");


int
main(int argc, char *argv[])
{
  BlockWindow	*bw = new BlockWindow(BLOCK_COLS * BLOCK_SIZE,
                                      BLOCK_ROWS * BLOCK_SIZE + 20,
		                      "Block Attack!");


  bw->show(argc, argv);

  return (Fl::run());
}


BlockWindow::BlockWindow(int X, int Y, int W, int H, const char *L)
    : Fl_Double_Window(X, Y, W, H, L)
{
  init();

  sound_ = new BlockSound();
     
  prefs_.get("high_score", high_score_, 0);
}


BlockWindow::BlockWindow(int W, int H, const char *L)
    : Fl_Double_Window(W, H, L)
{
  init();

  sound_ = new BlockSound();
      
  prefs_.get("high_score", high_score_, 0);
}


BlockWindow::~BlockWindow()
{
}


int
BlockWindow::bomb(int color)
{
  int		i, j;
  int		count;
  Block		*b;
  Column	*c;


  if (color >= BLOCK_BLAST)
    return (0);

  for (i = num_columns_, c = columns_, count = 1; i > 0; i --, c ++)
    for (j = c->num_blocks, b = c->blocks; j > 0; j --, b ++)
      if (b->color == color)
      {
        b->color = -color;
	count ++;
      }
  
  return (count);
}


int
BlockWindow::click(int col, int row)
{
  Block		*b;
  Column	*c;
  int		count, color;
  
  
  c     = columns_ + col;
  b     = c->blocks + row;
  color = b->color;
  
  if (color < 0 || color >= BLOCK_BLAST)
    return (0);
  
  // Find the bottom block...
  while (row > 0 && b[-1].color == color)
  {
    row --;
    b --;
  }
  
  count = 0;
  
  while (row < c->num_blocks && b->color == color)
  {
    b->color = -color;
    
    if (col > 0 && row < c[-1].num_blocks &&
        c[-1].blocks[row].color == color)
      count += click(col - 1, row);
    
    if (col < (num_columns_ - 1) && row < c[1].num_blocks &&
        c[1].blocks[row].color == color)
      count += click(col + 1, row);
    
    count ++;
    row ++;
    b ++;
  }
  
  return (count);
}


void
BlockWindow::draw()
{
  int		i, j, xx, yy;
  Block		*b;
  Column	*c;


  fl_color(FL_BLACK);
  fl_rectf(0, 0, w(), h());

  for (i = num_columns_, c = columns_; i > 0; i --, c ++)
    for (j = c->num_blocks, b = c->blocks; j > 0; j --, b ++)
    {
      xx = w() - c->x;
      yy = h() - BLOCK_SIZE - b->y;

      if (b->color >= BLOCK_BLAST)
      {
	b->color ++;
        blast_pixmap.draw(xx, yy);
      }
      else if (b->color < 0)
      {
        if (b->bomb)
	  bomb_pixmaps[-b->color - 1]->draw(xx, yy);
	else
	  normal_pixmaps[-b->color - 1]->draw(xx, yy);
      }
      else
      {
        if (b->bomb)
	  bomb_pixmaps[b->color - 1]->draw(xx, yy);
	else
	  normal_pixmaps[b->color - 1]->draw(xx, yy);
      }
    }

  if (paused_ || interval_ < 0.0)
  {
    const char *s;

    if (interval_ < 0.0)
    {
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

      if (num_columns_)
        s = "Game Over\n\nPress N to Start";
      else
        s = "Block Attack!\nby\nMichael R Sweet\n\nPress N to Start";
    }
    else if (paused_)
      s = "Paused\n\nSpace to Continue";

    fl_font(FL_HELVETICA_BOLD, 32);
    fl_color(FL_BLACK);
    fl_draw(s, 6, 6, w() - 6, h() - 6, FL_ALIGN_CENTER);

    fl_color(FL_YELLOW);
    fl_draw(s, 0, 0, w(), h(), FL_ALIGN_CENTER);
  }

  char s[255];
  
  sprintf(s, " Score: %d", score_);
  fl_color(FL_WHITE);
  fl_font(FL_HELVETICA, 14);
  fl_draw(s, 0, 0, w(), 20, FL_ALIGN_LEFT);
  
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


int
BlockWindow::handle(int event)
{
  int		i, j, mx, my, count;
  Block		*b;
  Column	*c;


  switch (event)
  {
    case FL_SHORTCUT :
        if (interval_ < 0.0 && Fl::event_key() == 'n')
	{
	  new_game();
	  return (1);
	}
#ifdef DEBUG
	else if (interval_ > 0.001 && Fl::event_key() == FL_Up)
	  interval_ *= 0.8;
	else if (interval_ > 0.0 && Fl::event_key() == FL_Down)
	  interval_ *= 1.25;
#endif // DEBUG
	else if (interval_ > 0.0 && Fl::event_key() == ' ')
	{
	  paused_ = !paused_;
	  redraw();

	  if (paused_)
	    Fl::remove_timeout((Fl_Timeout_Handler)timeout_cb, (void *)this);
	  else
	    Fl::add_timeout(interval_, (Fl_Timeout_Handler)timeout_cb,
	                    (void *)this);
	}
        break;

    case FL_PUSH :
        mx    = w() - Fl::event_x() + BLOCK_SIZE;
	my    = h() - Fl::event_y();
	count = 0;

	for (i = 0, c = columns_; !count && i < num_columns_; i ++, c ++)
	  for (j = 0, b = c->blocks; !count && j < c->num_blocks; j ++, b ++)
	    if (mx >= c->x && mx < (c->x + BLOCK_SIZE) &&
	        my >= b->y && my < (b->y + BLOCK_SIZE))
	    {
	      if (b->bomb)
	        count = bomb(b->color);
	      else
	        count = click(i, j);

              break;
	    }

        if (count < 2)
	{
	  for (i = 0, c = columns_; i < num_columns_; i ++, c ++)
	    for (j = 0, b = c->blocks; j < c->num_blocks; j ++, b ++)
	      if (b->color < 0)
	        b->color = -b->color;
	}
	else
	{
	  count --;
	  if (b->bomb)
          {
	    sound_->play_explosion(0.19 + 0.005 * count);

	    interval_ *= 0.99;
	    score_ += count;
	  }
	  else
	  {
	    sound_->play_explosion(0.09 + 0.005 * count);

	    interval_ *= 0.999;
	    score_ += count * count;
	  }

	  if (score_ > high_score_)
	  {
	    high_score_ = score_;
	    prefs_.set("high_score", high_score_);
	  }

	  for (i = 0, c = columns_; i < num_columns_; i ++, c ++)
	    for (j = 0, b = c->blocks; j < c->num_blocks; j ++, b ++)
	      if (b->color < 0)
	        b->color = BLOCK_BLAST;
	}
	return (1);
  }

  return (Fl_Double_Window::handle(event));
}


void
BlockWindow::init()
{
  count_       = 0;
  interval_    = -1.0;
  level_       = 1;
  num_colors_  = 3;
  num_columns_ = 0;
  paused_      = false;
  score_       = 0;
  title_[0]    = '\0';
  title_y_     = 0;
}


void
BlockWindow::new_game()
{
  srand(time(NULL));

  init();

  interval_       = 0.08;
  opened_columns_ = 0;

  strcpy(title_, "Level: 1");
  title_y_ = h();

  if (!paused_)
    Fl::add_timeout(interval_, (Fl_Timeout_Handler)timeout_cb, (void *)this);

  redraw();
}


void
BlockWindow::timeout_cb(BlockWindow *bw)
{
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

  if (bw->paused_ || bw->interval_ < 0.0)
    return;

  if (bw->title_y_ > 0)
    bw->title_y_ -= 5;

  for (i = 0, c = bw->columns_; i < bw->num_columns_; i ++, c ++)
    for (j = 0, b = c->blocks; j < c->num_blocks; j ++, b ++)
      if (b->color > (BLOCK_BLAST + 1))
      {
	c->num_blocks --;
	
	if (j < c->num_blocks)
	  memmove(b, b + 1, (c->num_blocks - j) * sizeof(Block));
	
	j --;
	b --;
	
	if (c->num_blocks == 0)
	{
	  bw->num_columns_ --;
	  
	  if (i < bw->num_columns_)
	    memmove(c, c + 1, (bw->num_columns_ - i) * sizeof(Column));

	  i --;
	  c --;
	  j = c->num_blocks;
	}
      }

  for (i = bw->num_columns_, c = bw->columns_, lastx = c->x; i > 0; i --, c ++)
  {
    if (c->x > lastx)
      c->x -= 8;

    lastx = c->x + BLOCK_SIZE;

    c->x ++;

    for (j = c->num_blocks, b = c->blocks, lasty = 0; j > 0; j --, b ++)
    {
      if (b->y > lasty)
        b->y -= 8;

      lasty = b->y + BLOCK_SIZE;
    }
  }

  bw->count_ --;

  if (bw->count_ <= 0)
  {
    bw->count_ = BLOCK_SIZE;

    if (bw->num_columns_ == BLOCK_COLS)
    {
      bw->interval_ = -1.0;
      bw->redraw();
      bw->sound_->play_explosion(0.8);
    }
    else
    {
      bw->opened_columns_ ++;
      if (bw->opened_columns_ > (2 * BLOCK_COLS))
      {
        bw->interval_ *= 0.95;
	bw->opened_columns_ = 0;

        if (bw->num_colors_ < 7)
	  bw->num_colors_ ++;

        bw->level_ ++;
	sprintf(bw->title_, "Level: %d", bw->level_);
	bw->title_y_ = bw->h();
      }

      c = bw->columns_;

      if (bw->num_columns_)
        memmove(c + 1, c, bw->num_columns_ * sizeof(Column));

      bw->num_columns_ ++;
      c->x          = 0;
      c->num_blocks = BLOCK_ROWS;

      for (j = 0, b = c->blocks; j < BLOCK_ROWS; j ++, b ++)
      {
        b->bomb  = bw->num_colors_ > 3 && (rand() & 127) < bw->num_colors_;
        b->color = 1 + (rand() % bw->num_colors_);
	b->y     = j * (BLOCK_SIZE + 8) + 24;
      }
    }
  }

  bw->redraw();

  if (bw->interval_ > 0.0)
    Fl::repeat_timeout(bw->interval_, (Fl_Timeout_Handler)timeout_cb,
                       (void *)bw);
}


//
// End of "$Id$".
//
