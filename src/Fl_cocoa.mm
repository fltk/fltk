//
// MacOS-Cocoa specific code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     https://www.fltk.org/str.php
//

//// From the inner edge of a MetroWerks CodeWarrior CD:
// (without permission)
//
// "Three Compiles for 68Ks under the sky,
// Seven Compiles for PPCs in their fragments of code,
// Nine Compiles for Mortal Carbon doomed to die,
// One Compile for Mach-O Cocoa on its Mach-O throne,
// in the Land of MacOS X where the Drop-Shadows lie.
// 
// One Compile to link them all, One Compile to merge them,
// One Compile to copy them all and in the bundle bind them,
// in the Land of MacOS X where the Drop-Shadows lie."

#ifdef __APPLE__

#ifndef FLTK_CONSOLIDATE_MOTION
#define FLTK_CONSOLIDATE_MOTION 0
#endif

extern "C" {
#include <pthread.h>
}

#include <FL/x.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Printer.H>
#include <FL/Fl_Copy_Surface.H>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>
#include <dlfcn.h>
#include <string.h>

#import <Cocoa/Cocoa.h>

// #define DEBUG_SELECT		// UNCOMMENT FOR SELECT()/THREAD DEBUGGING
#ifdef DEBUG_SELECT
#include <stdio.h>		// testing
#define DEBUGMSG(msg)		if ( msg ) fprintf(stderr, msg);
#define DEBUGPERRORMSG(msg)	if ( msg ) perror(msg)
#define DEBUGTEXT(txt)		txt
#else
#define DEBUGMSG(msg)
#define DEBUGPERRORMSG(msg)
#define DEBUGTEXT(txt)		NULL
#endif /*DEBUG_SELECT*/

// external functions
extern void fl_fix_focus();
extern unsigned short *fl_compute_macKeyLookUp();
extern int fl_send_system_handlers(void *e);

// forward definition of functions in this file
// converting cr lf converter function
static size_t convert_crlf(char * string, size_t len);
static void createAppleMenu(void);
static void cocoaMouseHandler(NSEvent *theEvent);
static void clipboard_check(void);
static NSBitmapImageRep* rect_to_NSBitmapImageRep_subwins(Fl_Window *win, int x, int y, int w, int h, bool capture_subwins);
static void drain_dropped_files_list(void);
static NSPoint FLTKtoCocoa(Fl_Window *win, int x, int y, int H);
static int get_window_frame_sizes(const Fl_Window *win, int *pbx = NULL, int *pby = NULL);

int fl_mac_os_version = Fl_X::calc_mac_os_version();		// the version number of the running Mac OS X (e.g., 100604 for 10.6.4)
int fl_mac_quit_early = 1; // set it to 0 so cmd-Q does not terminate app but merely terminates the event loop

// public variables
CGContextRef fl_gc = 0;
void *fl_capture = 0;			// (NSWindow*) we need this to compensate for a missing(?) mouse capture
bool fl_show_iconic;                    // true if called from iconize() - shows the next created window in collapsed state
//int fl_disable_transient_for;           // secret method of removing TRANSIENT_FOR
Window fl_window;
Fl_Window *Fl_Window::current_;

// forward declarations of variables in this file
static int got_events = 0;
static int main_screen_height; // height of menubar-containing screen used to convert between Cocoa and FLTK global screen coordinates
// through_drawRect = YES means the drawRect: message was sent to the view, 
// thus the graphics context was prepared by the system
static BOOL through_drawRect = NO; 
// through_Fl_X_flush = YES means Fl_X::flush() was called
static BOOL through_Fl_X_flush = NO;
static BOOL views_use_CA = NO; // YES means views are layer-backed, as on macOS 10.14 when linked with SDK 10.14
static int im_enabled = -1;
// OS version-dependent pasteboard type names
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_6
#define NSPasteboardTypeTIFF @"public.tiff"
#define NSPasteboardTypePDF @"com.adobe.pdf"
#define NSPasteboardTypeString @"public.utf8-plain-text"
#endif
static NSString *TIFF_pasteboard_type = (fl_mac_os_version >= 100600 ? NSPasteboardTypeTIFF : NSTIFFPboardType);
static NSString *PDF_pasteboard_type = (fl_mac_os_version >= 100600 ? NSPasteboardTypePDF : NSPDFPboardType);
static NSString *PICT_pasteboard_type = (fl_mac_os_version >= 100600 ? @"com.apple.pict" : NSPICTPboardType);
static NSString *UTF8_pasteboard_type = (fl_mac_os_version >= 100600 ? NSPasteboardTypeString : NSStringPboardType);
static bool in_nsapp_run = false; // true during execution of [NSApp run]
static NSMutableArray *dropped_files_list = nil; // list of files dropped at app launch
typedef void (*open_cb_f_type)(const char *);
static Fl_Window *starting_moved_window = NULL; // the moved window which brings its subwins with it
#if FLTK_CONSOLIDATE_MOTION
static Fl_Window* send_motion;
extern Fl_Window* fl_xmousewin;
#endif

enum { FLTKTimerEvent = 1, FLTKDataReadyEvent };

// Carbon functions and definitions

typedef void *TSMDocumentID;

extern "C" enum {
 kTSMDocumentEnabledInputSourcesPropertyTag = 'enis' //  from Carbon/TextServices.h
};

// Undocumented voodoo. Taken from Mozilla.
static const int smEnableRomanKybdsOnly = -23;

typedef TSMDocumentID (*TSMGetActiveDocument_type)(void);
static TSMGetActiveDocument_type TSMGetActiveDocument;
typedef OSStatus (*TSMSetDocumentProperty_type)(TSMDocumentID, OSType, UInt32, void*);
static TSMSetDocumentProperty_type TSMSetDocumentProperty;
typedef OSStatus (*TSMRemoveDocumentProperty_type)(TSMDocumentID, OSType);
static TSMRemoveDocumentProperty_type TSMRemoveDocumentProperty;
typedef CFArrayRef (*TISCreateInputSourceList_type)(CFDictionaryRef, Boolean);
static TISCreateInputSourceList_type TISCreateInputSourceList;
static CFStringRef kTISTypeKeyboardLayout;
static CFStringRef kTISPropertyInputSourceType;

typedef void (*KeyScript_type)(short);
static KeyScript_type KeyScript;


/* fltk-utf8 placekeepers */
void fl_reset_spot()
{
}

void fl_set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
{
}

void fl_set_status(int x, int y, int w, int h)
{
}

/*
 * Mac keyboard lookup table
 */
static unsigned short* macKeyLookUp = NULL;

/*
 * convert the current mouse chord into the FLTK modifier state
 */
static unsigned int mods_to_e_state( NSUInteger mods )
{
  long state = 0;
  if ( mods & NSCommandKeyMask ) state |= FL_META;
  if ( mods & NSAlternateKeyMask ) state |= FL_ALT;
  if ( mods & NSControlKeyMask ) state |= FL_CTRL;
  if ( mods & NSShiftKeyMask ) state |= FL_SHIFT;
  if ( mods & NSAlphaShiftKeyMask ) state |= FL_CAPS_LOCK;
  unsigned int ret = ( Fl::e_state & 0xff000000 ) | state;
  Fl::e_state = ret;
  //printf( "State 0x%08x (%04x)\n", Fl::e_state, mods );
  return ret;
}

// these pointers are set by the Fl::lock() function:
static void nothing() {}
void (*fl_lock_function)() = nothing;
void (*fl_unlock_function)() = nothing;

//
// Select interface -- how it's implemented:
//     When the user app configures one or more file descriptors to monitor
//     with Fl::add_fd(), we start a separate thread to select() the  data,
//     sending a custom OSX 'FLTK data ready event' to the parent  thread's
//     RunApplicationLoop(), so that it triggers the data  ready  callbacks
//     in the parent thread.                               -erco 04/04/04
//     
#define POLLIN  1
#define POLLOUT 4
#define POLLERR 8

// Class to handle select() 'data ready'
class DataReady
{
  struct FD
  {
    int fd;
    short events;
    void (*cb)(int, void*);
    void* arg;
  };
  int nfds, fd_array_size;
  FD *fds;
  pthread_t tid;		// select()'s thread id
  
  // Data that needs to be locked (all start with '_')
  pthread_mutex_t _datalock;	// data lock
  fd_set _fdsets[3];		// r/w/x sets user wants to monitor
  int _maxfd;			// max fd count to monitor
  int _cancelpipe[2];		// pipe used to help cancel thread
  
public:
  DataReady()
  {
    nfds = 0;
    fd_array_size = 0;
    fds = 0;
    tid = 0;
    
    pthread_mutex_init(&_datalock, NULL);
    FD_ZERO(&_fdsets[0]); FD_ZERO(&_fdsets[1]); FD_ZERO(&_fdsets[2]);
    _cancelpipe[0] = _cancelpipe[1] = 0;
    _maxfd = -1;
  }
  
  ~DataReady()
  {
    CancelThread(DEBUGTEXT("DESTRUCTOR\n"));
    if (fds) { free(fds); fds = 0; }
    nfds = 0;
  }
  
  // Locks
  //    The convention for locks: volatile vars start with '_',
  //    and must be locked before use. Locked code is prefixed 
  //    with /*LOCK*/ to make painfully obvious esp. in debuggers. -erco
  //
  void DataLock() { pthread_mutex_lock(&_datalock); }
  void DataUnlock() { pthread_mutex_unlock(&_datalock); }
  
  // Accessors
  int IsThreadRunning() { return(tid ? 1 : 0); }
  int GetNfds() { return(nfds); }
  int GetCancelPipe(int ix) { return(_cancelpipe[ix]); }
  fd_set GetFdset(int ix) { return(_fdsets[ix]); }
  
  // Methods
  void AddFD(int n, int events, void (*cb)(int, void*), void *v);
  void RemoveFD(int n, int events);
  int CheckData(fd_set& r, fd_set& w, fd_set& x);
  void HandleData(fd_set& r, fd_set& w, fd_set& x);
  static void* DataReadyThread(void *self);
  void StartThread(void);
  void CancelThread(const char *reason);
};

static DataReady dataready;

void DataReady::AddFD(int n, int events, void (*cb)(int, void*), void *v)
{
  RemoveFD(n, events);
  int i = nfds++;
  if (i >= fd_array_size) 
  {
    fl_open_display(); // necessary for NSApp to be defined and the event loop to work
    FD *temp;
    fd_array_size = 2*fd_array_size+1;
    if (!fds) { temp = (FD*)malloc(fd_array_size*sizeof(FD)); }
    else { temp = (FD*)realloc(fds, fd_array_size*sizeof(FD)); }
    if (!temp) return;
    fds = temp;
  }
  fds[i].cb  = cb;
  fds[i].arg = v;
  fds[i].fd  = n;
  fds[i].events = events;
  DataLock();
  /*LOCK*/  if (events & POLLIN)  FD_SET(n, &_fdsets[0]);
  /*LOCK*/  if (events & POLLOUT) FD_SET(n, &_fdsets[1]);
  /*LOCK*/  if (events & POLLERR) FD_SET(n, &_fdsets[2]);
  /*LOCK*/  if (n > _maxfd) _maxfd = n;
  DataUnlock();
}

// Remove an FD from the array
void DataReady::RemoveFD(int n, int events)
{
  int i,j;
  _maxfd = -1; // recalculate maxfd on the fly
  for (i=j=0; i<nfds; i++) {
    if (fds[i].fd == n) {
      int e = fds[i].events & ~events;
      if (!e) continue; // if no events left, delete this fd
      fds[i].events = e;
    }
    if (fds[i].fd > _maxfd) _maxfd = fds[i].fd;
    // move it down in the array if necessary:
    if (j<i) {
      fds[j] = fds[i];
    }
    j++;
  }
  nfds = j;
  DataLock();
  /*LOCK*/  if (events & POLLIN)  FD_CLR(n, &_fdsets[0]);
  /*LOCK*/  if (events & POLLOUT) FD_CLR(n, &_fdsets[1]);
  /*LOCK*/  if (events & POLLERR) FD_CLR(n, &_fdsets[2]);
  DataUnlock();
}

// CHECK IF USER DATA READY, RETURNS r/w/x INDICATING WHICH IF ANY
int DataReady::CheckData(fd_set& r, fd_set& w, fd_set& x)
{
  int ret;
  DataLock();
  /*LOCK*/  timeval t = { 0, 1 };		// quick check
  /*LOCK*/  r = _fdsets[0], w = _fdsets[1], x = _fdsets[2];
  /*LOCK*/  ret = ::select(_maxfd+1, &r, &w, &x, &t);
  DataUnlock();
  if ( ret == -1 ) {
    DEBUGPERRORMSG("CheckData(): select()");
  }
  return(ret);
}

// HANDLE DATA READY CALLBACKS
void DataReady::HandleData(fd_set& r, fd_set& w, fd_set& x)
{
  for (int i=0; i<nfds; i++) {
    int f = fds[i].fd;
    short revents = 0;
    if (FD_ISSET(f, &r)) revents |= POLLIN;
    if (FD_ISSET(f, &w)) revents |= POLLOUT;
    if (FD_ISSET(f, &x)) revents |= POLLERR;
    if (fds[i].events & revents) {
      DEBUGMSG("DOING CALLBACK: ");
      fds[i].cb(f, fds[i].arg);
      DEBUGMSG("DONE\n");
    }
  }
}

// DATA READY THREAD
//    This thread watches for changes in user's file descriptors.
//    Sends a 'data ready event' to the main thread if any change.
//
void* DataReady::DataReadyThread(void *o)
{
  DataReady *self = (DataReady*)o;
  while ( 1 ) {					// loop until thread cancel or error
    // Thread safe local copies of data before each select()
    self->DataLock();
    /*LOCK*/  int maxfd = self->_maxfd;
    /*LOCK*/  fd_set r = self->GetFdset(0);
    /*LOCK*/  fd_set w = self->GetFdset(1);
    /*LOCK*/  fd_set x = self->GetFdset(2);
    /*LOCK*/  int cancelpipe = self->GetCancelPipe(0);
    /*LOCK*/  if ( cancelpipe > maxfd ) maxfd = cancelpipe;
    /*LOCK*/  FD_SET(cancelpipe, &r);		// add cancelpipe to fd's to watch
    /*LOCK*/  FD_SET(cancelpipe, &x);
    self->DataUnlock();
    // timeval t = { 1000, 0 };	// 1000 seconds;
    timeval t = { 2, 0 };	// HACK: 2 secs prevents 'hanging' problem
    int ret = ::select(maxfd+1, &r, &w, &x, &t);
    pthread_testcancel();	// OSX 10.0.4 and older: needed for parent to cancel
    switch ( ret ) {
      case 0:	// NO DATA
        continue;
      case -1:	// ERROR
      {
        DEBUGPERRORMSG("CHILD THREAD: select() failed");
        return(NULL);		// error? exit thread
      }
      default:	// DATA READY
      {
        if (FD_ISSET(cancelpipe, &r) || FD_ISSET(cancelpipe, &x)) 	// cancel?
	  { return(NULL); }						// just exit
        DEBUGMSG("CHILD THREAD: DATA IS READY\n");
        NSAutoreleasePool *localPool = [[NSAutoreleasePool alloc] init];
        NSEvent *event = [NSEvent otherEventWithType:NSApplicationDefined
                                            location:NSMakePoint(0,0)
                                       modifierFlags:0
                                           timestamp:0
                                        windowNumber:0 context:NULL subtype:FLTKDataReadyEvent data1:0 data2:0];
        [NSApp postEvent:event atStart:NO];
        [localPool release];
        return(NULL);		// done with thread
      }
    }
  }
}

// START 'DATA READY' THREAD RUNNING, CREATE INTER-THREAD PIPE
void DataReady::StartThread(void)
{
  CancelThread(DEBUGTEXT("STARTING NEW THREAD\n"));
  DataLock();
  /*LOCK*/  pipe(_cancelpipe);	// pipe for sending cancel msg to thread
  DataUnlock();
  DEBUGMSG("*** START THREAD\n");
  pthread_create(&tid, NULL, DataReadyThread, (void*)this);
}

// CANCEL 'DATA READY' THREAD, CLOSE PIPE
void DataReady::CancelThread(const char *reason)
{
  if ( tid ) {
    DEBUGMSG("*** CANCEL THREAD: ");
    DEBUGMSG(reason);
    if ( pthread_cancel(tid) == 0 ) {		// cancel first
      DataLock();
      /*LOCK*/  write(_cancelpipe[1], "x", 1);	// wake thread from select
      DataUnlock();
      pthread_join(tid, NULL);			// wait for thread to finish
    }
    tid = 0;
    DEBUGMSG("(JOINED) OK\n");
  }
  // Close pipe if open
  DataLock();
  /*LOCK*/  if ( _cancelpipe[0] ) { close(_cancelpipe[0]); _cancelpipe[0] = 0; }
  /*LOCK*/  if ( _cancelpipe[1] ) { close(_cancelpipe[1]); _cancelpipe[1] = 0; }
  DataUnlock();
}

void Fl::add_fd( int n, int events, void (*cb)(int, void*), void *v )
{
  dataready.AddFD(n, events, cb, v);
}

void Fl::add_fd(int fd, void (*cb)(int, void*), void* v)
{
  dataready.AddFD(fd, POLLIN, cb, v);
}

void Fl::remove_fd(int n, int events)
{
  dataready.RemoveFD(n, events);
}

void Fl::remove_fd(int n)
{
  dataready.RemoveFD(n, -1);
}

/*
 * Check if there is actually a message pending
 */
int fl_ready()
{
  NSEvent *retval = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate dateWithTimeIntervalSinceNow:0]
				    inMode:NSDefaultRunLoopMode dequeue:NO];
  return retval != nil;
}


static void processFLTKEvent(void) {
  fl_lock_function();
  dataready.CancelThread(DEBUGTEXT("DATA READY EVENT\n"));
  
  // CHILD THREAD TELLS US DATA READY
  //     Check to see what's ready, and invoke user's cb's
  //
  fd_set r,w,x;
  switch(dataready.CheckData(r,w,x)) {
    case 0:	// NO DATA
      break;
    case -1:	// ERROR
      break;
    default:	// DATA READY
      dataready.HandleData(r,w,x);
      break;
  }
  fl_unlock_function();
  return;
}


/*
 * break the current event loop
 */
static void breakMacEventLoop()
{  
  NSEvent *event = [NSEvent otherEventWithType:NSApplicationDefined location:NSMakePoint(0,0)
                                 modifierFlags:0 timestamp:0
                                  windowNumber:0 context:NULL subtype:FLTKTimerEvent data1:0 data2:0];
  [NSApp postEvent:event atStart:NO];
}

//
// MacOS X timers
//

struct MacTimeout {
  Fl_Timeout_Handler callback;
  void* data;
  CFRunLoopTimerRef timer;
  char pending; 
  CFAbsoluteTime next_timeout; // scheduled time for this timer
};
static MacTimeout* mac_timers;
static int mac_timer_alloc;
static int mac_timer_used;
static MacTimeout* current_timer;  // the timer that triggered its callback function

static void realloc_timers()
{
  if (mac_timer_alloc == 0) {
    mac_timer_alloc = 8;
    fl_open_display(); // needed because the timer creates an event
  }
  mac_timer_alloc *= 2;
  MacTimeout* new_timers = new MacTimeout[mac_timer_alloc];
  memset(new_timers, 0, sizeof(MacTimeout)*mac_timer_alloc);
  memcpy(new_timers, mac_timers, sizeof(MacTimeout) * mac_timer_used);
  if (current_timer) {
    MacTimeout* newCurrent = new_timers + (current_timer - mac_timers);
    current_timer = newCurrent;
  }
  MacTimeout* delete_me = mac_timers;
  mac_timers = new_timers;
  delete [] delete_me;
}

static void delete_timer(MacTimeout& t)
{
  if (t.timer) {
    CFRunLoopRemoveTimer(CFRunLoopGetCurrent(),
		      t.timer,
		      kCFRunLoopDefaultMode);
    CFRelease(t.timer);
    memset(&t, 0, sizeof(MacTimeout));
  }
}

static void do_timer(CFRunLoopTimerRef timer, void* data)
{
  fl_lock_function();
  fl_intptr_t timerId = (fl_intptr_t)data;
  current_timer = &mac_timers[timerId];
  current_timer->pending = 0;
  (current_timer->callback)(current_timer->data);
  if (current_timer && current_timer->pending == 0)
    delete_timer(*current_timer);
  current_timer = NULL;

  breakMacEventLoop();
  fl_unlock_function();
}

void Fl::add_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
  // always create a new timer entry
  fl_intptr_t timer_id = -1;
  // find an empty slot in the timer array
  for (int i = 0; i < mac_timer_used; ++i) {
    if ( !mac_timers[i].timer ) {
      timer_id = i;
      break;
    }
  }
  // if there was no empty slot, append a new timer
  if (timer_id == -1) {
    // make space if needed
    if (mac_timer_used == mac_timer_alloc) {
      realloc_timers();
    }
    timer_id = mac_timer_used++;
  }
  // now install a brand new timer
  MacTimeout& t = mac_timers[timer_id];
  CFRunLoopTimerContext context = {0, (void*)timer_id, NULL,NULL,NULL};
  CFRunLoopTimerRef timerRef = CFRunLoopTimerCreate(kCFAllocatorDefault, 
						    CFAbsoluteTimeGetCurrent() + time,
						    1E30,  
						    0,
						    0,
						    do_timer,
						    &context
						    );
  if (timerRef) {
    CFRunLoopAddTimer(CFRunLoopGetCurrent(),
		      timerRef,
		      kCFRunLoopDefaultMode);
    t.callback = cb;
    t.data     = data;
    t.timer    = timerRef;
    t.pending  = 1;
    t.next_timeout = CFRunLoopTimerGetNextFireDate(timerRef);
  }
}

void Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
  if (!current_timer) {
    add_timeout(time, cb, data);
    return;
  }
  // k = how many times 'time' seconds after the last scheduled timeout until the future
  double k = ceil( (CFAbsoluteTimeGetCurrent() - current_timer->next_timeout) / time);
  if (k < 1) k = 1;
  current_timer->next_timeout += k * time;
  CFRunLoopTimerSetNextFireDate(current_timer->timer, current_timer->next_timeout );
  current_timer->callback = cb;
  current_timer->data = data;
  current_timer->pending = 1;
}

int Fl::has_timeout(Fl_Timeout_Handler cb, void* data)
{
  for (int i = 0; i < mac_timer_used; ++i) {
    MacTimeout& t = mac_timers[i];
    if (t.callback == cb  &&  t.data == data && t.pending) {
      return 1;
    }
  }
  return 0;
}

void Fl::remove_timeout(Fl_Timeout_Handler cb, void* data)
{
  for (int i = 0; i < mac_timer_used; ++i) {
    MacTimeout& t = mac_timers[i];
    if (t.callback == cb  && ( t.data == data || data == NULL)) {
      delete_timer(t);
    }
  }
}

@interface FLWindow : NSWindow {
  Fl_Window *w;
}
- (FLWindow*)initWithFl_W:(Fl_Window *)flw
	      contentRect:(NSRect)rect 
		styleMask:(NSUInteger)windowStyle;
- (Fl_Window *)getFl_Window;
- (void)recursivelySendToSubwindows:(SEL)sel applyToSelf:(BOOL)b;
- (void)setSubwindowFrame;
- (void)checkSubwindowFrame;
- (void)waitForExpose;
- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen *)screen;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
- (NSPoint)convertBaseToScreen:(NSPoint)aPoint;
#endif
@end


@interface FLView : NSView <NSTextInput
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
, NSTextInputClient
#endif
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
,NSDraggingSource
#endif
> {
  BOOL in_key_event; // YES means keypress is being processed by handleEvent
  BOOL need_handle; // YES means Fl::handle(FL_KEYBOARD,) is needed after handleEvent processing
  NSInteger identifier;
  NSRange selectedRange;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
@public
  CGContextRef aux_bitmap; // stores a copy of the last content of the view, also used to draw outside drawRect:
#endif
}
+ (void)prepareEtext:(NSString*)aString;
+ (void)concatEtext:(NSString*)aString;
- (BOOL)process_keydown:(NSEvent*)theEvent;
- (id)initWithFrame:(NSRect)frameRect;
- (void)drawRect:(NSRect)rect;
- (BOOL)acceptsFirstResponder;
- (BOOL)acceptsFirstMouse:(NSEvent*)theEvent;
- (void)resetCursorRects;
- (BOOL)performKeyEquivalent:(NSEvent*)theEvent;
- (void)mouseUp:(NSEvent *)theEvent;
- (void)rightMouseUp:(NSEvent *)theEvent;
- (void)otherMouseUp:(NSEvent *)theEvent;
- (void)mouseDown:(NSEvent *)theEvent;
- (void)rightMouseDown:(NSEvent *)theEvent;
- (void)otherMouseDown:(NSEvent *)theEvent;
- (void)mouseMoved:(NSEvent *)theEvent;
- (void)mouseDragged:(NSEvent *)theEvent;
- (void)rightMouseDragged:(NSEvent *)theEvent;
- (void)otherMouseDragged:(NSEvent *)theEvent;
- (void)scrollWheel:(NSEvent *)theEvent;
- (void)magnifyWithEvent:(NSEvent *)theEvent;
- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;
- (void)flagsChanged:(NSEvent *)theEvent;
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender;
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
- (void)draggingExited:(id < NSDraggingInfo >)sender;
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal;
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange;
- (void)setMarkedText:(id)aString selectedRange:(NSRange)newSelection replacementRange:(NSRange)replacementRange;
- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange;
- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange;
- (NSInteger)windowLevel;
#endif
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context;
- (void)draggingSession:(NSDraggingSession *)session
           endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation;
#endif
- (BOOL)did_view_resolution_change;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
- (void)create_aux_bitmap:(CGContextRef)gc retina:(BOOL)r;
- (void)reset_aux_bitmap;
#endif
@end


@implementation FLWindow
- (void)close
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
  if (views_use_CA) [(FLView*)[self contentView] reset_aux_bitmap];
#endif
  [[self standardWindowButton:NSWindowDocumentIconButton] setImage:nil];
  [super close];
  // when a fullscreen window is closed, windowDidResize may be sent after the close message was sent
  // and before the FLWindow receives the final dealloc message
  w = NULL;
}
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
- (NSPoint)convertBaseToScreen:(NSPoint)aPoint
{
  if (fl_mac_os_version >= 100700) {
    NSRect r = [self convertRectToScreen:NSMakeRect(aPoint.x, aPoint.y, 0, 0)];
    return r.origin;
    }
  else {
    // replaces return [super convertBaseToScreen:aPoint] that may trigger a compiler warning
    typedef NSPoint (*convertIMP)(id, SEL, NSPoint);
    static convertIMP addr = (convertIMP)[NSWindow instanceMethodForSelector:@selector(convertBaseToScreen:)];
    return addr(self, @selector(convertBaseToScreen:), aPoint);
    }
}
#endif

- (FLWindow*)initWithFl_W:(Fl_Window *)flw 
	      contentRect:(NSRect)rect 
		styleMask:(NSUInteger)windowStyle 
{
  self = [super initWithContentRect:rect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
  if (self) {
    w = flw;
    if (fl_mac_os_version >= 100700) {
      // replaces [self setRestorable:NO] that may trigger a compiler warning
      typedef void (*setIMP)(id, SEL, BOOL);
      static setIMP addr = (setIMP)[NSWindow instanceMethodForSelector:@selector(setRestorable:)];
      addr(self, @selector(setRestorable:), NO);
      }
  }
  return self;
}
- (Fl_Window *)getFl_Window;
{
  return w;
}

- (BOOL)canBecomeKeyWindow
{
  if (Fl::modal_ && (Fl::modal_ != w))
    return NO;	// prevent the caption to be redrawn as active on click
		//  when another modal window is currently the key win
  return !(!w || w->output() || w->tooltip_window() || w->menu_window() || w->parent());
}

- (BOOL)canBecomeMainWindow
{
  if (Fl::modal_ && (Fl::modal_ != w))
    return NO;	// prevent the caption to be redrawn as active on click
		//  when another modal window is currently the key win

  return !(!w || w->tooltip_window() || w->menu_window() || w->parent());
}

- (void)recursivelySendToSubwindows:(SEL)sel applyToSelf:(BOOL)b
{
  if (b) [self performSelector:sel];
  NSEnumerator *enumerator = [[self childWindows] objectEnumerator];
  id child;
  while ((child = [enumerator nextObject]) != nil) {
    if ([child isKindOfClass:[FLWindow class]]) [child recursivelySendToSubwindows:sel applyToSelf:YES];
  }
}

- (void)setSubwindowFrame { // have the cocoa position and size of a (sub)window follow its FLTK data
  Fl_Window *parent = w->window();
  if (!w->visible_r()) return;
  NSPoint pt = FLTKtoCocoa(w, w->x(), w->y(), w->h());
  int bt = parent ? 0 : get_window_frame_sizes(w);
  NSRect rp = NSMakeRect(round(pt.x), round(pt.y),  w->w(), w->h() + bt);
  if (!NSEqualRects(rp, [self frame])) {
    [self setFrame:rp display:(views_use_CA ? NO : YES)];
  }
  if (parent && ![self parentWindow]) { // useful when subwin is first shown, not when moved
    FLWindow *pxid = fl_xid(parent);
    [pxid addChildWindow:self ordered:NSWindowAbove]; // needs OS X 10.2
    [self orderWindow:NSWindowAbove relativeTo:[pxid windowNumber]]; // necessary under 10.3
  }
}

- (void)checkSubwindowFrame {
  if (!w->parent()) return;
  // make sure this subwindow doesn't leak out of its parent window
  Fl_Window *from = w, *parent;
  CGRect full = CGRectMake(0, 0, w->w(), w->h()); // full subwindow area
  CGRect srect = full; // will become new subwindow clip
  int fromx = 0, fromy = 0;
  while ((parent = from->window()) != NULL) { // loop over all parent windows
    fromx -= from->x(); // parent origin in subwindow's coordinates
    fromy -= from->y();
    CGRect prect = CGRectMake(fromx, fromy, parent->w(), parent->h());
    srect = CGRectIntersection(prect, srect); // area of subwindow inside its parent
    from = parent;
  }
  CGRect *r = Fl_X::i(w)->subRect();
  CGRect current_clip = (r ? *r : full); // current subwindow clip
  if (!CGRectEqualToRect(srect, current_clip)) { // if new clip differs from current clip
    delete r;
    FLWindow *xid = fl_xid(w);
    NSView *view = [xid contentView];
    if (CGRectEqualToRect(srect, full)) r = NULL;
    else {
      r = new CGRect(srect);
      if (r->size.width == 0 && r->size.height == 0) r->origin.x = r->origin.y = 0;
    }
    Fl_X::i(w)->subRect(r);
    w->redraw();
    if (fl_mac_os_version < 100900) {
      NSInteger parent_num = [fl_xid(w->window()) windowNumber];
      [xid orderWindow:NSWindowBelow relativeTo:parent_num];
      [xid orderWindow:NSWindowAbove relativeTo:parent_num];
    }
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
    if (!views_use_CA || ((FLView*)view)->aux_bitmap)
#endif
      [view display]; // subwindow needs redrawn
  }
}

-(void)waitForExpose
{
  if ([self getFl_Window]->shown()) {
    // this makes freshly created windows appear on the screen, if they are not there already
    NSModalSession session = [NSApp beginModalSessionForWindow:self];
    [NSApp runModalSession:session];
    [NSApp endModalSession:session];
  }
}

/* With Mac OS 10.11 the green window button makes window fullscreen (covers system menu bar and dock).
 When there are subwindows, they are by default constrained not to cover the menu bar
 (this is arguably a Mac OS bug).
 Overriding the constrainFrameRect:toScreen: method removes this constraint.
 */
- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen *)screen
{
  if ([self parentWindow]) return frameRect; // do not constrain subwindows
  return [super constrainFrameRect:frameRect toScreen:screen]; // will prevent a window from going above the menu bar
}
@end

@interface FLApplication : NSObject
{
}
+ (void)sendEvent:(NSEvent *)theEvent;
@end

/*
 * This function is the central event handler.
 * It reads events from the event queue using the given maximum time
 */
static int do_queued_events( double time = 0.0 )
{
  got_events = 0;
  
  // Check for re-entrant condition
  if ( dataready.IsThreadRunning() ) {
    dataready.CancelThread(DEBUGTEXT("AVOID REENTRY\n"));
  }
  
  // Start thread to watch for data ready
  if ( dataready.GetNfds() ) {
    dataready.StartThread();
  }
  
  fl_unlock_function();
  NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                      untilDate:[NSDate dateWithTimeIntervalSinceNow:time]
                                         inMode:NSDefaultRunLoopMode dequeue:YES];
  if (event != nil) {
    got_events = 1;
    [FLApplication sendEvent:event]; // will then call [NSApplication sendevent:]
  }
  fl_lock_function();
  
#if FLTK_CONSOLIDATE_MOTION
  if (send_motion && send_motion == fl_xmousewin) {
    send_motion = 0;
    Fl::handle(FL_MOVE, fl_xmousewin);
  }
#endif
  return got_events;
}

double fl_mac_flush_and_wait(double time_to_wait) {
  if (dropped_files_list) { // when the list of dropped files is not empty, open one and remove it from list
    drain_dropped_files_list();
  }
  static int in_idle = 0;
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  if (Fl::idle) {
    if (!in_idle) {
      in_idle = 1;
      Fl::idle();
      in_idle = 0;
    }
    // the idle function may turn off idle, we can then wait:
    if (Fl::idle) time_to_wait = 0.0;
  }
  if (fl_mac_os_version < 101100) NSDisableScreenUpdates(); // 10.3 Makes updates to all windows appear as a single event
  Fl::flush();
  if (fl_mac_os_version < 101100) NSEnableScreenUpdates(); // 10.3
  if (Fl::idle && !in_idle) // 'idle' may have been set within flush()
    time_to_wait = 0.0;
  int retval = do_queued_events(time_to_wait);

  Fl_X::q_release_context();
  [pool release];
  return (double)retval;
}


static NSInteger max_normal_window_level(void)
{
  Fl_X *x;
  NSInteger max_level;

  max_level = 0;

  for (x = Fl_X::first;x;x = x->next) {
    NSInteger level;
    FLWindow *cw = x->xid;
    Fl_Window *win = x->w;
    if (!win || !cw || ![cw isVisible])
      continue;
    if (win->modal() || win->non_modal())
      continue;
    level = [cw level];
    if (level >= max_level)
      max_level = level;
  }

  return max_level;
}

// appropriate window level for modal windows
static NSInteger modal_window_level(void)
{
  NSInteger level;

  level = max_normal_window_level();
  if (level < NSStatusWindowLevel)
    return NSStatusWindowLevel;

  // Need some room for non-modal windows
  level += 2;

  // We cannot exceed this
  if (level > CGShieldingWindowLevel())
    return CGShieldingWindowLevel();

  return level;
}

// appropriate window level for non-modal windows
static NSInteger non_modal_window_level(void)
{
  NSInteger level;

  level = max_normal_window_level();
  if (level < NSFloatingWindowLevel)
    return NSFloatingWindowLevel;

  level += 1;

  if (level > CGShieldingWindowLevel())
    return CGShieldingWindowLevel();

  return level;
}

// makes sure modal and non-modal windows stay on top
static void fixup_window_levels(void)
{
  NSInteger modal_level, non_modal_level;

  Fl_X *x;
  FLWindow *prev_modal, *prev_non_modal;

  modal_level = modal_window_level();
  non_modal_level = non_modal_window_level();

  prev_modal = NULL;
  prev_non_modal = NULL;

  for (x = Fl_X::first;x;x = x->next) {
    FLWindow *cw = x->xid;
    Fl_Window *win = x->w;
    if (!win || !cw || ![cw isVisible])
      continue;
    if (win->modal()) {
      if ([cw level] != modal_level) {
        [cw setLevel:modal_level];
        // changing level puts then in front, so make sure the
        // stacking isn't messed up
        if (prev_modal != NULL)
          [cw orderWindow:NSWindowBelow
              relativeTo:[prev_modal windowNumber]];
      }
      prev_modal = cw;
    } else if (win->non_modal()) {
      if ([cw level] != non_modal_level) {
        [cw setLevel:non_modal_level];
        if (prev_non_modal != NULL)
          [cw orderWindow:NSWindowBelow
              relativeTo:[prev_non_modal windowNumber]];
      }
      prev_non_modal = cw;
    }
  }
}


// updates Fl::e_x, Fl::e_y, Fl::e_x_root, and Fl::e_y_root
static void update_e_xy_and_e_xy_root(NSWindow *nsw)
{
  NSPoint pt;
  pt = [nsw mouseLocationOutsideOfEventStream];
  Fl::e_x = int(pt.x);
  Fl::e_y = int([[nsw contentView] frame].size.height - pt.y);
  pt = [NSEvent mouseLocation];
  Fl::e_x_root = int(pt.x);
  Fl::e_y_root = int(main_screen_height - pt.y);
}


/*
 * Cocoa Mousewheel handler
 */
static void cocoaMouseWheelHandler(NSEvent *theEvent)
{
  // Handle the new "MightyMouse" mouse wheel events. Please, someone explain
  // to me why Apple changed the API on this even though the current API
  // supports two wheels just fine. Matthias,
  fl_lock_function();
  
  Fl_Window *window = (Fl_Window*)[(FLWindow*)[theEvent window] getFl_Window];
  if ( !window->shown() ) {
    fl_unlock_function();
    return;
  }
  Fl::first_window(window);
  
  // Under OSX, single mousewheel increments are 0.1,
  // so make sure they show up as at least 1..
  //
  float dx = [theEvent deltaX]; if ( fabs(dx) < 1.0 ) dx = (dx > 0) ? 1.0 : -1.0;
  float dy = [theEvent deltaY]; if ( fabs(dy) < 1.0 ) dy = (dy > 0) ? 1.0 : -1.0;
  if ([theEvent deltaX] != 0) {
    Fl::e_dx = (int)-dx;
    Fl::e_dy = 0;
    if ( Fl::e_dx) Fl::handle( FL_MOUSEWHEEL, window );
  } else if ([theEvent deltaY] != 0) {
    Fl::e_dx = 0;
    Fl::e_dy = (int)-dy;
    if ( Fl::e_dy) Fl::handle( FL_MOUSEWHEEL, window );
  } else {
    fl_unlock_function();
    return;
  }
  
  fl_unlock_function();
  
  //  return noErr;
}

/*
 * Cocoa Magnify Gesture Handler
 */
static void cocoaMagnifyHandler(NSEvent *theEvent)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
  fl_lock_function();
  Fl_Window *window = (Fl_Window*)[(FLWindow*)[theEvent window] getFl_Window];
  if ( !window->shown() ) {
    fl_unlock_function();
    return;
  }
  Fl::first_window(window);
  Fl::e_dy = [theEvent magnification]*1000; // 10.5.2
  if ( Fl::e_dy) {
    NSPoint pos = [theEvent locationInWindow];
    pos.y = window->h() - pos.y;
    NSUInteger mods = [theEvent modifierFlags];
    mods_to_e_state( mods );
    update_e_xy_and_e_xy_root([theEvent window]);
    Fl::handle( FL_ZOOM_GESTURE, window );
  }
  fl_unlock_function();
#endif
}

/*
 * Cocoa Mouse Button Handler
 */
static void cocoaMouseHandler(NSEvent *theEvent)
{
  static int keysym[] = { 0, FL_Button+1, FL_Button+3, FL_Button+2 };
  static int px, py;
  
  fl_lock_function();
  
  Fl_Window *window = (Fl_Window*)[(FLWindow*)[theEvent window] getFl_Window];
  if ( !window->shown() ) {
    fl_unlock_function();
    return;
  }
  Fl_Window *first = Fl::first_window();
  if (first != window && !(first->modal() || first->non_modal())) Fl::first_window(window);
  NSPoint pos = [theEvent locationInWindow];
  pos.y = window->h() - pos.y;
  NSInteger btn = [theEvent buttonNumber]  + 1;
  NSUInteger mods = [theEvent modifierFlags];  
  int sendEvent = 0;
  
  NSEventType etype = [theEvent type];
  if (etype == NSLeftMouseDown || etype == NSRightMouseDown || etype == NSOtherMouseDown) {
    if (btn == 1) Fl::e_state |= FL_BUTTON1;
    else if (btn == 3) Fl::e_state |= FL_BUTTON2;
    else if (btn == 2) Fl::e_state |= FL_BUTTON3;
  }
  else if (etype == NSLeftMouseUp || etype == NSRightMouseUp || etype == NSOtherMouseUp) {
    if (btn == 1) Fl::e_state &= ~FL_BUTTON1;
    else if (btn == 3) Fl::e_state &= ~FL_BUTTON2;
    else if (btn == 2) Fl::e_state &= ~FL_BUTTON3;
    }
    
  switch ( etype ) {
    case NSLeftMouseDown:
    case NSRightMouseDown:
    case NSOtherMouseDown:
      sendEvent = FL_PUSH;
      Fl::e_is_click = 1; 
      px = (int)pos.x; py = (int)pos.y;
      if ([theEvent clickCount] > 1) 
        Fl::e_clicks++;
      else
        Fl::e_clicks = 0;
      // fall through
    case NSLeftMouseUp:
    case NSRightMouseUp:
    case NSOtherMouseUp:
      if ( !window ) break;
      if ( !sendEvent ) {
        sendEvent = FL_RELEASE; 
      }
      Fl::e_keysym = keysym[ btn ];
      // fall through
    case NSMouseMoved:
      if ( !sendEvent ) {
        sendEvent = FL_MOVE; 
      }
      // fall through
    case NSLeftMouseDragged:
    case NSRightMouseDragged:
    case NSOtherMouseDragged: {
      if ( !sendEvent ) {
        sendEvent = FL_MOVE; // Fl::handle will convert into FL_DRAG
        if (fabs(pos.x-px)>5 || fabs(pos.y-py)>5) 
          Fl::e_is_click = 0;
      }
      mods_to_e_state( mods );
      update_e_xy_and_e_xy_root([theEvent window]);
      if (fl_mac_os_version < 100500) {
        // before 10.5, mouse moved events aren't sent to borderless windows such as tooltips
        Fl_Window *tooltip = Fl_Tooltip::current_window();
        int inside = 0;
        if (tooltip && tooltip->shown() ) { // check if a tooltip window is currently opened
          // check if mouse is inside the tooltip
          inside = (Fl::event_x_root() >= tooltip->x() && Fl::event_x_root() < tooltip->x() + tooltip->w() &&
                    Fl::event_y_root() >= tooltip->y() && Fl::event_y_root() < tooltip->y() + tooltip->h() );
        }
        // if inside, send event to tooltip window instead of background window
        if (inside)
          window = tooltip;
      }
      Fl::handle( sendEvent, window );
      }
      break;
    default:
      break;
  }
  
  fl_unlock_function();
  
  return;
}

@interface FLTextView : NSTextView // this subclass is only needed under OS X < 10.6 
{
  BOOL isActive;
}
+ (void)initialize;
+ (FLTextView*)singleInstance;
- (void)insertText:(id)aString;
- (void)doCommandBySelector:(SEL)aSelector;
- (void)setActive:(BOOL)a;
@end
static FLTextView *fltextview_instance = nil;
@implementation FLTextView
+ (void)initialize {
  NSRect rect={{0,0},{20,20}};
  fltextview_instance = [[FLTextView alloc] initWithFrame:rect];
}
+ (FLTextView*)singleInstance {
  return fltextview_instance;
}
- (void)insertText:(id)aString
{
  if (isActive) [[[NSApp keyWindow] contentView] insertText:aString];
}
- (void)doCommandBySelector:(SEL)aSelector
{
  [[[NSApp keyWindow] contentView] doCommandBySelector:aSelector];
}
- (void)setActive:(BOOL)a
{
  isActive = a;
}
@end


@interface FLWindowDelegate : NSObject 
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
<NSWindowDelegate>
#endif
+ (void)initialize;
+ (FLWindowDelegate*)singleInstance;
- (void)windowDidMove:(NSNotification *)notif;
- (void)view_did_resize:(NSNotification *)notif;
- (void)windowDidResignKey:(NSNotification *)notif;
- (void)windowDidBecomeKey:(NSNotification *)notif;
- (void)windowDidBecomeMain:(NSNotification *)notif;
- (void)windowDidDeminiaturize:(NSNotification *)notif;
- (void)fl_windowMiniaturize:(NSNotification *)notif;
- (void)windowDidMiniaturize:(NSNotification *)notif;
- (BOOL)windowShouldClose:(id)fl;
- (void)anyWindowWillClose:(NSNotification *)notif;
- (void)doNothing:(id)unused;
- (BOOL)window:(NSWindow *)window shouldPopUpDocumentPathMenu:(NSMenu *)menu;
@end


/* make subwindows re-appear after appl unhide or window deminiaturize
 (not necessary with 10.5 and above)
 */
static void orderfront_subwindows(FLWindow *xid)
{
  NSArray *children = [xid childWindows]; // 10.2
  NSEnumerator *enumerator = [children objectEnumerator];
  id child;
  while ((child = [enumerator nextObject]) != nil) { // this undo-redo seems necessary under 10.3
    [xid removeChildWindow:child];
    [xid addChildWindow:child ordered:NSWindowAbove];
    [child orderWindow:NSWindowAbove relativeTo:[xid windowNumber]];
    orderfront_subwindows(child);
  }
}

#if FLTK_ABI_VERSION >= 10304
static const unsigned windowDidResize_mask = 1;
#else
static const unsigned long windowDidResize_mask = 1;
#endif

bool Fl_X::in_windowDidResize() {
#if FLTK_ABI_VERSION >= 10304
  return mapped_to_retina_ & windowDidResize_mask;
#else
  return (unsigned long)xidChildren & windowDidResize_mask;
#endif
}

void Fl_X::in_windowDidResize(bool b) {
#if FLTK_ABI_VERSION >= 10304
  if (b) mapped_to_retina_ |= windowDidResize_mask;
  else mapped_to_retina_ &= ~windowDidResize_mask;
#else
  if (b) xidChildren = (Fl_X*)((unsigned long)xidChildren | windowDidResize_mask);
  else xidChildren = (Fl_X*)((unsigned long)xidChildren & ~windowDidResize_mask);
#endif
}

#if FLTK_ABI_VERSION >= 10304
static const unsigned mapped_mask = 2;
static const unsigned changed_mask = 4;
#else
static const unsigned long mapped_mask = 2; // sizeof(unsigned long) = sizeof(Fl_X*)
static const unsigned long changed_mask = 4;
#endif

bool Fl_X::mapped_to_retina() {
#if FLTK_ABI_VERSION >= 10304
  return mapped_to_retina_ & mapped_mask;
#else
  return (unsigned long)xidChildren & mapped_mask;
#endif
}

void Fl_X::mapped_to_retina(bool b) {
#if FLTK_ABI_VERSION >= 10304
  if (b) mapped_to_retina_ |= mapped_mask;
  else mapped_to_retina_ &= ~mapped_mask;
#else
  if (b) xidChildren = (Fl_X*)((unsigned long)xidChildren | mapped_mask);
  else xidChildren = (Fl_X*)((unsigned long)xidChildren & ~mapped_mask);
#endif
}

bool Fl_X::changed_resolution() {
#if FLTK_ABI_VERSION >= 10304
  return mapped_to_retina_ & changed_mask;
#else
  return (unsigned long)xidChildren & changed_mask;
#endif
}

void Fl_X::changed_resolution(bool b) {
#if FLTK_ABI_VERSION >= 10304
  if (b) mapped_to_retina_ |= changed_mask;
  else mapped_to_retina_ &= ~changed_mask;
#else
  if (b) xidChildren = (Fl_X*)((unsigned long)xidChildren | changed_mask);
  else xidChildren = (Fl_X*)((unsigned long)xidChildren & ~changed_mask);
#endif
}


@interface FLWindowDelegateBefore10_6 : FLWindowDelegate
- (id)windowWillReturnFieldEditor:(NSWindow *)sender toObject:(id)client;
@end
@implementation FLWindowDelegateBefore10_6
- (id)windowWillReturnFieldEditor:(NSWindow *)sender toObject:(id)client
{
  return [FLTextView singleInstance];
}
@end

@interface FLWindowDelegateBefore10_5 : FLWindowDelegateBefore10_6
-(void)windowDidDeminiaturize:(NSNotification *)notif;
-(void)windowWillMiniaturize:(NSNotification *)notif;
@end
@implementation FLWindowDelegateBefore10_5
-(void)windowDidDeminiaturize:(NSNotification *)notif
{
  [super windowDidDeminiaturize:notif];
  fl_lock_function();
  orderfront_subwindows([notif object]);
  fl_unlock_function();
}
-(void)windowWillMiniaturize:(NSNotification *)notif
{
  [super fl_windowMiniaturize:notif];
  NSArray *children = [(NSWindow*)[notif object] childWindows]; // 10.2
  NSEnumerator *enumerator = [children objectEnumerator];
  id child;
  while ((child = [enumerator nextObject]) != nil) [child orderOut:self];
}
@end

// compute coordinates of the win top left in FLTK units
static void CocoatoFLTK(Fl_Window *win, int &x, int &y) {
  NSPoint ori;
  FLWindow *nsw = fl_xid(win);
  ori = [nsw convertBaseToScreen:NSMakePoint(0, [[nsw contentView] frame].size.height)];
  x = ori.x ;
  y = (main_screen_height - ori.y);
  while (win->parent()) {win = win->window(); x -= win->x(); y -= win->y();}
}

// return Cocoa coordinates of the point in window win at (x,y) FLTK units
static NSPoint FLTKtoCocoa(Fl_Window *win, int x, int y, int H) {
  while (win->parent()) {win = win->window(); x += win->x(); y += win->y();}
  return NSMakePoint(x, main_screen_height - (y + H));
}

static FLWindowDelegate *flwindowdelegate_instance = nil;
@implementation FLWindowDelegate
+ (void)initialize
{
  if (self == [FLWindowDelegate self]) {
    if (fl_mac_os_version < 100500) flwindowdelegate_instance = [FLWindowDelegateBefore10_5 alloc];
    else if (fl_mac_os_version < 100600) flwindowdelegate_instance = [FLWindowDelegateBefore10_6 alloc];
    else flwindowdelegate_instance = [FLWindowDelegate alloc];
    flwindowdelegate_instance = [flwindowdelegate_instance init];
  }
}
+ (FLWindowDelegate*)singleInstance {
  return flwindowdelegate_instance;
}
- (void)windowDidMove:(NSNotification *)notif
{
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  if (!window->parent()) starting_moved_window = window;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
  FLView *view = (FLView*)[nsw contentView];
  if (views_use_CA && [view did_view_resolution_change]) {
    if (window->as_gl_window() && Fl::use_high_res_GL()) [view setNeedsDisplay:YES]; // necessary with  macOS ≥ 10.14.2; harmless before
  }
#endif
  if (window == starting_moved_window) {
    // we update 'main_screen_height' here because it's wrong just after screen config changes
    main_screen_height = CGDisplayBounds(CGMainDisplayID()).size.height;
    int X, Y;
    CocoatoFLTK(window, X, Y);
    if (window->x() != X || window->y() != Y) window->position(X, Y);
    update_e_xy_and_e_xy_root(nsw);
    // at least since MacOS 10.9: OS moves subwindows contained in a moved window
    // setSubwindowFrame is no longer necessary.
    if (fl_mac_os_version < 100900) [nsw recursivelySendToSubwindows:@selector(setSubwindowFrame) applyToSelf:NO];
    if (window->parent()) [nsw recursivelySendToSubwindows:@selector(checkSubwindowFrame) applyToSelf:YES];
    starting_moved_window = NULL;
  }
  fl_unlock_function();
}
- (void)view_did_resize:(NSNotification *)notif
{
  if (![[notif object] isKindOfClass:[FLView class]]) return;
  FLView *view = (FLView*)[notif object];
  FLWindow *nsw = (FLWindow*)[view window];
  if (!nsw || ![nsw getFl_Window]) return;
  fl_lock_function();
  Fl_Window *window = [nsw getFl_Window];
  int X, Y;
  CocoatoFLTK(window, X, Y);
  NSRect r = [view frame];
  Fl_X::i(window)->in_windowDidResize(1);
  window->resize(X, Y, r.size.width, r.size.height);
  Fl_X::i(window)->in_windowDidResize(0);
  update_e_xy_and_e_xy_root(nsw);
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
  if (views_use_CA && !window->as_gl_window()) {
    [view reset_aux_bitmap];
    window->redraw();
  }
#endif
  fl_unlock_function();
}
- (void)windowDidResignKey:(NSNotification *)notif
{
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  /* Fullscreen windows obscure all other windows so we need to return
   to a "normal" level when the user switches to another window or other app */
  if (window->fullscreen_active()) {
    [nsw setLevel:NSNormalWindowLevel];
    fixup_window_levels();
  }
  Fl::handle( FL_UNFOCUS, window);
  fl_unlock_function();
}
- (void)windowDidBecomeKey:(NSNotification *)notif
{
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *w = [nsw getFl_Window];
  /* Restore previous fullscreen level */
  if (w->fullscreen_active()) {
    [nsw setLevel:NSStatusWindowLevel];
    fixup_window_levels();
  }
  Fl::handle( FL_FOCUS, w);
  fl_unlock_function();
}
- (void)windowDidBecomeMain:(NSNotification *)notif
{
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  Fl::first_window(window);
  if (!window->parent()) [nsw orderFront:nil];
  update_e_xy_and_e_xy_root(nsw);
  fl_unlock_function();
}
- (void)windowDidDeminiaturize:(NSNotification *)notif
{
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  if ([nsw miniwindowImage]) { [nsw setMiniwindowImage:nil]; }
  Fl_Window *window = [nsw getFl_Window];
  Fl::handle(FL_SHOW, window);
  // necessary when resolutions before miniaturization and after deminiaturization differ
  // or if GUI was resized while window was minimized
  [nsw recursivelySendToSubwindows:@selector(setSubwindowFrame) applyToSelf:YES];
  update_e_xy_and_e_xy_root(nsw);
  Fl::flush(); // Process redraws set by FL_SHOW.
  fl_unlock_function();
}
- (void)fl_windowMiniaturize:(NSNotification *)notif
{
  // subwindows are not captured in system-built miniature window image
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  if ([[nsw childWindows] count]) {
    Fl_Window *window = [nsw getFl_Window];
    // capture the window and its subwindows and use as miniature window image
    NSBitmapImageRep *bitmap = rect_to_NSBitmapImageRep_subwins(window, 0, 0, window->w(), window->h(), true);
    if (bitmap) {
      NSImage *img = [[[NSImage alloc] initWithSize:NSMakeSize([bitmap pixelsWide], [bitmap pixelsHigh])] autorelease];
      [img addRepresentation:bitmap];
      [bitmap release];
      [nsw setMiniwindowImage:img];
    }
  }
  fl_unlock_function();
}
- (void)windowDidMiniaturize:(NSNotification *)notif
{
  if (fl_mac_os_version >= 100500) [self fl_windowMiniaturize:notif];
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  Fl::handle(FL_HIDE, window);
  fl_unlock_function();
}
- (BOOL)windowShouldClose:(id)fl
{
  fl_lock_function();
  Fl_Window *win = [(FLWindow *)fl getFl_Window];
  if (win) Fl::handle(FL_CLOSE, win); // this might or might not close the window
  fl_unlock_function();
  // the system doesn't need to send [fl close] because FLTK does it when needed
  return NO; 
}
- (void)anyWindowWillClose:(NSNotification *)notif
{
  fl_lock_function();
  if ([[notif object] isKeyWindow]) {
    // If the closing window is the key window,
    // find a bordered top-level window to become the new key window
    Fl_Window *w = Fl::first_window();
    while (w && (w->parent() || !w->border() || !w->visible())) {
      w = Fl::next_window(w);
    }
    if (w) {
      [Fl_X::i(w)->xid makeKeyWindow];
    }
  }
  fl_unlock_function();
}
- (void)doNothing:(id)unused
{
  return;
}
- (BOOL)window:(NSWindow *)window shouldPopUpDocumentPathMenu:(NSMenu *)menu {
  return NO;
}
@end

@interface FLAppDelegate : NSObject 
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
<NSApplicationDelegate>
#endif
{
  @public
  open_cb_f_type open_cb;
  TSMDocumentID currentDoc;
}
- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app;
- (void)applicationDidFinishLaunching:(NSNotification *)notification;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender;
- (void)applicationDidBecomeActive:(NSNotification *)notify;
- (void)applicationDidChangeScreenParameters:(NSNotification *)aNotification;
- (void)applicationDidUpdate:(NSNotification *)aNotification;
- (void)applicationWillResignActive:(NSNotification *)notify;
- (void)applicationWillHide:(NSNotification *)notify;
- (void)applicationWillUnhide:(NSNotification *)notify;
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename;
@end

@implementation FLAppDelegate
- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
  // Avoids macOS 14 warning message when app is launched from command line:
  // "WARNING: Secure coding is automatically enabled for restorable state!
  // However, not on all supported macOS versions of this application.
  // Opt-in to secure coding explicitly by implementing
  // NSApplicationDelegate.applicationSupportsSecureRestorableState:."
  return (fl_mac_os_version >= 140000);
}
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
  if (fl_mac_os_version >= 101300 && [NSApp isRunning]) [NSApp stop:nil];
}
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
  fl_lock_function();
  NSApplicationTerminateReply reply = (fl_mac_quit_early ? NSTerminateNow : NSTerminateCancel);
  while ( Fl_X::first ) {
    Fl_Window *win = Fl::first_window();
    if (win->parent()) win = win->top_window();
    Fl_Widget_Tracker wt(win); // track the window object
    Fl::handle(FL_CLOSE, win);
    if (wt.exists() && win->shown()) { // the user didn't close win
      reply = NSTerminateCancel; // so we return to the main program now
      break;
    }
  }
  fl_unlock_function();
  return reply;
}
- (void)applicationDidBecomeActive:(NSNotification *)notify
{
  fl_lock_function();

  // update clipboard status
  clipboard_check();

  /**
   * Cocoa organizes the Z depth of windows on a global priority. FLTK however
   * expects the window manager to organize Z level by application. The trickery
   * below will change Z order during activation and deactivation.
   */
  fixup_window_levels();

  fl_unlock_function();
}
- (void)applicationDidChangeScreenParameters:(NSNotification *)unused
{ // react to changes in screen numbers and positions
  fl_lock_function();
  main_screen_height = CGDisplayBounds(CGMainDisplayID()).size.height;
  Fl::call_screen_init();
  Fl::handle(FL_SCREEN_CONFIGURATION_CHANGED, NULL);
  fl_unlock_function();
}
- (void)applicationDidUpdate:(NSNotification *)aNotification
{
  if (im_enabled != -1) {
    TSMDocumentID newDoc;
    // It is extremely unclear when Cocoa decides to create/update
    // the input context, but debugging reveals that it is done
    // by NSApplication:updateWindows. So check if the input context
    // has shifted after each such run so that we can update our
    // input methods status.
    newDoc = TSMGetActiveDocument();
    if (newDoc != currentDoc) {
      TSMDocumentID doc;
    
      doc = TSMGetActiveDocument();
      
      if (im_enabled)
        TSMRemoveDocumentProperty(doc, kTSMDocumentEnabledInputSourcesPropertyTag);
      else {
        CFArrayRef inputSources;
        
        CFDictionaryRef filter;
        filter = CFDictionaryCreate(NULL, (const void **)kTISPropertyInputSourceType,
                                          (const void **)kTISTypeKeyboardLayout,
                                          1, NULL, NULL);
        inputSources = TISCreateInputSourceList(filter, false);
        CFRelease(filter);
        TSMSetDocumentProperty(doc, kTSMDocumentEnabledInputSourcesPropertyTag,
                               sizeof(CFArrayRef), &inputSources);
        CFRelease(inputSources);
      }
      currentDoc = newDoc;
    }
  }
}
- (void)applicationWillResignActive:(NSNotification *)notify
{
  fl_lock_function();
  Fl_X *x;
  FLWindow *top = 0;
  // sort in all regular windows
  for (x = Fl_X::first;x;x = x->next) {
    FLWindow *cw = x->xid;
    Fl_Window *win = x->w;
    if (win && cw) {
      if (win->modal()) {
      } else if (win->non_modal()) {
      } else {
        if (!top) top = cw;
      }
    }
  }
  // now sort in all modals
  for (x = Fl_X::first;x;x = x->next) {
    FLWindow *cw = x->xid;
    Fl_Window *win = x->w;
    if (win && cw && [cw isVisible]) {
      if (win->modal()) {
        [cw setLevel:NSNormalWindowLevel];
        if (top) [cw orderWindow:NSWindowAbove relativeTo:[top windowNumber]];
      }
    }
  }
  // finally all non-modals
  for (x = Fl_X::first;x;x = x->next) {
    FLWindow *cw = x->xid;
    Fl_Window *win = x->w;
    if (win && cw && [cw isVisible]) {
      if (win->non_modal()) {
        [cw setLevel:NSNormalWindowLevel];
        if (top) [cw orderWindow:NSWindowAbove relativeTo:[top windowNumber]];
      }
    }
  }
  fl_unlock_function();
}
- (void)applicationWillHide:(NSNotification *)notify
{
  fl_lock_function();
  Fl_X *x;
  for (x = Fl_X::first;x;x = x->next) {
    Fl_Window *window = x->w;
    if ( !window->parent() ) Fl::handle( FL_HIDE, window);
    }
  fl_unlock_function();
}
- (void)applicationWillUnhide:(NSNotification *)notify
{
  fl_lock_function();
  for (Fl_X *x = Fl_X::first;x;x = x->next) {
    Fl_Window *w = x->w;
    if ( !w->parent() && ![x->xid isMiniaturized]) {
      Fl::handle(FL_SHOW, w);
      }
  }
  fl_unlock_function();
}
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
  if (fl_mac_os_version < 101300) {
  // without the next two statements, the opening of the 1st window is delayed by several seconds
  // under 10.8 ≤ Mac OS < 10.13 when a file is dragged on the application icon
    Fl_Window *firstw = Fl::first_window();
    if (firstw) firstw->wait_for_expose();
  } else if (in_nsapp_run) { // memorize all dropped filenames
    if (!dropped_files_list) dropped_files_list = [[NSMutableArray alloc] initWithCapacity:1];
    [dropped_files_list addObject:filename];
    return YES;
  }
  if (open_cb) {
    fl_lock_function();
    (*open_cb)([filename UTF8String]);
    Fl::flush(); // useful for AppleScript that does not break the event loop
    fl_unlock_function();
    return YES;
  }
  return NO;
}
@end

@interface FLAppDelegateBefore10_5 : FLAppDelegate
- (void)applicationDidUnhide:(NSNotification *)notify;
- (void)applicationDidUpdate:(NSNotification *)aNotification;
@end
@implementation FLAppDelegateBefore10_5
- (void)applicationDidUnhide:(NSNotification *)notify
{ // before 10.5, subwindows are lost when application is unhidden
  fl_lock_function();
  for (Fl_X *x = Fl_X::first; x; x = x->next) {
    if (![x->xid parentWindow]) {
      orderfront_subwindows(x->xid);
    }
  }
  fl_unlock_function();
}
- (void)applicationDidUpdate:(NSNotification *)aNotification
{
}
@end

static void drain_dropped_files_list() {
  open_cb_f_type open_cb = ((FLAppDelegate*)[NSApp delegate])->open_cb;
  if (!open_cb) {
    [dropped_files_list removeAllObjects];
    [dropped_files_list release];
    dropped_files_list = nil;
    return;
  }
  NSString *s = (NSString*)[dropped_files_list objectAtIndex:0];
  char *fname = strdup([s UTF8String]);
  [dropped_files_list removeObjectAtIndex:0];
  if ([dropped_files_list count] == 0) {
    [dropped_files_list release];
    dropped_files_list = nil;
  }
  open_cb(fname);
  free(fname);
}

/*
 * Install an open documents event handler...
 */
void fl_open_callback(void (*cb)(const char *)) {
  fl_open_display();
  ((FLAppDelegate*)[NSApp delegate])->open_cb = cb;
}

@implementation FLApplication
+ (void)sendEvent:(NSEvent *)theEvent
{
  if (fl_send_system_handlers(theEvent))
    return;

  NSEventType type = [theEvent type];  
  if (type == NSLeftMouseDown) {
    fl_lock_function();
    Fl_Window *grab = Fl::grab();
    if (grab) {
      FLWindow *win = (FLWindow *)[theEvent window];
      if ( [win isKindOfClass:[FLWindow class]] && grab != [win getFl_Window]) {
	// a click event out of a menu window, so we should close this menu
	// done here to catch also clicks on window title bar/resize box 
	cocoaMouseHandler(theEvent);
      }
    }
    fl_unlock_function();
  } else if (type == NSApplicationDefined) {
    if ([theEvent subtype] == FLTKDataReadyEvent) {
      processFLTKEvent();
    }
    return;
  } else if (type == NSKeyUp) {
    // The default sendEvent turns key downs into performKeyEquivalent when
    // modifiers are down, but swallows the key up if the modifiers include
    // command.  This one makes all modifiers consistent by always sending key ups.
    // FLView treats performKeyEquivalent to keyDown, but performKeyEquivalent is
    // still needed for the system menu.
    [[NSApp keyWindow] sendEvent:theEvent];
    return;
    }
  [NSApp sendEvent:theEvent];
}
@end

/* Prototype of undocumented function needed to support Mac OS 10.2 or earlier
 extern "C" {
  OSErr CPSEnableForegroundOperation(ProcessSerialNumber*, UInt32, UInt32, UInt32, UInt32);
}
*/

static BOOL is_bundled() {
  static int value = 2;
  if (value == 2) {
    value = 1;
    NSBundle *bundle = [NSBundle mainBundle];
    if (bundle) {
      NSString *exe = [[bundle executablePath] stringByStandardizingPath];
      NSString *bpath = [bundle bundlePath];
      NSString *exe_dir = [exe stringByDeletingLastPathComponent];
      if ([bpath isEqualToString:exe] || [bpath isEqualToString:exe_dir]) value = 0;
    } else value = 0;
  }
  return value == 1;
}


static void foreground_and_activate() {
  if ( !is_bundled() ) { // only transform the application type for unbundled apps
    ProcessSerialNumber cur_psn = { 0, kCurrentProcess };
    TransformProcessType(&cur_psn, kProcessTransformToForegroundApplication); // needs Mac OS 10.3
    /* support of Mac OS 10.2 or earlier used this undocumented call instead
     err = CPSEnableForegroundOperation(&cur_psn, 0x03, 0x3C, 0x2C, 0x1103);
     */
  }
  [NSApp activateIgnoringOtherApps:YES];
}

void fl_open_display() {
  static char beenHereDoneThat = 0;
  if ( !beenHereDoneThat ) {
    beenHereDoneThat = 1;
    
    BOOL need_new_nsapp = (NSApp == nil);
    if (need_new_nsapp) [NSApplication sharedApplication];
    [[NSAutoreleasePool alloc] init]; // never released
    FLAppDelegate *delegate = (Fl_X::calc_mac_os_version() < 100500 ? [FLAppDelegateBefore10_5 alloc] : [FLAppDelegate alloc]);
    [(NSApplication*)NSApp setDelegate:[delegate init]];
    if (need_new_nsapp) {
      if (fl_mac_os_version >= 101300 && fl_mac_os_version < 140000 && is_bundled()) {
        [NSApp activateIgnoringOtherApps:YES];
        in_nsapp_run = true;
        [NSApp run];
        in_nsapp_run = false;
      }
      else [NSApp finishLaunching];
    }
    if (fl_mac_os_version < 140000) {
      // empty the event queue but keep system events for drag&drop of files at launch
      NSEvent *ign_event;
      do ign_event = [NSApp nextEventMatchingMask:(NSAnyEventMask & ~NSSystemDefinedMask)
                                        untilDate:[NSDate dateWithTimeIntervalSinceNow:0]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
      while (ign_event);
    }
    if (![NSApp isActive]) foreground_and_activate();
    if (![NSApp servicesMenu]) createAppleMenu();
    main_screen_height = CGDisplayBounds(CGMainDisplayID()).size.height;
    [[NSNotificationCenter defaultCenter] addObserver:[FLWindowDelegate singleInstance]
					     selector:@selector(anyWindowWillClose:) 
						 name:NSWindowWillCloseNotification 
					       object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:[FLWindowDelegate singleInstance]
                                             selector:@selector(view_did_resize:)
                                                 name:NSViewFrameDidChangeNotification
                                               object:nil];
    if (![NSThread isMultiThreaded]) {
      // With old OS X versions, it is necessary to create one thread for secondary pthreads to be
      // allowed to use cocoa, especially to create an NSAutoreleasePool.
      // We create a thread that does nothing so it completes very fast:
      [NSThread detachNewThreadSelector:@selector(doNothing:) toTarget:[FLWindowDelegate singleInstance] withObject:nil];
    }
  }
}


/*
 * get rid of allocated resources
 */
void fl_close_display() {
}

// Force a "Roman" or "ASCII" keyboard, which both the Mozilla and
// Safari people seem to think implies turning off advanced IME stuff
// (see nsTSMManager::SyncKeyScript in Mozilla and enableSecureTextInput
// in Safari/Webcore). Should be good enough for us then...

static int input_method_startup()
{
  static int retval = -1; // -1: not initialized, 0: not usable, 1: ready for use
  if (retval == -1) {
    fl_open_display();
    if (fl_mac_os_version >= 100500) {
      TSMGetActiveDocument = (TSMGetActiveDocument_type)Fl_X::get_carbon_function("TSMGetActiveDocument");
      TSMSetDocumentProperty = (TSMSetDocumentProperty_type)Fl_X::get_carbon_function("TSMSetDocumentProperty");
      TSMRemoveDocumentProperty = (TSMRemoveDocumentProperty_type)Fl_X::get_carbon_function("TSMRemoveDocumentProperty");
      TISCreateInputSourceList = (TISCreateInputSourceList_type)Fl_X::get_carbon_function("TISCreateInputSourceList");
      kTISTypeKeyboardLayout = (CFStringRef)Fl_X::get_carbon_function("kTISTypeKeyboardLayout");
      kTISPropertyInputSourceType = (CFStringRef)Fl_X::get_carbon_function("kTISPropertyInputSourceType");
      retval = (TSMGetActiveDocument && TSMSetDocumentProperty && TSMRemoveDocumentProperty && TISCreateInputSourceList && kTISTypeKeyboardLayout && kTISPropertyInputSourceType ? 1 : 0);
    } else {
      KeyScript = (KeyScript_type)Fl_X::get_carbon_function("KeyScript");
      retval = (KeyScript? 1 : 0);
    }
  }
  return retval;
}

void Fl::enable_im() {
  if (!input_method_startup()) return;

  im_enabled = 1;

  if (fl_mac_os_version >= 100500) {
    ((FLAppDelegate*)[NSApp delegate])->currentDoc = NULL;
    [NSApp updateWindows]; // triggers [FLAppDelegate applicationDidUpdate]
  }
  else
    KeyScript(smKeyEnableKybds);
}

void Fl::disable_im() {
  if (!input_method_startup()) return;

  im_enabled = 0;

  if (fl_mac_os_version >= 100500) {
    ((FLAppDelegate*)[NSApp delegate])->currentDoc = NULL;
    [NSApp updateWindows]; // triggers [FLAppDelegate applicationDidUpdate]
  }
  else
    KeyScript(smEnableRomanKybdsOnly);
}


// Gets the border sizes and the titlebar height
static int get_window_frame_sizes(const Fl_Window *win, int *pbx, int *pby) {
  if (pbx) *pbx = 0; if (pby) *pby = 0;
  if (win && !win->border()) return 0;
  FLWindow *flw = fl_xid(win);
  if (flw) {
    return [flw frame].size.height - [[flw contentView] frame].size.height;
  }
  static int top = 0, left, bottom;
  if (!top) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSRect inside = { {20,20}, {100,100} };
    NSRect outside = [NSWindow  frameRectForContentRect:inside styleMask:NSTitledWindowMask];
    left = int(outside.origin.x - inside.origin.x);
    bottom = int(outside.origin.y - inside.origin.y);
    top = int(outside.size.height - inside.size.height) - bottom;
    [pool release];
    }
  if (pbx) *pbx = left;
  if (pby) *pby = bottom;
  return top;
}

/*
 * smallest x coordinate in screen space of work area of menubar-containing display
 */
int Fl::x() {
  return int([[[NSScreen screens] objectAtIndex:0] visibleFrame].origin.x);
}


/*
 * smallest y coordinate in screen space of work area of menubar-containing display
 */
int Fl::y() {
  fl_open_display();
  NSRect visible = [[[NSScreen screens] objectAtIndex:0] visibleFrame];
  return int(main_screen_height - (visible.origin.y + visible.size.height));
}


/*
 * width of work area of menubar-containing display
 */
int Fl::w() {
  return int([[[NSScreen screens] objectAtIndex:0] visibleFrame].size.width);
}


/*
 * height of work area of menubar-containing display
 */
int Fl::h() {
  return int([[[NSScreen screens] objectAtIndex:0] visibleFrame].size.height);
}

// computes the work area of the nth screen (screen #0 has the menubar)
void Fl_X::screen_work_area(int &X, int &Y, int &W, int &H, int n)
{
  fl_open_display();
  NSRect r = [[[NSScreen screens] objectAtIndex:n] visibleFrame];
  X   = int(r.origin.x);
  Y   = main_screen_height - int(r.origin.y + r.size.height);
  W   = int(r.size.width);
  H   = int(r.size.height);
}

/*
 * get the current mouse pointer world coordinates
 */
void Fl::get_mouse(int &x, int &y) 
{
  fl_open_display();
  NSPoint pt = [NSEvent mouseLocation];
  x = int(pt.x);
  y = int(main_screen_height - pt.y);
}


int Fl_X::fake_X_wm(const Fl_Window* w,int &X,int &Y, int &bt,int &bx, int &by, int maxw, int minw, int maxh, int minh) {
  int W, H, xoff, yoff, dx, dy;
  int ret = bx = by = bt = 0;
  if (w->border() && !w->parent()) {
    if (maxw != minw || maxh != minh) {
      ret = 2;
    } else {
      ret = 1;
    }
    bt = get_window_frame_sizes(w, &bx, &by);
  }
  if (w->parent()) return 0;
  // The coordinates of the whole window, including non-client area
  xoff = bx;
  yoff = by + bt;
  dx = 2*bx;
  dy = 2*by + bt;
  X = w->x()-xoff;
  Y = w->y()-yoff;
  W = w->w()+dx;
  H = w->h()+dy;
  
  // Proceed to positioning the window fully inside the screen, if possible
  
  // let's get a little elaborate here. Mac OS X puts a lot of stuff on the desk
  // that we want to avoid when positioning our window, namely the Dock and the
  // top menu bar (and even more stuff in 10.4 Tiger). So we will go through the
  // list of all available screens and find the one that this window is most
  // likely to go to, and then reposition it to fit withing the 'good' area.
  //  Rect r;
  // find the screen, that the center of this window will fall into
  int R = X+W, B = Y+H; // right and bottom
  int cx = (X+R)/2, cy = (Y+B)/2; // center of window;
  NSScreen *gd = NULL;
  NSArray *a = [NSScreen screens]; int count = (int)[a count]; NSRect r; int i;
  for( i = 0; i < count; i++) {
    r = [[a objectAtIndex:i] frame];
    r.origin.y = main_screen_height - (r.origin.y + r.size.height); // use FLTK's multiscreen coordinates
    if (   cx >= r.origin.x && cx <= r.origin.x + r.size.width
        && cy >= r.origin.y && cy <= r.origin.y + r.size.height)
      break;
  }
  if (i < count) gd = [a objectAtIndex:i];
  
  // if the center doesn't fall on a screen, try the top left
  if (!gd) {
    for( i = 0; i < count; i++) {
      r = [[a objectAtIndex:i] frame];
      r.origin.y = main_screen_height - (r.origin.y + r.size.height); // use FLTK's multiscreen coordinates
      if (    X >= r.origin.x && X <= r.origin.x + r.size.width
          && Y >= r.origin.y  && Y <= r.origin.y + r.size.height)
        break;
    }
    if (i < count) gd = [a objectAtIndex:i];
  }
  // if that doesn't fall on a screen, try the top right
  if (!gd) {
    for( i = 0; i < count; i++) {
      r = [[a objectAtIndex:i] frame];
      r.origin.y = main_screen_height - (r.origin.y + r.size.height); // use FLTK's multiscreen coordinates
      if (    R >= r.origin.x && R <= r.origin.x + r.size.width
          && Y >= r.origin.y  && Y <= r.origin.y + r.size.height)
        break;
    }
    if (i < count) gd = [a objectAtIndex:i];
  }
  // if that doesn't fall on a screen, try the bottom left
  if (!gd) {
    for( i = 0; i < count; i++) {
      r = [[a objectAtIndex:i] frame];
      r.origin.y = main_screen_height - (r.origin.y + r.size.height); // use FLTK's multiscreen coordinates
      if (    X >= r.origin.x && X <= r.origin.x + r.size.width
          && Y+H >= r.origin.y  && Y+H <= r.origin.y + r.size.height)
        break;
    }
    if (i < count) gd = [a objectAtIndex:i];
  }
  // last resort, try the bottom right
  if (!gd) {
    for( i = 0; i < count; i++) {
      r = [[a objectAtIndex:i] frame];
      r.origin.y = main_screen_height - (r.origin.y + r.size.height); // use FLTK's multiscreen coordinates
      if (    R >= r.origin.x && R <= r.origin.x + r.size.width
          && Y+H >= r.origin.y  && Y+H <= r.origin.y + r.size.height)
        break;
    }
    if (i < count) gd = [a objectAtIndex:i];
  }
  // if we still have not found a screen, we will use the main
  // screen, the one that has the application menu bar.
  if (!gd) gd = [a objectAtIndex:0];
  if (gd) {
    r = [gd visibleFrame];
    r.origin.y = main_screen_height - (r.origin.y + r.size.height); // use FLTK's multiscreen coordinates
    if ( R > r.origin.x + r.size.width ) X -= int(R - (r.origin.x + r.size.width));
    if ( B > r.size.height + r.origin.y ) Y -= int(B - (r.size.height + r.origin.y));
    if ( X < r.origin.x ) X = int(r.origin.x);
    if ( Y < r.origin.y ) Y = int(r.origin.y);
  }
  
  // Return the client area's top left corner in (X,Y)
  X+=xoff;
  Y+=yoff;
  
  return ret;
}


Fl_Window *fl_dnd_target_window = 0;

static void  q_set_window_title(NSWindow *nsw, const char * name, const char *mininame) {
  CFStringRef title = CFStringCreateWithCString(NULL, (name ? name : ""), kCFStringEncodingUTF8);
  if(!title) { // fallback when name contains malformed UTF-8
    int l = strlen(name);
    unsigned short* utf16 = new unsigned short[l + 1];
    l = fl_utf8toUtf16(name, l, utf16, l + 1);
    title = CFStringCreateWithCharacters(NULL, utf16, l);
    delete[] utf16;
    }
  [nsw setTitle:(NSString*)title];
  CFRelease(title);
  if (mininame && strlen(mininame)) {
    CFStringRef minititle = CFStringCreateWithCString(NULL, mininame, kCFStringEncodingUTF8);
    if (minititle) {
      [nsw setMiniwindowTitle:(NSString*)minititle];
      CFRelease(minititle);
    }
  }
}

/**                 How FLTK handles Mac OS text input
 
 Let myview be the instance of the FLView class that has the keyboard focus. FLView is an FLTK-defined NSView subclass
 that implements the NSTextInputClient protocol to properly handle text input. It also implements the old NSTextInput
 protocol to run with OS <= 10.4. The few NSTextInput protocol methods that differ in signature from the NSTextInputClient 
 protocol transmit the received message to the corresponding NSTextInputClient method.

 Keyboard input sends keyDown: and performKeyEquivalent: messages to myview. The latter occurs for keys such as
 ForwardDelete, arrows and F1, and when the Ctrl or Cmd modifiers are used. Other key presses send keyDown: messages.
 The keyDown: method calls [myview process_keydown:theEvent] that is equivalent to
 [[myview inputContext] handleEvent:theEvent], and triggers system processing of keyboard events.
 The performKeyEquivalent: method directly calls Fl::handle(FL_KEYBOARD, focus-window)
 when the Ctrl or Cmd modifiers are used. If not, it also calls [[myview inputContext] handleEvent:theEvent].
 The performKeyEquivalent: method returns YES when the keystroke has been handled and NO otherwise, which allows
 shortcuts of the system menu to be processed. Three sorts of messages are then sent back by the system to myview: 
 doCommandBySelector:, setMarkedText: and insertText:. All 3 messages eventually produce Fl::handle(FL_KEYBOARD, win) calls.
 The doCommandBySelector: message allows to process events such as new-line, forward and backward delete, arrows, 
 escape, tab, F1. The message setMarkedText: is sent when marked text, that is, temporary text that gets replaced later 
 by some other text, is inserted. This happens when a dead key is pressed, and also 
 when entering complex scripts (e.g., Chinese). Fl_X::next_marked_length gives the byte
 length of marked text before the FL_KEYBOARD event is processed. Fl::compose_state gives this length after this processing.
 Message insertText: is sent to enter text in the focused widget. If there's marked text, Fl::compose_state is > 0, and this
 marked text gets replaced by the inserted text. If there's no marked text, the new text is inserted at the insertion point. 
 When the character palette is used to enter text, the system sends an insertText: message to myview. 
 The in_key_event field of the FLView class allows to differentiate keyboard from palette inputs.
 
 During processing of the handleEvent message, inserted and marked strings are concatenated in a single string
 inserted in a single FL_KEYBOARD event after return from handleEvent. The need_handle member variable of FLView allows 
 to determine when setMarkedText or insertText strings have been sent during handleEvent processing and must trigger 
 an FL_KEYBOARD event. Concatenating two insertText operations or an insertText followed by a setMarkedText is possible. 
 In contrast, setMarkedText followed by insertText or by another setMarkedText isn't correct if concatenated in a single 
 string. Thus, in such case, the setMarkedText and the next operation produce each an FL_KEYBOARD event. 
 
 OS >= 10.7 contains a feature where pressing and holding certain keys opens a menu window that shows a list 
 of possible accented variants of this key. The selectedRange field of the FLView class and the selectedRange, insertText:
 and setMarkedText: methods of the NSTextInputClient protocol are used to support this feature.
 The notion of selected text (!= marked text) is monitored by the selectedRange field. 
 The -(NSRange)[FLView selectedRange] method is used to control whether an FLTK widget opens accented character windows 
 by returning .location = NSNotFound to disable that, or returning the value of the selectedRange field to enable the feature.
 When selectedRange.location >= 0, the value of selectedRange.length is meaningful. 0 means no text is currently selected, 
 > 0 means this number of characters before the insertion point are selected. The insertText: method does
 selectedRange = NSMakeRange(100, 0); to indicate no text is selected. The setMarkedText: method does   
 selectedRange = NSMakeRange(100, newSelection.length); to indicate that this length of text is selected.

 With OS <= 10.5, the NSView class does not implement the inputContext message. [myview process_keydown:theEvent] is
 equivalent to [[FLTextInputContext singleInstance] handleEvent:theEvent].
 Method +[FLTextInputContext singleInstance] returns an instance of class FLTextInputContext that possesses
 a handleEvent: method. The class FLTextView implements the so-called view's "field editor". This editor is an instance
 of the FLTextView class allocated by the -(id)[FLWindowDelegate windowWillReturnFieldEditor: toObject:] method.
 The -(BOOL)[FLTextInputContext handleEvent:] method emulates the missing 10.6 -(BOOL)[NSTextInputContext handleEvent:]
 by sending the interpretKeyEvents: message to the FLTextView object. The system sends back doCommandBySelector: and
 insertText: messages to the FLTextView object that are transmitted unchanged to myview to be processed as with OS >= 10.6. 
 The system also sends setMarkedText: messages directly to myview.
   
 There is furthermore an oddity of dead key processing with OS <= 10.5. It occurs when a dead key followed by a non-accented  
 key are pressed. Say, for example, that keys '^' followed by 'p' are pressed on a French or German keyboard. Resulting 
 messages are: [myview setMarkedText:@"^"], [myview insertText:@"^"], [myview insertText:@"p"], [FLTextView insertText:@"^p"]. 
 The 2nd '^' replaces the marked 1st one, followed by p^p. The resulting text in the widget is "^p^p" instead of the 
 desired "^p". To avoid that, the FLTextView object is deactivated by the insertText: message and reactivated after 
 the handleEvent: message has been processed.

 NSEvent's during a character composition sequence:
 - keyDown with deadkey -> [[theEvent characters] length] is 0
 - keyUp -> [theEvent characters] contains the deadkey
 - keyDown with next key -> [theEvent characters] contains the composed character
 - keyUp -> [theEvent characters] contains the standard character
 */

static void cocoaKeyboardHandler(NSEvent *theEvent)
{
  NSUInteger mods;
  // get the modifiers
  mods = [theEvent modifierFlags];
  // get the key code
  UInt32 keyCode = 0, maskedKeyCode = 0;
  unsigned short sym = 0;
  keyCode = [theEvent keyCode];
  // extended keyboards can also send sequences on key-up to generate Kanji etc. codes.
  // Some observed prefixes are 0x81 to 0x83, followed by an 8 bit keycode.
  // In this mode, there seem to be no key-down codes
  // printf("%08x %08x %08x\n", keyCode, mods, key);
  maskedKeyCode = keyCode & 0x7f;
  mods_to_e_state( mods ); // process modifier keys
  if (!macKeyLookUp) macKeyLookUp = fl_compute_macKeyLookUp();
  sym = macKeyLookUp[maskedKeyCode];
  if (sym < 0xff00) { // a "simple" key
    // find the result of this key without modifier
    NSString *sim = [theEvent charactersIgnoringModifiers];
    UniChar one;
    CFStringGetCharacters((CFStringRef)sim, CFRangeMake(0, 1), &one);
    // charactersIgnoringModifiers doesn't ignore shift, remove it when it's on
    if(one >= 'A' && one <= 'Z') one += 32;
    if (one > 0 && one <= 0x7f && (sym<'0' || sym>'9') ) sym = one;
  }
  Fl::e_keysym = Fl::e_original_keysym = sym;
  /*NSLog(@"cocoaKeyboardHandler: keycode=%08x keysym=%08x mods=%08x symbol=%@ (%@)",
   keyCode, sym, mods, [theEvent characters], [theEvent charactersIgnoringModifiers]);*/
  // If there is text associated with this key, it will be filled in later.
  Fl::e_length = 0;
  Fl::e_text = (char*)"";
}

@interface FLTextInputContext : NSObject // "emulates" NSTextInputContext before OS 10.6
+ (void)initialize;
+ (FLTextInputContext*)singleInstance;
-(BOOL)handleEvent:(NSEvent*)theEvent;
@end
static FLTextInputContext* fltextinputcontext_instance = nil;
@implementation FLTextInputContext
+ (void)initialize {
  fltextinputcontext_instance = [[FLTextInputContext alloc] init];
}
+ (FLTextInputContext*)singleInstance {
  return fltextinputcontext_instance;
}
-(BOOL)handleEvent:(NSEvent*)theEvent {
  FLTextView *edit = [FLTextView singleInstance];
  [edit setActive:YES];
  [edit interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
  [edit setActive:YES];
  return YES;
}
@end

/* Implementation note for the support of layer-backed views.
 MacOS 10.14 Mojave changes the way all drawing to displays is performed:
 all NSView objects become layer-backed, that is, the drawing is done by
 Core Animation to a CALayer object whose content is then displayed by the NSView.
 The global variable views_use_CA is set to YES when such change applies,
 that is, for apps running under 10.14 and linked to SDK 10.14.
 When views_use_CA is NO, views are not supposed to be layer-backed.
 
 Most drawing is done by [FLView drawRect:] which the system calls
 when a window is created or resized and when Fl_X::flush() runs which sends the display
 message to the view. Within drawRect:, [[NSGraphicsContext currentContext] CGContext]
 gives a graphics context whose product ultimately appears on screen. But the
 full content of the view must be redrawn each time drawRect: runs, in contrast
 to pre-10.14 where drawings were added to the previous window content.
 That is why FLView maintains a bitmap (view->aux_bitmap) to which FLTK directs all drawings.
 At the end of drawRect:, the content of that bitmap is copied to the graphics context.
  
 A problem arises to support drawing done outside Fl_X::flush(), that is,
 after the app calls Fl_Window::make_current() at any time it wants.
 That situation is identified by the condition (views_use_CA && !through_drawRect).
 A graphics context usable outside drawRect: that ultimately appears on screen,
 if it exists, was not identified. Drawing operations after the call to Fl_Window::make_current()
 are directed to aux_bitmap. Fl_Window::make_current() calls [view setNeedsDisplay:YES]
 which instructs the system to run drawRect: at the next event loop. Later,
 when drawRect: runs, the content of aux_bitmap is copied to drawRect's graphics context.
 
 OpenGL windows remain processed under 10.14 as before.
 */

@implementation FLView
- (BOOL)did_view_resolution_change {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
  if (fl_mac_os_version >= 100700) { // determine whether window is mapped to a retina display
    Fl_Window *window = [(FLWindow*)[self window] getFl_Window];
    Fl_X *i = Fl_X::i(window);
    bool previous = i->mapped_to_retina();
    NSView *view = (!views_use_CA && window->parent() && !window->as_gl_window()) ?
                      [fl_xid(window->top_window()) contentView] : self;
    if (view) {
      NSSize s = [view convertSizeToBacking:NSMakeSize(10, 10)]; // 10.7
      i->mapped_to_retina( int(s.width + 0.5) > 10 );
    }
    BOOL retval = (i->wait_for_expose == 0 && previous != i->mapped_to_retina());
    if (retval) {
      i->changed_resolution(true);
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
      if (views_use_CA && !window->as_gl_window() ) {
        [self reset_aux_bitmap];
        [self display];
      }
#endif
    }
    return retval;
  }
#endif
  return NO;
}
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
- (void)create_aux_bitmap:(CGContextRef)gc retina:(BOOL)r {
  if (!gc || fl_mac_os_version >= 101600) {
    // bitmap context-related functions (e.g., CGBitmapContextGetBytesPerRow) can't be used here with macOS 11.0 "Big Sur"
    static CGColorSpaceRef cspace = CGColorSpaceCreateDeviceRGB();
    int W = [self frame].size.width, H = [self frame].size.height;
    if (r) { W *= 2; H *= 2; }
    aux_bitmap = CGBitmapContextCreate(NULL, W, H, 8, 0, cspace, kCGImageAlphaPremultipliedFirst);
  } else {
    aux_bitmap = CGBitmapContextCreate(NULL, CGBitmapContextGetWidth(gc), CGBitmapContextGetHeight(gc),
                                       CGBitmapContextGetBitsPerComponent(gc), CGBitmapContextGetBytesPerRow(gc),
                                       CGBitmapContextGetColorSpace(gc), CGBitmapContextGetBitmapInfo(gc));
  }
  if (r) CGContextScaleCTM(aux_bitmap, 2, 2);
}
- (void)reset_aux_bitmap {
  CGContextRelease(aux_bitmap);
  aux_bitmap = NULL;
}
#endif
- (BOOL)process_keydown:(NSEvent*)theEvent
{
  id o = fl_mac_os_version >= 100600 ? [self performSelector:@selector(inputContext)] : [FLTextInputContext singleInstance];
  return [o handleEvent:theEvent];
}
- (id)initWithFrame:(NSRect)frameRect
{
  static NSInteger counter = 0;
  self = [super initWithFrame:frameRect];
  if (self) {
    in_key_event = NO;
    identifier = ++counter;
    }
  return self;
}

/* Used by all GL or non-GL windows.
 * Gets called when a window is created, resized, or moved between retina and non-retina displays.
 * For non-GL windows, also called by Fl_X::flush() because of the display message sent to the view.
 */
- (void)drawRect:(NSRect)rect
{
  FLWindow *cw = (FLWindow*)[self window];
  Fl_Window *window = [cw getFl_Window];
  if (!window) return; // may happen after closing full-screen window
  fl_lock_function();
  Fl_X *i = Fl_X::i(window);
  if (!through_Fl_X_flush
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
            && (!views_use_CA || !aux_bitmap)
#endif
      ) {
    [self did_view_resolution_change];
    if (i->wait_for_expose) {
      i->wait_for_expose = 0;
      if (window->as_gl_window() && views_use_CA && fl_mac_os_version < 101401) { // 1st drawing of layer-backed GL window
        window->size(window->w(), window->h()); // sends message [GLcontext update]
      }
    }
    if ( i->region ) {
      XDestroyRegion(i->region);
      i->region = 0;
    }
    window->clear_damage(FL_DAMAGE_ALL);
  }
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
  CGContextRef destination = NULL;
  if (views_use_CA && !window->as_gl_window()) {
    destination = [[NSGraphicsContext currentContext] CGContext];
    if (!aux_bitmap && !window->as_gl_window()) [self create_aux_bitmap:destination retina:i->mapped_to_retina()];
  }
#endif
  through_drawRect = YES;
  if (window->damage()) i->flush();
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
  if (destination) { // can be NULL with gl_start/gl_finish
    if (fl_mac_os_version < 101600 && CGBitmapContextGetBytesPerRow(aux_bitmap) == CGBitmapContextGetBytesPerRow(destination)) {
      memcpy(CGBitmapContextGetData(destination), CGBitmapContextGetData(aux_bitmap),
             CGBitmapContextGetHeight(aux_bitmap) * CGBitmapContextGetBytesPerRow(aux_bitmap));
    } else {
      CGImageRef img = CGBitmapContextCreateImage(aux_bitmap);
      CGContextDrawImage(destination, [self frame], img);
      CGImageRelease(img);
    }
  }
#endif
  Fl_X::q_release_context();
  if (!through_Fl_X_flush) window->clear_damage();
  through_drawRect = NO;
  fl_unlock_function();
}

- (BOOL)acceptsFirstResponder
{   
  return [[self window] parentWindow] ? NO : YES; // 10.2
}
- (BOOL)performKeyEquivalent:(NSEvent*)theEvent
{   
  //NSLog(@"performKeyEquivalent:");
  fl_lock_function();
  cocoaKeyboardHandler(theEvent);
  BOOL handled;
  NSUInteger mods = [theEvent modifierFlags];
  Fl_Window *w = [(FLWindow*)[theEvent window] getFl_Window];
  if ( (mods & NSControlKeyMask) || (mods & NSCommandKeyMask) ) {
    NSString *s = [theEvent characters];
    if ( (mods & NSShiftKeyMask) && (mods & NSCommandKeyMask) ) {
      s = [s uppercaseString]; // US keyboards return lowercase letter in s if cmd-shift-key is hit
      }
    [FLView prepareEtext:s];
    Fl::compose_state = 0;
    handled = Fl::handle(FL_KEYBOARD, w);
  }
  else {
    in_key_event = YES;
    need_handle = NO;
    handled = [self process_keydown:theEvent];
    if (need_handle) handled = Fl::handle(FL_KEYBOARD, w);
    in_key_event = NO;
    }
  fl_unlock_function();
  return handled;
}
- (BOOL)acceptsFirstMouse:(NSEvent*)theEvent
{   
  Fl_Window *w = [(FLWindow*)[theEvent window] getFl_Window];
  Fl_Window *first = Fl::first_window();
  return (first == w || !first->modal());
}
- (void)resetCursorRects {
  Fl_Window *w = [(FLWindow*)[self window] getFl_Window];
  if (!w) return;
  Fl_X *i = Fl_X::i(w);
  if (!i) return;  // fix for STR #3128
  // We have to have at least one cursor rect for invalidateCursorRectsForView
  // to work, hence the "else" clause.
  if (i->cursor)
    [self addCursorRect:[self visibleRect] cursor:(NSCursor*)i->cursor];
  else
    [self addCursorRect:[self visibleRect] cursor:[NSCursor arrowCursor]];
}
- (void)mouseUp:(NSEvent *)theEvent {
  cocoaMouseHandler(theEvent);
}
- (void)rightMouseUp:(NSEvent *)theEvent {
  cocoaMouseHandler(theEvent);
}
- (void)otherMouseUp:(NSEvent *)theEvent {
  cocoaMouseHandler(theEvent);
}
- (void)mouseDown:(NSEvent *)theEvent {
  cocoaMouseHandler(theEvent);
}
- (void)rightMouseDown:(NSEvent *)theEvent {
  cocoaMouseHandler(theEvent);
}
- (void)otherMouseDown:(NSEvent *)theEvent {
  cocoaMouseHandler(theEvent);
}
- (void)mouseMoved:(NSEvent *)theEvent {
  cocoaMouseHandler(theEvent);
}
- (void)mouseDragged:(NSEvent *)theEvent {
  cocoaMouseHandler(theEvent);
}
- (void)rightMouseDragged:(NSEvent *)theEvent {
  cocoaMouseHandler(theEvent);
}
- (void)otherMouseDragged:(NSEvent *)theEvent {
  cocoaMouseHandler(theEvent);
}
- (void)scrollWheel:(NSEvent *)theEvent {
  cocoaMouseWheelHandler(theEvent);
}
- (void)magnifyWithEvent:(NSEvent *)theEvent {
  cocoaMagnifyHandler(theEvent);
}
- (void)keyDown:(NSEvent *)theEvent {
  //NSLog(@"keyDown:%@",[theEvent characters]);
  fl_lock_function();
  Fl_Window *window = [(FLWindow*)[theEvent window] getFl_Window];
  Fl::first_window(window);
  cocoaKeyboardHandler(theEvent);
  in_key_event = YES;
  Fl_Widget *f = Fl::focus();
  if (f && f->as_gl_window()) { // ignore text input methods for GL windows
    need_handle = YES;
    [FLView prepareEtext:[theEvent characters]];
  } else {
    need_handle = NO;
    [self process_keydown:theEvent];
  }
  if (need_handle) Fl::handle(FL_KEYBOARD, window);
  in_key_event = NO;
  fl_unlock_function();
}
- (void)keyUp:(NSEvent *)theEvent {
  //NSLog(@"keyUp:%@",[theEvent characters]);
  fl_lock_function();
  Fl_Window *window = (Fl_Window*)[(FLWindow*)[theEvent window] getFl_Window];
  Fl::first_window(window);
  cocoaKeyboardHandler(theEvent);
  NSString *s = [theEvent characters];
  if ([s length] >= 1) [FLView prepareEtext:[s substringToIndex:1]];
  Fl::handle(FL_KEYUP,window);
  fl_unlock_function();
}
- (void)flagsChanged:(NSEvent *)theEvent {
  //NSLog(@"flagsChanged: ");
  fl_lock_function();
  static UInt32 prevMods = 0;
  NSUInteger mods = [theEvent modifierFlags];
  Fl_Window *window = (Fl_Window*)[(FLWindow*)[theEvent window] getFl_Window];
  UInt32 tMods = prevMods ^ mods;
  int sendEvent = 0;
  if ( tMods )
  {
    unsigned short keycode = [theEvent keyCode];
    if (!macKeyLookUp) macKeyLookUp = fl_compute_macKeyLookUp();
    Fl::e_keysym = Fl::e_original_keysym = macKeyLookUp[keycode & 0x7f];
    if ( Fl::e_keysym ) 
      sendEvent = ( prevMods<mods ) ? FL_KEYBOARD : FL_KEYUP;
    Fl::e_length = 0;
    Fl::e_text = (char*)"";
    prevMods = mods;
  }
  mods_to_e_state( mods );
  if (sendEvent) Fl::handle(sendEvent,window);
  fl_unlock_function();
}
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender
{
  fl_lock_function();
  Fl_Window *target = [(FLWindow*)[self window] getFl_Window];
  update_e_xy_and_e_xy_root([self window]);
  fl_dnd_target_window = target;
  int ret = Fl::handle( FL_DND_ENTER, target );
  breakMacEventLoop();
  fl_unlock_function();
  Fl::flush();
  return ret ? NSDragOperationCopy : NSDragOperationNone;
}
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender
{
  fl_lock_function();
  Fl_Window *target = [(FLWindow*)[self window] getFl_Window];
  update_e_xy_and_e_xy_root([self window]);
  fl_dnd_target_window = target;
  int ret = Fl::handle( FL_DND_DRAG, target );
  breakMacEventLoop();
  fl_unlock_function();
  // if the DND started in the same application, Fl::dnd() will not return until 
  // the DND operation is finished. The call below causes the drop indicator
  // to be drawn correctly (a full event handling would be better...)
  Fl::flush();
  return ret ? NSDragOperationCopy : NSDragOperationNone;
}
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender 
{
  static char *DragData = NULL;
  fl_lock_function();
  Fl_Window *target = [(FLWindow*)[self window] getFl_Window];
  if ( !Fl::handle( FL_DND_RELEASE, target ) ) { 
    breakMacEventLoop();
    fl_unlock_function();
    return NO;
  }
  NSPasteboard *pboard;
  // NSDragOperation sourceDragMask;
  // sourceDragMask = [sender draggingSourceOperationMask];
  pboard = [sender draggingPasteboard];
  update_e_xy_and_e_xy_root([self window]);
  if (DragData) { free(DragData); DragData = NULL; }
  if ( [[pboard types] containsObject:NSFilenamesPboardType] ) {
    CFArrayRef files = (CFArrayRef)[pboard propertyListForType:NSFilenamesPboardType];
    CFStringRef all = CFStringCreateByCombiningStrings(NULL, files, CFSTR("\n"));
    int l = CFStringGetMaximumSizeForEncoding(CFStringGetLength(all), kCFStringEncodingUTF8);
    DragData = (char *)malloc(l + 1);
    CFStringGetCString(all, DragData, l + 1, kCFStringEncodingUTF8);
    CFRelease(all);
  }
  else if ( [[pboard types] containsObject:UTF8_pasteboard_type] ) {
    NSData *data = [pboard dataForType:UTF8_pasteboard_type];
    DragData = (char *)malloc([data length] + 1);
    [data getBytes:DragData];
    DragData[[data length]] = 0;
    convert_crlf(DragData, strlen(DragData));
  }
  else {
    breakMacEventLoop();
    fl_unlock_function();
    return NO;
  }
  Fl::e_text = DragData;
  Fl::e_length = strlen(DragData);
  int old_event = Fl::e_number;
  Fl::belowmouse()->handle(Fl::e_number = FL_PASTE);
  Fl::e_number = old_event;
  if (DragData) { free(DragData); DragData = NULL; }
  Fl::e_text = NULL;
  Fl::e_length = 0;
  fl_dnd_target_window = NULL;
  breakMacEventLoop();
  fl_unlock_function();
  return YES;
}
- (void)draggingExited:(id < NSDraggingInfo >)sender
{
  fl_lock_function();
  if ( fl_dnd_target_window ) {
    Fl::handle( FL_DND_LEAVE, fl_dnd_target_window );
    fl_dnd_target_window = 0;
  }
  fl_unlock_function();
}
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal
{
  return NSDragOperationGeneric;
}

+ (void)prepareEtext:(NSString*)aString {
  // fills Fl::e_text with UTF-8 encoded aString using an adequate memory allocation
  static char *received_utf8 = NULL;
  static int lreceived = 0;
  char *p = (char*)[aString UTF8String];
  int l = strlen(p);
  if (l > 0) {
    if (lreceived == 0) {
      received_utf8 = (char*)malloc(l + 1);
      lreceived = l;
    }
    else if (l > lreceived) {
      received_utf8 = (char*)realloc(received_utf8, l + 1);
      lreceived = l;
    }
    strcpy(received_utf8, p);
    Fl::e_text = received_utf8;
  }
  Fl::e_length = l;
}

+ (void)concatEtext:(NSString*)aString {
  // extends Fl::e_text with aString
  NSString *newstring = [[NSString stringWithUTF8String:Fl::e_text] stringByAppendingString:aString];
  [FLView prepareEtext:newstring];
}

- (void)doCommandBySelector:(SEL)aSelector {
  NSString *s = [[NSApp currentEvent] characters];
  //NSLog(@"doCommandBySelector:%s text='%@'",sel_getName(aSelector), s);
  s = [s substringFromIndex:[s length] - 1];
  [FLView prepareEtext:s]; // use the last character of the event; necessary for deadkey + Tab
  Fl_Window *target = [(FLWindow*)[self window] getFl_Window];
  Fl::handle(FL_KEYBOARD, target);
}

- (void)insertText:(id)aString {
  [self insertText:aString replacementRange:NSMakeRange(NSNotFound, 0)];
}

- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange {
  NSString *received;
  if ([aString isKindOfClass:[NSAttributedString class]]) {
    received = [(NSAttributedString*)aString string];
  } else {
    received = (NSString*)aString;
  }
  /*NSLog(@"insertText='%@' l=%d Fl::compose_state=%d range=%d,%d",
	received,strlen([received UTF8String]),Fl::compose_state,replacementRange.location,replacementRange.length);*/
  fl_lock_function();
  Fl_Window *target = [(FLWindow*)[self window] getFl_Window];
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
  if (fl_mac_os_version >= 101400 && replacementRange.length > 0) {
    // occurs after a key was pressed and maintained and an auxiliary window appeared
    // prevents marking dead key from deactivation
    [[self inputContext] discardMarkedText];
  }
#endif
  while (replacementRange.length--) { // delete replacementRange.length characters before insertion point
    int saved_keysym = Fl::e_keysym;
    Fl::e_keysym = FL_BackSpace;
    Fl::handle(FL_KEYBOARD, target);
    Fl::e_keysym = saved_keysym;
    }
  if (in_key_event && Fl_X::next_marked_length && Fl::e_length) {
    // if setMarkedText + insertText is sent during handleEvent, text cannot be concatenated in single FL_KEYBOARD event
    Fl::handle(FL_KEYBOARD, target);
    Fl::e_length = 0;
    }
  if (in_key_event && Fl::e_length) [FLView concatEtext:received];
  else [FLView prepareEtext:received];
  Fl_X::next_marked_length = 0;
  // We can get called outside of key events (e.g., from the character palette, from CJK text input). 
  BOOL palette = !(in_key_event || Fl::compose_state);
  if (palette) Fl::e_keysym = 0;
  // YES if key has text attached
  BOOL has_text_key = Fl::e_keysym <= '~' || Fl::e_keysym == FL_Iso_Key ||
                      (Fl::e_keysym >= FL_KP && Fl::e_keysym <= FL_KP_Last && Fl::e_keysym != FL_KP_Enter);
  // insertText sent during handleEvent of a key without text cannot be processed in a single FL_KEYBOARD event.
  // Occurs with deadkey followed by non-text key
  if (!in_key_event || !has_text_key) {
    Fl::handle(FL_KEYBOARD, target);
    Fl::e_length = 0;
    }
  else need_handle = YES;
  selectedRange = NSMakeRange(100, 0); // 100 is an arbitrary value
  // for some reason, with the palette, the window does not redraw until the next mouse move or button push
  // sending a 'redraw()' or 'awake()' does not solve the issue!
  if (palette) Fl::flush();
  if (fl_mac_os_version < 100600) [[FLTextView singleInstance] setActive:NO];
  fl_unlock_function();
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)newSelection  {
  [self setMarkedText:aString selectedRange:newSelection replacementRange:NSMakeRange(NSNotFound, 0)];
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)newSelection replacementRange:(NSRange)replacementRange {
  NSString *received;
  if ([aString isKindOfClass:[NSAttributedString class]]) {
    received = [(NSAttributedString*)aString string];
  } else {
    received = (NSString*)aString;
  }
  fl_lock_function();
  /*NSLog(@"setMarkedText:%@ l=%d newSelection=%d,%d Fl::compose_state=%d replacement=%d,%d", 
	received, strlen([received UTF8String]), newSelection.location, newSelection.length, Fl::compose_state,
	replacementRange.location, replacementRange.length);*/
  Fl_Window *target = [(FLWindow*)[self window] getFl_Window];
  while (replacementRange.length--) { // delete replacementRange.length characters before insertion point
    Fl::e_keysym = FL_BackSpace;
    Fl::compose_state = 0;
    Fl_X::next_marked_length = 0;
    Fl::handle(FL_KEYBOARD, target);
    Fl::e_keysym = 'a'; // pretend a letter key was hit
  }
  if (in_key_event && Fl_X::next_marked_length && Fl::e_length) {
    // if setMarkedText + setMarkedText is sent during handleEvent, text cannot be concatenated in single FL_KEYBOARD event
    Fl::handle(FL_KEYBOARD, target);
    Fl::e_length = 0;
  }
  if (in_key_event && Fl::e_length) [FLView concatEtext:received];
  else [FLView prepareEtext:received];
  Fl_X::next_marked_length = strlen([received UTF8String]);
  if (!in_key_event) Fl::handle( FL_KEYBOARD, target);
  else need_handle = YES;
  selectedRange = NSMakeRange(100, newSelection.length);
  fl_unlock_function();
}

- (void)unmarkText {
  fl_lock_function();
  Fl::reset_marked_text();
  fl_unlock_function();
  //NSLog(@"unmarkText");
}

- (NSRange)selectedRange {
  Fl_Widget *w = Fl::focus();
  if (w && w->use_accents_menu()) return selectedRange;
  return NSMakeRange(NSNotFound, 0);
}

- (NSRange)markedRange {
  //NSLog(@"markedRange=%d %d", Fl::compose_state > 0?0:NSNotFound, Fl::compose_state);
  return NSMakeRange(Fl::compose_state > 0?0:NSNotFound, Fl::compose_state);
}

- (BOOL)hasMarkedText {
  //NSLog(@"hasMarkedText %s", Fl::compose_state > 0?"YES":"NO");
  return (Fl::compose_state > 0);
}

- (NSAttributedString *)attributedSubstringFromRange:(NSRange)aRange {
  return [self attributedSubstringForProposedRange:aRange actualRange:NULL];
}
- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange {
  //NSLog(@"attributedSubstringFromRange: %d %d",aRange.location,aRange.length);
  return nil;
}

- (NSArray *)validAttributesForMarkedText {
  return nil;
}

- (NSRect)firstRectForCharacterRange:(NSRange)aRange {
  return [self firstRectForCharacterRange:aRange actualRange:NULL];
}
- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange {
  //NSLog(@"firstRectForCharacterRange %d %d actualRange=%p",aRange.location, aRange.length,actualRange);
  NSRect glyphRect;
  fl_lock_function();
  Fl_Widget *focus = Fl::focus();
  Fl_Window *wfocus = [(FLWindow*)[self window] getFl_Window];
  if (!focus) focus = wfocus;
  glyphRect.size.width = 0;
  
  int x, y, height;
  if (Fl_X::insertion_point_location(&x, &y, &height)) {
    glyphRect.origin.x = (CGFloat)x;
    glyphRect.origin.y = (CGFloat)y;
  } else {
    if (focus->as_window()) {
      glyphRect.origin.x = 0;
      glyphRect.origin.y = focus->h();
      }
    else {
      glyphRect.origin.x = focus->x();
      glyphRect.origin.y = focus->y() + focus->h();
      }
    height = 12;
  }
  glyphRect.size.height = height;
  Fl_Window *win = focus->as_window();
  if (!win) win = focus->window();
  while (win != NULL && win != wfocus) {
    glyphRect.origin.x += win->x();
    glyphRect.origin.y += win->y();
    win = win->window();
  }
  // Convert the rect to screen coordinates
  glyphRect.origin.y = wfocus->h() - glyphRect.origin.y;
  glyphRect.origin = [(FLWindow*)[self window] convertBaseToScreen:glyphRect.origin];
  if (actualRange) *actualRange = aRange;
  fl_unlock_function();
  return glyphRect;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint {
  return 0;
}

- (NSInteger)windowLevel {
  return [[self window] level];
}

- (
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
   NSInteger
#else
   long
#endif
)conversationIdentifier {
  return identifier;
}

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context
{
  return NSDragOperationCopy;
}
- (void)draggingSession:(NSDraggingSession *)session
           endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
  Fl_Widget *w = Fl::pushed();
  if ( w ) {
    int old_event = Fl::e_number;
    w->handle(Fl::e_number = FL_RELEASE);
    Fl::e_number = old_event;
    Fl::pushed( 0 );
  }
}
#endif

@end


NSOpenGLPixelFormat* Fl_X::mode_to_NSOpenGLPixelFormat(int m, const int *alistp)
{
  NSOpenGLPixelFormatAttribute attribs[32];
  int n = 0;
  // AGL-style code remains commented out for comparison
  if (!alistp) {
    if (m & FL_INDEX) {
      //list[n++] = AGL_BUFFER_SIZE; list[n++] = 8;
    } else {
      //list[n++] = AGL_RGBA;
      //list[n++] = AGL_GREEN_SIZE;
      //list[n++] = (m & FL_RGB8) ? 8 : 1;
      attribs[n++] = NSOpenGLPFAColorSize;
      attribs[n++] = (NSOpenGLPixelFormatAttribute)((m & FL_RGB8) ? 32 : 1);
      if (m & FL_ALPHA) {
        //list[n++] = AGL_ALPHA_SIZE;
        attribs[n++] = NSOpenGLPFAAlphaSize;
        attribs[n++] = (NSOpenGLPixelFormatAttribute)((m & FL_RGB8) ? 8 : 1);
      }
      if (m & FL_ACCUM) {
        //list[n++] = AGL_ACCUM_GREEN_SIZE; list[n++] = 1;
        attribs[n++] = NSOpenGLPFAAccumSize;
        attribs[n++] = (NSOpenGLPixelFormatAttribute)1;
        if (m & FL_ALPHA) {
          //list[n++] = AGL_ACCUM_ALPHA_SIZE; list[n++] = 1;
        }
      }
    }
    if (m & FL_DOUBLE) {
      //list[n++] = AGL_DOUBLEBUFFER;
      attribs[n++] = NSOpenGLPFADoubleBuffer;
    }
    if (m & FL_DEPTH) {
      //list[n++] = AGL_DEPTH_SIZE; list[n++] = 24;
      attribs[n++] = NSOpenGLPFADepthSize;
      attribs[n++] = (NSOpenGLPixelFormatAttribute)24;
    }
    if (m & FL_STENCIL) {
      //list[n++] = AGL_STENCIL_SIZE; list[n++] = 1;
      attribs[n++] = NSOpenGLPFAStencilSize;
      attribs[n++] = (NSOpenGLPixelFormatAttribute)1;
    }
    if (m & FL_STEREO) {
      //list[n++] = AGL_STEREO;
      attribs[n++] = NSOpenGLPFAStereo;
    }
    if ((m & FL_MULTISAMPLE) && fl_mac_os_version >= 100400) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
      attribs[n++] = NSOpenGLPFAMultisample, // 10.4
#endif
      attribs[n++] = NSOpenGLPFASampleBuffers; attribs[n++] = (NSOpenGLPixelFormatAttribute)1;
      attribs[n++] = NSOpenGLPFASamples; attribs[n++] = (NSOpenGLPixelFormatAttribute)4;
    }
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_7
#define NSOpenGLPFAOpenGLProfile      (NSOpenGLPixelFormatAttribute)99
#define kCGLPFAOpenGLProfile          NSOpenGLPFAOpenGLProfile
#define NSOpenGLProfileVersionLegacy  (NSOpenGLPixelFormatAttribute)0x1000
#define NSOpenGLProfileVersion3_2Core  (NSOpenGLPixelFormatAttribute)0x3200
#define kCGLOGLPVersion_Legacy        NSOpenGLProfileVersionLegacy
#endif
    if (fl_mac_os_version >= 100700) {
      attribs[n++] = NSOpenGLPFAOpenGLProfile;
      attribs[n++] =  (m & FL_OPENGL3) ? NSOpenGLProfileVersion3_2Core : NSOpenGLProfileVersionLegacy;
    }
  } else {
    while (alistp[n] && n < 30) {
      if (alistp[n] == kCGLPFAOpenGLProfile) {
        if (fl_mac_os_version < 100700) {
          if (alistp[n+1] != kCGLOGLPVersion_Legacy) return nil;
          n += 2;
          continue;
        }
      }
      attribs[n] = (NSOpenGLPixelFormatAttribute)alistp[n];
      n++;
    }
  }
  attribs[n] = (NSOpenGLPixelFormatAttribute)0;
  NSOpenGLPixelFormat *pixform = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
  /*GLint color,alpha,accum,depth;
  [pixform getValues:&color forAttribute:NSOpenGLPFAColorSize forVirtualScreen:0];
  [pixform getValues:&alpha forAttribute:NSOpenGLPFAAlphaSize forVirtualScreen:0];
  [pixform getValues:&accum forAttribute:NSOpenGLPFAAccumSize forVirtualScreen:0];
  [pixform getValues:&depth forAttribute:NSOpenGLPFADepthSize forVirtualScreen:0];
  NSLog(@"color=%d alpha=%d accum=%d depth=%d",color,alpha,accum,depth);*/
  return pixform;
}

NSOpenGLContext* Fl_X::create_GLcontext_for_window(NSOpenGLPixelFormat *pixelformat,
                                              NSOpenGLContext *shared_ctx, Fl_Window *window)
{
  NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:pixelformat shareContext:shared_ctx];
  if (context) {
    FLView *view = (FLView*)[fl_xid(window) contentView];
    if (fl_mac_os_version >= 100700) {
      //replaces  [view setWantsBestResolutionOpenGLSurface:YES]  without compiler warning
      typedef void (*bestResolutionIMP)(id, SEL, BOOL);
      static bestResolutionIMP addr = (bestResolutionIMP)[NSView instanceMethodForSelector:@selector(setWantsBestResolutionOpenGLSurface:)];
      addr(view, @selector(setWantsBestResolutionOpenGLSurface:), Fl::use_high_res_GL() != 0);
    }
    [context setView:view];
  }
  return context;
}

void Fl_X::GLcontext_update(NSOpenGLContext* ctxt)
{
  [ctxt update];
}

void Fl_X::GLcontext_flushbuffer(NSOpenGLContext* ctxt)
{
  [ctxt flushBuffer];
}

void Fl_X::GLcontext_release(NSOpenGLContext* ctxt)
{
  [ctxt release];
}

void Fl_X::GL_cleardrawable(void)
{
  [[NSOpenGLContext currentContext] clearDrawable];
}

void Fl_X::GLcontext_makecurrent(NSOpenGLContext* ctxt)
{
  [ctxt makeCurrentContext];
}

void Fl_Window::fullscreen_x() {
  _set_fullscreen();
  if (fl_mac_os_version < 101000) {
    // On OS X < 10.6, it is necessary to recreate the window. This is done with hide+show.
    // The alternative procedure isn't stable until MacOS 10.10
    hide();
    show();
  } else {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
    [i->xid setStyleMask:NSBorderlessWindowMask]; //10.6
#endif
    if ([i->xid isKeyWindow]) {
       if ([i->xid level] != NSStatusWindowLevel) {
         [i->xid setLevel:NSStatusWindowLevel];
         fixup_window_levels();
       }
    } else if([i->xid level] != NSNormalWindowLevel) {
       [i->xid setLevel:NSNormalWindowLevel];
       fixup_window_levels();
    }
    int sx, sy, sw, sh, X, Y, W, H;
    int top = fullscreen_screen_top;
    int bottom = fullscreen_screen_bottom;
    int left = fullscreen_screen_left;
    int right = fullscreen_screen_right;
    if ((top < 0) || (bottom < 0) || (left < 0) || (right < 0)) {
      top = Fl::screen_num(x(), y(), w(), h());
      bottom = top;
      left = top;
      right = top;
    }
    Fl::screen_xywh(sx, sy, sw, sh, top);
    Y = sy;
    Fl::screen_xywh(sx, sy, sw, sh, bottom);
    H = sy + sh - Y;
    Fl::screen_xywh(sx, sy, sw, sh, left);
    X = sx;
    Fl::screen_xywh(sx, sy, sw, sh, right);
    W = sx + sw - X;
    resize(X, Y, W, H);
  }
  Fl::handle(FL_FULLSCREEN, this);
}

void Fl_Window::fullscreen_off_x(int X, int Y, int W, int H) {
  _clear_fullscreen();
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
  if (fl_mac_os_version >= 100600) {
    NSInteger level = NSNormalWindowLevel;
    if (modal()) level = modal_window_level();
    else if (non_modal()) level = non_modal_window_level();
    BOOL has_focus = [i->xid isKeyWindow];
    [i->xid orderOut:nil];
    [i->xid setLevel:level];
    NSUInteger winstyle = (border() ?
                           (NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask) : NSBorderlessWindowMask);
    if (!modal()) winstyle |= NSMiniaturizableWindowMask;
    [i->xid setStyleMask:winstyle]; //10.6
    resize(X, Y, W, H);
    if (has_focus) [i->xid makeKeyAndOrderFront:nil];
    else [i->xid orderFront:nil];
  } else
#endif
  {
    hide();
    resize(X, Y, W, H);
    show();
  }
  Fl::handle(FL_FULLSCREEN, this);
}

/*
 * Initialize the given port for redraw and call the window's flush() to actually draw the content
 */ 
void Fl_X::flush()
{
  if (w->as_gl_window() || through_drawRect) {
    w->flush();
  } else {
    through_Fl_X_flush = YES;
    NSView *view = [xid contentView];
    if (views_use_CA) [view display];
    else {
      [view setNeedsDisplay:YES];
      [view displayIfNeededIgnoringOpacity];
    }
    through_Fl_X_flush = NO;
  }
}

/*
 * go ahead, create that (sub)window
 */
void Fl_X::make(Fl_Window* w)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    Fl_Group::current(0);
    fl_open_display();
    NSInteger winlevel = NSNormalWindowLevel;
    NSUInteger winstyle;
    if (w->parent()) {
      w->border(0);
      fl_show_iconic = 0;
    }
    if (w->border()) winstyle = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
    else winstyle = NSBorderlessWindowMask;
    if (fl_show_iconic && !w->parent()) { // prevent window from being out of work area when created iconized
      int sx, sy, sw, sh;
      Fl::screen_work_area (sx, sy, sw, sh, w->x(), w->y());
      if (w->x() < sx) w->x(sx);
      if (w->y() < sy) w->y(sy);
    }
    int xp = w->x();
    int yp = w->y();
    if (w->size_range_set) {
      if ( w->minh != w->maxh || w->minw != w->maxw) {
        if (w->border()) winstyle |= NSResizableWindowMask;
      }
    } else {
      if (w->resizable()) {
        Fl_Widget *o = w->resizable();
        int minw = o->w(); if (minw > 100) minw = 100;
        int minh = o->h(); if (minh > 100) minh = 100;
        w->size_range(w->w() - o->w() + minw, w->h() - o->h() + minh, 0, 0);
        if (w->border()) winstyle |= NSResizableWindowMask;
      } else {
        w->size_range(w->w(), w->h(), w->w(), w->h());
      }
    }
    int xwm = xp, ywm = yp, bt, bx, by;
    
    if (!fake_X_wm(w, xwm, ywm, bt, bx, by, w->maxw, w->minw, w->maxh, w->minh)) {
      // menu windows and tooltips
      if (w->modal()||w->tooltip_window()) {
        winlevel = modal_window_level();
      }
    }
    if (w->modal()) {
      winstyle &= ~NSMiniaturizableWindowMask;
      winlevel = modal_window_level();
    }
    else if (w->non_modal()) {
      winlevel = non_modal_window_level();
    }
    
    if (w->force_position()) {
      if (!Fl::grab()) {
        xp = xwm; yp = ywm;
        w->x(xp);w->y(yp);
      }
      xp -= bx;
      yp -= by+bt;
    }
  
    Fl_X* x = new Fl_X;
    x->other_xid = 0; // room for doublebuffering image map. On OS X this is only used by overlay windows
    x->region = 0;
    x->subRect(0);
    x->cursor = NULL;
    x->gc = 0;
    x->mapped_to_retina(false);
    x->changed_resolution(false);
    x->in_windowDidResize(false);
  
    NSRect crect;
    if (w->fullscreen_active()) {
      int top, bottom, left, right;
      int sx, sy, sw, sh, X, Y, W, H;

      top = w->fullscreen_screen_top;
      bottom = w->fullscreen_screen_bottom;
      left = w->fullscreen_screen_left;
      right = w->fullscreen_screen_right;

      if ((top < 0) || (bottom < 0) || (left < 0) || (right < 0)) {
        top = Fl::screen_num(w->x(), w->y(), w->w(), w->h());
        bottom = top;
        left = top;
        right = top;
      }

      Fl::screen_xywh(sx, sy, sw, sh, top);
      Y = sy;
      Fl::screen_xywh(sx, sy, sw, sh, bottom);
      H = sy + sh - Y;
      Fl::screen_xywh(sx, sy, sw, sh, left);
      X = sx;
      Fl::screen_xywh(sx, sy, sw, sh, right);
      W = sx + sw - X;

      w->resize(X, Y, W, H);

      winstyle = NSBorderlessWindowMask;
      winlevel = NSStatusWindowLevel;
    }
    crect.origin.x = w->x(); // correct origin set later for subwindows
    crect.origin.y = main_screen_height - (w->y() + w->h());
    crect.size.width=w->w(); 
    crect.size.height=w->h();
    FLWindow *cw = [[FLWindow alloc] initWithFl_W:w 
				      contentRect:crect  
					styleMask:winstyle];
    [cw setFrameOrigin:crect.origin];
    if (!w->parent()) {
      [cw setHasShadow:YES];
      [cw setAcceptsMouseMovedEvents:YES];
    }
    if (w->shape_data_) {
      [cw setOpaque:NO]; // shaped windows must be non opaque
      [cw setBackgroundColor:[NSColor clearColor]]; // and with transparent background color
      }
    x->xid = cw;
    x->w = w; w->i = x;
    x->wait_for_expose = 1;
    if (!w->parent()) {
      x->next = Fl_X::first;
      Fl_X::first = x;
    } else if (Fl_X::first) {
      x->next = Fl_X::first->next;
      Fl_X::first->next = x;
    }
    else {
      x->next = NULL;
      Fl_X::first = x;
    }
    FLView *myview = [[FLView alloc] initWithFrame:crect];
    [cw setContentView:myview];
    [myview release];
    [cw setLevel:winlevel];
    
    q_set_window_title(cw, w->label(), w->iconlabel());
    if (!w->force_position()) {
      if (w->modal()) {
        [cw center];
      } else if (w->non_modal()) {
        [cw center];
      } else if (!w->fullscreen_active()) {
        static NSPoint delta = NSZeroPoint;
        delta = [cw cascadeTopLeftFromPoint:delta];
      }
      crect = [cw frame]; // synchronize FLTK's and the system's window coordinates
      w->x(int(crect.origin.x));
      w->y(int(main_screen_height - (crect.origin.y + w->h())));
    }
    if(w->menu_window()) { // make menu windows slightly transparent
      [cw setAlphaValue:0.97];
    }
    // Install DnD handlers 
    [myview registerForDraggedTypes:[NSArray arrayWithObjects:UTF8_pasteboard_type,  NSFilenamesPboardType, nil]];
  
    if (w->size_range_set) w->size_range_();
    
    if ( w->border() || (!w->modal() && !w->tooltip_window()) ) {
      Fl_Tooltip::enter(0);
    }

    if (w->modal()) Fl::modal_ = w; 

    w->set_visible();
    if ( w->border() || (!w->modal() && !w->tooltip_window()) ) Fl::handle(FL_FOCUS, w);
    [cw setDelegate:[FLWindowDelegate singleInstance]];
  if (fl_show_iconic) {
    fl_show_iconic = 0;
    w->handle(FL_SHOW); // create subwindows if any
    if (fl_mac_os_version < 101300) { // TODO: threshold may be smaller
      // draw the window and its subwindows before its icon is computed
      [cw recursivelySendToSubwindows:@selector(display) applyToSelf:YES];
    }
    [cw miniaturize:nil];
  } else if (w->parent()) { // a subwindow
    [cw setIgnoresMouseEvents:YES]; // needs OS X 10.2
    // next 2 statements so a subwindow doesn't leak out of its parent window
    [cw setOpaque:NO];
    [cw setBackgroundColor:[NSColor clearColor]]; // transparent background color
    starting_moved_window = w;
    [cw setSubwindowFrame];
    starting_moved_window = NULL;
    // needed if top window was first displayed miniaturized
    FLWindow *pxid = fl_xid(w->top_window());
    [pxid makeFirstResponder:[pxid contentView]];
  } else { // a top-level window
    if ([cw canBecomeKeyWindow]) [cw makeKeyAndOrderFront:nil];
    else [cw orderFront:nil];
  }
  
    int old_event = Fl::e_number;
    w->handle(Fl::e_number = FL_SHOW);
    Fl::e_number = old_event;
    
    // if (w->modal()) { Fl::modal_ = w; fl_fix_focus(); }
    if (!w->parent()) [myview did_view_resolution_change]; // to set mapped_to_retina to its current state
    [pool release];
}


/*
 * Tell the OS what window sizes we want to allow
 */
void Fl_Window::size_range_() {
  int bx, by, bt;
  bt = get_window_frame_sizes(this, &bx, &by);
  size_range_set = 1;
  NSSize minSize = NSMakeSize(minw, minh + bt);
  NSSize maxSize = NSMakeSize(maxw?maxw:32000, maxh?maxh + bt:32000);
  if (i && i->xid) {
    [i->xid setMinSize:minSize];
    [i->xid setMaxSize:maxSize];
  }
}

void Fl_Window::wait_for_expose()
{
    if (fl_mac_os_version < 101300) {
      [fl_xid(this) recursivelySendToSubwindows:@selector(waitForExpose) applyToSelf:YES];
    } else {
      if (!shown()) return;
      while (!i || i->wait_for_expose) {
        Fl::wait();
      }
    }
}

/*
 * returns pointer to the filename, or null if name ends with ':'
 */
const char *fl_filename_name( const char *name ) 
{
  const char *p, *q;
  if (!name) return (0);
  for ( p = q = name ; *p ; ) {
    if ( ( p[0] == ':' ) && ( p[1] == ':' ) ) {
      q = p+2;
      p++;
    }
    else if (p[0] == '/') {
      q = p + 1;
    }
    p++;
  }
  return q;
}


/*
 * set the window title bar name
 */
void Fl_Window::label(const char *name, const char *mininame) {
  Fl_Widget::label(name);
  iconlabel_ = mininame;
  if (shown() || i) {
    q_set_window_title(i->xid, name, mininame);
  }
}


/*
 * make a window visible
 */
void Fl_Window::show() {
  image(Fl::scheme_bg_);
  if (Fl::scheme_bg_) {
    labeltype(FL_NORMAL_LABEL);
    align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
  } else {
    labeltype(FL_NO_LABEL);
  }
  Fl_Tooltip::exit(this);
  Fl_X *top = NULL;
  if (parent()) top = top_window()->i;
  if (!shown() && (!parent() || (top && ![top->xid isMiniaturized]))) {
    Fl_X::make(this);
  } else {
    if ( !parent() ) {
      if ([i->xid isMiniaturized]) {
        i->w->redraw();
        [i->xid deminiaturize:nil];
      }
      if (!fl_capture) {
        [i->xid makeKeyAndOrderFront:nil];
      }
    }
    else set_visible();
  }
}


/*
 * resize a window
 */
void Fl_Window::resize(int X, int Y, int W, int H) {
  if (!shown() && (X != x() || Y != y())) force_position(1);
  if (!shown() || Fl_X::i(this)->in_windowDidResize() || !visible_r()) {
    Fl_Group::resize(X, Y, W, H);
    if (!shown()) init_sizes();
  } else {
    NSPoint pt = FLTKtoCocoa(this, X, Y, H);
    FLWindow *xid = fl_xid(this);
    if (W != w() || H != h()) {
      NSRect r;
      int bt = get_window_frame_sizes(this);
      r.origin = pt;
      r.size.width = W;
      r.size.height = H + bt;
      if (NSEqualRects(r, [xid frame])) {
        Fl_Group::resize(X, Y, W, H); // runs rarely, e.g. with scaled down test/tabs
        redraw();
      } else {
        [xid setFrame:r display:YES];
        [[xid contentView] displayIfNeededIgnoringOpacity];
      }
    }
    else {
      if (parent()) starting_moved_window = this;
      [xid setFrameOrigin:pt]; // set cocoa coords to FLTK position
      x(X); y(Y); // useful when frame did not move but X or Y changed
      if (parent()) starting_moved_window = NULL;
    }
  }
}


/*
 * make all drawing go into this window (called by subclass flush() impl.)
 
 This can be called in 3 different situations:
 
 1) When a window is created, resized or moved between low/high resolution displays.
 macOS sends the drawRect: message to the window view after having prepared the
 current graphics context to draw to this view.  The drawRect: method sets through_drawRect
 to YES and calls Fl_X::flush_window(). Fl_X::flush_window() calls
 Fl_Window::flush() that calls Fl_Window::make_current() that uses the graphics
 context of the window or the layer. The window's draw() function is then executed.
 
 2) At each round of the FLTK event loop.
 Fl::flush() is called, that calls Fl_X::flush() on each window that needs drawing.
 Fl_X::flush() sets through_Fl_X_Flush to YES and marks the view as
 needing display. The view is sent the displayIfNeededIgnoringOpacity or display message which makes
 the OS send the view the drawRect: message. The program proceeds next as in 1) above.
 
 3) An FLTK application can call Fl_Window::make_current() at any time before it draws to a window.
 This occurs for instance in the idle callback function of the mandelbrot test program. Variables
 through_Fl_X_flush and through_drawRect equal NO.
 Before 10.14: The window graphics context is obtained. Subsequent drawing requests go to the window.
 After 10.14: The layered view is marked as needing display. It will be sent the drawRect: message
 at the next event loop. Subsequent drawing operations, until drawRect: runs, are sent to view->aux_bitmap.
 
 CAUTION: it's not possible to call Fl::wait(), Fl::check() nor Fl::ready() while in the draw()
 function of a widget. Use an idle callback instead.
 */
void Fl_Window::make_current() 
{
  Fl_X::q_release_context();
  Fl_X *i = Fl_X::i(this);
  //NSLog(@"region-count=%d damage=%u",i->region?i->region->count:0, pWindow->damage());
  fl_window = i->xid;
  current_ = this;
  Fl_X::set_high_resolution( i->mapped_to_retina() );
  #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
    FLView *view = (FLView*)[fl_window contentView];
    if (views_use_CA && !through_drawRect) { // detect direct calls from the app
      [view setNeedsDisplay:YES];
    }
    if (views_use_CA && view->aux_bitmap) {
      i->gc = view->aux_bitmap;
    } else
  #endif
    {
      NSGraphicsContext *nsgc =   through_drawRect ? [NSGraphicsContext currentContext] : [NSGraphicsContext graphicsContextWithWindow:fl_window];
      static SEL gc_sel = fl_mac_os_version >= 101000 ? @selector(CGContext) : @selector(graphicsPort);
      i->gc = (CGContextRef)[nsgc performSelector:gc_sel];
    }
  
  fl_gc = i->gc;
  CGContextSaveGState(fl_gc); // native context
  // antialiasing must be deactivated because it applies to rectangles too
  // and escapes even clipping!!!
  // it gets activated when needed (e.g., draw text)
  CGContextSetShouldAntialias(fl_gc, false);
  CGFloat hgt = [[fl_window contentView] frame].size.height;
  CGContextTranslateCTM(fl_gc, 0.5f, hgt-0.5f);
  CGContextScaleCTM(fl_gc, 1.0f, -1.0f); // now 0,0 is top-left point of the window
  // for subwindows, limit drawing to inside of parent window
  // half pixel offset is necessary for clipping as done by fl_cgrectmake_cocoa()
  if (i->subRect()) {
    CGContextClipToRect(fl_gc, CGRectOffset(*(i->subRect()), -0.5, -0.5));
  }
// this is the context with origin at top left of (sub)window
  CGContextSaveGState(fl_gc);
#if defined(FLTK_USE_CAIRO)
  if (Fl::cairo_autolink_context()) Fl::cairo_make_current(this); // capture gc changes automatically to update the cairo context adequately
#endif
  fl_clip_region( 0 );
  
#if defined(FLTK_USE_CAIRO)
  // update the cairo_t context
  if (Fl::cairo_autolink_context()) Fl::cairo_make_current(this);
#endif
}

// helper function to manage the current CGContext fl_gc
extern void fl_quartz_restore_line_style_();

// FLTK has only one global graphics state. This function copies the FLTK state into the
// current Quartz context
void Fl_X::q_fill_context() {
  if (!fl_gc) return;
  if ( ! fl_window) { // a bitmap context
    CGFloat hgt = CGBitmapContextGetHeight(fl_gc);
    CGAffineTransform at = CGContextGetCTM(fl_gc);
    CGFloat offset = 0.5;
    if (at.a != 1 && at.a == at.d && at.b == 0 && at.c == 0) {
      hgt /= at.a;
      offset /= at.a;
    }
    CGContextTranslateCTM(fl_gc, offset, hgt-offset);
    CGContextScaleCTM(fl_gc, 1.0f, -1.0f); // now 0,0 is top-left point of the context
    }
  fl_color(fl_graphics_driver->color());
  fl_quartz_restore_line_style_();
}

// The only way to reset clipping to its original state is to pop the current graphics
// state and restore the global state.
void Fl_X::q_clear_clipping() {
  if (!fl_gc) return;
  CGContextRestoreGState(fl_gc);
  CGContextSaveGState(fl_gc);
}

// Give the Quartz context back to the system
void Fl_X::q_release_context(Fl_X *x) {
  if (x && x->gc!=fl_gc) return;
  if (!fl_gc) return;
  CGContextRestoreGState(fl_gc); // match the CGContextSaveGState's of make_current
  CGContextRestoreGState(fl_gc);
  Fl_X::set_high_resolution(false);
  CGContextFlush(fl_gc);
  fl_gc = 0;
#if defined(FLTK_USE_CAIRO)
  if (Fl::cairo_autolink_context()) Fl::cairo_make_current((Fl_Window*) 0); // capture gc changes automatically to update the cairo context adequately
#endif
}

void Fl_X::q_begin_image(CGRect &rect, int cx, int cy, int w, int h) {
  CGContextSaveGState(fl_gc);
  CGRect r2 = rect;
  r2.origin.x -= 0.5f;
  r2.origin.y -= 0.5f;
  CGContextClipToRect(fl_gc, r2);
  // move graphics context to origin of vertically reversed image
  // The 0.5 here cancels the 0.5 offset present in Quartz graphics contexts.
  // Thus, image and surface pixels are in phase if there's no scaling.
  // Below, we handle x2 and /2 scalings that occur when drawing to
  // a double-resolution bitmap, and when drawing a double-resolution bitmap to display.
  CGContextTranslateCTM(fl_gc, rect.origin.x - cx - 0.5, rect.origin.y - cy + h - 0.5);
  CGContextScaleCTM(fl_gc, 1, -1);
  CGAffineTransform at = CGContextGetCTM(fl_gc);
  if (at.a == at.d && at.b == 0 && at.c == 0) { // proportional scaling, no rotation
    // phase image with display pixels
    CGFloat deltax = 0, deltay = 0;
    if (at.a == 2) { // make .tx and .ty have even values
      deltax = (at.tx/2 - round(at.tx/2));
      deltay = (at.ty/2 - round(at.ty/2));
    } else if (at.a == 0.5) {
      if (Fl_Display_Device::high_resolution()) { // make .tx and .ty have int or half-int values
        deltax = -(at.tx*2 - round(at.tx*2));
        deltay = (at.ty*2 - round(at.ty*2));
      } else { // make .tx and .ty have integral values
        deltax = (at.tx - round(at.tx))*2;
        deltay = (at.ty - round(at.ty))*2;
      }
    }
    CGContextTranslateCTM(fl_gc, -deltax, -deltay);
  }
  rect.origin.x = rect.origin.y = 0;
  rect.size.width = w;
  rect.size.height = h;
}

void Fl_X::q_end_image() {
  CGContextRestoreGState(fl_gc);
}

void Fl_X::set_high_resolution(bool new_val)
{
  Fl_Display_Device::high_res_window_ = new_val;
}

static NSBitmapImageRep *pdf_to_nsbitmapimagerep(NSData *pdfdata) {
  NSImage *image = [[NSImage alloc] initWithData:pdfdata];
  NSInteger width = [image size].width * 2;
  NSInteger height = [image size].height * 2;
  NSBitmapImageRep *bitmap = [NSBitmapImageRep alloc];
  NSRect dest_r = NSMakeRect(0, 0, width, height);
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_9
  if (fl_mac_os_version >= 100900) {
    // This procedure is necessary because initWithFocusedViewRect is deprecated in macOS 10.14
    // and because it produces a bitmap with floating point pixel values with macOS 11.x on arm64
    bitmap = [bitmap initWithBitmapDataPlanes:NULL
                                   pixelsWide:width
                                   pixelsHigh:height
                                bitsPerSample:8
                              samplesPerPixel:4
                                     hasAlpha:YES
                                     isPlanar:NO
                               colorSpaceName:NSDeviceRGBColorSpace
                                  bytesPerRow:4 * width
                                 bitsPerPixel:32];
    NSAutoreleasePool *localPool = [[NSAutoreleasePool alloc] init];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:bitmap]];// 10.4
    [image drawInRect:dest_r]; // 10.9
    [NSGraphicsContext restoreGraphicsState];
    [localPool release];
  } else
#endif
  {
    [image lockFocus];
    bitmap = [bitmap initWithFocusedViewRect:dest_r]; // deprecated 10.14
    [image unlockFocus];
  }
  [bitmap setSize:[image size]];
  [image release];
  return bitmap;
}

void Fl_Copy_Surface::complete_copy_pdf_and_tiff()
{
  CGContextRestoreGState(gc);
  CGContextEndPage(gc);
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
  if (fl_mac_os_version >= 100500) CGPDFContextClose(gc); // needs 10.5, necessary with macOS 10.15
#endif
  CGContextRelease(gc);
  NSPasteboard *clip = [NSPasteboard generalPasteboard];
  [clip declareTypes:[NSArray arrayWithObjects:PDF_pasteboard_type, TIFF_pasteboard_type, nil] owner:nil];
  [clip setData:(NSData*)pdfdata forType:PDF_pasteboard_type];
  //second, transform this PDF to a bitmap image and put it as tiff in clipboard with retina resolution
  NSBitmapImageRep *bitmap = pdf_to_nsbitmapimagerep((NSData*)pdfdata);
  CFRelease(pdfdata);
  [clip setData:[bitmap TIFFRepresentation] forType:TIFF_pasteboard_type];
  [bitmap release];
}

////////////////////////////////////////////////////////////////
// Copy & Paste fltk implementation.
////////////////////////////////////////////////////////////////

static size_t convert_crlf(char * s, size_t len)
{
  // turn \r characters into \n and "\r\n" sequences into \n:
  char *p;
  size_t l = len;
  while ((p = strchr(s, '\r'))) {
    if (*(p+1) == '\n') {
      memmove(p, p+1, l-(p-s));
      len--; l--;
    } else *p = '\n';
    l -= p-s;
    s = p + 1;
  }
  return len;
}

// clipboard variables definitions :
char *fl_selection_buffer[2] = {NULL, NULL};
int fl_selection_length[2] = {0, 0};
static int fl_selection_buffer_length[2];

extern void fl_trigger_clipboard_notify(int source);

void fl_clipboard_notify_change() {
  // No need to do anything here...
}

static void clipboard_check(void)
{
  static NSInteger oldcount = -1;
  NSInteger newcount = [[NSPasteboard generalPasteboard] changeCount];
  if (newcount == oldcount) return;
  oldcount = newcount;
  fl_trigger_clipboard_notify(1);
}

static void resize_selection_buffer(int len, int clipboard) {
  if (len <= fl_selection_buffer_length[clipboard])
    return;
  delete[] fl_selection_buffer[clipboard];
  fl_selection_buffer[clipboard] = new char[len+100];
  fl_selection_buffer_length[clipboard] = len+100;
}

/*
 * create a selection
 * stuff: pointer to selected data
 * len: size of selected data
 * type: always "plain/text" for now
 */
void Fl::copy(const char *stuff, int len, int clipboard, const char *type) {
  if (!stuff || len<0) return;
  if (clipboard >= 2)
    clipboard = 1; // Only on X11 do multiple clipboards make sense.

  resize_selection_buffer(len+1, clipboard);
  memcpy(fl_selection_buffer[clipboard], stuff, len);
  fl_selection_buffer[clipboard][len] = 0; // needed for direct paste
  fl_selection_length[clipboard] = len;
  if (clipboard) {
    CFDataRef text = CFDataCreate(kCFAllocatorDefault, (UInt8*)fl_selection_buffer[1], len);
    if (text==NULL) return; // there was a pb creating the object, abort.
    NSPasteboard *clip = [NSPasteboard generalPasteboard];
    [clip declareTypes:[NSArray arrayWithObject:UTF8_pasteboard_type] owner:nil];
    [clip setData:(NSData*)text forType:UTF8_pasteboard_type];
    CFRelease(text);
  }
}

static int get_plain_text_from_clipboard(int clipboard)
{
  NSInteger length = 0;
  NSPasteboard *clip = [NSPasteboard generalPasteboard];
  NSString *found = [clip availableTypeFromArray:[NSArray arrayWithObjects:UTF8_pasteboard_type, @"public.utf16-plain-text", @"com.apple.traditional-mac-plain-text", nil]];
  if (found) {
    NSData *data = [clip dataForType:found];
    if (data) {
      NSInteger len;
      char *aux_c = NULL;
      if (![found isEqualToString:UTF8_pasteboard_type]) {
	NSString *auxstring;
	auxstring = (NSString *)CFStringCreateWithBytes(NULL, 
							(const UInt8*)[data bytes], 
							[data length],
							[found isEqualToString:@"public.utf16-plain-text"] ? kCFStringEncodingUnicode : kCFStringEncodingMacRoman,
							false);
	aux_c = strdup([auxstring UTF8String]);
	[auxstring release];
	len = strlen(aux_c) + 1;
      }
      else len = [data length] + 1;
      resize_selection_buffer(len, clipboard);
      if (![found isEqualToString:UTF8_pasteboard_type]) {
        strcpy(fl_selection_buffer[clipboard], aux_c);
        free(aux_c);
      }
      else {
        [data getBytes:fl_selection_buffer[clipboard]];
      }
      fl_selection_buffer[clipboard][len - 1] = 0;
      length = convert_crlf(fl_selection_buffer[clipboard], len - 1); // turn all \r characters into \n:
      Fl::e_clipboard_type = Fl::clipboard_plain_text;
    }
  }    
  return length;
}

static Fl_Image* get_image_from_clipboard(Fl_Widget *receiver)
{
  NSPasteboard *clip = [NSPasteboard generalPasteboard];
  NSArray *present = [clip types]; // types in pasteboard in order of decreasing preference
  NSArray  *possible = [NSArray arrayWithObjects:TIFF_pasteboard_type, PDF_pasteboard_type, PICT_pasteboard_type, nil];
  NSString *found = nil;
  NSUInteger rank;
  for (NSUInteger i = 0; (!found) && i < [possible count]; i++) {
    for (rank = 0; rank < [present count]; rank++) { // find first of possible types present in pasteboard
      if ([[present objectAtIndex:rank] isEqualToString:[possible objectAtIndex:i]]) {
        found = [present objectAtIndex:rank];
        break;
      }
    }
  }
  if (!found) return NULL;
  NSData *data = [clip dataForType:found];
  if (!data) return NULL;
  NSBitmapImageRep *bitmap = nil;
  if ([found isEqualToString:TIFF_pasteboard_type]) {
    bitmap = [[NSBitmapImageRep alloc] initWithData:data];
  }
  else if ([found isEqualToString:PDF_pasteboard_type] || [found isEqualToString:PICT_pasteboard_type]) {
    bitmap = pdf_to_nsbitmapimagerep(data);
  }
  if (!bitmap) return NULL;
  int bytesPerPixel([bitmap bitsPerPixel]/8);
  int bpr([bitmap bytesPerRow]);
  int hh([bitmap pixelsHigh]);
  int ww([bitmap pixelsWide]);
  uchar *imagedata = new uchar[bpr * hh];
  memcpy(imagedata, [bitmap bitmapData], bpr * hh);
  Fl_RGB_Image *image = new Fl_RGB_Image(imagedata, ww, hh, bytesPerPixel, (bpr == ww * bytesPerPixel ? 0 : bpr) );
  image->alloc_array = 1;
  [bitmap release];
  Fl::e_clipboard_type = Fl::clipboard_image;
  return image;
}

// Call this when a "paste" operation happens:
void Fl::paste(Fl_Widget &receiver, int clipboard, const char *type) {
  if (type[0] == 0) type = Fl::clipboard_plain_text;
  if (clipboard) {
    Fl::e_clipboard_type = "";
    if (strcmp(type, Fl::clipboard_plain_text) == 0) {
      fl_selection_length[1] = get_plain_text_from_clipboard(1);
    }
    else if (strcmp(type, Fl::clipboard_image) == 0) {
      Fl::e_clipboard_data = get_image_from_clipboard(&receiver);
      if (Fl::e_clipboard_data) {
        int done = receiver.handle(FL_PASTE);
        Fl::e_clipboard_type = "";
        if (done == 0) {
          delete (Fl_Image*)Fl::e_clipboard_data;
          Fl::e_clipboard_data = NULL;
        }
      }
      return;
    }
    else
      fl_selection_length[1] = 0;
  }
  Fl::e_text = fl_selection_buffer[clipboard];
  Fl::e_length = fl_selection_length[clipboard];
  if (!Fl::e_length) Fl::e_text = (char *)"";
  receiver.handle(FL_PASTE);
}

int Fl::clipboard_contains(const char *type) {
  NSString *found = nil;
  if (strcmp(type, Fl::clipboard_plain_text) == 0) {
    found = [[NSPasteboard generalPasteboard] availableTypeFromArray:[NSArray arrayWithObjects:UTF8_pasteboard_type, @"public.utf16-plain-text", @"com.apple.traditional-mac-plain-text", nil]];
    }
  else if (strcmp(type, Fl::clipboard_image) == 0) {
    found = [[NSPasteboard generalPasteboard] availableTypeFromArray:[NSArray arrayWithObjects:TIFF_pasteboard_type, PDF_pasteboard_type, PICT_pasteboard_type, nil]];
    }
  return found != nil;
}

void Fl_X::destroy() {
  [[xid parentWindow] removeChildWindow:xid]; // necessary until 10.6 at least
  if (xid) {
    [xid close];
  }
  delete subRect();
}

void Fl_X::map() {
  if (w && xid && ![xid parentWindow]) { // 10.2
    // after a subwindow has been unmapped, it has lost its parent window and its frame may be wrong
    [xid setSubwindowFrame];
  }
  if (cursor) {
    [(NSCursor*)cursor release];
    cursor = NULL;
  }
}

void Fl_X::unmap() {
  if (w && xid) {
    if (w->parent()) [[xid parentWindow] removeChildWindow:xid]; // necessary with at least 10.5
    [xid orderOut:nil];
  }
}

// intersects current and x,y,w,h rectangle and returns result as a new Fl_Region
Fl_Region Fl_X::intersect_region_and_rect(Fl_Region current, int x,int y,int w, int h)
{
  if (current == NULL) return XRectangleRegion(x,y,w,h);
  CGRect r = fl_cgrectmake_cocoa(x, y, w, h);
  Fl_Region outr = (Fl_Region)malloc(sizeof(*outr));
  outr->count = current->count;
  outr->rects =(CGRect*)malloc(outr->count * sizeof(CGRect));
  int j = 0;
  for(int i = 0; i < current->count; i++) {
    CGRect test = CGRectIntersection(current->rects[i], r);
    if (!CGRectIsEmpty(test)) outr->rects[j++] = test;
  }
  if (j) {
    outr->count = j;
    outr->rects = (CGRect*)realloc(outr->rects, outr->count * sizeof(CGRect));
  }
  else {
    XDestroyRegion(outr);
    outr = XRectangleRegion(0,0,0,0);
  }
  return outr;
}

void Fl_X::collapse() {
  [xid miniaturize:nil];
}

static NSImage *CGBitmapContextToNSImage(CGContextRef c)
// the returned NSImage is autoreleased
{
  NSImage* image;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
  if (fl_mac_os_version >= 100600) {
    CGImageRef cgimg = CGBitmapContextCreateImage(c);  // requires 10.4
    image = [[NSImage alloc] initWithCGImage:cgimg size:NSZeroSize]; // requires 10.6
    CFRelease(cgimg);
  }
  else 
#endif
    {
      NSBitmapImageRep *imagerep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
									   pixelsWide:CGBitmapContextGetWidth(c)
									   pixelsHigh:CGBitmapContextGetHeight(c)
									bitsPerSample:8
								      samplesPerPixel:4
									     hasAlpha:YES
									     isPlanar:NO
								       colorSpaceName:NSDeviceRGBColorSpace
									  bytesPerRow:CGBitmapContextGetBytesPerRow(c)
									 bitsPerPixel:CGBitmapContextGetBitsPerPixel(c)];
      memcpy([imagerep bitmapData], CGBitmapContextGetData(c), [imagerep bytesPerRow] * [imagerep pixelsHigh]);
      image = [[NSImage alloc] initWithSize:NSMakeSize([imagerep pixelsWide], [imagerep pixelsHigh])];
      [image addRepresentation:imagerep];
      [imagerep release];
    }
  return [image autorelease];
}

int Fl_X::set_cursor(Fl_Cursor c)
{
  if (cursor) {
    [(NSCursor*)cursor release];
    cursor = NULL;
  }

  switch (c) {
  case FL_CURSOR_ARROW:   cursor = [NSCursor arrowCursor]; break;
  case FL_CURSOR_CROSS:   cursor = [NSCursor crosshairCursor]; break;
  case FL_CURSOR_INSERT:  cursor = [NSCursor IBeamCursor]; break;
  case FL_CURSOR_HAND:    cursor = [NSCursor pointingHandCursor]; break;
  case FL_CURSOR_MOVE:    cursor = [NSCursor openHandCursor]; break;
  case FL_CURSOR_NS:      cursor = [NSCursor resizeUpDownCursor]; break;
  case FL_CURSOR_WE:      cursor = [NSCursor resizeLeftRightCursor]; break;
  case FL_CURSOR_N:       cursor = [NSCursor resizeUpCursor]; break;
  case FL_CURSOR_E:       cursor = [NSCursor resizeRightCursor]; break;
  case FL_CURSOR_W:       cursor = [NSCursor resizeLeftCursor]; break;
  case FL_CURSOR_S:       cursor = [NSCursor resizeDownCursor]; break;
  default:
    return 0;
  }

  [(NSCursor*)cursor retain];

  [(NSWindow*)xid invalidateCursorRectsForView:[(NSWindow*)xid contentView]];

  return 1;
}

int Fl_X::set_cursor(const Fl_RGB_Image *image, int hotx, int hoty) {
  if (cursor) {
    [(NSCursor*)cursor release];
    cursor = NULL;
  }

  if ((hotx < 0) || (hotx >= image->w()))
    return 0;
  if ((hoty < 0) || (hoty >= image->h()))
    return 0;

  // OS X >= 10.6 can create a NSImage from a CGImage, but we need to
  // support older versions, hence this pesky handling.

  NSBitmapImageRep *bitmap = [[NSBitmapImageRep alloc]
                              initWithBitmapDataPlanes:NULL
                              pixelsWide:image->w()
                              pixelsHigh:image->h()
                              bitsPerSample:8
                              samplesPerPixel:image->d()
                              hasAlpha:!(image->d() & 1)
                              isPlanar:NO
                              colorSpaceName:(image->d()<=2) ? NSDeviceWhiteColorSpace : NSDeviceRGBColorSpace
                              bytesPerRow:(image->w() * image->d())
                              bitsPerPixel:(image->d()*8)];

  // Alpha needs to be premultiplied for this format

  const uchar *i = (const uchar*)*image->data();
  const int extra_data = image->ld() ? (image->ld() - image->w() * image->d()) : 0;
  unsigned char *o = [bitmap bitmapData];
  for (int y = 0;y < image->h();y++) {
    if (!(image->d() & 1)) {
      for (int x = 0;x < image->w();x++) {
        unsigned int alpha;
        if (image->d() == 4) {
          alpha = i[3];
          *o++ = (unsigned char)((unsigned int)*i++ * alpha / 255);
          *o++ = (unsigned char)((unsigned int)*i++ * alpha / 255);
        }

        alpha = i[1];
        *o++ = (unsigned char)((unsigned int)*i++ * alpha / 255);
        *o++ = alpha;
        i++;
  }
    } else {
      // No alpha, so we can just copy everything directly.
      int len = image->w() * image->d();
      memcpy(o, i, len);
      o += len;
      i += len;
    }
    i += extra_data;
  }

  NSImage *nsimage = [[NSImage alloc]
                      initWithSize:NSMakeSize(image->w(), image->h())];

  [nsimage addRepresentation:bitmap];

  cursor = [[NSCursor alloc]
            initWithImage:nsimage
            hotSpot:NSMakePoint(hotx, hoty)];

  [(NSWindow*)xid invalidateCursorRectsForView:[(NSWindow*)xid contentView]];

  [bitmap release];
  [nsimage release];

  return 1;
}

@interface FLaboutItemTarget : NSObject 
{
}
- (BOOL)validateMenuItem:(NSMenuItem *)item;
- (void)showPanel;
- (void)printPanel;
- (void)terminate:(id)sender;
@end
@implementation FLaboutItemTarget
- (BOOL)validateMenuItem:(NSMenuItem *)item
{ // invalidate the Quit item of the application menu when running modal or when in native file chooser
  if ([[NSApp keyWindow] isKindOfClass:[NSSavePanel class]]) return NO;
  if (!Fl::modal() || [item action] != @selector(terminate:)) return YES;
  return NO;
}
- (void)showPanel
{
    NSDictionary *options;
    options = [NSDictionary dictionaryWithObjectsAndKeys:
	       [[[NSAttributedString alloc] 
		initWithString:[NSString stringWithFormat:@" GUI with FLTK %d.%d", 
		FL_MAJOR_VERSION, FL_MINOR_VERSION ]] autorelease], @"Credits",
                	     nil];
    [NSApp orderFrontStandardAboutPanelWithOptions:options];
}
//#include <FL/Fl_PostScript.H>
- (void)printPanel
{  
  Fl_Printer printer;
  //Fl_PostScript_File_Device printer;
  int w, h, ww, wh;
  Fl_Window *win = Fl::first_window();
  if(!win) return;
  if (win->parent()) win = win->top_window();
  if( printer.start_job(1) ) return;
  if( printer.start_page() ) return;
  fl_lock_function();
  // scale the printer device so that the window fits on the page
  float scale = 1;
  printer.printable_rect(&w, &h);
  ww = win->decorated_w();
  wh = win->decorated_h();
  if (ww>w || wh>h) {
    scale = (float)w/win->w();
    if ((float)h/wh < scale) scale = (float)h/wh;
    printer.scale(scale);
    printer.printable_rect(&w, &h);
  }
//#define ROTATE 1
#ifdef ROTATE
  printer.scale(scale * 0.8, scale * 0.8);
  printer.printable_rect(&w, &h);
  printer.origin(w/2, h/2 );
  printer.rotate(20.);
#else
  printer.origin(w/2, h/2);
#endif
  printer.print_window(win, -ww/2, -wh/2);
  //printer.print_window_part(win,0,0,win->w(),win->h(), -ww/2, -wh/2);
  printer.end_page();
  printer.end_job();
  fl_unlock_function();
}
- (void)terminate:(id)sender
{
  [NSApp terminate:sender];
}
@end


static void createAppleMenu(void)
{
  static BOOL donethat = NO;
  if (donethat) return;
  donethat = YES;
  NSMenu *mainmenu, *services = nil, *appleMenu;
  NSMenuItem *menuItem;
  NSString *title;
  
  SEL infodictSEL = (fl_mac_os_version >= 100200 ? @selector(localizedInfoDictionary) : @selector(infoDictionary));
  NSString *nsappname = [[[NSBundle mainBundle] performSelector:infodictSEL] objectForKey:@"CFBundleName"];  
  if (nsappname == nil)
    nsappname = [[NSProcessInfo processInfo] processName];
  appleMenu = [[NSMenu alloc] initWithTitle:@""];
  /* Add menu items */
  title = [NSString stringWithFormat:NSLocalizedString([NSString stringWithUTF8String:Fl_Mac_App_Menu::about],nil), nsappname];
  menuItem = [appleMenu addItemWithTitle:title action:@selector(showPanel) keyEquivalent:@""];
  FLaboutItemTarget *about = [[FLaboutItemTarget alloc] init];
  [menuItem setTarget:about];
  [appleMenu addItem:[NSMenuItem separatorItem]];
  // Print front window
  title = NSLocalizedString([NSString stringWithUTF8String:Fl_Mac_App_Menu::print], nil);
  if ([title length] > 0) {
    menuItem = [appleMenu 
		addItemWithTitle:title
		action:@selector(printPanel) 
		keyEquivalent:@""];
    [menuItem setTarget:about];
    [menuItem setEnabled:YES];
    [appleMenu addItem:[NSMenuItem separatorItem]];
    }
  if (fl_mac_os_version >= 100400) { // services+hide+quit already in menu in OS 10.3
    // Services Menu
    services = [[NSMenu alloc] initWithTitle:@""];
    menuItem = [appleMenu 
		addItemWithTitle:NSLocalizedString([NSString stringWithUTF8String:Fl_Mac_App_Menu::services], nil)
		action:nil 
		keyEquivalent:@""];
    [appleMenu setSubmenu:services forItem:menuItem];
    [appleMenu addItem:[NSMenuItem separatorItem]];
    // Hide AppName
    title = [NSString stringWithFormat:NSLocalizedString([NSString stringWithUTF8String:Fl_Mac_App_Menu::hide],nil), nsappname];
    [appleMenu addItemWithTitle:title 
			 action:@selector(hide:) 
		  keyEquivalent:@"h"];
    // Hide Others
    menuItem = [appleMenu 
		addItemWithTitle:NSLocalizedString([NSString stringWithUTF8String:Fl_Mac_App_Menu::hide_others] , nil)
		action:@selector(hideOtherApplications:) 
		keyEquivalent:@"h"];
    [menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];
    // Show All
    [appleMenu addItemWithTitle:NSLocalizedString([NSString stringWithUTF8String:Fl_Mac_App_Menu::show] , nil)
			 action:@selector(unhideAllApplications:) keyEquivalent:@""];
    [appleMenu addItem:[NSMenuItem separatorItem]];
    // Quit AppName
    title = [NSString stringWithFormat:NSLocalizedString([NSString stringWithUTF8String:Fl_Mac_App_Menu::quit] , nil),
	     nsappname];
    menuItem = [appleMenu addItemWithTitle:title
                                    action:@selector(terminate:)
                             keyEquivalent:@"q"];
    [menuItem setTarget:about];
    }
  /* Put menu into the menubar */
  menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
  [menuItem setSubmenu:appleMenu];
  mainmenu = [[NSMenu alloc] initWithTitle:@""];
  [mainmenu addItem:menuItem];
  if (fl_mac_os_version < 100600) {
    //	[NSApp setAppleMenu:appleMenu];
    //	to avoid compiler warning raised by use of undocumented setAppleMenu	:
    [NSApp performSelector:@selector(setAppleMenu:) withObject:appleMenu];
  }
  [NSApp setMainMenu:mainmenu];
  if (services) {
    [NSApp setServicesMenu:services];
    [services release];
    }
  [mainmenu release];
  [appleMenu release];
  [menuItem release];
}


void Fl_X::set_key_window()
{
  [xid makeKeyWindow];
}

static NSImage *imageFromText(const char *text, int *pwidth, int *pheight)
{
  const char *p, *q;
  int width = 0, height, w2, ltext = strlen(text);
  fl_font(FL_HELVETICA, 10);
  p = text;
  int nl = 0;
  while(nl < 100 && (q=strchr(p, '\n')) != NULL) { 
    nl++; 
    w2 = int(fl_width(p, q - p));
    if (w2 > width) width = w2;
    p = q + 1; 
  }
  if (text[ ltext - 1] != '\n') {
    nl++;
    w2 = int(fl_width(p));
    if (w2 > width) width = w2;
  }
  height = nl * fl_height() + 3;
  width += 6;
  Fl_Offscreen off = Fl_Quartz_Graphics_Driver::create_offscreen_with_alpha(width, height);
  fl_begin_offscreen(off);
  CGContextSetRGBFillColor( (CGContextRef)off, 0,0,0,0);
  fl_rectf(0,0,width,height);
  fl_color(FL_BLACK);
  p = text;
  fl_font(FL_HELVETICA, 10);
  int y = fl_height();
  while(TRUE) {
    q = strchr(p, '\n');
    if (q) {
      fl_draw(p, q - p, 3, y);
    } else {
      fl_draw(p, 3, y);
      break;
    }
    y += fl_height();
    p = q + 1;
  }
  fl_end_offscreen();
  NSImage* image = CGBitmapContextToNSImage( (CGContextRef)off );
  fl_delete_offscreen( off );
  *pwidth = width;
  *pheight = height;
  return image;
}

static NSImage *defaultDragImage(int *pwidth, int *pheight)
{
  const int version_threshold = 100700;
  int width, height;
  if (fl_mac_os_version >= version_threshold) {
    width = 50; height = 40;
    }
  else {
    width = 16; height = 16;
    }
  void *data = calloc(width * height, 4);
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  CGContextRef off = CGBitmapContextCreate(data, width, height, 8, width*4, lut, kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(lut);
  fl_begin_offscreen(off);
  if (fl_mac_os_version >= version_threshold) {
    fl_font(FL_HELVETICA, 20);
    fl_color(FL_BLACK);
    char str[4];
    int l = fl_utf8encode(0x1F69A, str); // the "Delivery truck" Unicode character from "Apple Color Emoji" font
    fl_draw(str, l, 1, 16);
    }
  else { // draw two squares
    CGContextSetRGBFillColor( (CGContextRef)off, 0,0,0,0);
    fl_rectf(0,0,width,height);
    CGContextSetRGBStrokeColor( (CGContextRef)off, 0,0,0,0.6);
    fl_rect(0,0,width,height);
    fl_rect(2,2,width-4,height-4);
  }
  fl_end_offscreen();
  NSImage* image = CGBitmapContextToNSImage( (CGContextRef)off );
  fl_delete_offscreen( off );
  *pwidth = width;
  *pheight = height;
  return image;
}

int Fl::dnd()
{
  return Fl_X::dnd(0);
}

int Fl_X::dnd(int use_selection)
{
  CFDataRef text = CFDataCreate(kCFAllocatorDefault, (UInt8*)fl_selection_buffer[0], fl_selection_length[0]);
  if (text==NULL) return false;
  NSAutoreleasePool *localPool;
  localPool = [[NSAutoreleasePool alloc] init]; 
  Fl_Widget *w = Fl::pushed();
  Fl_Window *win = w->top_window();
  FLView *myview = (FLView*)[Fl_X::i(win)->xid contentView];
  NSEvent *theEvent = [NSApp currentEvent];
  
  int width, height;
  NSImage *image;
  if (use_selection) {
    fl_selection_buffer[0][ fl_selection_length[0] ] = 0;
    image = imageFromText(fl_selection_buffer[0], &width, &height);
  } else {
    image = defaultDragImage(&width, &height);
  }
  
  NSPoint pt = [theEvent locationInWindow];
  pt.x -= width/2;
  pt.y -= height/2;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
  if (fl_mac_os_version >= 100700) {
    NSPasteboardItem *pbItem = [[[NSPasteboardItem alloc] init] autorelease];
    [pbItem setData:(NSData*)text forType:UTF8_pasteboard_type];
    NSDraggingItem *dragItem = [[[NSDraggingItem alloc] initWithPasteboardWriter:pbItem] autorelease];
    NSRect r = {pt, {CGFloat(width), CGFloat(height)}};
    [dragItem setDraggingFrame:r contents:image];
    [myview beginDraggingSessionWithItems:[NSArray arrayWithObject:dragItem] event:theEvent source:myview];
  } else
#endif
  {
    static NSSize offset={0,0};
    NSPasteboard *mypasteboard = [NSPasteboard pasteboardWithName:NSDragPboard];
    [mypasteboard declareTypes:[NSArray arrayWithObject:UTF8_pasteboard_type] owner:nil];
    [mypasteboard setData:(NSData*)text forType:UTF8_pasteboard_type];
    [myview dragImage:image  at:pt  offset:offset // deprecated in 10.7
                event:theEvent  pasteboard:mypasteboard
               source:myview  slideBack:YES];
    if ( w ) {
      int old_event = Fl::e_number;
      w->handle(Fl::e_number = FL_RELEASE);
      Fl::e_number = old_event;
      Fl::pushed( 0 );
    }
  }
  CFRelease(text);
  [localPool release];
  return true;
}

// rescales an NSBitmapImageRep
static NSBitmapImageRep *scale_nsbitmapimagerep(NSBitmapImageRep *img, float scale)
{
  int w = [img pixelsWide];
  int h = [img pixelsHigh];
  long int scaled_w = lround(scale * w);
  long int scaled_h = lround(scale * h);
  NSBitmapImageRep *scaled = [[NSBitmapImageRep alloc]  initWithBitmapDataPlanes:NULL
                                                                      pixelsWide:scaled_w
                                                                      pixelsHigh:scaled_h
                                                                   bitsPerSample:8
                                                                 samplesPerPixel:4
                                                                        hasAlpha:YES
                                                                        isPlanar:NO
                                                                  colorSpaceName:NSDeviceRGBColorSpace
                                                                     bytesPerRow:scaled_w*4
                                                                    bitsPerPixel:32];
  NSDictionary *dict = [NSDictionary dictionaryWithObject:scaled
                                                   forKey:NSGraphicsContextDestinationAttributeName];
  NSGraphicsContext *oldgc = [NSGraphicsContext currentContext];
  [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithAttributes:dict]];
  [[NSColor clearColor] set];
  NSRect r = NSMakeRect(0, 0, scaled_w, scaled_h);
  NSRectFill(r);
  [img drawInRect:r];
  [NSGraphicsContext setCurrentContext:oldgc];
  [img release];
  return scaled;
}

static void write_bitmap_inside(NSBitmapImageRep *to, int to_width, NSBitmapImageRep *from,
                                int to_x, int to_y)
/* Copies in bitmap "to" the bitmap "from" with its top-left angle at coordinates to_x, to_y
 On retina displays both bitmaps have double width and height
 to_width is the width in screen units of "to". On retina, its pixel width is twice that.
 */
{
  const uchar *from_data = [from bitmapData];
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (fl_mac_os_version >= 100400) { // 10.4 required by the bitmapFormat message
    if (([to bitmapFormat] & NSAlphaFirstBitmapFormat) && !([from bitmapFormat] & NSAlphaFirstBitmapFormat) ) {
      // "to" is ARGB and "from" is RGBA --> convert "from" to ARGB
      // it is enough to read "from" starting one byte earlier, because A is always 0xFF:
      // RGBARGBA becomes (A)RGBARGB
      from_data--;
    } else if ( !([to bitmapFormat] & NSAlphaFirstBitmapFormat) && ([from bitmapFormat] & NSAlphaFirstBitmapFormat) ) {
      // "from" is ARGB and "to" is RGBA --> convert "from" to RGBA
      // it is enough to offset reading by one byte because A is always 0xFF
      // so ARGBARGB becomes RGBARGB(A) as needed
      from_data++;
    }
  }
#endif
  int to_w = (int)[to pixelsWide]; // pixel width of "to"
  int from_w = (int)[from pixelsWide]; // pixel width of "from"
  int from_h = [from pixelsHigh]; // pixel height of "from"
  int to_depth = [to samplesPerPixel], from_depth = [from samplesPerPixel];
  int depth = 0;
  if (to_depth > from_depth) depth = from_depth;
  else if (from_depth > to_depth) depth = to_depth;
  float factor = to_w / (float)to_width; // scaling factor is 1 for classic displays and 2 for retina
  to_x = factor*to_x; // transform offset from screen unit to pixels
  to_y = factor*to_y;
  // perform the copy
  uchar *tobytes = [to bitmapData] + to_y * to_w * to_depth + to_x * to_depth;
  const uchar *frombytes = from_data;
  for (int i = 0; i < from_h; i++) {
    if (depth == 0) { // depth is always 0 in case of RGBA <-> ARGB conversion
      if (i == 0 && from_data < [from bitmapData]) {
        memcpy(tobytes+1, frombytes+1, from_w * from_depth-1); // avoid reading before [from bitmapData]
        *tobytes = 0xFF; // set the very first A byte
      } else if (i == from_h - 1 && from_data > [from bitmapData]) {
        memcpy(tobytes, frombytes, from_w * from_depth - 1); // avoid reading after end of [from bitmapData]
        *(tobytes + from_w * from_depth - 1) = 0xFF; // set the very last A byte
      } else {
        memcpy(tobytes, frombytes, from_w * from_depth);
      }
    } else {
      for (int j = 0; j < from_w; j++) {
        memcpy(tobytes + j * to_depth, frombytes + j * from_depth, depth);
      }
    }
    tobytes += to_w * to_depth;
    frombytes += from_w * from_depth;
  }
}


static NSBitmapImageRep* GL_rect_to_nsbitmap(Fl_Window *win, int x, int y, int w, int h)
// captures a rectangle from a GL window and returns it as an allocated NSBitmapImageRep
// the capture has high res on retina
{
  Fl_Plugin_Manager pm("fltk:device");
  Fl_Device_Plugin *pi = (Fl_Device_Plugin*)pm.plugin("opengl.device.fltk.org");
  if (!pi) return nil;
  Fl_RGB_Image *img = pi->rectangle_capture(win, x, y, w, h);
  NSBitmapImageRep* bitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:img->w() pixelsHigh:img->h() bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:4*img->w() bitsPerPixel:32];
  memset([bitmap bitmapData], 0xFF, [bitmap bytesPerPlane]);
  const uchar *from = img->array;
  for (int r = 0; r < img->h(); r++) {
    uchar *to = [bitmap bitmapData] + r * [bitmap bytesPerRow];
    for (int c = 0; c < img->w(); c++) {
      memcpy(to, from, 3);
      from += 3;
      to += 4;
    }
  }
  delete img;
  return bitmap;
}

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
static NSBitmapImageRep* rect_to_NSBitmapImage_layer(Fl_Window *win, int x, int y, int w, int h)
{ // capture window data for layer-based views because initWithFocusedViewRect: does not work for them
  FLView *view = (FLView*)[fl_xid(win) contentView];
  // make sure to get the most recent content of the view
  CGContextRef source_bitmap = (CGContextRef)fl_gc;
  if (!source_bitmap || win != Fl_Window::current()) source_bitmap = view->aux_bitmap;
  if (!source_bitmap) return nil;
  CGImageRef cgimg = CGBitmapContextCreateImage(source_bitmap);
  if (x || y || w != win->w() || h != win->h()) {
    float s = 1;
    if (Fl_X::i(win)->mapped_to_retina()) s *= 2;
    CGRect rect = CGRectMake(x * s, y * s, w * s, h * s);
    CGImageRef cgimg2 = CGImageCreateWithImageInRect(cgimg, rect);
    CGImageRelease(cgimg);
    cgimg = cgimg2;
  }
  NSBitmapImageRep *bitmap = (cgimg ? [[NSBitmapImageRep alloc] initWithCGImage:cgimg/*10.5*/] : nil);
  CGImageRelease(cgimg);
  return bitmap;
}
#endif

static NSBitmapImageRep* rect_to_NSBitmapImageRep(Fl_Window *win, int x, int y, int w, int h) {
    NSBitmapImageRep *bitmap = nil;
    NSRect rect;
    if (win->as_gl_window() && y >= 0) {
      bitmap = GL_rect_to_nsbitmap(win, x, y, w, h);
    }
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
    else if (views_use_CA) {
        bitmap = rect_to_NSBitmapImage_layer(win, x, y, w, h);
    }
#endif
    else {
      NSView *winview = nil;
      if ( through_Fl_X_flush  && Fl_Window::current() == win ) {
        rect = NSMakeRect(x - 0.5, y - 0.5, w, h);
      }
      else {
        winview = [fl_xid(win) contentView];
        int view_h = [winview frame].size.height;
        rect = NSMakeRect(x, int(view_h-y-h), w, h);
        // lock focus to win's view
  #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
        if (fl_mac_os_version >= 101100) [[fl_xid(win) graphicsContext] saveGraphicsState]; // necessary under 10.11
  #endif
        [winview lockFocus];
      }
      // The image depth is 3 until 10.5 and 4 with 10.6 and above
      bitmap = [[NSBitmapImageRep alloc] initWithFocusedViewRect:rect];
      if ( !( through_Fl_X_flush && Fl_Window::current() == win) ) {
        [winview unlockFocus];
  #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
        if (fl_mac_os_version >= 101100) [[fl_xid(win) graphicsContext] restoreGraphicsState];
  #endif
      }
    }
  return bitmap;
}

static NSBitmapImageRep* rect_to_NSBitmapImageRep_subwins(Fl_Window *win, int x, int y, int w, int h, bool capture_subwins)
/* Captures a rectangle from a mapped window.
 On retina displays, the resulting bitmap has 2 pixels per screen unit.
 The returned value is to be released after use
 */
{
  NSBitmapImageRep *bitmap = rect_to_NSBitmapImageRep(win, x, y, w, h);
  if (!capture_subwins || !bitmap) return bitmap;

  // capture also subwindows
  NSArray *children = [fl_xid(win) childWindows]; // 10.2
  NSEnumerator *enumerator = [children objectEnumerator];
  id child;
  while ((child = [enumerator nextObject]) != nil) {
    if (![child isKindOfClass:[FLWindow class]]) continue;
    Fl_Window *sub = [(FLWindow*)child getFl_Window];
    CGRect rsub = CGRectMake(sub->x(), win->h() -(sub->y()+sub->h()), sub->w(), sub->h());
    CGRect clip = CGRectMake(x, win->h()-(y+h), w, h);
    clip = CGRectIntersection(rsub, clip);
    if (CGRectIsNull(clip)) continue;
    NSBitmapImageRep *childbitmap = rect_to_NSBitmapImageRep_subwins(sub, clip.origin.x - sub->x(),
                                                             win->h() - clip.origin.y - sub->y() - clip.size.height, clip.size.width, clip.size.height, true);
    if (childbitmap) {
      // if bitmap is high res and childbitmap is not, childbitmap must be rescaled
      if (!win->as_gl_window() && Fl_X::i(win)->mapped_to_retina() && sub->as_gl_window() && !Fl::use_high_res_GL()) {
        childbitmap = scale_nsbitmapimagerep(childbitmap, 2);
      }
      write_bitmap_inside(bitmap, w, childbitmap,
                          clip.origin.x - x, win->h() - clip.origin.y - clip.size.height - y );
      [childbitmap release];
    }
  }
  return bitmap;
}


unsigned char *Fl_X::bitmap_from_window_rect(Fl_Window *win, int x, int y, int w, int h, int *bytesPerPixel)
/* Returns a capture of a rectangle of a mapped window as a pre-multiplied RGBA array of bytes.
 Alpha values are always 1 (except for the angles of a window title bar)
 so pre-multiplication can be ignored.
 *bytesPerPixel is set to the value 3 or 4 upon return.
 delete[] the returned pointer after use
 */
{
  NSBitmapImageRep *bitmap = rect_to_NSBitmapImageRep(win, x, y, w, h);
  if (bitmap == nil) return NULL;
  *bytesPerPixel = [bitmap bitsPerPixel]/8;
  int bpp = (int)[bitmap bytesPerPlane];
  int bpr = (int)[bitmap bytesPerRow];
  int hh = bpp/bpr; // sometimes hh = h-1 for unclear reason, and hh = 2*h with retina
  int ww = bpr/(*bytesPerPixel); // sometimes ww = w-1, and ww = 2*w with retina
  const uchar *start = [bitmap bitmapData]; // start of the bitmap data
  bool convert = false;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if (fl_mac_os_version >= 100400 && ([bitmap bitmapFormat] & NSAlphaFirstBitmapFormat)) {
    // bitmap is ARGB --> convert it to RGBA (ARGB happens with Mac OS 10.11)
    // it is enough to offset reading by one byte because A is always 0xFF
    // so ARGBARGB becomes RGBARGBA as needed
    start++;
    convert = true;
  }
#endif
  unsigned char *data;
  size_t tocopy;
  if (ww > w) { // with a retina display, we have to scale the image by a factor of 2
    uchar *data2 = [bitmap bitmapData];
    if (convert) { // duplicate the NSBitmapImageRep data taking care not to access beyond its end
      tocopy = ww*hh*4;
      data2 = new uchar[tocopy];
      memcpy(data2, start, --tocopy);
      data2[tocopy] = 0xFF; // set the last A byte
    }
    Fl_RGB_Image *rgb = new Fl_RGB_Image(data2, ww, hh, 4);
    rgb->alloc_array = (convert ? 1 : 0);
    Fl_RGB_Scaling save_scaling = Fl_Image::RGB_scaling();
    Fl_Image::RGB_scaling(FL_RGB_SCALING_BILINEAR);
    Fl_RGB_Image *rgb2 = (Fl_RGB_Image*)rgb->copy(w, h);
    Fl_Image::RGB_scaling(save_scaling);
    delete rgb;
    rgb2->alloc_array = 0;
    data = (uchar*)rgb2->array;
    delete rgb2;
  }
  else {
    data = new unsigned char[w * h *  *bytesPerPixel];
    if (w == ww) { // the NSBitmapImageRep data can be copied in one step
      tocopy = w * hh * (*bytesPerPixel);
      if (convert) { // take care not to access beyond the image end
        data[--tocopy] = 0xFF; // set the last A byte
      }
      memcpy(data, start, tocopy);
    } else {  // copy the NSBitmapImageRep data line by line
      const uchar *p = start;
      unsigned char *q = data;
      tocopy = bpr;
      for (int i = 0; i < hh; i++) {
        if (i == hh-1 && convert) tocopy--; // take care not to access beyond the image end
        memcpy(q, p, tocopy);
        p += bpr;
        q += w * (*bytesPerPixel);
      }
    }
  }
  [bitmap release];
  return data;
}

static void nsbitmapProviderReleaseData (void *info, const void *data, size_t size)
{
  [(NSBitmapImageRep*)info release];
}

CGImageRef Fl_X::CGImage_from_window_rect(Fl_Window *win, int x, int y, int w, int h)
/* Returns a capture of a rectangle of a mapped window as a CGImage.
 With retina displays, the returned image has twice the width and height.
 CFRelease the returned CGImageRef after use
 */
{
  CGImageRef img;
  NSBitmapImageRep *bitmap = rect_to_NSBitmapImageRep_subwins(win, x, y, w, h, true);
  if (fl_mac_os_version >= 100500) {
    img = (CGImageRef)[bitmap performSelector:@selector(CGImage)]; // requires Mac OS 10.5
    CGImageRetain(img);
    [bitmap release];
  } else {
    CGColorSpaceRef cspace = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(bitmap, [bitmap bitmapData],
                                                              [bitmap bytesPerRow]*[bitmap pixelsHigh],
                                                              nsbitmapProviderReleaseData);
    img = CGImageCreate([bitmap pixelsWide], [bitmap pixelsHigh], 8, [bitmap bitsPerPixel], [bitmap bytesPerRow],
                        cspace,
                        [bitmap bitsPerPixel] == 32 ? kCGImageAlphaPremultipliedLast : kCGImageAlphaNone,
                        provider, NULL, false, kCGRenderingIntentDefault);
    CGColorSpaceRelease(cspace);
    CGDataProviderRelease(provider);
  }
  return img;
}

WindowRef Fl_X::window_ref() // useless with cocoa GL windows
{
  return (WindowRef)[xid windowRef];
}

// so a CGRect matches exactly what is denoted x,y,w,h for clipping purposes
CGRect fl_cgrectmake_cocoa(int x, int y, int w, int h) {
  return CGRectMake(x - 0.5, y - 0.5, w, h);
}

Window fl_xid(const Fl_Window* w)
{
  Fl_X *temp = Fl_X::i(w);
  return temp ? temp->xid : 0;
}

int Fl_Window::decorated_w()
{
  if (!shown() || parent() || !border() || !visible())
    return w();
  int bx=0;
  get_window_frame_sizes(this, &bx);
  return w() + 2 * bx;
}

int Fl_Window::decorated_h()
{
  if (!shown() || parent() || !border() || !visible())
    return h();
  int bx = 0, by = 0;
  int bt = get_window_frame_sizes(this, &bx, &by);
  return h() + bt + by;
}

// clip the graphics context to rounded corners
void Fl_X::clip_to_rounded_corners(CGContextRef gc, int w, int h) {
  const CGFloat radius = 5;
  CGContextMoveToPoint(gc, 0, 0);
  CGContextAddLineToPoint(gc, 0, h - radius);
  CGContextAddArcToPoint(gc, 0, h,  radius, h, radius);
  CGContextAddLineToPoint(gc, w - radius, h);
  CGContextAddArcToPoint(gc, w, h, w, h - radius, radius);
  CGContextAddLineToPoint(gc, w, 0);
  CGContextClip(gc);
}


void *Fl_X::get_titlebar_layer(Fl_Window *win)
{
  // a compilation warning appears with SDK 10.5, so we require SDK 10.6 instead
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
  return fl_mac_os_version >= 101000 ? [[[fl_xid(win) standardWindowButton:NSWindowCloseButton] superview] layer] : nil; // 10.5
#else
  return nil;
#endif
}


void Fl_X::draw_layer_to_context(void *layer, CGContextRef gc, int w, int h)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
  CGContextSaveGState(gc);
  clip_to_rounded_corners(gc, w, h);
  if (fl_mac_os_version < 101500) { // exact OS threshold might be lower
    CGContextSetRGBFillColor(gc, .79, .79, .79, 1.); // equiv. to FL_DARK1
    CGContextFillRect(gc, CGRectMake(0, 0, w, h));
  }
  CGContextSetShouldAntialias(gc, true);
  [(CALayer*)layer renderInContext:gc]; // 10.5
  CGContextRestoreGState(gc);
#endif
}

void Fl_Paged_Device::print_window(Fl_Window *win, int x_offset, int y_offset)
{
  int bx, by, bt, bpp;
  bt = get_window_frame_sizes(win, &bx, &by);
  if (!win->shown() || win->parent() || !win->border() || !win->visible() || !bt) {
    this->print_widget(win, x_offset, y_offset);
    return;
  }
  BOOL to_quartz =  (this->driver()->class_name() == Fl_Quartz_Graphics_Driver::class_id);
  void *layer = Fl_X::get_titlebar_layer(win);
  if (layer) { // if title bar uses a layer
    if (to_quartz) { // to Quartz printer
      CGContextSaveGState(fl_gc);
      CGContextTranslateCTM(fl_gc, x_offset - 0.5, y_offset + bt - 0.5);
      CGContextScaleCTM(fl_gc, 1, -1);
      Fl_X::draw_layer_to_context(layer, fl_gc, win->w(), bt);
      CGContextRestoreGState(fl_gc);
    }
    else { // to PostScript
      CGColorSpaceRef cspace = CGColorSpaceCreateDeviceRGB ();
      CGContextRef gc = CGBitmapContextCreate(NULL, win->w(), bt, 8, 0, cspace, kCGImageAlphaPremultipliedLast);
      CGColorSpaceRelease(cspace);
      CGContextClearRect(gc, CGRectMake(0, 0, win->w(), bt));
      Fl_X::draw_layer_to_context(layer, gc, win->w(), bt);
      Fl_RGB_Image *image = new Fl_RGB_Image((const uchar*)CGBitmapContextGetData(gc), win->w(), bt, 4,
                                             CGBitmapContextGetBytesPerRow(gc)); // 10.2
      image->draw(x_offset, y_offset); // draw title bar to PostScript
      delete image;
      CGContextRelease(gc);
    }
    this->print_widget(win, x_offset, y_offset + bt);
    return;
  }
  Fl_Display_Device::display_device()->set_current(); // send win to front and make it current
  NSString *title = [fl_xid(win) title];
  [title retain];
  [fl_xid(win) setTitle:@""]; // temporarily set a void window title
  win->show();
  fl_gc = NULL;
  Fl::check();
  // capture the window title bar with no title
  CGImageRef img = NULL;
  unsigned char *bitmap = NULL;
  if (to_quartz)
    img = Fl_X::CGImage_from_window_rect(win, 0, -bt, win->w(), bt);
  else
    bitmap = Fl_X::bitmap_from_window_rect(win, 0, -bt, win->w(), bt, &bpp);
  [fl_xid(win) setTitle:title]; // put back the window title
  this->set_current(); // back to the Fl_Paged_Device
  if (img && to_quartz) { // print the title bar
    CGRect rect = CGRectMake(x_offset, y_offset, win->w(), bt);
    Fl_X::q_begin_image(rect, 0, 0, win->w(), bt);
    CGContextDrawImage(fl_gc, rect, img);
    Fl_X::q_end_image();
    CFRelease(img);
  }
  else if(!to_quartz) {
    Fl_RGB_Image *rgb = new Fl_RGB_Image(bitmap, win->w(), bt, bpp);
    rgb->draw(x_offset, y_offset);
    delete rgb;
    delete[] bitmap;
  }
  if (win->label()) { // print the window title
    const int skip = 65; // approx width of the zone of the 3 window control buttons
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    if (fl_mac_os_version >= 100400 && to_quartz) { // use Cocoa string drawing with exact title bar font
      // the exact font is LucidaGrande 13 pts (and HelveticaNeueDeskInterface-Regular with 10.10)
      NSGraphicsContext *current = [NSGraphicsContext currentContext];
      [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithGraphicsPort:fl_gc flipped:YES]];//10.4
      NSDictionary *attr = [NSDictionary dictionaryWithObject:[NSFont titleBarFontOfSize:0] 
						       forKey:NSFontAttributeName];
      NSSize size = [title sizeWithAttributes:attr];
      int x = x_offset + win->w()/2 - size.width/2;
      if (x < x_offset+skip) x = x_offset+skip;
      NSRect r = NSMakeRect(x, y_offset+bt/2+4, win->w() - skip, bt);
      [[NSGraphicsContext currentContext] setShouldAntialias:YES];
      [title drawWithRect:r options:(NSStringDrawingOptions)0 attributes:attr]; // 10.4
      [[NSGraphicsContext currentContext] setShouldAntialias:NO];
      [NSGraphicsContext setCurrentContext:current];
    }
    else
#endif
    {
      fl_font(FL_HELVETICA, 14);
      fl_color(FL_BLACK);
      int x = x_offset + win->w()/2 - fl_width(win->label())/2;
      if (x < x_offset+skip) x = x_offset+skip;
      fl_push_clip(x_offset, y_offset, win->w(), bt);
      fl_draw(win->label(), x, y_offset+bt/2+4);
      fl_pop_clip();
    }
  }
  [title release];
  this->print_widget(win, x_offset, y_offset + bt); // print the window inner part
}

void Fl_X::gl_start(NSOpenGLContext *ctxt) {
  [ctxt update]; // supports window resizing
}


/* Returns the address of a Carbon function after dynamically loading the Carbon library if needed.
 Supports old Mac OS X versions that may use a couple of Carbon calls:
 GetKeys used by OS X 10.3 or before (in Fl::get_key())
 PMSessionPageSetupDialog and PMSessionPrintDialog used by 10.4 or before (in Fl_Printer::start_job())
 */
void *Fl_X::get_carbon_function(const char *function_name) {
  static void *carbon = dlopen("/System/Library/Frameworks/Carbon.framework/Carbon", RTLD_LAZY);
  return (carbon ? dlsym(carbon, function_name) : NULL);
}
  
/* Returns the version of the running Mac OS as an int such as 100802 for 10.8.2
 */
int Fl_X::calc_mac_os_version() {
  if (fl_mac_os_version) return fl_mac_os_version;
  int M, m, b = 0;
  NSAutoreleasePool *localPool = [[NSAutoreleasePool alloc] init];
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_10
  if ([NSProcessInfo instancesRespondToSelector:@selector(operatingSystemVersion)]) {
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    M = version.majorVersion;
    m = version.minorVersion;
    b = version.patchVersion;
  }
  else
#endif
  {
    NSDictionary * sv = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
    const char *s = [[sv objectForKey:@"ProductVersion"] UTF8String];
    sscanf(s, "%d.%d.%d", &M, &m, &b);
  }
  [localPool release];
  fl_mac_os_version = M*10000 + m*100 + b;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
  if (fl_mac_os_version >= 101400) views_use_CA = YES;
#endif
  return fl_mac_os_version;
}

#endif // __APPLE__

//
// End of "$Id$".
//
