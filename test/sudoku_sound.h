//
// Sudoku game sound using the Fast Light Tool Kit (FLTK).
//
// Copyright 2005-2018 by Michael Sweet.
// Copyright 2019-2023 by Bill Spitzak and others.
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

#ifndef _SUDOKU_SOUND_H_
#define _SUDOKU_SOUND_H_

// Audio headers...
#include <config.h>

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

#endif // _SUDOKU_SOUND_H_
