/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include "../../test/button.cxx"


/*

 Missing:
  - screen scale and size: most desktop apps expect to be in a draggable window
    on a larger desktop surface. For Android, there is usually no desktop, and
    screen resolution is often very high, so that a regular FLTK window would
    hide as a tiny gray spot in the top left corner
      * windows should probably be centered by default
      ? the screen resolution should adapt to the first opened window
      ? we should be able to hint at a prefered screen resolution
      * drawing call must scale at some point (line width!)
      * rotating the screen must call the app handler and(?) window resize
      * proportions: pixels should be square
 Need Work:
  - Fl_Android_Graphics_Driver::pie(int) needs refactoring
  - ...::line(...) has round ing issues (see rounded box type)
  - grab() not working when leaving window (adjuster...)
  - scrolling if implemented as a complete redraw. Must implement real scrolling
  - the 'hotspot' idea to position dialogs under the mouse cursor makes little sense on touch screen devices
  - fix screen when keyboard pops up in front of the text cursor or input field (temporarily shift up?)
  - ending 'message' will not quit the app right away, but wait for some timeout
  - no support for dead-key entry
  - no Shift-Tab

  * test/CubeMain.cxx     : UNSUPPROTED - needs OpenGL
  * test/CubeView.cxx     : UNSUPPROTED - needs OpenGL
  * test/shape.cxx        : UNSUPPROTED - needs OpenGL
  * test/cube.cxx         : UNSUPPROTED - needs OpenGL
  * test/fractals.cxx     : UNSUPPROTED - needs OpenGL
  * test/fracviewer.cxx   : UNSUPPROTED - needs OpenGL
  * test/fullscreen.cxx   : UNSUPPROTED - needs OpenGL
  * test/gl_overlay.cxx   : UNSUPPROTED - needs OpenGL
  * test/glpuzzle.cxx     : UNSUPPROTED - needs OpenGL
  * test/mandelbrot.cxx   : UNSUPPORTED - needs Fluid
  * test/keyboard.cxx     : UNSUPPORTED - needs Fluid
  * test/CubeViewUI.fl
  * test/keyboard_ui.fl
  * test/radio.fl
  * test/tree.fl
  * test/fast_slow.fl
  * test/mandelbrot_ui.fl
  * test/resize.fl
  * test/valuators.fl
  * test/inactive.fl
  * test/preferences.fl
  * test/tabs.fl
  * test/cairo_test.cxx   : UNSUPPORTED - needs Cairo
  * test/connect.cxx      : UNSUPPORTED - Android is not Linux
  * test/tiled_image.cxx  : UNSUPPORTED - X11 only
  * test/forms.cxx        : UNSUPPORTED - needs Forms

  * test/doublebuffer.cxx : FIXME - redering is completely wrong
  * test/line_style.cxx   : TODO - no line styles yet
  * test/list_visuals.cxx : TODO - needs config.h
  * test/threads.cxx      : TODO - needs config.h for pthreads
  * test/animated.cxx     : TODO - redering errors (alpha channel?)
  * test/native-filechooser.cxx : TODO - not yet implemented
  * test/blocks.cxx       : TODO - needs config.h
  * test/offscreen.cxx    : TODO - not yet implemented
  * test/overlay.cxx      : TODO - no overlay yet
  * test/pixmap_browser.cxx : TODO - filebrowser not yet implemented, no images, no printer
  * test/clock.cxx        : TODO - no system clock call yet
  * test/resizebox.cxx    : TODO - no window manager yet
  * test/rotated_text.cxx : TODO - no rotated text
  * test/subwindow.cxx    : TODO - no subwindows yet
  * test/sudoku.cxx       : TODO - sound support is in our way
  * test/demo.cxx         : TODO - fails to open window, but is is useful at all?
  * test/device.cxx       : TODO - printing support
  * test/tile.cxx         : TODO - subwindow support
  * test/editor.cxx       : TODO - file chooser missing
  * test/file_chooser.cxx : TODO - file chooser missing
  * test/fonts.cxx        : TODO - works, but does not list system fonts or resource fonts
  * test/help_dialog.cxx  : TODO - not implemented
  * test/icon.cxx         : TODO - what does this do on Android?
  * test/iconize.cxx      : TODO - no window manager
  * test/utf8.cxx         : TODO - window manager, clipping
  * test/windowfocus.cxx  : TODO - what does this do?
  * test/browser.cxx      : TODO - needs text resource to load browser content
  * test/unittests.cxx    : TODO - crashing, no alpha in image drawing, clipping issues

  * test/image.cxx        : + works
  * test/twowin.cxx       : + works
  * test/table.cxx        : + works, but window is much too large for mobile device
  * test/cursor.cxx       : + works, but no cursor on Android
  * test/colbrowser.cxx   : + works
  * test/checkers.cxx     : + works
  * test/pixmap.cxx       : + works
  * test/navigation.cxx   : + works
  * test/curve.cxx        : + works
  * test/input_choice.cxx : + works
  * test/input.cxx        : + works
  * test/scroll.cxx       : - works ok
                            - some dirt when a popup draws over another menu button!?
                            - on touch-screens, menuitem should be selected when released
                            - on touch-screens, scroll groups should scroll on multitouch, or when not causing any other action
  * test/bitmap.cxx       : + 'bitmap' works
  * test/message.cxx      : - 'message' mostly works
                            - when ending the app, it will not close right away but instead hang around for a few seconds
  * test/menubar.cxx      : - 'menubar' mostly works including unicode
                            ! pressing 'button' will hang the app
                            - shortcut modifiers don't work
                            - right-click does not work (should this be emulated via click-and-hold?)
  * test/output.cxx       : + 'output' works
  * test/ask.cxx          : + 'ask' works
  * test/button.cxx       : + 'button' works, including beep
  * test/pack.cxx         : + 'pack' works
  * test/adjuster.cxx     : + 'adjuster' works
  * test/arc.cxx          : + 'arc' works as expected
  * test/minimum.cxx      : + 'minimum' works
  * test/boxtype.cxx      : + 'boxtype' works
  * test/buttons.cxx      : + 'buttons' works
  * test/color_chooser.cxx: + 'color_chooser' works
  * test/symbols.cxx      : + 'symbols' working as expected
  * test/hello.cxx        : + 'hello' works fine, italics, shadow, etc.
  * test/label.cxx        : + 'label' works

 */