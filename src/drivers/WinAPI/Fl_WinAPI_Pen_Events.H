//
// Definition of Windows (WinAPI) Tablet/Pen event driver for FLTK.
//
// Copyright 2025-2026 by Bill Spitzak and others.
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

#ifndef FL_WINAPI_PEN_DRIVER_H
#define FL_WINAPI_PEN_DRIVER_H

#include <config.h>
#include <FL/platform.H>

#include "src/drivers/Base/Fl_Base_Pen_Events.H"

// The Pointer Input API (GetPointerType, GetPointerPenInfo, POINTER_PEN_INFO,
// PEN_MASK_*, IS_POINTER_*_WPARAM, ...) requires Windows 8 (_WIN32_WINNT >=
// 0x0602). Define this *before* <windows.h> is first included anywhere in the
// translation unit. If FL/Fl.H or another FLTK header pulls in <windows.h>
// first with a lower target, raise the project-wide _WIN32_WINNT instead
// (e.g. in fl_config.h) and remove the guard below.
#ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0602
#endif
#ifndef WINVER
#  define WINVER _WIN32_WINNT
#endif
#include <windows.h>

namespace Fl {

namespace Pen {

/* Windows Pointer Input API driver.

  Subscriber management, traits(), and pen_traits() are implemented here.
  All other Fl::Pen API entry points (event_x(), event_pressure(), ...) are
  inherited unchanged from the base driver and read from Fl::Pen::e, which is
  filled in by fl_winapi_pen_handle() below.
*/
class WinAPI_Driver : public Driver {
public:
  WinAPI_Driver() = default;
  Trait traits()           override;
  Trait pen_traits(int id) override;
  void  release()          override;
};

} // namespace Pen

} // namespace Fl


/*
  Called from the WinAPI window procedure (Fl_WinAPI_Window_Driver's WndProc)
  for every WM_POINTER* message.

  Handles: WM_POINTERENTER, WM_POINTERLEAVE, WM_POINTERDOWN, WM_POINTERUP,
  WM_POINTERUPDATE, WM_POINTERCAPTURECHANGED.

  Returns true if the message described pen/stylus input and was processed
  by the Fl::Pen subscriber dispatch (regardless of whether a subscriber
  consumed the resulting FLTK event). Returns false if the pointer is not a
  pen (e.g. touch or mouse-emulated pointer), in which case the caller should
  continue with its normal handling of the message.

  Either way, the WndProc should still call DefWindowProc() for WM_POINTER*
  messages so the system can release its internal pointer-frame resources;
  this function does not do so itself.

  Integration note: Windows synthesizes legacy WM_*BUTTON* / WM_MOUSEMOVE
  messages from pen input automatically (unless the process has called
  EnableMouseInPointer(TRUE), which this driver does not do). When a pen
  event IS consumed by an Fl::Pen subscriber, the corresponding synthesized
  mouse message should be discarded by the normal mouse handler to avoid a
  duplicate FLTK event; such messages can be recognized via
  GetMessageExtraInfo() returning a value with the high 24 bits equal to
  0xFF515700 (the MI_WP_SIGNATURE used for pen/touch-injected input). This
  filtering must be added to the existing mouse-message handling in
  Fl_WinAPI_Window_Driver.cxx; it is outside the scope of this driver.
*/
FL_EXPORT bool fl_winapi_pen_handle(HWND hwnd, UINT msg, WPARAM wParam,
                                    LPARAM lParam);


#endif // FL_WINAPI_PEN_EVENTS_H
