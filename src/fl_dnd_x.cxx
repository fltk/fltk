//
// Drag & Drop code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2025 by Bill Spitzak and others.
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

// Note: this file contains platform specific code and will therefore
// not be processed by doxygen (see Doxyfile.in).

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/platform.H>
#include <FL/Fl_Drag_Data.H>
#include "flstring.h"
#include <sys/time.h>
#include "drivers/X11/Fl_X11_Screen_Driver.H"
#include "Fl_Window_Driver.H"
#include "drivers/Unix/Fl_Unix_System_Driver.H"
#include <FL/Fl_Image.H>

extern Atom fl_XdndAware;
extern Atom fl_XdndSelection;
extern Atom fl_XdndEnter;
extern Atom fl_XdndTypeList;
extern Atom fl_XdndPosition;
extern Atom fl_XdndLeave;
extern Atom fl_XdndDrop;
extern Atom fl_XdndStatus;
extern Atom fl_XdndActionCopy;
extern Atom fl_XdndFinished;
extern Atom fl_XdndURIList;
extern Atom fl_XaUtf8String;
extern Atom fl_Xatextplain;

extern char fl_i_own_selection[2];

extern void fl_sendClientMessage(Window window, Atom message,
                                 unsigned long d0,
                                 unsigned long d1=0,
                                 unsigned long d2=0,
                                 unsigned long d3=0,
                                 unsigned long d4=0);

// return version # of Xdnd this window supports.  Also change the
// window to the proxy if it uses a proxy:
static int dnd_aware(Window& window) {
  Atom actual; int format; unsigned long count, remaining;
  unsigned char *data = 0;
  XGetWindowProperty(fl_display, window, fl_XdndAware,
                     0, 4, False, XA_ATOM,
                     &actual, &format,
                     &count, &remaining, &data);
  int ret = 0;
  if (actual == XA_ATOM && format==32 && count && data)
    ret = int(*(Atom*)data);
  if (data) { XFree(data); data = 0; }
  return ret;
}

/* Returns true if the DnD data looks like a list of URIs: the buffer starts
 with a common URI scheme, does not contain spaces, and contains at least
 one CR LF. A single file:// URI is also accepted without CR LF.
 */
bool Fl_X11_Screen_Driver::dnd_text_is_uri_list() {
  const char *p = selection_string[0].c_str();
  if (strchr(p, ' ')) return false;
  if (!strncmp(p, "file:///", 8) && !strpbrk(p, "\r\n")) return true;
  return (!strncmp(p, "file:///", 8) ||
          !strncmp(p, "ftp://", 6) ||
          !strncmp(p, "http://", 7) ||
          !strncmp(p, "https://", 8) ||
          !strncmp(p, "ipp://", 6) ||
          !strncmp(p, "ldap:", 5) ||
          !strncmp(p, "mailto:", 7) ||
          !strncmp(p, "news:", 5) ||
          !strncmp(p, "smb://", 6)) &&
         strstr(p, "\r\n");
}

static double now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1e6;
}

static int grabfunc(int event) {
  if (event == FL_RELEASE) Fl::pushed(0);
  return 0;
}

extern int (*fl_local_grab)(int); // in Fl.cxx

// send an event to an fltk window belonging to this program:
static int local_handle(int event, Fl_Window* window) {
  fl_local_grab = 0;
  Fl::e_x = Fl::e_x_root-window->x();
  Fl::e_y = Fl::e_y_root-window->y();
  int ret = Fl::handle(event,window);
  fl_local_grab = grabfunc;
  return ret;
}

std::vector<std::pair<Atom, std::string> > Fl_X11_Screen_Driver::dnd_typed_offers;

extern Atom fl_dnd_action_atom(int action);   // in Fl_x.cxx
extern int fl_dnd_action_from_atom(Atom a);   // in Fl_x.cxx
extern Window fl_dnd_status_window;           // XdndStatus received from this window
extern int fl_dnd_status_accept;              // ... accepts the drop
extern Atom fl_dnd_status_action;             // ... with this action
extern unsigned fl_dnd_status_count;          // incremented for each XdndStatus received
extern void fl_dnd_trace(const char *format, ...);
extern const char *fl_dnd_atom_name(Atom a);

std::vector<Atom> Fl_X11_Screen_Driver::dnd_local_types;

/* Finds the window a drag at root coordinates x, y is over: a window of this program,
 or the deepest XdndAware window, or else the visible top-level window at x, y.
 The top-level windows are searched from top to bottom. Invisible (InputOnly)
 windows that don't support XDND are skipped: compositors use them, e.g. the
 guard window of mutter under Xwayland, which can be stacked above the window
 mutter provides as drop target for Wayland clients.
 Returns the window, or 0, and sets *version to its XDND version (0 if none)
 and *local to the window of this program, if it is one.
 */
static Window find_dnd_target(int x, int y, int *version, Fl_Window **local) {
  *version = 0;
  *local = NULL;
  Window root = RootWindow(fl_display, fl_screen), root_return, parent, *children = NULL;
  unsigned int n = 0;
  if (!XQueryTree(fl_display, root, &root_return, &parent, &children, &n)) return 0;
  Window found = 0;
  for (int i = (int)n - 1; i >= 0 && !found; i--) { // children are listed bottom to top
    XWindowAttributes a;
    if (!XGetWindowAttributes(fl_display, children[i], &a) || a.map_state != IsViewable) continue;
    if (x < a.x || y < a.y ||
        x >= a.x + a.width + 2 * a.border_width || y >= a.y + a.height + 2 * a.border_width) continue;
    // look in this window and in its descendants at x, y (e.g. the client window in a frame)
    for (Window w = children[i]; w; ) {
      if ((*local = fl_find(w)) || (*version = dnd_aware(w))) { found = w; break; }
      int wx, wy;
      Window child = 0;
      if (!XTranslateCoordinates(fl_display, root, w, x, y, &wx, &wy, &child)) break;
      w = child;
    }
    if (found) break;
    if (a.c_class == InputOnly) {
      static Window last_skipped = 0;
      if (children[i] != last_skipped) fl_dnd_trace("skipping invisible window 0x%lx", children[i]);
      last_skipped = children[i];
      continue;
    }
    found = children[i]; // a visible window that doesn't accept drops
  }
  if (children) XFree(children);
  if (!found && (*version = dnd_aware(root))) found = root; // e.g. some desktops
  return found;
}

// Returns the action proposed by the modifier keys, following GTK and Qt:
// Shift = move, Control = copy, both = link, none = copy
static int proposed_action(unsigned int state, int allowed) {
  int a = FL_DND_COPY;
  if ((state & ShiftMask) && (state & ControlMask)) a = FL_DND_LINK;
  else if (state & ShiftMask) a = FL_DND_MOVE;
  if (allowed & a) return a;
  if (allowed & FL_DND_COPY) return FL_DND_COPY;
  if (allowed & FL_DND_MOVE) return FL_DND_MOVE;
  return FL_DND_LINK;
}

// Returns the data of a typed drag for a MIME type, or NULL
static const std::string *find_offer(const char *mime_type) {
  Atom a = XInternAtom(fl_display, mime_type, True);
  std::vector<std::pair<Atom, std::string> > &offers = Fl_X11_Screen_Driver::dnd_typed_offers;
  for (size_t i = 0; a && i < offers.size(); i++) if (offers[i].first == a) return &offers[i].second;
  return NULL;
}

/* Delivers data dropped on a widget of this program by a drag of this program.
 Returns the action performed, or FL_DND_NONE.
 */
static int local_drop(const std::string *local_text, int allowed) {
  Fl_Widget *w = Fl::belowmouse();
  const std::string &selection = Fl_X11_Screen_Driver::selection_string[0];
  bool has_files, has_image, has_text;
  if (local_text) {
    has_files = (find_offer("text/uri-list") != NULL);
    has_image = (find_offer("image/bmp") != NULL);
    has_text = (find_offer("UTF8_STRING") != NULL);
  } else {
    has_files = Fl_X11_Screen_Driver::dnd_text_is_uri_list();
    has_image = false;
    has_text = true;
  }
  const std::string *raw = NULL;
  if (local_text && Fl_Screen_Driver::dnd_requests_raw())
    raw = find_offer(Fl_Screen_Driver::dnd_requested_type.c_str());
  const char *type = Fl_Screen_Driver::dnd_delivery_type(has_files, has_image, has_text, raw != NULL);
  if (!type) return FL_DND_NONE;
  int action = Fl_Screen_Driver::dnd_target_action(allowed);
  Fl::e_dnd_action = action;
  int old_event = Fl::e_number;
  if (type == Fl::clipboard_image) {
    Fl_RGB_Image *image = Fl_Unix_System_Driver::own_bmp_to_RGB(find_offer("image/bmp")->c_str());
    Fl::e_clipboard_data = image;
    Fl::e_clipboard_type = Fl::clipboard_image;
    Fl::e_text = (char *)"";
    Fl::e_length = 0;
    if (!w->handle(Fl::e_number = FL_PASTE)) delete image;
    Fl::e_clipboard_data = NULL;
  } else {
    std::string text;
    if (raw && type == Fl_Screen_Driver::dnd_requested_type.c_str())
      text = *raw;
    else if (type == Fl::clipboard_files)
      text = local_text ? *local_text : Fl_Screen_Driver::decode_uri_list(selection.c_str(), selection.length());
    else
      text = local_text ? *find_offer("UTF8_STRING") : selection;
    Fl::e_text = (char *)text.c_str();
    Fl::e_length = (int)text.length();
    Fl::e_clipboard_type = type;
    w->handle(Fl::e_number = FL_PASTE);
    Fl::e_text = (char *)"";
    Fl::e_length = 0;
  }
  Fl::e_number = old_event;
  return action;
}

/* Runs the drag and drop loop until the mouse button is released.
 If local_text is NULL, the drag offers the text in selection_string[0].
 Otherwise it offers dnd_typed_offers, and an FLTK widget of this program
 receives local_text when the data is dropped on it.
 allowed is a combination of FL_DND_COPY, FL_DND_MOVE, and FL_DND_LINK.
 Returns the action performed by the drop target, FL_DND_NONE if there was no drop.
 */
static int dnd_loop(const std::string *local_text, int allowed) {
  std::vector<std::pair<Atom, std::string> > &offers = Fl_X11_Screen_Driver::dnd_typed_offers;
  Fl_Window *source_fl_win = Fl::pushed() ? Fl::pushed()->top_window() : Fl::first_window();
  if (!source_fl_win || !fl_xid(source_fl_win)) return FL_DND_NONE;
  source_fl_win->cursor(FL_CURSOR_MOVE);
  Window source_window = fl_xid(source_fl_win);
  fl_local_grab = grabfunc;
  Window target_window = 0;
  Fl_Window* local_window = 0;
  int dndversion = 0; // XDND version of target_window, 0 if it doesn't support XDND
  int dest_x, dest_y;
  int action = FL_DND_NONE;
  unsigned int state = 0; // modifier keys and mouse buttons

  // The data types offered by this drag. In order to support dragging of
  // files/URLs as well as arbitrary text, a text drag that is a list of URIs
  // is flagged both as a URI list (MIME media type "text/uri-list") and plain
  // text. Otherwise, plain text is sent as "text/plain" (required by Firefox),
  // "UTF8_STRING", and "STRING".
  std::vector<Atom> types;
  if (local_text) {
    for (size_t i = 0; i < offers.size(); i++) types.push_back(offers[i].first);
  } else {
    types.push_back(Fl_X11_Screen_Driver::dnd_text_is_uri_list() ? fl_XdndURIList : fl_Xatextplain);
    types.push_back(fl_XaUtf8String);
    types.push_back(XA_STRING);
  }
  // Always announce the types in the XdndTypeList property of the source window,
  // not only when there are more than 3: compositors that connect X11 drags to
  // Wayland clients (e.g. mutter for Xwayland) read this property when the owner
  // of XdndSelection changes. It must therefore be set before taking ownership.
  XChangeProperty(fl_display, source_window, fl_XdndTypeList, XA_ATOM, 32,
                  PropModeReplace, (unsigned char *)&types[0], (int)types.size());
  XSetSelectionOwner(fl_display, fl_XdndSelection, source_window, fl_event_time);
  if (XGetSelectionOwner(fl_display, fl_XdndSelection) != source_window) {
    // The server ignores the request if the time is not acceptable. Compositors
    // that connect X11 drags to Wayland clients only notice a drag when this
    // program owns XdndSelection.
    fl_dnd_trace("XSetSelectionOwner failed with time %lu, using CurrentTime", (unsigned long)fl_event_time);
    XSetSelectionOwner(fl_display, fl_XdndSelection, source_window, CurrentTime);
  }
  fl_dnd_trace("drag started by 0x%lx, %d types, owner of XdndSelection: 0x%lx",
               source_window, (int)types.size(), XGetSelectionOwner(fl_display, fl_XdndSelection));
  fl_dnd_status_window = 0;
  // The XDND protocol requires to wait for XdndStatus before sending the next
  // XdndPosition, and to send XdndPosition only when something changed. Sending it
  // continuously floods the target - e.g. the compositor when mutter connects the
  // drag to a Wayland client under Xwayland.
  bool waiting_for_status = false;
  unsigned status_count = fl_dnd_status_count;
  double position_time = 0;            // when the last XdndPosition was sent
  int sent_x = -1, sent_y = -1;        // position sent in the last XdndPosition
  Atom sent_action = None;             // action sent in the last XdndPosition

  while (Fl::pushed()) {
    // figure out what window we are pointing at:
    Window root = RootWindow(fl_display, fl_screen), root_return, child_return;
    XQueryPointer(fl_display, root, &root_return, &child_return,
                  &Fl::e_x_root, &Fl::e_y_root, &dest_x, &dest_y, &state);
    int new_version = 0;
    Fl_Window* new_local_window = 0;
    Window new_window = find_dnd_target(Fl::e_x_root, Fl::e_y_root, &new_version, &new_local_window);
    if (new_window) { // pointer position in new_window, used by the fake middle-click drop
      Window child;
      XTranslateCoordinates(fl_display, root, new_window, Fl::e_x_root, Fl::e_y_root,
                            &dest_x, &dest_y, &child);
    }
#if USE_XFT || FLTK_USE_CAIRO
    if (new_local_window) {
      float s = Fl::screen_driver()->scale(Fl_Window_Driver::driver(new_local_window)->screen_num());
      Fl::e_x_root /= s;
      Fl::e_y_root /= s;
    }
#endif
    int proposed = proposed_action(state, allowed);

    if (new_window != target_window) {
      if (local_window) {
        local_handle(FL_DND_LEAVE, local_window);
        Fl_X11_Screen_Driver::dnd_local_types.clear();
      } else if (dndversion) {
        fl_sendClientMessage(target_window, fl_XdndLeave, source_window);
      }
      dndversion = new_version;
      target_window = new_window;
      local_window = new_local_window;
      fl_dnd_trace("pointer over 0x%lx, %s, XDND version %d", target_window,
                   local_window ? "window of this program" : "other program", dndversion);
      fl_dnd_status_window = 0;
      waiting_for_status = false;
      sent_x = sent_y = -1; // send a position to the new target
      if (local_window) {
        Fl_Screen_Driver::dnd_target_reset();
        Fl_X11_Screen_Driver::dnd_local_types = types;
        Fl::e_dnd_action = proposed;
        local_handle(FL_DND_ENTER, local_window);
      } else if (dndversion) {
        // Send an X-DND message to the target window with the first 3 types;
        // bit 0 tells the target to read XdndTypeList if there are more
        Atom t[3] = { None, None, None };
        for (size_t i = 0; i < types.size() && i < 3; i++) t[i] = types[i];
        fl_sendClientMessage(target_window, fl_XdndEnter, source_window,
                             (dndversion<<24) | (types.size() > 3 ? 1 : 0),
                             t[0], t[1], t[2]);
      }
    }
    if (local_window) {
      Fl::e_dnd_action = proposed;
      local_handle(FL_DND_DRAG, local_window);
    } else if (dndversion) {
      int exroot = Fl::e_x_root, eyroot = Fl::e_y_root;
#if USE_XFT || FLTK_USE_CAIRO
      Fl_Window *target = fl_find(target_window);
      if (target) {
        float s = Fl::screen_driver()->scale(Fl_Window_Driver::driver(target)->screen_num());
        exroot *= s; eyroot *= s;
      }
#endif
      if (fl_dnd_status_count != status_count) { // XdndStatus received
        status_count = fl_dnd_status_count;
        waiting_for_status = false;
      }
      // don't wait forever for targets that don't answer
      if (waiting_for_status && now() - position_time > 0.5) waiting_for_status = false;
      Atom action_atom = fl_dnd_action_atom(proposed);
      if (!waiting_for_status &&
          (exroot != sent_x || eyroot != sent_y || action_atom != sent_action)) {
        fl_sendClientMessage(target_window, fl_XdndPosition, source_window,
                             0, (exroot<<16)|eyroot, fl_event_time,
                             action_atom);
        sent_x = exroot;
        sent_y = eyroot;
        sent_action = action_atom;
        waiting_for_status = true;
        position_time = now();
      }
    }
    // wait for an event, but come back regularly to send a pending position
    // in case a target never answers, or the modifier keys change
    Fl::wait(0.1);
  }

  if (local_window) {
    Fl::e_dnd_action = proposed_action(state, allowed);
    if (local_handle(FL_DND_RELEASE, local_window) && Fl::belowmouse())
      action = local_drop(local_text, allowed);
    Fl_X11_Screen_Driver::dnd_local_types.clear();
  } else if (dndversion) {
    if (fl_dnd_status_window == target_window && !fl_dnd_status_accept) {
      // the target refused the drop
      fl_sendClientMessage(target_window, fl_XdndLeave, source_window);
    } else {
      fl_sendClientMessage(target_window, fl_XdndDrop, source_window,
                           0, fl_event_time);
      if (fl_dnd_status_window == target_window)
        action = fl_dnd_action_from_atom(fl_dnd_status_action);
      else // the target didn't answer: assume it accepts the proposed action
        action = proposed_action(state, allowed);
    }
  } else if (target_window && !local_text) {
    // fake a drop by clicking the middle mouse button:
    XButtonEvent msg;
    msg.type = ButtonPress;
    msg.window = target_window;
    msg.root = RootWindow(fl_display, fl_screen);
    msg.subwindow = 0;
    msg.time = fl_event_time+1;
    msg.x = dest_x;
    msg.y = dest_y;
    float s = 1;
#if USE_XFT || FLTK_USE_CAIRO
    Fl_Window *target = fl_find(target_window);
    if (target) s = Fl::screen_driver()->scale(Fl_Window_Driver::driver(target)->screen_num());
#endif
    msg.x_root = Fl::e_x_root * s;
    msg.y_root = Fl::e_y_root * s;
    msg.state = 0x0;
    msg.button = Button2;
    XSendEvent(fl_display, target_window, False, 0L, (XEvent*)&msg);
    msg.time++;
    msg.state = 0x200;
    msg.type = ButtonRelease;
    XSendEvent(fl_display, target_window, False, 0L, (XEvent*)&msg);
    action = FL_DND_COPY;
  }

  fl_dnd_trace("drag ended over 0x%lx, action %d", target_window, action);
  fl_local_grab = 0;
  Fl::handle(FL_RELEASE, source_fl_win);
  source_fl_win->cursor(FL_CURSOR_DEFAULT);
  return action;
}

int Fl_X11_Screen_Driver::dnd(int unused) {
  dnd_typed_offers.clear();
  dnd_loop(NULL, FL_DND_COPY);
  return 1;
}

int Fl_X11_Screen_Driver::dnd_data(Fl_Drag_Data *data) {
  Dnd_Offers offers;
  std::string local_text;
  dnd_offers(data, offers, local_text);
  dnd_typed_offers.clear();
  for (size_t i = 0; i < offers.size(); i++) {
    Atom a = XInternAtom(fl_display, offers[i].first.c_str(), False);
    dnd_typed_offers.push_back(std::make_pair(a, offers[i].second));
  }
  int allowed = data->actions() ? data->actions() : FL_DND_COPY;
  int action = offers.empty() ? FL_DND_NONE : dnd_loop(&local_text, allowed);
  // the drop target may still request the data, which remains in dnd_typed_offers
  data->do_callback(action);
  delete data;
  return action != FL_DND_NONE;
}
