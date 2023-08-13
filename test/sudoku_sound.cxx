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

#include "sudoku_sound.h"

#include <FL/Fl.H>

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

