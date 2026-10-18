//
// Drag & Drop code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

// This file contains Windows-specific code for FLTK which is always linked
// in.  Search other files for "_WIN32" or filenames ending in _win32.cxx
// for other system-specific code.

#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Window.H>
#include "Fl_Window_Driver.H"
#include <FL/fl_utf8.h>
#include <FL/Fl_Drag_Data.H>
#include <FL/Fl_Image.H>
#include <FL/filename.H> // FL_PATH_MAX
#include <FL/Fl_PNG_Image.H> // Fl_Load_PNG_Plugin
#include <FL/Fl_Plugin.H>
#include "drivers/WinAPI/Fl_WinAPI_Screen_Driver.H"
#include "flstring.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <objidl.h>
#include <time.h>
#include <string>
#include <vector>
#include <memory> // std::shared_ptr
#if defined(__CYGWIN__)
#include <sys/time.h>
#include <unistd.h>
#endif

extern char *fl_selection_buffer[2];
extern int fl_selection_length[2];
extern int fl_selection_buffer_length[2];
extern char fl_i_own_selection[2];
extern char *fl_locale2utf8(const char *s, UINT codepage = 0);
extern unsigned int fl_codepage;

Fl_Window *fl_dnd_target_window = 0;

#include <ole2.h>
#include <shellapi.h>
#include <shlobj.h>

#ifndef FD_PROGRESSUI
#  define FD_PROGRESSUI 0x4000
#endif
#ifndef MK_ALT
#  define MK_ALT 0x20
#endif
#ifndef CF_DIBV5
#  define CF_DIBV5 17
#endif
#ifndef FD_ATTRIBUTES
#  define FD_ATTRIBUTES 0x4
#endif
#ifndef FD_FILESIZE
#  define FD_FILESIZE 0x40
#endif


extern Fl_RGB_Image *fl_image_from_dib(LPBITMAPINFO lpBI);           // in Fl_win32.cxx
extern Fl_RGB_Image *fl_image_from_emf(HENHMETAFILE h, Fl_Window *win); // in Fl_win32.cxx

// The data object of the drag in progress over a window of this program, if any
static IDataObject *fl_dnd_data = NULL;

static void set_dnd_data(IDataObject *data) {
  if (data) data->AddRef();
  if (fl_dnd_data) fl_dnd_data->Release();
  fl_dnd_data = data;
}

static bool has_format(IDataObject *data, CLIPFORMAT cf, DWORD tymed = TYMED_HGLOBAL) {
  FORMATETC fmt = { cf, NULL, DVASPECT_CONTENT, -1, tymed };
  return data && data->QueryGetData(&fmt) == S_OK;
}

static bool has_text(IDataObject *data) {
  return has_format(data, CF_UNICODETEXT) || has_format(data, CF_TEXT);
}

// virtual files, e.g. images dragged from web browsers or attachments dragged from mail programs
static CLIPFORMAT virtual_files_format(int contents) {
  static CLIPFORMAT formats[2] = { 0, 0 };
  if (!formats[0]) {
    formats[0] = (CLIPFORMAT)RegisterClipboardFormatW(L"FileGroupDescriptorW");
    formats[1] = (CLIPFORMAT)RegisterClipboardFormatW(L"FileContents");
  }
  return formats[contents ? 1 : 0];
}

static bool has_files(IDataObject *data) {
  return has_format(data, CF_HDROP) || has_format(data, virtual_files_format(0));
}

// PNG data: "PNG" is the name most Windows applications use, some use the MIME type
static CLIPFORMAT png_format(int i) {
  static CLIPFORMAT formats[2] = { 0, 0 };
  if (!formats[0]) {
    formats[0] = (CLIPFORMAT)RegisterClipboardFormatW(L"PNG");
    formats[1] = (CLIPFORMAT)RegisterClipboardFormatW(L"image/png");
  }
  return formats[i];
}

// Clipboard format of a MIME type: the format registered with this name
static CLIPFORMAT mime_format(const char *mime_type) {
  int l = (int)strlen(mime_type);
  unsigned n = fl_utf8toUtf16(mime_type, l, NULL, 0);
  std::vector<unsigned short> wtype(n + 1);
  fl_utf8toUtf16(mime_type, l, &wtype[0], n + 1);
  return (CLIPFORMAT)RegisterClipboardFormatW((LPCWSTR)&wtype[0]);
}

static bool has_image(IDataObject *data) {
  return has_format(data, png_format(0)) || has_format(data, png_format(1)) ||
         has_format(data, CF_DIBV5) || has_format(data, CF_DIB) ||
         has_format(data, CF_ENHMETAFILE, TYMED_ENHMF);
}

/* Prints the formats a drag offers to stderr if the environment variable
 FLTK_DND_DEBUG is set. This helps finding out what other programs offer.
 */
static void trace_formats(IDataObject *data) {
  static int enabled = -1;
  if (enabled < 0) enabled = (fl_getenv("FLTK_DND_DEBUG") != NULL);
  if (!enabled || !data) return;
  static const struct { CLIPFORMAT cf; const char *name; } standard[] = {
    { CF_TEXT, "CF_TEXT" }, { CF_BITMAP, "CF_BITMAP" }, { CF_DIB, "CF_DIB" },
    { CF_UNICODETEXT, "CF_UNICODETEXT" }, { CF_ENHMETAFILE, "CF_ENHMETAFILE" },
    { CF_HDROP, "CF_HDROP" }, { CF_LOCALE, "CF_LOCALE" }, { CF_DIBV5, "CF_DIBV5" }
  };
  IEnumFORMATETC *formats = NULL;
  if (data->EnumFormatEtc(DATADIR_GET, &formats) != S_OK || !formats) {
    fprintf(stderr, "DND: the drag source doesn't list its formats\n");
    return;
  }
  fprintf(stderr, "DND: offered formats:");
  FORMATETC fmt;
  ULONG n;
  while (formats->Next(1, &fmt, &n) == S_OK && n == 1) {
    const char *name = NULL;
    for (size_t i = 0; i < sizeof(standard) / sizeof(standard[0]); i++)
      if (standard[i].cf == fmt.cfFormat) name = standard[i].name;
    wchar_t wname[256];
    char utf8[1024];
    int l = name ? 0 : GetClipboardFormatNameW(fmt.cfFormat, wname, 256);
    if (name) fprintf(stderr, " %s", name);
    else if (l > 0) { fl_utf8fromwc(utf8, sizeof(utf8), wname, l); fprintf(stderr, " \"%s\"", utf8); }
    else fprintf(stderr, " %u", (unsigned)fmt.cfFormat);
    fprintf(stderr, "(tymed %lu)", (unsigned long)fmt.tymed);
    if (fmt.ptd) CoTaskMemFree(fmt.ptd);
  }
  fprintf(stderr, "\n");
  fflush(stderr);
  formats->Release();
}

int Fl_WinAPI_Screen_Driver::dnd_contains(const char *type) {
  if (!fl_dnd_data) return 0;
  if (!strcmp(type, Fl::clipboard_files)) return has_files(fl_dnd_data);
  if (!strcmp(type, Fl::clipboard_plain_text)) return has_text(fl_dnd_data);
  if (!strcmp(type, Fl::clipboard_image)) return has_image(fl_dnd_data);
  if (has_format(fl_dnd_data, mime_format(type)))
    return 1;
  // "PNG" is the name most Windows applications use for PNG data
  return !strcmp(type, "image/png") && has_format(fl_dnd_data, (CLIPFORMAT)RegisterClipboardFormatW(L"PNG"));
}

// Returns the dropped text in UTF-8 encoding, allocated with malloc(), or NULL
static char *read_text(IDataObject *data) {
  FORMATETC fmt = { CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  STGMEDIUM medium = { 0 };
  char *result = NULL;
  // if it is UNICODE text, return a UTF-8-converted copy of it
  if ( data->GetData( &fmt, &medium )==S_OK )
  {
    void *stuff = GlobalLock( medium.hGlobal );
    unsigned srclen = 0;
    const wchar_t *wstuff = (const wchar_t *)stuff;
    while (*wstuff++) srclen++;
    wstuff = (const wchar_t *)stuff;
    unsigned utf8len = fl_utf8fromwc(NULL, 0, wstuff, srclen);
    result = (char*)malloc(utf8len + 1);
    fl_utf8fromwc(result, utf8len+1, wstuff, srclen+1); // include null-byte
    GlobalUnlock( medium.hGlobal );
    ReleaseStgMedium( &medium );
    return result;
  }
  fmt.cfFormat = CF_TEXT;
  // if it is CP1252 text, return a UTF-8-converted copy of it
  if ( data->GetData( &fmt, &medium )==S_OK )
  {
    int len;
    char *p, *q, *last;
    unsigned u;
    void *stuff = GlobalLock( medium.hGlobal );
    result = (char*)malloc(3 * strlen((char*)stuff) + 10);
    p = (char*)stuff;
    last = p + strlen(p);
    q = result;
    while (p < last) {
      u = fl_utf8decode(p, last, &len);
      p += len;
      len = fl_utf8encode(u, q);
      q += len;
    }
    *q = 0;
    GlobalUnlock( medium.hGlobal );
    ReleaseStgMedium( &medium );
  }
  return result;
}

// Returns the '\n' separated list of dropped files in UTF-8 encoding, allocated with malloc(), or NULL
static char *read_files(IDataObject *data) {
  FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  STGMEDIUM medium = { 0 };
  if ( data->GetData( &fmt, &medium )!=S_OK ) return NULL;
  HDROP hdrop = (HDROP)medium.hGlobal;
  int i, n, nn = 0, nf = DragQueryFileW( hdrop, (UINT)-1, 0, 0 );
  for ( i=0; i<nf; i++ ) nn += DragQueryFileW( hdrop, i, 0, 0 );
  nn += nf;
  wchar_t *dst = (wchar_t *)malloc((nn + 1) * sizeof(wchar_t));
  wchar_t *bu = dst;
  for ( i=0; i<nf; i++ ) {
    n = DragQueryFileW( hdrop, i, (WCHAR*)dst, nn );
    dst += n;
    if ( i<nf-1 ) {
      *dst++ = L'\n';
    }
  }
  *dst=0;
  char *result = (char*) malloc(nn * 5 + 1);
  int size = fl_utf8fromwc(result, (nn*5+1), bu, (unsigned)(dst - bu));
  result[size] = 0;
  free(bu);
  ReleaseStgMedium( &medium );
  return result;
}

// true if a relative path contains a ".." component
static bool has_parent_component(const char *path) {
  for (const char *p = path; (p = strstr(p, "..")) != NULL; p += 2) {
    if ((p == path || p[-1] == '/') && (p[2] == 0 || p[2] == '/')) return true;
  }
  return false;
}

/* Writes the virtual files of a drop into a new temporary directory, which is
 removed when the program exits. Returns the '\n' separated list of the dropped
 files and directories, allocated with malloc(), or NULL.
 */
static char *read_virtual_files(IDataObject *data) {
  FORMATETC fmt = { virtual_files_format(0), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  STGMEDIUM medium = { 0 };
  if (data->GetData(&fmt, &medium) != S_OK) return NULL;
  const FILEGROUPDESCRIPTORW *fgd = (const FILEGROUPDESCRIPTORW *)GlobalLock(medium.hGlobal);
  std::string dir = Fl_Screen_Driver::dnd_make_temp_dir("fltk-dnd-received");
  std::string list;
  if (fgd && !dir.empty()) {
    Fl_Screen_Driver::dnd_remove_at_exit(dir);
    for (UINT i = 0; i < fgd->cItems; i++) {
      const FILEDESCRIPTORW &fd = fgd->fgd[i];
      char name[MAX_PATH * 4];
      unsigned l = fl_utf8fromwc(name, sizeof(name), fd.cFileName, (unsigned)wcslen(fd.cFileName));
      name[l < sizeof(name) ? l : sizeof(name) - 1] = 0;
      // names are relative paths for the content of dropped folders: reject anything else
      for (char *p = name; *p; p++) if (*p == '\\') *p = '/';
      if (!name[0] || name[0] == '/' || strchr(name, ':') || has_parent_component(name)) continue;
      std::string path = dir + "/" + name;
      bool top_level = (strchr(name, '/') == NULL);
      if ((fd.dwFlags & FD_ATTRIBUTES) && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        fl_make_path(path.c_str());
        Fl_Screen_Driver::dnd_remove_at_exit(path);
      } else {
        fl_make_path_for_file(path.c_str());
        FORMATETC fc = { virtual_files_format(1), NULL, DVASPECT_CONTENT, (LONG)i,
                         TYMED_ISTREAM | TYMED_HGLOBAL };
        STGMEDIUM content = { 0 };
        if (data->GetData(&fc, &content) != S_OK) continue;
        FILE *f = fl_fopen(path.c_str(), "wb");
        if (f) {
          if (content.tymed == TYMED_ISTREAM) {
            char buffer[16384];
            HRESULT hr;
            ULONG n;
            do { // the last chunk may come with S_FALSE
              n = 0;
              hr = content.pstm->Read(buffer, sizeof(buffer), &n);
              if (n > 0) fwrite(buffer, 1, n, f);
            } while (hr == S_OK && n > 0);
          } else if (content.tymed == TYMED_HGLOBAL) {
            SIZE_T size = GlobalSize(content.hGlobal);
            // the size of global memory can be larger than the file
            if (fd.dwFlags & FD_FILESIZE) {
              unsigned long long fsize = ((unsigned long long)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
              if (fsize < size) size = (SIZE_T)fsize;
            }
            fwrite(GlobalLock(content.hGlobal), 1, size, f);
            GlobalUnlock(content.hGlobal);
          }
          fclose(f);
          Fl_Screen_Driver::dnd_remove_at_exit(path);
        }
        ReleaseStgMedium(&content);
        if (!f) continue;
      }
      if (top_level) {
        if (!list.empty()) list += '\n';
        for (size_t j = 0; j < path.length(); j++) if (path[j] == '/') path[j] = '\\';
        list += path;
      }
    }
  }
  if (fgd) GlobalUnlock(medium.hGlobal);
  ReleaseStgMedium(&medium);
  return list.empty() ? NULL : strdup(list.c_str());
}

// Returns the dropped image, or NULL
static Fl_RGB_Image *read_image(IDataObject *data, Fl_Window *win) {
  STGMEDIUM medium = { 0 };
  Fl_RGB_Image *image = NULL;
  // PNG keeps transparency; decoding it requires fltk_images (see fl_register_images())
  static Fl_Load_PNG_Plugin *png_plugin = NULL;
  if (!png_plugin) {
    Fl_Plugin_Manager pm("loadpng.fltk.org");
    png_plugin = (Fl_Load_PNG_Plugin*)pm.plugin("implementation.loadpng.fltk.org");
  }
  for (int i = 0; png_plugin && i < 2; i++) {
    FORMATETC fmt = { png_format(i), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    if (data->GetData(&fmt, &medium) != S_OK) continue;
    const char *bytes = (const char *)GlobalLock(medium.hGlobal);
    image = (Fl_RGB_Image *)png_plugin->png_load_from_memory(bytes, (int)GlobalSize(medium.hGlobal));
    GlobalUnlock(medium.hGlobal);
    ReleaseStgMedium(&medium);
    if (image) return image;
  }
  static const CLIPFORMAT dibs[2] = { CF_DIBV5, CF_DIB };
  for (int i = 0; i < 2; i++) {
    FORMATETC fmt = { dibs[i], NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    if (data->GetData(&fmt, &medium) != S_OK) continue;
    image = fl_image_from_dib((LPBITMAPINFO)GlobalLock(medium.hGlobal));
    GlobalUnlock(medium.hGlobal);
    ReleaseStgMedium(&medium);
    if (image) return image;
  }
  FORMATETC fmt2 = { CF_ENHMETAFILE, NULL, DVASPECT_CONTENT, -1, TYMED_ENHMF };
  if (data->GetData(&fmt2, &medium) == S_OK) {
    image = fl_image_from_emf(medium.hEnhMetaFile, win);
    ReleaseStgMedium(&medium);
  }
  return image;
}

// Reads the data of a MIME type, returns false if not available
static bool read_raw(IDataObject *data, const char *mime_type, std::string &bytes) {
  CLIPFORMAT formats[2] = { mime_format(mime_type), 0 };
  if (!strcmp(mime_type, "image/png")) formats[1] = png_format(0); // "PNG"
  for (int i = 0; i < 2 && formats[i]; i++) {
    FORMATETC fmt = { formats[i], NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium = { 0 };
    if (data->GetData(&fmt, &medium) != S_OK) continue;
    // note: the size of global memory can be larger than the data it contains
    const char *p = (const char *)GlobalLock(medium.hGlobal);
    bytes.assign(p, GlobalSize(medium.hGlobal));
    GlobalUnlock(medium.hGlobal);
    ReleaseStgMedium(&medium);
    return true;
  }
  return false;
}

static int actions_from_effect(DWORD effect) {
  int actions = 0;
  if (effect & DROPEFFECT_COPY) actions |= FL_DND_COPY;
  if (effect & DROPEFFECT_MOVE) actions |= FL_DND_MOVE;
  if (effect & DROPEFFECT_LINK) actions |= FL_DND_LINK;
  return actions;
}

static DWORD effect_from_action(int action) {
  if (action == FL_DND_COPY) return DROPEFFECT_COPY;
  if (action == FL_DND_MOVE) return DROPEFFECT_MOVE;
  if (action == FL_DND_LINK) return DROPEFFECT_LINK;
  return DROPEFFECT_NONE;
}

// Returns the action proposed by the modifier keys, following the Windows conventions:
// Control = copy, Shift = move, Control+Shift or Alt = link, none = copy
static int proposed_action(DWORD keys, int allowed) {
  int a = FL_DND_COPY;
  if (((keys & MK_CONTROL) && (keys & MK_SHIFT)) || (keys & MK_ALT)) a = FL_DND_LINK;
  else if (keys & MK_SHIFT) a = FL_DND_MOVE;
  if (allowed & a) return a;
  if (allowed & FL_DND_COPY) return FL_DND_COPY;
  if (allowed & FL_DND_MOVE) return FL_DND_MOVE;
  return FL_DND_LINK;
}

// shows the drag image of the source over windows of this program
static IDropTargetHelper *drop_target_helper() {
  static IDropTargetHelper *helper = NULL;
  static bool tried = false;
  if (!tried) {
    tried = true;
    if (CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                         IID_IDropTargetHelper, (void **)&helper) != S_OK) helper = NULL;
  }
  return helper;
}

static HWND top_level_hwnd(Fl_Window *win) {
  while (win && win->window()) win = win->window();
  return win ? fl_xid(win) : NULL;
}

/**
 subclass the IDropTarget to receive data from DnD operations
 */
class FLDropTarget : public IDropTarget
{
  DWORD m_cRefCount; // for "statistics" only (issue #569)
  DWORD lastEffect;
  DWORD lastKeys;
  int px, py;

  // set the event coordinates from a position in screen pixels
  static void set_event_xy(POINTL pt, Fl_Window *target) {
    Fl::e_x_root = pt.x;
    Fl::e_y_root = pt.y;
    if (target) {
      float s = Fl::screen_driver()->scale(Fl_Window_Driver::driver(target)->screen_num());
      Fl::e_x_root = int(Fl::e_x_root / s);
      Fl::e_y_root = int(Fl::e_y_root / s);
      Fl::e_x = Fl::e_x_root-target->x();
      Fl::e_y = Fl::e_y_root-target->y();
    }
  }
  // returns the effect to answer if the widget below the mouse accepts the drag
  static DWORD answer(int accepted, DWORD allowed_effects) {
    if (!accepted) return DROPEFFECT_NONE;
    return effect_from_action(Fl_Screen_Driver::dnd_target_action(actions_from_effect(allowed_effects)));
  }

public:
  FLDropTarget() : m_cRefCount(0), lastEffect(DROPEFFECT_NONE), lastKeys(0), px(0), py(0) { }
  virtual ~FLDropTarget() { }
  HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, LPVOID *ppvObject ) FL_OVERRIDE {
    if (IID_IUnknown==riid || IID_IDropTarget==riid)
    {
      *ppvObject=this;
      ((LPUNKNOWN)*ppvObject)->AddRef();
      return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
  }
  ULONG STDMETHODCALLTYPE AddRef() FL_OVERRIDE { return ++m_cRefCount; }
  ULONG STDMETHODCALLTYPE Release() FL_OVERRIDE {
    long nTemp;
    nTemp = --m_cRefCount;
    // this is a static object, do not 'delete this' (issue #569)
    return nTemp;
  }
  HRESULT STDMETHODCALLTYPE DragEnter( IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) FL_OVERRIDE {
    if( !pDataObj ) return E_INVALIDARG;
    POINT ppt;
    ppt.x = pt.x;
    ppt.y = pt.y;
    HWND hWnd = WindowFromPoint( ppt );
    Fl_Window *target = fl_find( hWnd );
    set_event_xy(pt, target);
    fl_dnd_target_window = target;
    px = pt.x; py = pt.y;
    lastKeys = grfKeyState;
    DWORD allowed = *pdwEffect;
    trace_formats(pDataObj);
    // check if FLTK can deliver this data (i.e. not a BMP drag without filename)
    if (target && (has_text(pDataObj) || has_files(pDataObj) || has_image(pDataObj))) {
      set_dnd_data(pDataObj);
      Fl_Screen_Driver::dnd_target_reset();
      Fl::e_dnd_action = proposed_action(grfKeyState, actions_from_effect(allowed));
      *pdwEffect = answer(Fl::handle( FL_DND_ENTER, target ), allowed);
    } else {
      set_dnd_data(NULL);
      *pdwEffect = DROPEFFECT_NONE;
    }
    lastEffect = *pdwEffect;
    if (target && drop_target_helper())
      drop_target_helper()->DragEnter(top_level_hwnd(target), pDataObj, &ppt, *pdwEffect);
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE DragOver( DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) FL_OVERRIDE {
    POINT ppt;
    ppt.x = pt.x;
    ppt.y = pt.y;
    if ( px==pt.x && py==pt.y && grfKeyState==lastKeys )
    {
      *pdwEffect = lastEffect;
      if (drop_target_helper()) drop_target_helper()->DragOver(&ppt, *pdwEffect);
      return S_OK;
    }
    if ( !fl_dnd_target_window || !fl_dnd_data )
    {
      *pdwEffect = lastEffect = DROPEFFECT_NONE;
      if (drop_target_helper()) drop_target_helper()->DragOver(&ppt, *pdwEffect);
      return S_OK;
    }
    set_event_xy(pt, fl_dnd_target_window);
    DWORD allowed = *pdwEffect;
    Fl::e_dnd_action = proposed_action(grfKeyState, actions_from_effect(allowed));
    // Fl_Group will change DND_DRAG into DND_ENTER and DND_LEAVE if needed
    *pdwEffect = answer(Fl::handle( FL_DND_DRAG, fl_dnd_target_window ), allowed);
    px = pt.x; py = pt.y;
    lastKeys = grfKeyState;
    lastEffect = *pdwEffect;
    if (drop_target_helper()) drop_target_helper()->DragOver(&ppt, *pdwEffect);
    // show insert position if dnd'ing in the same window/process (STR #3209)
    Fl::flush();
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE DragLeave() FL_OVERRIDE {
    if (drop_target_helper()) drop_target_helper()->DragLeave();
    if ( fl_dnd_target_window && fl_dnd_data )
    {
      Fl::handle( FL_DND_LEAVE, fl_dnd_target_window );
    }
    fl_dnd_target_window = 0;
    set_dnd_data(NULL);
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE Drop( IDataObject *data, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) FL_OVERRIDE {
    if (drop_target_helper()) {
      POINT ppt;
      ppt.x = pt.x;
      ppt.y = pt.y;
      drop_target_helper()->Drop(data, &ppt, *pdwEffect);
    }
    DWORD allowed = *pdwEffect;
    *pdwEffect = DROPEFFECT_NONE;
    Fl_Window *target = fl_dnd_target_window;
    fl_dnd_target_window = 0;
    if ( !target || !fl_dnd_data || !data ) {
      set_dnd_data(NULL);
      return S_OK;
    }
    set_event_xy(pt, target);
    Fl::e_dnd_action = proposed_action(grfKeyState, actions_from_effect(allowed));
    // tell FLTK that the user released an object on this widget
    if ( !Fl::handle( FL_DND_RELEASE, target ) || !Fl::belowmouse() ) {
      set_dnd_data(NULL);
      return S_OK;
    }
    std::string raw;
    bool has_raw = Fl_Screen_Driver::dnd_requests_raw() &&
                   read_raw(data, Fl_Screen_Driver::dnd_requested_type.c_str(), raw);
    const char *type = Fl_Screen_Driver::dnd_delivery_type(has_files(data), has_image(data),
                                                           has_text(data), has_raw);
    int action = Fl_Screen_Driver::dnd_target_action(actions_from_effect(allowed));
    Fl::e_dnd_action = action;

    Fl_Widget *w = target;
    while (w->parent()) w = w->window();
    HWND hwnd = fl_xid( (Fl_Window*)w );
    int old_event = Fl::e_number;
    bool delivered = false;
    if (has_raw && type == Fl_Screen_Driver::dnd_requested_type.c_str()) {
      // data of the MIME type requested with Fl::dnd_request()
      Fl::e_text = (char *)raw.c_str();
      Fl::e_length = (int)raw.length();
      Fl::e_clipboard_type = type;
      Fl::belowmouse()->handle(Fl::e_number = FL_PASTE);
      Fl::e_number = old_event;
      Fl::e_text = (char *)"";
      Fl::e_length = 0;
      delivered = true;
    } else if (type == Fl::clipboard_image) {
      Fl_RGB_Image *image = read_image(data, target);
      if (image) {
        Fl::e_clipboard_data = image;
        Fl::e_clipboard_type = Fl::clipboard_image;
        Fl::e_text = (char *)"";
        Fl::e_length = 0;
        if (!Fl::belowmouse()->handle(Fl::e_number = FL_PASTE)) delete image;
        Fl::e_number = old_event;
        Fl::e_clipboard_data = NULL;
        delivered = true;
      }
    } else if (type) {
      char *text;
      if (type == Fl::clipboard_files)
        text = has_format(data, CF_HDROP) ? read_files(data) : read_virtual_files(data);
      else
        text = read_text(data);
      if (text) {
        char *a, *b;
        a = b = text;
        while (*a) { // strip the CRLF pairs
          if (*a == '\r' && a[1] == '\n') a++;
          else *b++ = *a++;
        }
        *b = 0;
        Fl::e_text = text;
        Fl::e_length = (int) (b - text);
        Fl::e_clipboard_type = type;
        Fl::belowmouse()->handle(Fl::e_number = FL_PASTE); // e_text will be invalid after this call
        Fl::e_number = old_event;
        Fl::e_text = (char *)"";
        Fl::e_length = 0;
        free(text);
        delivered = true;
      }
    }
    if (delivered) {
      *pdwEffect = effect_from_action(action);
      SetForegroundWindow( hwnd );
    }
    set_dnd_data(NULL);
    return S_OK;
  }
} flDropTarget;

IDropTarget *flIDropTarget = &flDropTarget;

/**
 this class is needed to allow FLTK apps to be a DnD source
 */
class FLDropSource : public IDropSource
{
  DWORD m_cRefCount;
public:
  FLDropSource() { m_cRefCount = 0; }
  virtual ~FLDropSource() { }
  HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, LPVOID *ppvObject ) FL_OVERRIDE {
    if (IID_IUnknown==riid || IID_IDropSource==riid)
    {
      *ppvObject=this;
      ((LPUNKNOWN)*ppvObject)->AddRef();
      return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
  }
  ULONG STDMETHODCALLTYPE AddRef() FL_OVERRIDE { return ++m_cRefCount; }
  ULONG STDMETHODCALLTYPE Release() FL_OVERRIDE {
    long nTemp;
    nTemp = --m_cRefCount;
    if(nTemp==0)
      delete this;
    return nTemp;
  }
  STDMETHODIMP GiveFeedback( DWORD ) FL_OVERRIDE { return DRAGDROP_S_USEDEFAULTCURSORS; }
  STDMETHODIMP QueryContinueDrag( BOOL esc, DWORD keyState ) FL_OVERRIDE {
    if ( esc )
      return DRAGDROP_S_CANCEL;
    if ( !(keyState & (MK_LBUTTON|MK_MBUTTON|MK_RBUTTON)) )
      return DRAGDROP_S_DROP;
    return S_OK;
  }
};
/**
  This is the actual object that FLTK can drop somewhere.

  The implementation is minimal, but it should work with all decent Win32 drop targets
*/
class FLDataObject : public IDataObject
{
  DWORD m_cRefCount;
public:
  FLDataObject() { m_cRefCount = 1; }
  virtual ~FLDataObject() { }
  HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, LPVOID *ppvObject ) FL_OVERRIDE {
    if (IID_IUnknown==riid || IID_IDataObject==riid)
    {
      *ppvObject=this;
      ((LPUNKNOWN)*ppvObject)->AddRef();
      return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
  }
  ULONG STDMETHODCALLTYPE AddRef() FL_OVERRIDE { return ++m_cRefCount; }
  ULONG STDMETHODCALLTYPE Release() FL_OVERRIDE {
    long nTemp;
    nTemp = --m_cRefCount;
    if(nTemp==0)
      delete this;
    return nTemp;
  }
  // GetData currently allows UNICODE text through Global Memory only
  HRESULT STDMETHODCALLTYPE GetData( FORMATETC *pformatetcIn, STGMEDIUM *pmedium ) FL_OVERRIDE {
    if ((pformatetcIn->dwAspect & DVASPECT_CONTENT) &&
        (pformatetcIn->tymed & TYMED_HGLOBAL) &&
        (pformatetcIn->cfFormat == CF_UNICODETEXT))
    {
      int utf16_len = fl_utf8toUtf16(fl_selection_buffer[0], fl_selection_length[0], 0, 0);
      HGLOBAL gh = GlobalAlloc( GHND, utf16_len * 2 + 2 );
      char *pMem = (char*)GlobalLock( gh );
      fl_utf8toUtf16(fl_selection_buffer[0], fl_selection_length[0], (unsigned short*)pMem, utf16_len + 1);
      pmedium->tymed          = TYMED_HGLOBAL;
      pmedium->hGlobal        = gh;
      pmedium->pUnkForRelease = NULL;
      GlobalUnlock( gh );
      return S_OK;
    }
    return DV_E_FORMATETC;
  }
  HRESULT STDMETHODCALLTYPE QueryGetData( FORMATETC *pformatetc ) FL_OVERRIDE
  {
    if ((pformatetc->dwAspect & DVASPECT_CONTENT) &&
        (pformatetc->tymed & TYMED_HGLOBAL) &&
        (pformatetc->cfFormat == CF_UNICODETEXT))
      return S_OK;
    return DV_E_FORMATETC;
  }
  // Many drop targets (e.g. Explorer) enumerate the available formats before calling GetData
  HRESULT STDMETHODCALLTYPE EnumFormatEtc( DWORD dir, IEnumFORMATETC** ppenumFormatEtc) FL_OVERRIDE {
    if (!ppenumFormatEtc) return E_INVALIDARG;
    *ppenumFormatEtc = NULL;
    if (dir != DATADIR_GET) return E_NOTIMPL;
    FORMATETC fmt = { CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    return SHCreateStdEnumFmtEtc(1, &fmt, ppenumFormatEtc);
  }

  // all the following methods are not really needed for a DnD object
  HRESULT STDMETHODCALLTYPE GetDataHere( FORMATETC* /*pformatetcIn*/, STGMEDIUM* /*pmedium*/) FL_OVERRIDE { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc( FORMATETC* /*in*/, FORMATETC* /*out*/) FL_OVERRIDE { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetData( FORMATETC* /*pformatetc*/, STGMEDIUM* /*pmedium*/, BOOL /*fRelease*/) FL_OVERRIDE { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE DAdvise( FORMATETC* /*pformatetc*/, DWORD /*advf*/,
      IAdviseSink* /*pAdvSink*/, DWORD* /*pdwConnection*/) FL_OVERRIDE { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE DUnadvise( DWORD /*dwConnection*/) FL_OVERRIDE { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE EnumDAdvise( IEnumSTATDATA** /*ppenumAdvise*/) FL_OVERRIDE { return E_NOTIMPL; }
};


int Fl_WinAPI_Screen_Driver::dnd(int unused)
{
  DWORD dropEffect;
  ReleaseCapture();

  FLDataObject *fdo = new FLDataObject;
  fdo->AddRef();
  FLDropSource *fds = new FLDropSource;
  fds->AddRef();

  HRESULT ret = DoDragDrop( fdo, fds, DROPEFFECT_MOVE|DROPEFFECT_LINK|DROPEFFECT_COPY, &dropEffect );

  fdo->Release();
  fds->Release();

  Fl_Widget *w = Fl::pushed();
  if ( w )
  {
    int old_event = Fl::e_number;
    w->handle(Fl::e_number = FL_RELEASE);
    Fl::e_number = old_event;
    Fl::pushed( 0 );
  }
  if ( ret==DRAGDROP_S_DROP ) return 1; // or DD_S_CANCEL
  return 0;
}


// ---- Typed drag and drop with Fl::dnd(Fl_Drag_Data*) ----

// State of a drag started with Fl::dnd(Fl_Drag_Data*), shared by Fl_WinAPI_Screen_Driver::dnd_data()
// and the data object, which a drop target may keep after DoDragDrop() has returned.
// When the last reference is gone, the callback of the Fl_Drag_Data object runs
// and the object is deleted.
struct FLDragState {
  Fl_Drag_Data *data;
  int action;
  FLDragState(Fl_Drag_Data *d) : data(d), action(FL_DND_NONE) {}
  ~FLDragState() {
    data->do_callback(action);
    delete data;
  }
};

static std::wstring utf8_to_wide(const char *s, size_t len) {
  unsigned n = fl_utf8toUtf16(s, (unsigned)len, NULL, 0);
  std::wstring w(n + 1, L'\0');
  fl_utf8toUtf16(s, (unsigned)len, (unsigned short *)&w[0], n + 1);
  w.resize(n);
  return w;
}

// Reads a whole file into bytes, returns 0 on success
static int read_file(const char *path, std::string &bytes) {
  FILE *f = fl_fopen(path, "rb");
  if (!f) return -1;
  char buffer[16384];
  size_t n;
  bytes.clear();
  while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0) bytes.append(buffer, n);
  int err = ferror(f);
  fclose(f);
  return err ? -1 : 0;
}

// Returns the pixels of img as a packed DIB (BITMAPINFOHEADER followed by bottom-up
// 24-bit BGR rows), composited over white
static std::string image_to_dib(const Fl_RGB_Image *img) {
  int w = img->data_w(), h = img->data_h(), d = img->d();
  int ld = img->ld() ? img->ld() : w * d;
  int row = ((w * 3 + 3) / 4) * 4;
  BITMAPINFOHEADER bih;
  memset(&bih, 0, sizeof(bih));
  bih.biSize = sizeof(bih);
  bih.biWidth = w;
  bih.biHeight = h;
  bih.biPlanes = 1;
  bih.biBitCount = 24;
  bih.biCompression = BI_RGB;
  bih.biSizeImage = row * h;
  std::string dib((const char *)&bih, sizeof(bih));
  dib.resize(sizeof(bih) + row * h, '\0');
  unsigned char *bits = (unsigned char *)&dib[sizeof(bih)];
  for (int y = 0; y < h; y++) {
    const uchar *p = img->array + y * ld;
    unsigned char *q = bits + (h - 1 - y) * row;
    for (int x = 0; x < w; x++, p += d) {
      unsigned r, g, b, a;
      if (d < 3) { r = g = b = p[0]; a = (d == 2 ? p[1] : 255); }
      else { r = p[0]; g = p[1]; b = p[2]; a = (d == 4 ? p[3] : 255); }
      *q++ = (unsigned char)((b * a + 255 * (255 - a)) / 255);
      *q++ = (unsigned char)((g * a + 255 * (255 - a)) / 255);
      *q++ = (unsigned char)((r * a + 255 * (255 - a)) / 255);
    }
  }
  return dib;
}

/**
 The data object of a drag started with Fl::dnd(Fl_Drag_Data*).
 Existing files are offered as CF_HDROP. If at least one file is a promise, all
 files are offered as virtual files (CFSTR_FILEDESCRIPTORW and CFSTR_FILECONTENTS),
 so that the drop target creates them and promises are written only when dropped.
 */
class FLTypedDataObject : public IDataObject
{
  LONG m_cRefCount;
  std::shared_ptr<FLDragState> state;
  struct Entry {
    FORMATETC fmt;
    std::string bytes;  // the data, unless file >= 0
    int file;           // >= 0: index of the file whose contents are delivered as a stream
  };
  std::vector<Entry> entries;
  CLIPFORMAT cf_file_contents;

  void add(CLIPFORMAT cf, const std::string &bytes, int file = -1, LONG lindex = -1,
           DWORD tymed = TYMED_HGLOBAL) {
    Entry e;
    FORMATETC fmt = { cf, NULL, DVASPECT_CONTENT, lindex, tymed };
    e.fmt = fmt;
    e.bytes = bytes;
    e.file = file;
    entries.push_back(e);
  }
  const Entry *find(const FORMATETC *f) const {
    if (!f || !(f->dwAspect & DVASPECT_CONTENT)) return NULL;
    for (size_t i = 0; i < entries.size(); i++) {
      const Entry &e = entries[i];
      if (e.fmt.cfFormat != f->cfFormat || !(e.fmt.tymed & f->tymed)) continue;
      if (e.file >= 0 && e.fmt.lindex != f->lindex) continue; // FileContents is selected by lindex
      return &e;
    }
    return NULL;
  }

public:
  FLTypedDataObject(const std::shared_ptr<FLDragState> &s) : m_cRefCount(1), state(s) {
    Fl_Drag_Data *data = state->data;
    cf_file_contents = (CLIPFORMAT)RegisterClipboardFormatW(L"FileContents");
    if (data->has_text()) {
      std::wstring w = utf8_to_wide(data->text().data(), data->text().length());
      add(CF_UNICODETEXT, std::string((const char *)w.c_str(), (w.length() + 1) * sizeof(wchar_t)));
    }
    bool promises = false;
    for (int i = 0; i < data->file_count(); i++) if (data->file_is_promise(i)) promises = true;
    if (data->file_count() && !promises) {
      // DROPFILES followed by a double-nul terminated list of wide character paths
      std::wstring list;
      for (int i = 0; i < data->file_count(); i++) {
        std::wstring path = utf8_to_wide(data->file_path(i), strlen(data->file_path(i)));
        for (size_t j = 0; j < path.length(); j++) if (path[j] == L'/') path[j] = L'\\';
        list += path;
        list += L'\0';
      }
      list += L'\0';
      DROPFILES df;
      memset(&df, 0, sizeof(df));
      df.pFiles = sizeof(DROPFILES);
      df.fWide = TRUE;
      std::string bytes((const char *)&df, sizeof(df));
      bytes.append((const char *)list.data(), list.length() * sizeof(wchar_t));
      add(CF_HDROP, bytes);
    } else if (data->file_count()) {
      int n = data->file_count();
      std::string bytes(sizeof(FILEGROUPDESCRIPTORW) + (n - 1) * sizeof(FILEDESCRIPTORW), '\0');
      FILEGROUPDESCRIPTORW *fgd = (FILEGROUPDESCRIPTORW *)&bytes[0];
      fgd->cItems = n;
      for (int i = 0; i < n; i++) {
        FILEDESCRIPTORW *fd = &fgd->fgd[i];
        fd->dwFlags = FD_PROGRESSUI;
        std::wstring name = utf8_to_wide(data->file_name(i), strlen(data->file_name(i)));
        wcsncpy(fd->cFileName, name.c_str(), MAX_PATH - 1);
        add(cf_file_contents, std::string(), i, i, TYMED_ISTREAM);
      }
      add((CLIPFORMAT)RegisterClipboardFormatW(L"FileGroupDescriptorW"), bytes);
    }
    if (data->image() && data->image()->array) {
      add(CF_DIB, image_to_dib(data->image()));
    }
    for (int i = 0; i < data->data_count(); i++) {
      std::string mime = data->data_mime_type(i);
      std::wstring wmime = utf8_to_wide(mime.c_str(), mime.length());
      add((CLIPFORMAT)RegisterClipboardFormatW(wmime.c_str()), data->data_bytes(i));
      if (mime == "image/png") // the name used by most Windows applications
        add((CLIPFORMAT)RegisterClipboardFormatW(L"PNG"), data->data_bytes(i));
    }
  }
  virtual ~FLTypedDataObject() { }
  bool empty() const { return entries.empty(); }

  HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, LPVOID *ppvObject ) FL_OVERRIDE {
    if (IID_IUnknown==riid || IID_IDataObject==riid) {
      *ppvObject = this;
      AddRef();
      return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
  }
  ULONG STDMETHODCALLTYPE AddRef() FL_OVERRIDE { return InterlockedIncrement(&m_cRefCount); }
  ULONG STDMETHODCALLTYPE Release() FL_OVERRIDE {
    LONG count = InterlockedDecrement(&m_cRefCount);
    if (count == 0) delete this;
    return count;
  }
  HRESULT STDMETHODCALLTYPE GetData( FORMATETC *pformatetcIn, STGMEDIUM *pmedium ) FL_OVERRIDE {
    if (!pformatetcIn || !pmedium) return E_INVALIDARG;
    const Entry *e = find(pformatetcIn);
    if (!e) return DV_E_FORMATETC;
    memset(pmedium, 0, sizeof(*pmedium));
    if (e->file < 0 && e->fmt.tymed == TYMED_ISTREAM) { // data stored by SetData()
      IStream *stream = NULL;
      if (CreateStreamOnHGlobal(NULL, TRUE, &stream) != S_OK) return E_OUTOFMEMORY;
      if (!e->bytes.empty()) stream->Write(e->bytes.data(), (ULONG)e->bytes.length(), NULL);
      LARGE_INTEGER zero;
      zero.QuadPart = 0;
      stream->Seek(zero, STREAM_SEEK_SET, NULL);
      pmedium->tymed = TYMED_ISTREAM;
      pmedium->pstm = stream;
      return S_OK;
    }
    if (e->file >= 0) {
      // write the file to a temporary location and deliver its contents as a stream
      std::string bytes;
      Fl_Drag_Data *data = state->data;
      if (data->file_is_promise(e->file)) {
        const char *tmp = fl_getenv("TEMP");
        if (!tmp || !*tmp) tmp = ".";
        char path[FL_PATH_MAX];
        snprintf(path, sizeof(path), "%s\\fltk-dnd-%lu-%d.tmp", tmp,
                 (unsigned long)GetCurrentProcessId(), e->file);
        int err = data->write_file(e->file, path);
        if (!err) err = read_file(path, bytes);
        fl_unlink(path);
        if (err) return E_FAIL;
      } else if (read_file(data->file_path(e->file), bytes)) {
        return E_FAIL;
      }
      IStream *stream = NULL;
      if (CreateStreamOnHGlobal(NULL, TRUE, &stream) != S_OK) return E_OUTOFMEMORY;
      ULONG written = 0;
      if (!bytes.empty()) stream->Write(bytes.data(), (ULONG)bytes.length(), &written);
      LARGE_INTEGER zero;
      zero.QuadPart = 0;
      stream->Seek(zero, STREAM_SEEK_SET, NULL);
      pmedium->tymed = TYMED_ISTREAM;
      pmedium->pstm = stream;
      return S_OK;
    }
    HGLOBAL gh = GlobalAlloc(GMEM_MOVEABLE, e->bytes.length() ? e->bytes.length() : 1);
    if (!gh) return E_OUTOFMEMORY;
    void *p = GlobalLock(gh);
    memcpy(p, e->bytes.data(), e->bytes.length());
    GlobalUnlock(gh);
    pmedium->tymed = TYMED_HGLOBAL;
    pmedium->hGlobal = gh;
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE QueryGetData( FORMATETC *pformatetc ) FL_OVERRIDE {
    if (!pformatetc) return E_INVALIDARG;
    if (pformatetc->cfFormat == cf_file_contents && pformatetc->lindex == -1) {
      // FileContents is enumerated with lindex -1 and requested with the file index
      for (size_t i = 0; i < entries.size(); i++) if (entries[i].file >= 0) return S_OK;
    }
    return find(pformatetc) ? S_OK : DV_E_FORMATETC;
  }
  HRESULT STDMETHODCALLTYPE EnumFormatEtc( DWORD dir, IEnumFORMATETC** ppenumFormatEtc) FL_OVERRIDE {
    if (!ppenumFormatEtc) return E_INVALIDARG;
    *ppenumFormatEtc = NULL;
    if (dir != DATADIR_GET) return E_NOTIMPL;
    std::vector<FORMATETC> formats;
    bool file_contents = false;
    for (size_t i = 0; i < entries.size(); i++) {
      if (entries[i].file >= 0) { // list FileContents once, with lindex -1
        if (file_contents) continue;
        file_contents = true;
        FORMATETC fmt = entries[i].fmt;
        fmt.lindex = -1;
        formats.push_back(fmt);
      } else {
        formats.push_back(entries[i].fmt);
      }
    }
    if (formats.empty()) return E_FAIL;
    return SHCreateStdEnumFmtEtc((UINT)formats.size(), &formats[0], ppenumFormatEtc);
  }
  HRESULT STDMETHODCALLTYPE GetDataHere( FORMATETC* /*pformatetcIn*/, STGMEDIUM* /*pmedium*/) FL_OVERRIDE { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc( FORMATETC* /*in*/, FORMATETC* out) FL_OVERRIDE {
    if (out) out->ptd = NULL;
    return E_NOTIMPL;
  }
  // The shell stores data of its own here, e.g. the drag image of IDragSourceHelper,
  // and drop targets may store their performed drop effect
  HRESULT STDMETHODCALLTYPE SetData( FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease) FL_OVERRIDE {
    if (!pformatetc || !pmedium) return E_INVALIDARG;
    std::string bytes;
    if (pmedium->tymed == TYMED_HGLOBAL) {
      const char *p = (const char *)GlobalLock(pmedium->hGlobal);
      if (p) bytes.assign(p, GlobalSize(pmedium->hGlobal));
      GlobalUnlock(pmedium->hGlobal);
    } else if (pmedium->tymed == TYMED_ISTREAM && pmedium->pstm) {
      LARGE_INTEGER zero;
      zero.QuadPart = 0;
      pmedium->pstm->Seek(zero, STREAM_SEEK_SET, NULL);
      char buffer[4096];
      HRESULT hr;
      ULONG n;
      do {
        n = 0;
        hr = pmedium->pstm->Read(buffer, sizeof(buffer), &n);
        bytes.append(buffer, n);
      } while (hr == S_OK && n > 0);
    } else {
      return DV_E_TYMED;
    }
    DWORD tymed = pmedium->tymed;
    if (fRelease) ReleaseStgMedium(pmedium);
    for (size_t i = 0; i < entries.size(); i++) {
      if (entries[i].file < 0 && entries[i].fmt.cfFormat == pformatetc->cfFormat) {
        entries[i].bytes = bytes;
        entries[i].fmt.tymed = tymed;
        return S_OK;
      }
    }
    add(pformatetc->cfFormat, bytes, -1, -1, tymed);
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE DAdvise( FORMATETC* /*pformatetc*/, DWORD /*advf*/,
      IAdviseSink* /*pAdvSink*/, DWORD* /*pdwConnection*/) FL_OVERRIDE { return OLE_E_ADVISENOTSUPPORTED; }
  HRESULT STDMETHODCALLTYPE DUnadvise( DWORD /*dwConnection*/) FL_OVERRIDE { return OLE_E_ADVISENOTSUPPORTED; }
  HRESULT STDMETHODCALLTYPE EnumDAdvise( IEnumSTATDATA** /*ppenumAdvise*/) FL_OVERRIDE { return OLE_E_ADVISENOTSUPPORTED; }
};


// Returns a 32-bit top-down DIB section with premultiplied alpha, as required by
// IDragSourceHelper, showing img with at most max_size pixels in each direction
static HBITMAP drag_bitmap(const Fl_RGB_Image *img, int max_size, SIZE *size) {
  int w = img->data_w(), h = img->data_h();
  Fl_RGB_Image *copy = NULL;
  if (w > max_size || h > max_size) { // make a thumbnail
    float f = float(max_size) / (w > h ? w : h);
    copy = (Fl_RGB_Image *)img->copy(int(w * f + 0.5f), int(h * f + 0.5f));
    img = copy;
    w = img->data_w();
    h = img->data_h();
  }
  int d = img->d(), ld = img->ld() ? img->ld() : w * d;
  BITMAPINFO bmi;
  memset(&bmi, 0, sizeof(bmi));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = w;
  bmi.bmiHeader.biHeight = -h; // top-down
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  void *bits = NULL;
  HBITMAP hbm = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
  if (hbm && bits && img->array) {
    unsigned char *q = (unsigned char *)bits;
    for (int y = 0; y < h; y++) {
      const uchar *p = img->array + y * ld;
      for (int x = 0; x < w; x++, p += d, q += 4) {
        unsigned r, g, b, a;
        if (d < 3) { r = g = b = p[0]; a = (d == 2 ? p[1] : 255); }
        else { r = p[0]; g = p[1]; b = p[2]; a = (d == 4 ? p[3] : 255); }
        q[0] = (unsigned char)(b * a / 255);
        q[1] = (unsigned char)(g * a / 255);
        q[2] = (unsigned char)(r * a / 255);
        q[3] = (unsigned char)a;
      }
    }
  }
  size->cx = w;
  size->cy = h;
  delete copy;
  return hbm;
}

int Fl_WinAPI_Screen_Driver::dnd_data(Fl_Drag_Data *data)
{
  // from now on, data is deleted when the last reference to state is gone
  std::shared_ptr<FLDragState> state(new FLDragState(data));
  FLTypedDataObject *fdo = new FLTypedDataObject(state);
  if (fdo->empty()) {
    fdo->Release();
    return 0;
  }
  DWORD allowed = 0;
  if (data->actions() & FL_DND_COPY) allowed |= DROPEFFECT_COPY;
  if (data->actions() & FL_DND_MOVE) allowed |= DROPEFFECT_MOVE;
  if (data->actions() & FL_DND_LINK) allowed |= DROPEFFECT_LINK;
  if (!allowed) allowed = DROPEFFECT_COPY;

  // show the drag image, or a thumbnail of a dragged image
  IDragSourceHelper *helper = NULL;
  const Fl_RGB_Image *icon = data->drag_image() ? data->drag_image() : data->image();
  if (icon && icon->array &&
      CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                       IID_IDragSourceHelper, (void **)&helper) == S_OK) {
    SHDRAGIMAGE sdi;
    memset(&sdi, 0, sizeof(sdi));
    sdi.hbmpDragImage = drag_bitmap(icon, data->drag_image() ? 1024 : 128, &sdi.sizeDragImage);
    if (data->drag_image()) { // the hot spot, in pixels of the bitmap
      sdi.ptOffset.x = data->drag_image_hot_x() * sdi.sizeDragImage.cx / icon->w();
      sdi.ptOffset.y = data->drag_image_hot_y() * sdi.sizeDragImage.cy / icon->h();
    } else {
      sdi.ptOffset.x = sdi.sizeDragImage.cx / 2;
      sdi.ptOffset.y = sdi.sizeDragImage.cy / 2;
    }
    sdi.crColorKey = 0xFFFFFFFF; // CLR_NONE: use the alpha channel
    // on success, the helper owns the bitmap
    if (!sdi.hbmpDragImage || helper->InitializeFromBitmap(&sdi, fdo) != S_OK) {
      if (sdi.hbmpDragImage) DeleteObject(sdi.hbmpDragImage);
    }
  }

  ReleaseCapture();
  FLDropSource *fds = new FLDropSource;
  fds->AddRef();
  DWORD dropEffect = DROPEFFECT_NONE;
  HRESULT ret = DoDragDrop( fdo, fds, allowed, &dropEffect );
  if (helper) helper->Release();
  int action = FL_DND_NONE;
  if (ret == DRAGDROP_S_DROP) {
    if (dropEffect & DROPEFFECT_MOVE) action = FL_DND_MOVE;
    else if (dropEffect & DROPEFFECT_COPY) action = FL_DND_COPY;
    else if (dropEffect & DROPEFFECT_LINK) action = FL_DND_LINK;
  }
  state->action = action;
  fdo->Release();
  fds->Release();
  state.reset(); // runs the callback unless the drop target still holds the data object

  Fl_Widget *w = Fl::pushed();
  if ( w )
  {
    int old_event = Fl::e_number;
    w->handle(Fl::e_number = FL_RELEASE);
    Fl::e_number = old_event;
    Fl::pushed( 0 );
  }
  return action != FL_DND_NONE;
}
