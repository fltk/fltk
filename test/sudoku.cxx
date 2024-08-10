//
// Sudoku game using the Fast Light Tool Kit (FLTK).
//
// Copyright 2005-2018 by Michael Sweet.
// Copyright 2019-2021 by Bill Spitzak and others.
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
#include <FL/Enumerations.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/platform.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Bitmap.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <FL/math.h>

#include "pixmaps/sudoku.xbm"

// Audio headers...
#include <config.h>

#ifndef _WIN32
#  include <unistd.h>
#endif // !_WIN32
#if defined __APPLE__ && !defined(FLTK_USE_X11)
#  define USE_MACOS 1
#endif

#ifdef HAVE_ALSA_ASOUNDLIB_H
#  define ALSA_PCM_NEW_HW_PARAMS_API
#  include <alsa/asoundlib.h>
#endif // HAVE_ALSA_ASOUNDLIB_H
#ifdef __APPLE__
#  include <CoreAudio/AudioHardware.h>
#endif // __APPLE__
#ifdef _WIN32
#  include <mmsystem.h>
#endif // _WIN32


//
// Default sizes...
//

#define GROUP_SIZE      160
#define CELL_SIZE       50
#define CELL_OFFSET     5
#ifdef USE_MACOS
#  define MENU_OFFSET   0
#else
#  define MENU_OFFSET   25
#endif // USE_MACOS

// Sound class for Sudoku...
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
class SudokuSound {
  // Private, OS-specific data...
#ifdef __APPLE__
  AudioDeviceID device;
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
#elif defined(_WIN32)
  HWAVEOUT      device;
  HGLOBAL       header_handle;
  LPWAVEHDR     header_ptr;
  HGLOBAL       data_handle;
  LPSTR         data_ptr;

#else
#  ifdef HAVE_ALSA_ASOUNDLIB_H
  snd_pcm_t *handle;
#  endif // HAVE_ALSA_ASOUNDLIB_H
#endif // __APPLE__

  // Common data...
  static int frequencies[9];
  static short *sample_data[9];
  static int sample_size;

  public:

  SudokuSound();
  ~SudokuSound();

  void  play(char note);
};

typedef unsigned int State;
typedef State GameState[81];

// Sudoku cell class...
class SudokuCell : public Fl_Widget {
  int           row_;
  int           col_;
  bool          readonly_;
  int           value_;
  int           hint_map_;

  public:

                SudokuCell(int row, int col, int X, int Y, int W, int H);
  int           row() { return row_; }
  int           col() { return col_; }
  void          draw() FL_OVERRIDE;
  int           handle(int event) FL_OVERRIDE;
  void          readonly(bool r) { readonly_ = r; redraw(); }
  bool          readonly() const { return readonly_; }
  void          set_hint(int n) { hint_map_ |= (1<<n); redraw(); }
  void          clear_hint(int n) { hint_map_ &= ~(1<<n); redraw(); }
  void          clear_hints() { hint_map_ = 0; redraw(); }
  void          set_hint_map(int v) { hint_map_ = v; }
  int           get_hint_map() { return hint_map_; }
  bool          hint_set(int n) const { return ((hint_map_&(1<<n))!=0); }
  void          value(int v) { value_ = v; redraw(); }
  int           value() const { return value_; }
  State         state() { return (hint_map_>>1) | (value_<<12) | (readonly_<<9); }
  void          state(State s) {
    hint_map_   = (s & 0x000001ff) << 1;
    readonly_   = (s & 0x00000200) >> 9;
    value_      = (s & 0x0000f000) >> 12;
    if (readonly_) color(FL_GRAY); else color(FL_LIGHT3);
    redraw();
  }
};


// Sudoku window class...
class Sudoku : public Fl_Double_Window {
  Fl_Sys_Menu_Bar *menubar_;
  Fl_Group      *grid_;
  time_t        seed_;
  char          grid_values_[9][9];
  SudokuCell    *grid_cells_[9][9];
  Fl_Group      *grid_groups_[3][3];
  int           difficulty_;
  SudokuSound   *sound_;
  GameState     undo_stack[64];
  int           undo_head_, undo_tail_, redo_head_;

  static void   check_cb(Fl_Widget *widget, void *);
  static void   close_cb(Fl_Widget *widget, void *);
  static void   diff_cb(Fl_Widget *widget, void *d);
  static void   update_helpers_cb(Fl_Widget *, void *);
  static void   help_cb(Fl_Widget *, void *);
  static void   mute_cb(Fl_Widget *widget, void *);
  static void   new_cb(Fl_Widget *widget, void *);
  static void   reset_cb(Fl_Widget *widget, void *);
  static void   restart_cb(Fl_Widget *widget, void *);
  void          set_title();
  static void   solve_cb(Fl_Widget *widget, void *);

  static Fl_Help_Dialog *help_dialog_;
  static Fl_Preferences prefs_;
  public:

                Sudoku();
                ~Sudoku();

  void          check_game(bool highlight = true);
  void          load_game();
  void          new_game(time_t seed);
  int           next_value(SudokuCell *c);
  void          resize(int X, int Y, int W, int H) FL_OVERRIDE;
  void          save_game();
  void          solve_game();
  void          update_helpers();
  void          clear_hints_for(int row, int col, int val);
  void          save_state(GameState &s);
  void          load_state(GameState &s);
  void          undo();
  void          redo();
  void          clear_undo();
  void          undo_checkpoint();
};

Sudoku *sudoku = NULL;

// Sound class globals...
int SudokuSound::frequencies[9] = {
  880,  // A(5)
  988,  // B(5)
  1046, // C(5)
  1174, // D(5)
  1318, // E(5)
  1396, // F(5)
  1568, // G(5)
  1760, // H (A6)
  1976  // I (B6)
};
short *SudokuSound::sample_data[9] = { 0 };
int SudokuSound::sample_size = 0;

// Initialize the SudokuSound class
SudokuSound::SudokuSound() {
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
  format.mSampleRate       = 44100.0;   // 44.1kHz
  format.mChannelsPerFrame = 2;         // stereo

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

  sample_size = (int)format.mSampleRate / 20;

#elif defined(_WIN32)
  WAVEFORMATEX  format;

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

  header_ptr->lpData         = data_ptr;
  header_ptr->dwBufferLength = format.nSamplesPerSec / 5;
  header_ptr->dwFlags        = 0;
  header_ptr->dwLoops        = 0;

  if (waveOutOpen(&device, WAVE_MAPPER, &format, 0, 0, WAVE_ALLOWSYNC)
          != MMSYSERR_NOERROR) return;

  waveOutPrepareHeader(device, header_ptr, sizeof(WAVEHDR));

  sample_size = 44100 / 20;

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
    snd_pcm_uframes_t period = (int)rate / 4;
    snd_pcm_hw_params_set_period_size_near(handle, params, &period, &dir);

    sample_size = rate / 20;

    if (snd_pcm_hw_params(handle, params) < 0) {
      sample_size = 0;
      snd_pcm_close(handle);
      handle = NULL;
    }
  }
#  endif // HAVE_ALSA_ASOUNDLIB_H
#endif // __APPLE__

  if (sample_size) {
    // Make each of the notes using a combination of sine and sawtooth waves
    int attack = sample_size / 10;
    int decay = 4 * sample_size / 5;

    for (int i = 0; i < 9; i ++) {
      sample_data[i] = new short[2 * sample_size];

      short *sample_ptr = sample_data[i];

      for (int j = 0; j < sample_size; j ++, sample_ptr += 2) {
        double theta = 0.05 * frequencies[i] * j / sample_size;
        double val = 0.5 * sin(2.0 * M_PI * theta) + theta - (int)theta - 0.5;

        if (j < attack) {
          *sample_ptr = (int)(32767 * val * j / attack);
        } else if (j > decay) {
          *sample_ptr = (int)(32767 * val * (sample_size - j + decay) /
                              sample_size);
        } else *sample_ptr = (int)(32767 * val);

        sample_ptr[1] = *sample_ptr;
      }
    }
  }
}


// Cleanup the SudokuSound class
SudokuSound::~SudokuSound() {
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

#elif defined(_WIN32)
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
    for (int i = 0; i < 9; i ++) {
      delete[] sample_data[i];
    }
  }
}


#ifdef __APPLE__
// Callback function for writing audio data...
OSStatus
SudokuSound::audio_cb(AudioDeviceID device,
                      const AudioTimeStamp *current_time,
                      const AudioBufferList *data_in,
                      const AudioTimeStamp *time_in,
                      AudioBufferList *data_out,
                      const AudioTimeStamp *time_out,
                      void *client_data) {
  SudokuSound *ss = (SudokuSound *)client_data;
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

#define NOTE_DURATION 50

// Play a note for <NOTE_DURATION> ms...
void SudokuSound::play(char note) {
  Fl::check();

#ifdef __APPLE__
  // Point to the next note...
  data      = sample_data[note - 'A'];
  remaining = sample_size * 2;

  // Wait for the sound to complete...
  usleep(NOTE_DURATION*1000);

#elif defined(_WIN32)
  if (sample_size) {
    memcpy(data_ptr, sample_data[note - 'A'], sample_size * 4);

    waveOutWrite(device, header_ptr, sizeof(WAVEHDR));

    Sleep(NOTE_DURATION);
  } else Beep(frequencies[note - 'A'], NOTE_DURATION);

#elif defined(FLTK_USE_X11)
#  ifdef HAVE_ALSA_ASOUNDLIB_H
  if (handle) {
    // Use ALSA to play the sound...
    if (snd_pcm_writei(handle, sample_data[note - 'A'], sample_size) < 0) {
      snd_pcm_prepare(handle);
      snd_pcm_writei(handle, sample_data[note - 'A'], sample_size);
    }
    usleep(NOTE_DURATION*1000);
    return;
  }
#  endif // HAVE_ALSA_ASOUNDLIB_H

  // Just use standard X11 stuff...
  XKeyboardState        state;
  XKeyboardControl      control;

  // Get original pitch and duration...
  XGetKeyboardControl(fl_display, &state);

  // Sound a tone for the given note...
  control.bell_percent  = 100;
  control.bell_pitch    = frequencies[note - 'A'];
  control.bell_duration = NOTE_DURATION;

  XChangeKeyboardControl(fl_display,
                         KBBellPercent | KBBellPitch | KBBellDuration,
                         &control);
  XBell(fl_display, 100);
  XFlush(fl_display);

  // Restore original pitch and duration...
  control.bell_percent  = state.bell_percent;
  control.bell_pitch    = state.bell_pitch;
  control.bell_duration = state.bell_duration;

  XChangeKeyboardControl(fl_display,
                         KBBellPercent | KBBellPitch | KBBellDuration,
                         &control);
#endif // __APPLE__
}


// Create a cell widget
SudokuCell::SudokuCell(int row, int col, int X, int Y, int W, int H)
  : Fl_Widget(X, Y, W, H, 0), row_(row), col_(col)
{
  value(0);
}


// Draw cell
void
SudokuCell::draw() {
  static Fl_Align align[9] = {
    FL_ALIGN_TOP_LEFT,
    FL_ALIGN_TOP,
    FL_ALIGN_TOP_RIGHT,
    FL_ALIGN_LEFT,
    0,
    FL_ALIGN_RIGHT,
    FL_ALIGN_BOTTOM_LEFT,
    FL_ALIGN_BOTTOM,
    FL_ALIGN_BOTTOM_RIGHT,
  };


  // Draw the cell box...
  if (readonly()) fl_draw_box(FL_UP_BOX, x(), y(), w(), h(), color());
  else fl_draw_box(FL_DOWN_BOX, x(), y(), w(), h(), color());

  // Draw the cell background...
  if (Fl::focus() == this) {
    Fl_Color c = fl_color_average(FL_SELECTION_COLOR, color(), 0.5f);
    fl_color(c);
    fl_rectf(x() + 4, y() + 4, w() - 8, h() - 8);
    fl_color(fl_contrast(labelcolor(), c));
  } else fl_color(labelcolor());

  // Draw the cell value...
  char s[2];

  s[1] = '\0';

  if (value_) {
    s[0] = value_ + '0';
    fl_font(FL_HELVETICA_BOLD, h() - 10);
    fl_draw(s, x(), y(), w(), h(), FL_ALIGN_CENTER);
  } else {
    fl_font(FL_HELVETICA_BOLD, h() / 5);
    for (int i = 1; i <= 9; i ++) {
      if (hint_set(i)) {
        s[0] = i + '0';
        fl_draw(s, x() + 5, y() + 5, w() - 10, h() - 10, align[i-1]);
      }
    }
  }
}


// Handle events in cell
int
SudokuCell::handle(int event) {
  switch (event) {
    case FL_FOCUS :
      Fl::focus(this);
      redraw();
      return 1;

    case FL_UNFOCUS :
      redraw();
      return 1;

    case FL_PUSH :
      if (!readonly() && Fl::event_inside(this)) {
        if (Fl::event_clicks()) {
          // 2+ clicks increments/sets value
          if (value()) {
            if (value() < 9) value(value() + 1);
            else value(1);
          } else value(sudoku->next_value(this));
        }
        // TODO: add this to the undo process
        Fl::focus(this);
        redraw();
        return 1;
      }
      break;

    case FL_KEYDOWN :
      if (Fl::event_state() & FL_CTRL) break;
      int key = Fl::event_key() - '0';
      if (key < 0 || key > 9) key = Fl::event_key() - FL_KP - '0';
      if (key > 0 && key <= 9) {
        if (readonly()) {
          fl_beep(FL_BEEP_ERROR);
          return 1;
        }

        if (Fl::event_state() & (FL_SHIFT | FL_CAPS_LOCK)) {
          if (hint_set(key)) {
            clear_hint(key);
          } else {
            set_hint(key);
          }
          sudoku->undo_checkpoint();
          redraw();
        } else {
          value(key);
          sudoku->clear_hints_for(row(), col(), key);
          sudoku->undo_checkpoint();
          do_callback();
        }
        return 1;
      } else if (key == 0 || Fl::event_key() == FL_BackSpace ||
                 Fl::event_key() == FL_Delete) {
        if (readonly()) {
          fl_beep(FL_BEEP_ERROR);
          return 1;
        }
        if (Fl::event_state() & (FL_SHIFT | FL_CAPS_LOCK)) {
          clear_hints();
          sudoku->undo_checkpoint();
        } else {
          value(0);
          do_callback();
          sudoku->undo_checkpoint();
        }
        return 1;
      }
      break;
  }

  return Fl_Widget::handle(event);
}


// Sudoku class globals...
Fl_Help_Dialog  *Sudoku::help_dialog_ = (Fl_Help_Dialog *)0;
Fl_Preferences  Sudoku::prefs_(Fl_Preferences::USER_L, "fltk.org", "sudoku");

static void undo_cb(Fl_Widget*, void*) {
  sudoku->undo();
}

static void redo_cb(Fl_Widget*, void*) {
  sudoku->redo();
}

// Create a Sudoku game window...
Sudoku::Sudoku()
  : Fl_Double_Window(GROUP_SIZE * 3, GROUP_SIZE * 3 + MENU_OFFSET, "Sudoku"),
  undo_head_(0), undo_tail_(0), redo_head_(0)
{
  int j, k;
  Fl_Group *g;
  SudokuCell *cell;
  static Fl_Menu_Item   items[] = {
    { "&Game", 0, 0, 0, FL_SUBMENU },
    {   "&New Game", FL_COMMAND | 'n', new_cb, 0, FL_MENU_DIVIDER },
    {   "&Check Game", FL_COMMAND | 'c', check_cb, 0, 0 },
    {   "&Restart Game", FL_COMMAND | 'r', restart_cb, 0, 0 },
    {   "&Solve Game", FL_COMMAND | 's', solve_cb, 0, FL_MENU_DIVIDER },
    {   "&Update Helpers", 0, update_helpers_cb, 0, 0 },
    {   "&Mute Sound", FL_COMMAND | 'm', mute_cb, 0, FL_MENU_TOGGLE | FL_MENU_DIVIDER },
#ifndef USE_MACOS
    {   "&Quit", FL_COMMAND | 'q', close_cb, 0, 0 },
#endif
    {   0 },
    { "&Edit", 0, 0, 0, FL_SUBMENU },
    {   "&Undo", FL_COMMAND | 'z', undo_cb },
    {   "&Redo", FL_COMMAND | 'Z', redo_cb },
    {   0 },
    { "&Difficulty", 0, 0, 0, FL_SUBMENU },
    {   "&Easy", 0, diff_cb, (void *)"0", FL_MENU_RADIO },
    {   "&Medium", 0, diff_cb, (void *)"1", FL_MENU_RADIO },
    {   "&Hard", 0, diff_cb, (void *)"2", FL_MENU_RADIO },
    {   "&Impossible", 0, diff_cb, (void *)"3", FL_MENU_RADIO },
    {   0 },
#ifndef USE_MACOS
    { "&Help", 0, 0, 0, FL_SUBMENU },
    {   "&About Sudoku", FL_F + 1, help_cb, 0, 0 },
    {   0 },
#endif
    { 0 }
  };


  // Setup sound output...
  prefs_.get("mute_sound", j, 0);
  if (j) {
    // Mute sound?
    sound_ = NULL;
    items[6].flags |= FL_MENU_VALUE;
  } else sound_ = new SudokuSound();

  // Menubar...
  prefs_.get("difficulty", difficulty_, 0);
  if (difficulty_ < 0 || difficulty_ > 3) difficulty_ = 0;

  items[10 + difficulty_].flags |= FL_MENU_VALUE;

  menubar_ = new Fl_Sys_Menu_Bar(0, 0, 3 * GROUP_SIZE, 25);
  menubar_->menu(items);
#ifdef USE_MACOS
  menubar_->about(help_cb, NULL);
#endif

  // Create the grids...
  grid_ = new Fl_Group(0, MENU_OFFSET, 3 * GROUP_SIZE, 3 * GROUP_SIZE);

  for (j = 0; j < 3; j ++)
    for (k = 0; k < 3; k ++) {
      g = new Fl_Group(k * GROUP_SIZE, j * GROUP_SIZE + MENU_OFFSET,
                       GROUP_SIZE, GROUP_SIZE);
      g->box(FL_BORDER_BOX);
      if ((int)(j == 1) ^ (int)(k == 1)) g->color(FL_DARK3);
      else g->color(FL_DARK2);
      g->end();

      grid_groups_[j][k] = g;
    }

  for (j = 0; j < 9; j ++)
    for (k = 0; k < 9; k ++) {
      cell = new SudokuCell(j, k,
                            k * CELL_SIZE + CELL_OFFSET +
                                (k / 3) * (GROUP_SIZE - 3 * CELL_SIZE),
                            j * CELL_SIZE + CELL_OFFSET + MENU_OFFSET +
                                (j / 3) * (GROUP_SIZE - 3 * CELL_SIZE),
                            CELL_SIZE, CELL_SIZE);
      cell->callback(reset_cb);
      grid_cells_[j][k] = cell;
    }

  // Set icon for window
  Fl_Bitmap bm(sudoku_bits, sudoku_width, sudoku_height);
  Fl_Image_Surface surf(sudoku_width, sudoku_height, 1);
  Fl_Surface_Device::push_current(&surf);
  fl_color(FL_WHITE);
  fl_rectf(0, 0, sudoku_width, sudoku_height);
  fl_color(FL_BLACK);
  bm.draw(0, 0);
  Fl_Surface_Device::pop_current();
  icon(surf.image());

  // Catch window close events...
  callback(close_cb);

  // Make the window resizable...
  resizable(grid_);
  size_range(3 * GROUP_SIZE, 3 * GROUP_SIZE + MENU_OFFSET, 0, 0, 5, 5, 1);

  // Restore the previous window dimensions...
  int X, Y, W, H;

  if (prefs_.get("x", X, -1)) {
    prefs_.get("y", Y, -1);
    prefs_.get("width", W, 3 * GROUP_SIZE);
    prefs_.get("height", H, 3 * GROUP_SIZE + MENU_OFFSET);

    resize(X, Y, W, H);
  }

  set_title();
  clear_undo();
}


// Destroy the sudoku window...
Sudoku::~Sudoku() {
  if (sound_) delete sound_;
}


// Check for a solution to the game...
void
Sudoku::check_cb(Fl_Widget *widget, void *) {
  sudoku->check_game();
}


// Check if the user has correctly solved the game...
void
Sudoku::check_game(bool highlight) {
  bool empty = false;
  bool correct = true;
  int j, k, m;

  // Check the game for right/wrong answers...
  for (j = 0; j < 9; j ++)
    for (k = 0; k < 9; k ++) {
      SudokuCell *cell = grid_cells_[j][k];
      int val = cell->value();

      if (cell->readonly()) continue;

      if (!val) empty = true;
      else {
        for (m = 0; m < 9; m ++)
          if ((j != m && grid_cells_[m][k]->value() == val) ||
              (k != m && grid_cells_[j][m]->value() == val)) break;

        if (m < 9) {
          if (highlight) {
            cell->color(FL_YELLOW);
            cell->redraw();
          }

          correct = false;
        } else if (highlight) {
          cell->color(FL_LIGHT3);
          cell->redraw();
        }
      }
    }

  // Check subgrids for duplicate numbers...
  for (j = 0; j < 9; j += 3)
    for (k = 0; k < 9; k += 3)
      for (int jj = 0; jj < 3; jj ++)
        for (int kk = 0; kk < 3; kk ++) {
          SudokuCell *cell = grid_cells_[j + jj][k + kk];
          int val = cell->value();

          if (cell->readonly() || !val) continue;

          int jjj;

          for (jjj = 0; jjj < 3; jjj ++) {
            int kkk;

            for (kkk = 0; kkk < 3; kkk ++)
              if (jj != jjj && kk != kkk &&
                  grid_cells_[j + jjj][k + kkk]->value() == val) break;

            if (kkk < 3) break;
          }

          if (jjj < 3) {
            if (highlight) {
              cell->color(FL_YELLOW);
              cell->redraw();
            }

            correct = false;
          }
        }

  if (!empty && correct) {
    // Success!
    for (j = 0; j < 9; j ++) {
      for (k = 0; k < 9; k ++) {
        SudokuCell *cell = grid_cells_[j][k];
        cell->color(FL_GREEN);
        cell->readonly(1);
      }

      if (sound_) sound_->play('A' + grid_cells_[j][8]->value() - 1);
    }
  }
}


// Close the window, saving the game first...
void
Sudoku::close_cb(Fl_Widget *widget, void *) {
  sudoku->save_game();
  sudoku->hide();

  if (help_dialog_) help_dialog_->hide();
}


// Set the level of difficulty...
void
Sudoku::diff_cb(Fl_Widget *widget, void *d) {
  int diff = atoi((char *)d);

  if (diff != sudoku->difficulty_) {
    sudoku->difficulty_ = diff;
    sudoku->new_cb(widget, NULL);
    sudoku->set_title();

    if (diff > 1)
    {
      // Display a message about the higher difficulty levels for the
      // Sudoku zealots of the world...
      int val;

      prefs_.get("difficulty_warning", val, 0);

      if (!val)
      {
        prefs_.set("difficulty_warning", 1);
        fl_alert("Note: 'Hard' and 'Impossible' puzzles may have more than "
                 "one possible solution.\n"
                 "This is not an error or bug.");
      }
    }

    prefs_.set("difficulty", sudoku->difficulty_);
  }
}

// Update the little marker numbers in all cells
void
Sudoku::update_helpers_cb(Fl_Widget *widget, void *) {
  sudoku->update_helpers();
}

void
Sudoku::update_helpers() {
  int j, k, m;

  // First we delete any entries that the user may have made
  for (j = 0; j < 9; j ++) {
    for (k = 0; k < 9; k ++) {
      SudokuCell *cell = grid_cells_[j][k];
      cell->clear_hints();
    }
  }

  // Now go through all cells and find out, what we can not be
  for (j = 0; j < 81; j ++) {
    char taken[10] = { 0 };
    // Find our destination cell
    int row = j / 9;
    int col = j % 9;
    SudokuCell *dst_cell = grid_cells_[row][col];
    if (dst_cell->value()) continue;
    // Find all values already taken in this row
    for (k = 0; k < 9; k ++) {
      SudokuCell *cell = grid_cells_[row][k];
      int v = cell->value();
      if (v) taken[v] = 1;
    }
    // Find all values already taken in this column
    for (k = 0; k < 9; k ++) {
      SudokuCell *cell = grid_cells_[k][col];
      int v = cell->value();
      if (v) taken[v] = 1;
    }
    // Now find all values already taken in this square
    int ro = (row / 3) * 3;
    int co = (col / 3) * 3;
    for (k = 0; k < 3; k ++) {
      for (m = 0; m < 3; m ++) {
        SudokuCell *cell = grid_cells_[ro + k][co + m];
        int v = cell->value();
        if (v) taken[v] = 1;
      }
    }
    // transfer our findings to the markers
    for (m = 1; m <= 9; m ++) {
      if (!taken[m])
        dst_cell->set_hint(m);
    }
  }
  undo_checkpoint();
}

void Sudoku::clear_hints_for(int row, int col, int val) {
  int i, j;
  // clear row
  for (i = 0; i < 9; ++i)
    grid_cells_[row][i]->clear_hint(val);
  // clear column
  for (i = 0; i < 9; ++i)
    grid_cells_[i][col]->clear_hint(val);
  // clear block
  row = (row / 3) * 3;
  col = (col / 3) * 3;
  for (i = 0; i < 3; ++i)
    for (j = 0; j < 3; ++j)
      grid_cells_[row+i][col+j]->clear_hint(val);
}

// Show the on-line help...
void
Sudoku::help_cb(Fl_Widget *, void *) {
  if (!help_dialog_) {
    help_dialog_ = new Fl_Help_Dialog();

    help_dialog_->value(
        "<HTML>\n"
        "<HEAD>\n"
        "<TITLE>Sudoku Help</TITLE>\n"
        "</HEAD>\n"
        "<BODY BGCOLOR='#ffffff'>\n"

        "<H2>About the Game</H2>\n"

        "<P>Sudoku (pronounced soo-dough-coo with the emphasis on the\n"
        "first syllable) is a simple number-based puzzle/game played on a\n"
        "9x9 grid that is divided into 3x3 subgrids. The goal is to enter\n"
        "a number from 1 to 9 in each cell so that each number appears\n"
        "only once in each column and row. In addition, each 3x3 subgrid\n"
        "may only contain one of each number.</P>\n"

        "<P>This version of the puzzle is copyright 2005-2010 by Michael R\n"
        "Sweet.</P>\n"

        "<P><B>Note:</B> The 'Hard' and 'Impossible' difficulty\n"
        "levels generate Sudoku puzzles with multiple possible solutions.\n"
        "While some purists insist that these cannot be called 'Sudoku'\n"
        "puzzles, the author (me) has personally solved many such puzzles\n"
        "in published/printed Sudoku books and finds them far more\n"
        "interesting than the simple single solution variety. If you don't\n"
        "like it, don't play with the difficulty set to 'High' or\n"
        "'Impossible'.</P>\n"

        "<H2>How to Play the Game</H2>\n"

        "<P>At the start of a new game, Sudoku fills in a random selection\n"
        "of cells for you - the number of cells depends on the difficulty\n"
        "level you use. Click in any of the empty cells or use the arrow\n"
        "keys to highlight individual cells and press a number from 1 to 9\n"
        "to fill in the cell. To clear a cell, press 0, Delete, or\n"
        "Backspace. When you have successfully completed all subgrids, the\n"
        "entire puzzle is highlighted in green until you start a new\n"
        "game.</P>\n"

        "<P>As you work to complete the puzzle, you can display possible\n"
        "solutions inside each cell by holding the Shift key and pressing\n"
        "each number in turn. Repeat the process to remove individual\n"
        "numbers, or press a number without the Shift key to replace them\n"
        "with the actual number to use.</P>\n"
        "</BODY>\n"
    );
  }

  help_dialog_->show();
}


// Load the game from saved preferences...
void
Sudoku::load_game() {
  // Load the current values and state of each grid...
  memset(grid_values_, 0, sizeof(grid_values_));

  bool solved = true;

  for (int j = 0; j < 9; j ++)
    for (int k = 0; k < 9; k ++) {
      char name[255];
      int val;

      SudokuCell *cell = grid_cells_[j][k];

      snprintf(name, sizeof(name), "value%d.%d", j, k);
      if (!prefs_.get(name, val, 0)) {
        j = 9;
        grid_values_[0][0] = 0;
        break;
      }

      grid_values_[j][k] = val;

      snprintf(name, sizeof(name), "state%d.%d", j, k);
      prefs_.get(name, val, 0);
      cell->value(val);

      snprintf(name, sizeof(name), "readonly%d.%d", j, k);
      prefs_.get(name, val, 0);
      cell->readonly(val != 0);

      if (val) cell->color(FL_GRAY);
      else {
        cell->color(FL_LIGHT3);
        solved = false;
      }

      snprintf(name, sizeof(name), "hint%d.%d", j, k);
      prefs_.get(name, val, 0);
      cell->set_hint_map(val);
    }

  // If we didn't load any values or the last game was solved, then
  // create a new game automatically...
  if (solved || !grid_values_[0][0]) new_game(time(NULL));
  else check_game(false);
  clear_undo();
}


// Mute/unmute sound...
void
Sudoku::mute_cb(Fl_Widget *widget, void *) {
  if (sudoku->sound_) {
    delete sudoku->sound_;
    sudoku->sound_ = NULL;
    prefs_.set("mute_sound", 1);
  } else {
    sudoku->sound_ = new SudokuSound();
    prefs_.set("mute_sound", 0);
  }
}


// Create a new game...
void
Sudoku::new_cb(Fl_Widget *widget, void *) {
  if (sudoku->grid_cells_[0][0]->color() != FL_GREEN) {
    if (!fl_choice("Are you sure you want to change the difficulty level and "
                   "discard the current game?", "Keep Current Game", "Start New Game",
                   NULL)) return;
  }
  sudoku->new_game(time(NULL));
}


// Create a new game...
void
Sudoku::new_game(time_t seed) {
  int j, k, m, n, t, count;


  // Generate a new (valid) Sudoku grid...
  seed_ = seed;
  srand((unsigned int)seed);

  memset(grid_values_, 0, sizeof(grid_values_));

  for (j = 0; j < 9; j += 3) {
    for (k = 0; k < 9; k += 3) {
      for (t = 1; t <= 9; t ++) {
        for (count = 0; count < 20; count ++) {
          m = j + (rand() % 3);
          n = k + (rand() % 3);
          if (!grid_values_[m][n]) {
            int mm;

            for (mm = 0; mm < m; mm ++)
              if (grid_values_[mm][n] == t) break;

            if (mm < m) continue;

            int nn;

            for (nn = 0; nn < n; nn ++)
              if (grid_values_[m][nn] == t) break;

            if (nn < n) continue;

            grid_values_[m][n] = t;
            break;
          }
        }

        if (count == 20) {
          // Unable to find a valid puzzle so far, so start over...
          k = 9;
          j = -3;
          memset(grid_values_, 0, sizeof(grid_values_));
        }
      }
    }
  }

  // Start by making all cells editable
  SudokuCell *cell;

  for (j = 0; j < 9; j ++)
    for (k = 0; k < 9; k ++) {
      cell = grid_cells_[j][k];
      cell->value(0);
      cell->readonly(0);
      cell->color(FL_LIGHT3);
      cell->clear_hints();
    }

  // Show N cells...
  count = 11 * (5 - difficulty_);

  int numbers[9];

  for (j = 0; j < 9; j ++) numbers[j] = j + 1;

  while (count > 0) {
    for (j = 0; j < 20; j ++) {
      k          = rand() % 9;
      m          = rand() % 9;
      t          = numbers[k];
      numbers[k] = numbers[m];
      numbers[m] = t;
    }

    for (j = 0; count > 0 && j < 9; j ++) {
      t = numbers[j];

      for (k = 0; count > 0 && k < 9; k ++) {
        cell = grid_cells_[j][k];

        if (grid_values_[j][k] == t && !cell->readonly()) {
          cell->value(grid_values_[j][k]);
          cell->readonly(1);
          cell->color(FL_GRAY);

          count --;
          break;
        }
      }
    }
  }
  clear_undo();
}


// Return the next available value for a cell...
int
Sudoku::next_value(SudokuCell *c) {
  int   j = 0, k = 0, m = 0, n = 0;


  for (j = 0; j < 9; j ++) {
    for (k = 0; k < 9; k ++)
      if (grid_cells_[j][k] == c) break;

    if (k < 9) break;
  }

  if (j == 9) return 1;

  j -= j % 3;
  k -= k % 3;

  int numbers[9];

  memset(numbers, 0, sizeof(numbers));

  for (m = 0; m < 3; m ++)
    for (n = 0; n < 3; n ++) {
      c = grid_cells_[j + m][k + n];
      if (c->value()) numbers[c->value() - 1] = 1;
    }

  for (j = 0; j < 9; j ++)
    if (!numbers[j]) return j + 1;

  return 1;
}


// Reset widget color to gray...
void
Sudoku::reset_cb(Fl_Widget *widget, void *) {
  widget->color(FL_LIGHT3);
  widget->redraw();

  ((Sudoku *)(widget->window()))->check_game(false);
}


// Resize the window...
void
Sudoku::resize(int X, int Y, int W, int H) {
  // Resize the window...
  Fl_Double_Window::resize(X, Y, W, H);

  // Save the new window geometry...
  prefs_.set("x", X);
  prefs_.set("y", Y);
  prefs_.set("width", W);
  prefs_.set("height", H);
}


// Restart game from beginning...
void
Sudoku::restart_cb(Fl_Widget *widget, void *) {
  bool solved = true;

  for (int j = 0; j < 9; j ++)
    for (int k = 0; k < 9; k ++) {
      SudokuCell *cell = sudoku->grid_cells_[j][k];
      cell->clear_hints();
      if (!cell->readonly()) {
        solved = false;
        int v = cell->value();
        cell->value(0);
        cell->color(FL_LIGHT3);
        if (v && sudoku->sound_) sudoku->sound_->play('A' + v - 1);
      }
    }

  if (solved)
    sudoku->new_game(sudoku->seed_);
  else
    sudoku->clear_undo();
}


// Save the current game state...
void
Sudoku::save_game() {
  // Save the current values and state of each grid...
  for (int j = 0; j < 9; j ++)
    for (int k = 0; k < 9; k ++) {
      char name[255];
      SudokuCell *cell = grid_cells_[j][k];

      snprintf(name, sizeof(name), "value%d.%d", j, k);
      prefs_.set(name, grid_values_[j][k]);

      snprintf(name, sizeof(name), "state%d.%d", j, k);
      prefs_.set(name, cell->value());

      snprintf(name, sizeof(name), "readonly%d.%d", j, k);
      prefs_.set(name, cell->readonly());

      for (int m = 0; m < 8; m ++) {
        snprintf(name, sizeof(name), "hint%d.%d", j, k);
        prefs_.set(name, cell->get_hint_map());
      }
    }
}

void Sudoku::save_state(GameState &s) {
  for (int j = 0; j < 9; j ++) {
    for (int k = 0; k < 9; k ++) {
      s[j*9+k] = grid_cells_[j][k]->state();
    }
  }
}

void Sudoku::load_state(GameState &s) {
  for (int j = 0; j < 9; j ++) {
    for (int k = 0; k < 9; k ++) {
      grid_cells_[j][k]->state(s[j*9+k]);
    }
  }
}

void Sudoku::undo() {
  if (undo_head_ != undo_tail_) {
    undo_head_ = ((undo_head_-1) & 63);
    load_state(undo_stack[undo_head_]);
    redraw();
  } else {
    fl_beep(FL_BEEP_ERROR);
  }
}

void Sudoku::redo() {
  if (undo_head_ != redo_head_) {
    undo_head_ = ((undo_head_+1) & 63);
    load_state(undo_stack[undo_head_]);
    redraw();
  } else {
    fl_beep(FL_BEEP_ERROR);
  }
}

void Sudoku::clear_undo() {
  undo_head_ = undo_tail_ = redo_head_ = 0;
  save_state(undo_stack[undo_head_]);
}

void Sudoku::undo_checkpoint() {
  if (undo_head_ == redo_head_) {
    redo_head_ = ((redo_head_+1) & 63);
  }
  undo_head_ = ((undo_head_+1) & 63);
  if (undo_head_ == undo_tail_) {
    undo_tail_ = ((undo_tail_+1) & 63);
  }
  save_state(undo_stack[undo_head_]);
}


// Set title of window...
void
Sudoku::set_title() {
  static const char * const titles[] = {
    "Sudoku - Easy",
    "Sudoku - Medium",
    "Sudoku - Hard",
    "Sudoku - Impossible"
  };

  label(titles[difficulty_]);
}


// Solve the puzzle...
void
Sudoku::solve_cb(Fl_Widget *widget, void *) {
  sudoku->solve_game();
}


// Solve the puzzle...
void
Sudoku::solve_game() {
  int j, k;

  for (j = 0; j < 9; j ++) {
    for (k = 0; k < 9; k ++) {
      SudokuCell *cell = grid_cells_[j][k];

      cell->value(grid_values_[j][k]);
      cell->readonly(1);
      cell->color(FL_GRAY);
    }

    if (sound_) sound_->play('A' + grid_cells_[j][8]->value() - 1);
  }
  undo_checkpoint();
}


// Main entry for game...
int
main(int argc, char *argv[]) {
  Sudoku s;
  sudoku = &s;

  // Show the game...
  s.show(argc, argv);

  // Load the previous game...
  s.load_game();

  // Run until the user quits...
  return (Fl::run());
}
