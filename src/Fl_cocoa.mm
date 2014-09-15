//
// "$Id$"
//
// MacOS-Cocoa specific code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2013 by Bill Spitzak and others.
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

#define CONSOLIDATE_MOTION 0
extern "C" {
#include <pthread.h>
}


#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Printer.H>
#include <FL/Fl_Input_.H>
#include <FL/Fl_Text_Display.H>
#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"
#include <unistd.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>
#include <dlfcn.h>

#import <Cocoa/Cocoa.h>

#ifndef NSINTEGER_DEFINED // appears with 10.5 in NSObjCRuntime.h
#if defined(__LP64__) && __LP64__
typedef long NSInteger;
typedef unsigned long NSUInteger;
#else
typedef long NSInteger;
typedef unsigned int NSUInteger;
#endif
#endif


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
static void convert_crlf(char * string, size_t len);
static void createAppleMenu(void);
static Fl_Region MacRegionMinusRect(Fl_Region r, int x,int y,int w,int h);
static void cocoaMouseHandler(NSEvent *theEvent);
static int calc_mac_os_version();
static void clipboard_check(void);
static NSString *calc_utf8_format(void);
static void im_update(void);
static unsigned make_current_counts = 0; // if > 0, then Fl_Window::make_current() can be called only once
static Fl_X *fl_x_to_redraw = NULL; // set by Fl_X::flush() to the Fl_X object of the window to be redrawn

Fl_Display_Device *Fl_Display_Device::_display = new Fl_Display_Device(new Fl_Quartz_Graphics_Driver); // the platform display

// public variables
CGContextRef fl_gc = 0;
void *fl_capture = 0;			// (NSWindow*) we need this to compensate for a missing(?) mouse capture
bool fl_show_iconic;                    // true if called from iconize() - shows the next created window in collapsed state
//int fl_disable_transient_for;           // secret method of removing TRANSIENT_FOR
Window fl_window;
Fl_Window *Fl_Window::current_;
int fl_mac_os_version = calc_mac_os_version();		// the version number of the running Mac OS X (e.g., 100604 for 10.6.4)
static SEL inputContextSEL = (fl_mac_os_version >= 100600 ? @selector(inputContext) : @selector(FLinputContext));
Fl_Fontdesc* fl_fonts = Fl_X::calc_fl_fonts();
static NSString *utf8_format = calc_utf8_format();

// forward declarations of variables in this file
static int got_events = 0;
static Fl_Window* resize_from_system;
static int main_screen_height; // height of menubar-containing screen used to convert between Cocoa and FLTK global screen coordinates
// through_drawRect = YES means the drawRect: message was sent to the view, 
// thus the graphics context was prepared by the system
static BOOL through_drawRect = NO; 
static int im_enabled = -1;

#if CONSOLIDATE_MOTION
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
typedef CFArrayRef (*TISCreateASCIICapableInputSourceList_type)(void);
static TISCreateASCIICapableInputSourceList_type TISCreateASCIICapableInputSourceList;

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
static unsigned short* macKeyLookUp = fl_compute_macKeyLookUp();

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
        NSPoint pt={0,0};
	NSAutoreleasePool *localPool = [[NSAutoreleasePool alloc] init]; 
        NSEvent *event = [NSEvent otherEventWithType:NSApplicationDefined location:pt 
				       modifierFlags:0
                                           timestamp:0
                                        windowNumber:0 context:NULL 
					     subtype:FLTKDataReadyEvent data1:0 data2:0];
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
  NSPoint pt={0,0};
  NSEvent *event = [NSEvent otherEventWithType:NSApplicationDefined location:pt 
				 modifierFlags:0
                                     timestamp:0
                                  windowNumber:0 context:NULL 
				       subtype:FLTKTimerEvent data1:0 data2:0];
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
static MacTimeout* current_timer;  // the timer that triggered its callback function, or NULL

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
    if (&t == current_timer) current_timer = NULL;
  }
}

static void do_timer(CFRunLoopTimerRef timer, void* data)
{
  fl_lock_function();
  current_timer = (MacTimeout*)data;
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
  // check, if this timer slot exists already
  for (int i = 0; i < mac_timer_used; ++i) {
    MacTimeout& t = mac_timers[i];
    // if so, simply change the fire interval
    if (t.callback == cb  &&  t.data == data) {
      t.next_timeout = CFAbsoluteTimeGetCurrent() + time;
      CFRunLoopTimerSetNextFireDate(t.timer, t.next_timeout );
      t.pending = 1;
      return;
    }
  }
  // no existing timer to use. Create a new one:
  int timer_id = -1;
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
  CFRunLoopTimerContext context = {0, &t, NULL,NULL,NULL};
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
  if (current_timer) {
    // k = how many times 'time' seconds after the last scheduled timeout until the future
    double k = ceil( (CFAbsoluteTimeGetCurrent() - current_timer->next_timeout) / time);
    if (k < 1) k = 1;
    current_timer->next_timeout += k * time;
    CFRunLoopTimerSetNextFireDate(current_timer->timer, current_timer->next_timeout );
    current_timer->callback = cb;
    current_timer->data = data;
    current_timer->pending = 1;
    return;
  }
  add_timeout(time, cb, data);
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
  BOOL containsGLsubwindow;
}
- (FLWindow*)initWithFl_W:(Fl_Window *)flw 
	      contentRect:(NSRect)rect 
		styleMask:(NSUInteger)windowStyle;
- (Fl_Window *)getFl_Window;
/* These two functions allow to check if a window contains OpenGL-subwindows.
   This is useful only for Mac OS < 10.7 to repair a problem apparent with the "cube" test program:
   if the cube window is moved around rapidly (with OS < 10.7), the GL pixels leak away from where they should be.
   The repair is performed by [FLWindowDelegate windowDidMove:], only if OS < 10.7.
 */
- (BOOL)containsGLsubwindow;
- (void)containsGLsubwindow:(BOOL)contains;
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
- (NSPoint)convertBaseToScreen:(NSPoint)aPoint;
#endif
@end

@implementation FLWindow
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
    convertIMP addr = (convertIMP)[NSWindow instanceMethodForSelector:@selector(convertBaseToScreen:)];
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
    containsGLsubwindow = NO;
    if (fl_mac_os_version >= 100700) {
      // replaces [self setRestorable:NO] that may trigger a compiler warning
      typedef void (*setIMP)(id, SEL, BOOL);
      setIMP addr = (setIMP)[self methodForSelector:@selector(setRestorable:)];
      addr(self, @selector(setRestorable:), NO);
      }
  }
  return self;
}
- (Fl_Window *)getFl_Window;
{
  return w;
}
- (BOOL)containsGLsubwindow
{
  return containsGLsubwindow;
}
- (void)containsGLsubwindow:(BOOL)contains
{
  containsGLsubwindow = contains;
}

- (BOOL)canBecomeKeyWindow
{
  if (Fl::modal_ && (Fl::modal_ != w))
    return NO;	// prevent the caption to be redrawn as active on click
		//  when another modal window is currently the key win

  return !(w->tooltip_window() || w->menu_window());
}

- (BOOL)canBecomeMainWindow
{
  if (Fl::modal_ && (Fl::modal_ != w))
    return NO;	// prevent the caption to be redrawn as active on click
		//  when another modal window is currently the key win

  return !(w->tooltip_window() || w->menu_window());
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
 * Funny enough, it returns the same time that it got as the argument. 
 */
static double do_queued_events( double time = 0.0 ) 
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
  
#if CONSOLIDATE_MOTION
  if (send_motion && send_motion == fl_xmousewin) {
    send_motion = 0;
    Fl::handle(FL_MOVE, fl_xmousewin);
  }
#endif
  
  return time;
}


/*
 * This public function handles all events. It wait a maximum of 
 * 'time' seconds for an event. This version returns 1 if events
 * other than the timeout timer were processed.
 *
 * \todo there is no socket handling in this code whatsoever
 */
int fl_wait( double time ) 
{
  do_queued_events( time );
  return (got_events);
}

double fl_mac_flush_and_wait(double time_to_wait) {
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
  Fl::flush();
  if (Fl::idle && !in_idle) // 'idle' may have been set within flush()
    time_to_wait = 0.0;
  double retval = fl_wait(time_to_wait);
  if (fl_gc) {
    CGContextFlush(fl_gc);
    fl_gc = 0;
    }  
  [pool release];
  return retval;
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
  if (level < NSModalPanelWindowLevel)
    return NSModalPanelWindowLevel;

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
 * Cocoa Mouse Button Handler
 */
static void cocoaMouseHandler(NSEvent *theEvent)
{
  static int keysym[] = { 0, FL_Button+1, FL_Button+3, FL_Button+2 };
  static int px, py;
  static char suppressed = 0;
  
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
      suppressed = 0;
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
      if (suppressed) {
        suppressed = 0;
        break;
      }
      if ( !window ) break;
      if ( !sendEvent ) {
        sendEvent = FL_RELEASE; 
      }
      Fl::e_keysym = keysym[ btn ];
      // fall through
    case NSMouseMoved:
      suppressed = 0;
      if ( !sendEvent ) { 
        sendEvent = FL_MOVE; 
      }
      // fall through
    case NSLeftMouseDragged:
    case NSRightMouseDragged:
    case NSOtherMouseDragged: {
      if (suppressed) break;
      if ( !sendEvent ) {
        sendEvent = FL_MOVE; // Fl::handle will convert into FL_DRAG
        if (fabs(pos.x-px)>5 || fabs(pos.y-py)>5) 
          Fl::e_is_click = 0;
      }
      mods_to_e_state( mods );
      update_e_xy_and_e_xy_root([theEvent window]);
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
- (void)insertText:(id)aString;
- (void)doCommandBySelector:(SEL)aSelector;
- (void)setActive:(BOOL)a;
@end
@implementation FLTextView
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
+ (FLWindowDelegate*)createOnce;
- (void)windowDidMove:(NSNotification *)notif;
- (void)windowDidResize:(NSNotification *)notif;
- (void)windowDidResignKey:(NSNotification *)notif;
- (void)windowDidBecomeKey:(NSNotification *)notif;
- (void)windowDidBecomeMain:(NSNotification *)notif;
- (void)windowDidDeminiaturize:(NSNotification *)notif;
- (void)windowDidMiniaturize:(NSNotification *)notif;
- (BOOL)windowShouldClose:(id)fl;
- (id)windowWillReturnFieldEditor:(NSWindow *)sender toObject:(id)client;
- (void)anyWindowWillClose:(NSNotification *)notif;
@end
@implementation FLWindowDelegate
+ (FLWindowDelegate*)createOnce
{
  static FLWindowDelegate* delegate = nil;
  if (!delegate) {
    delegate = [[FLWindowDelegate alloc] init];
    }
  return delegate;
}
- (void)windowDidMove:(NSNotification *)notif
{
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  NSPoint pt, pt2; 
  pt.x = 0;
  pt.y = [[nsw contentView] frame].size.height;
  pt2 = [nsw convertBaseToScreen:pt];
  update_e_xy_and_e_xy_root(nsw);
  resize_from_system = window;
  window->position((int)pt2.x, (int)(main_screen_height - pt2.y));
  if ([nsw containsGLsubwindow] && fl_mac_os_version < 100700) {
    [nsw display];// with OS < 10.7, redraw window after moving if it contains OpenGL subwindows
  }
  fl_unlock_function();
}
- (void)windowDidResize:(NSNotification *)notif
{
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  NSRect r = [[nsw contentView] frame];
  NSPoint pt, pt2; 
  pt.x = 0;
  pt.y = [[nsw contentView] frame].size.height;
  pt2 = [nsw convertBaseToScreen:pt];
  resize_from_system = window;
  update_e_xy_and_e_xy_root(nsw);
  window->resize((int)pt2.x, 
                 (int)(main_screen_height - pt2.y),
		 (int)r.size.width,
		 (int)r.size.height);
  fl_unlock_function();
}
- (void)windowDidResignKey:(NSNotification *)notif
{
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  /* Fullscreen windows obscure all other windows so we need to return
   to a "normal" level when the user switches to another window */
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
  if ( w->border() || (!w->modal() && !w->tooltip_window()) ) Fl::handle( FL_FOCUS, w);
  fl_unlock_function();
}
- (void)windowDidBecomeMain:(NSNotification *)notif
{
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  Fl::first_window(window);
  update_e_xy_and_e_xy_root(nsw);
  fl_unlock_function();
}
- (void)windowDidDeminiaturize:(NSNotification *)notif
{
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  Fl::handle(FL_SHOW, window);
  update_e_xy_and_e_xy_root(nsw);
  fl_unlock_function();
}
- (void)windowDidMiniaturize:(NSNotification *)notif
{
  fl_lock_function();
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  Fl::handle(FL_HIDE, window);
  fl_unlock_function();
}
- (BOOL)windowShouldClose:(id)fl
{
  fl_lock_function();
  Fl::handle( FL_CLOSE, [(FLWindow *)fl getFl_Window] ); // this might or might not close the window
  fl_unlock_function();
  // the system doesn't need to send [fl close] because FLTK does it when needed
  return NO; 
}
- (id)windowWillReturnFieldEditor:(NSWindow *)sender toObject:(id)client
{
  if (fl_mac_os_version < 100600) {
    static FLTextView *view = nil;
    if (!view) {
      NSRect rect={{0,0},{20,20}};
      view = [[FLTextView alloc] initWithFrame:rect];
    }
    return view;
  }
  return nil;
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
@end

@interface FLAppDelegate : NSObject 
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
<NSApplicationDelegate>
#endif
{
  void (*open_cb)(const char*);
  TSMDocumentID currentDoc;
}
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender;
- (void)applicationDidBecomeActive:(NSNotification *)notify;
- (void)applicationDidChangeScreenParameters:(NSNotification *)aNotification;
- (void)applicationDidUpdate:(NSNotification *)aNotification;
- (void)applicationWillResignActive:(NSNotification *)notify;
- (void)applicationWillHide:(NSNotification *)notify;
- (void)applicationWillUnhide:(NSNotification *)notify;
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename;
- (void)open_cb:(void (*)(const char*))cb;
@end
@implementation FLAppDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
  fl_lock_function();
  NSApplicationTerminateReply reply = NSTerminateNow;
  while ( Fl_X::first ) {
    Fl_X *x = Fl_X::first;
    Fl::handle( FL_CLOSE, x->w );
    Fl::do_widget_deletion();
    if ( Fl_X::first == x ) {
      reply = NSTerminateCancel; // FLTK has not closed all windows, so we return to the main program now
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
  main_screen_height = [[[NSScreen screens] objectAtIndex:0] frame].size.height;
  Fl::call_screen_init();
  // FLTK windows have already been notified they were moved,
  // but they had the old main_screen_height, so they must be notified again.
  NSArray *windows = [NSApp windows];
  int count = [windows count];
  for (int i = 0; i < count; i++) {
    NSWindow *win = [windows objectAtIndex:i];
    if ([win isKindOfClass:[FLWindow class]]) {
      [[NSNotificationCenter defaultCenter] postNotificationName:NSWindowDidMoveNotification object:win];
      }
    }
  Fl::handle(FL_SCREEN_CONFIGURATION_CHANGED, NULL);
  fl_unlock_function();
}
- (void)applicationDidUpdate:(NSNotification *)aNotification
{
  if ((fl_mac_os_version >= 100500) && (im_enabled != -1) &&
      (TSMGetActiveDocument != NULL)) {
    TSMDocumentID newDoc;
    // It is extremely unclear when Cocoa decides to create/update
    // the input context, but debugging reveals that it is done
    // by NSApplication:updateWindows. So check if the input context
    // has shifted after each such run so that we can update our
    // input methods status.
    newDoc = TSMGetActiveDocument();
    if (newDoc != currentDoc) {
        im_update();
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
  Fl_X *x;
  for (x = Fl_X::first;x;x = x->next) {
    Fl_Window *w = x->w;
    if ( !w->parent() && ![x->xid isMiniaturized]) {
      Fl::handle( FL_SHOW, w);
      }
  }
  fl_unlock_function();
}
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
  // without the next statement, the opening of the 1st window is delayed by several seconds
  // under Mac OS ≥ 10.8 when a file is dragged on the application icon
  [[theApplication mainWindow] orderFront:self];
  if (open_cb) {
    fl_lock_function();
    (*open_cb)([filename UTF8String]);
    fl_unlock_function();
    return YES;
  }
  return NO;
}
- (void)open_cb:(void (*)(const char*))cb
{
  open_cb = cb;
}
@end

/*
 * Install an open documents event handler...
 */
void fl_open_callback(void (*cb)(const char *)) {
  fl_open_display();
  [(FLAppDelegate*)[NSApp delegate] open_cb:cb];
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

void fl_open_display() {
  static char beenHereDoneThat = 0;
  if ( !beenHereDoneThat ) {
    beenHereDoneThat = 1;

    TSMGetActiveDocument = (TSMGetActiveDocument_type)Fl_X::get_carbon_function("TSMGetActiveDocument");
    TSMSetDocumentProperty = (TSMSetDocumentProperty_type)Fl_X::get_carbon_function("TSMSetDocumentProperty");
    TSMRemoveDocumentProperty = (TSMRemoveDocumentProperty_type)Fl_X::get_carbon_function("TSMRemoveDocumentProperty");
    TISCreateASCIICapableInputSourceList = (TISCreateASCIICapableInputSourceList_type)Fl_X::get_carbon_function("TISCreateASCIICapableInputSourceList");

    KeyScript = (KeyScript_type)Fl_X::get_carbon_function("KeyScript");
    
    BOOL need_new_nsapp = (NSApp == nil);
    if (need_new_nsapp) [NSApplication sharedApplication];
    NSAutoreleasePool *localPool;
    localPool = [[NSAutoreleasePool alloc] init]; // never released
    [(NSApplication*)NSApp setDelegate:[[FLAppDelegate alloc] init]];
    if (need_new_nsapp) [NSApp finishLaunching];

    // empty the event queue but keep system events for drag&drop of files at launch
    NSEvent *ign_event;
    do ign_event = [NSApp nextEventMatchingMask:(NSAnyEventMask & ~NSSystemDefinedMask)
					untilDate:[NSDate dateWithTimeIntervalSinceNow:0] 
					   inMode:NSDefaultRunLoopMode 
					  dequeue:YES];
    while (ign_event);
    
    // bring the application into foreground without a 'CARB' resource
    bool i_am_in_front;
    ProcessSerialNumber cur_psn = { 0, kCurrentProcess };
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
    if (fl_mac_os_version >= 100600) {
      i_am_in_front = [[NSRunningApplication currentApplication] isActive];
    }
    else
#endif
    {
      Boolean same_psn;
      ProcessSerialNumber front_psn;
      //avoid compilation warnings triggered by GetFrontProcess() and SameProcess()
      void* h = dlopen(NULL, RTLD_LAZY);
      typedef OSErr (*GetFrontProcess_type)(ProcessSerialNumber*);
      GetFrontProcess_type  GetFrontProcess_ = (GetFrontProcess_type)dlsym(h, "GetFrontProcess");
      typedef OSErr (*SameProcess_type)(ProcessSerialNumber*, ProcessSerialNumber*, Boolean*);
      SameProcess_type  SameProcess_ = (SameProcess_type)dlsym(h, "SameProcess");
      i_am_in_front = (!GetFrontProcess_( &front_psn ) &&
                       !SameProcess_( &front_psn, &cur_psn, &same_psn ) && same_psn );
    }
    if (!i_am_in_front) {
      // only transform the application type for unbundled apps
      NSBundle *bundle = [NSBundle mainBundle];
      if (bundle) {
        NSString *exe = [[bundle executablePath] stringByStandardizingPath];
        NSString *bpath = [bundle bundlePath];
        NSString *exe_dir = [exe stringByDeletingLastPathComponent];
        if ([bpath isEqualToString:exe] || [bpath isEqualToString:exe_dir]) bundle = nil;
      }
      
      if ( !bundle ) {
        TransformProcessType(&cur_psn, kProcessTransformToForegroundApplication); // needs Mac OS 10.3
        /* support of Mac OS 10.2 or earlier used this undocumented call instead
         err = CPSEnableForegroundOperation(&cur_psn, 0x03, 0x3C, 0x2C, 0x1103);
         */
        [NSApp activateIgnoringOtherApps:YES];
      }
    }
    if (![NSApp servicesMenu]) createAppleMenu();
    main_screen_height = [[[NSScreen screens] objectAtIndex:0] frame].size.height;
    [[NSNotificationCenter defaultCenter] addObserver:[FLWindowDelegate createOnce] 
					     selector:@selector(anyWindowWillClose:) 
						 name:NSWindowWillCloseNotification 
					       object:nil];
    // necessary for secondary pthreads to be allowed to use cocoa, 
    // especially to create an NSAutoreleasePool.
    [NSThread detachNewThreadSelector:nil toTarget:nil withObject:nil];
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

static void im_update(void) {
  if (fl_mac_os_version >= 100500) {
    TSMDocumentID doc;

    if ((TSMGetActiveDocument == NULL) ||
        (TSMSetDocumentProperty == NULL) ||
        (TSMRemoveDocumentProperty == NULL) ||
        (TISCreateASCIICapableInputSourceList == NULL))
      return;

    doc = TSMGetActiveDocument();

    if (im_enabled)
      TSMRemoveDocumentProperty(doc, kTSMDocumentEnabledInputSourcesPropertyTag);
    else {
      CFArrayRef inputSources;

      inputSources = TISCreateASCIICapableInputSourceList();
      TSMSetDocumentProperty(doc, kTSMDocumentEnabledInputSourcesPropertyTag,
                             sizeof(CFArrayRef), &inputSources);
      CFRelease(inputSources);
    }
  } else {
    if (KeyScript == NULL)
      return;

    if (im_enabled)
      KeyScript(smKeyEnableKybds);
    else
      KeyScript(smEnableRomanKybdsOnly);
  }
}

void Fl::enable_im() {
  fl_open_display();

  im_enabled = 1;

  if (fl_mac_os_version >= 100500)
    [NSApp updateWindows];
  else
    im_update();
}

void Fl::disable_im() {
  fl_open_display();

  im_enabled = 0;

  if (fl_mac_os_version >= 100500)
    [NSApp updateWindows];
  else
    im_update();
}


// Gets the border sizes and the titlebar size
static void get_window_frame_sizes(int &bx, int &by, int &bt) {
  static bool first = true;
  static int top, left, bottom;
  if (first) {
    first = false;
    if (NSApp == nil) fl_open_display();
    NSRect inside = { {20,20}, {100,100} };
    NSRect outside = [NSWindow  frameRectForContentRect:inside styleMask:NSTitledWindowMask];
    left = int(outside.origin.x - inside.origin.x);
    bottom = int(outside.origin.y - inside.origin.y);
    top = int(outside.size.height - inside.size.height) - bottom;
    }
  bx = left;
  by = bottom;
  bt = top;
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


/*
 * Gets called when a window is created, resized, or deminiaturized
 */    
static void handleUpdateEvent( Fl_Window *window ) 
{
  if ( !window ) return;
  Fl_X *i = Fl_X::i( window );
  i->wait_for_expose = 0;

  if ( i->region ) {
    XDestroyRegion(i->region);
    i->region = 0;
  }
  
  for ( Fl_X *cx = i->xidChildren; cx; cx = cx->xidNext ) {
    if ( cx->region ) {
      XDestroyRegion(cx->region);
      cx->region = 0;
    }
    cx->w->clear_damage(FL_DAMAGE_ALL);
    CGContextRef gc = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    CGContextSaveGState(gc); // save original context
    cx->flush();
    CGContextRestoreGState(gc); // restore original context
    cx->w->clear_damage();
  }
  window->clear_damage(FL_DAMAGE_ALL);
  i->flush();
  window->clear_damage();
}     


int Fl_X::fake_X_wm(const Fl_Window* w,int &X,int &Y, int &bt,int &bx, int &by) {
  int W, H, xoff, yoff, dx, dy;
  int ret = bx = by = bt = 0;
  if (w->border() && !w->parent()) {
    if (w->maxw != w->minw || w->maxh != w->minh) {
      ret = 2;
    } else {
      ret = 1;
    }
    get_window_frame_sizes(bx, by, bt);
  }
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
 The keyDown: method calls [[myview inputContext] handleEvent:theEvent] that triggers system 
 processing of keyboard events. The performKeyEquivalent: method directly calls Fl::handle(FL_KEYBOARD, focus-window) 
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

 With OS <= 10.5, the crucial call [[myview inputContext] handleEvent:theEvent] is not possible because neither the 
 inputContext nor the handleEvent: methods are implemented. This call is re-written:
    static SEL inputContextSEL = (fl_mac_os_version >= 100600 ? @selector(inputContext) : @selector(FLinputContext));
    [[myview performSelector:inputContextSEL] handleEvent:theEvent];
 that replaces the 10.6 inputContext message by the FLinputContext message. This message and two FLTK-defined classes, 
 FLTextInputContext and FLTextView, are used to emulate with OS <= 10.5 what's possible with OS >= 10.6. 
 Method -(FLTextInputContext*)[FLView FLinputContext] returns an instance of class FLTextInputContext that possesses 
 a handleEvent: method. FLView's FLinputContext method also calls [[self window] fieldEditor:YES forObject:nil] which 
 returns the so-called view's "field editor". This editor is an instance of the FLTextView class allocated by the 
 -(id)[FLWindowDelegate windowWillReturnFieldEditor: toObject:] method.
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

@interface FLTextInputContext : NSObject { // "emulates" NSTextInputContext before OS 10.6
@public
  FLTextView *edit;
}
-(BOOL)handleEvent:(NSEvent*)theEvent;
@end
@implementation FLTextInputContext
-(BOOL)handleEvent:(NSEvent*)theEvent {
  [self->edit setActive:YES];
  [self->edit interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
  [self->edit setActive:YES];
  return YES;
}
@end

@interface FLView : NSView <NSTextInput
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
, NSTextInputClient
#endif
> {
  BOOL in_key_event; // YES means keypress is being processed by handleEvent
  BOOL need_handle; // YES means Fl::handle(FL_KEYBOARD,) is needed after handleEvent processing
  NSInteger identifier;
  NSRange selectedRange;
}
+ (void)prepareEtext:(NSString*)aString;
+ (void)concatEtext:(NSString*)aString;
- (id)init;
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
- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;
- (void)flagsChanged:(NSEvent *)theEvent;
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender;
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
- (void)draggingExited:(id < NSDraggingInfo >)sender;
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal;
- (FLTextInputContext*)FLinputContext;
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange;
- (void)setMarkedText:(id)aString selectedRange:(NSRange)newSelection replacementRange:(NSRange)replacementRange;
- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange;
- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange;
- (NSInteger)windowLevel;
#endif
@end

@implementation FLView
- (id)init
{
  static NSInteger counter = 0;
  self = [super init];
  if (self) {
    in_key_event = NO;
    identifier = ++counter;
    }
  return self;
}
- (void)drawRect:(NSRect)rect
{
  fl_lock_function();
  through_drawRect = YES;
  FLWindow *cw = (FLWindow*)[self window];
  Fl_Window *w = [cw getFl_Window];
  if (fl_x_to_redraw) fl_x_to_redraw->flush();
  else handleUpdateEvent(w);
  through_drawRect = NO;
  fl_unlock_function();
}

- (BOOL)acceptsFirstResponder
{   
  return YES;
}
- (BOOL)performKeyEquivalent:(NSEvent*)theEvent
{   
  //NSLog(@"performKeyEquivalent:");
  fl_lock_function();
  cocoaKeyboardHandler(theEvent);
  BOOL handled;
  NSUInteger mods = [theEvent modifierFlags];
  if ( (mods & NSControlKeyMask) || (mods & NSCommandKeyMask) ) {
    NSString *s = [theEvent characters];
    if ( (mods & NSShiftKeyMask) && (mods & NSCommandKeyMask) ) {
      s = [s uppercaseString]; // US keyboards return lowercase letter in s if cmd-shift-key is hit
      }
    [FLView prepareEtext:s];
    Fl::compose_state = 0;
    handled = Fl::handle(FL_KEYBOARD, [(FLWindow*)[theEvent window] getFl_Window]);
  }
  else {
    in_key_event = YES;
    need_handle = NO;
    handled = [[self performSelector:inputContextSEL] handleEvent:theEvent];
    if (need_handle) handled = Fl::handle(FL_KEYBOARD, [(FLWindow*)[theEvent window] getFl_Window]);
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
- (void)keyDown:(NSEvent *)theEvent {
  //NSLog(@"keyDown:%@",[theEvent characters]);
  fl_lock_function();
  Fl_Window *window = [(FLWindow*)[theEvent window] getFl_Window];
  Fl::first_window(window);
  cocoaKeyboardHandler(theEvent);
  in_key_event = YES;
  need_handle = NO;
  [[self performSelector:inputContextSEL] handleEvent:theEvent];
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
    Fl::e_keysym = Fl::e_original_keysym = macKeyLookUp[keycode & 0x7f];
    if ( Fl::e_keysym ) 
      sendEvent = ( prevMods<mods ) ? FL_KEYBOARD : FL_KEYUP;
    Fl::e_length = 0;
    Fl::e_text = (char*)"";
    prevMods = mods;
  }
  mods_to_e_state( mods );
  while (window->parent()) window = window->window();
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
  // the the DND operation is finished. The call below causes the drop indicator
  // to be draw correctly (a full event handling would be better...)
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
  else if ( [[pboard types] containsObject:utf8_format] ) {
    NSData *data = [pboard dataForType:utf8_format];
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

- (FLTextInputContext*)FLinputContext { // used only if OS < 10.6 to replace [NSView inputContext]
  static FLTextInputContext *context = NULL;
  if (!context) {
    context = [[FLTextInputContext alloc] init];
    }
  context->edit = (FLTextView*)[[self window] fieldEditor:YES forObject:nil];
  return context;
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
  if (fl_mac_os_version < 100600) [(FLTextView*)[[self window] fieldEditor:YES forObject:nil] setActive:NO];
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

- (NSInteger)conversationIdentifier {
  return identifier;
}

@end

void Fl_Window::fullscreen_x() {
  _set_fullscreen();
  /* On OS X < 10.6, it is necessary to recreate the window. This is done
     with hide+show. */
  hide();
  show();
  Fl::handle(FL_FULLSCREEN, this);
}

void Fl_Window::fullscreen_off_x(int X, int Y, int W, int H) {
  _clear_fullscreen();
  hide();
  resize(X, Y, W, H);
  show();
  Fl::handle(FL_FULLSCREEN, this);
}

/*
 * Initialize the given port for redraw and call the window's flush() to actually draw the content
 */ 
void Fl_X::flush()
{
  if (through_drawRect || w->as_gl_window()) {
    make_current_counts = 1;
    w->flush();
    make_current_counts = 0;
    Fl_X::q_release_context();
    return;
  }
  // have Cocoa immediately redraw the window's view
  FLView *view = (FLView*)[fl_xid(w) contentView];
  fl_x_to_redraw = this;
  [view setNeedsDisplay:YES];
  // will send the drawRect: message to the window's view after having prepared the adequate NSGraphicsContext
  [view displayIfNeededIgnoringOpacity]; 
  fl_x_to_redraw = NULL;
}

//bool Fl_X::make_shaped = false;

/*
 * go ahead, create that (sub)window
 */
void Fl_X::make(Fl_Window* w)
{
  if ( w->parent() ) {		// create a subwindow
    Fl_Group::current(0);
    // our subwindow needs this structure to know about its clipping. 
    Fl_X* x = new Fl_X;
    x->subwindow = true;
    x->other_xid = 0;
    x->region = 0;
    x->subRegion = 0;
    x->cursor = NULL;
    x->gc = 0;			// stay 0 for Quickdraw; fill with CGContext for Quartz
    w->set_visible();
    Fl_Window *win = w->window();
    Fl_X *xo = Fl_X::i(win);
    if (xo) {
      x->xidNext = xo->xidChildren;
      x->xidChildren = 0L;
      xo->xidChildren = x;
      x->xid = win->i->xid;
      x->w = w; w->i = x;
      x->wait_for_expose = 0;
      {
	Fl_X *z = xo->next;	// we don't want a subwindow in Fl_X::first
	xo->next = x;
	x->next = z;
      }
      int old_event = Fl::e_number;
      w->handle(Fl::e_number = FL_SHOW);
      Fl::e_number = old_event;
      w->redraw();		// force draw to happen
    }
    if (w->as_gl_window()) { // if creating a sub-GL-window
      while (win->window()) win = win->window();
      [Fl_X::i(win)->xid containsGLsubwindow:YES];
    }
    fl_show_iconic = 0;
  }
  else {			// create a desktop window
    Fl_Group::current(0);
    fl_open_display();
    NSInteger winlevel = NSNormalWindowLevel;
    NSUInteger winstyle;
    if (w->border()) winstyle = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
    else winstyle = NSBorderlessWindowMask;
    int xp = w->x();
    int yp = w->y();
    int wp = w->w();
    int hp = w->h();
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
    
    if (!fake_X_wm(w, xwm, ywm, bt, bx, by)) {
      // menu windows and tooltips
      if (w->modal()||w->tooltip_window()) {
        winlevel = modal_window_level();
      }
      //winstyle = NSBorderlessWindowMask;
    } 
    if (w->modal()) {
      winstyle &= ~NSMiniaturizableWindowMask;
      // winstyle &= ~(NSResizableWindowMask | NSMiniaturizableWindowMask);
      winlevel = modal_window_level();
    }
    else if (w->non_modal()) {
      winlevel = non_modal_window_level();
    }
    
    if (by+bt) {
      wp += 2*bx;
      hp += 2*by+bt;
    }
    if (w->force_position()) {
      if (!Fl::grab()) {
        xp = xwm; yp = ywm;
        w->x(xp);w->y(yp);
      }
      xp -= bx;
      yp -= by+bt;
    }
    
    if (w->non_modal() && Fl_X::first /*&& !fl_disable_transient_for*/) {
      // find some other window to be "transient for":
      Fl_Window* w = Fl_X::first->w;
      while (w->parent()) w = w->window(); // todo: this code does not make any sense! (w!=w??)
    }
        
    Fl_X* x = new Fl_X;
    x->subwindow = false;
    x->other_xid = 0; // room for doublebuffering image map. On OS X this is only used by overlay windows
    x->region = 0;
    x->subRegion = 0;
    x->cursor = NULL;
    x->xidChildren = 0;
    x->xidNext = 0;
    x->gc = 0;
	  
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
    crect.origin.x = w->x(); 
    crect.origin.y = main_screen_height - (w->y() + w->h());
    crect.size.width=w->w(); 
    crect.size.height=w->h();
    FLWindow *cw = [[FLWindow alloc] initWithFl_W:w 
				      contentRect:crect  
					styleMask:winstyle];
    [cw setFrameOrigin:crect.origin];
    [cw setHasShadow:YES];
    [cw setAcceptsMouseMovedEvents:YES];
    if (w->type() == FL_SHAPED_WINDOW) {
      [cw setOpaque:NO]; // shaped windows must be non opaque
      [cw setBackgroundColor:[NSColor clearColor]]; // and with transparent background color
      }
    x->xid = cw;
    x->w = w; w->i = x;
    x->wait_for_expose = 1;
    x->next = Fl_X::first;
    Fl_X::first = x;
    FLView *myview = [[FLView alloc] init];
    [cw setContentView:myview];
    [myview release];
    [cw setLevel:winlevel];
    
    q_set_window_title(cw, w->label(), w->iconlabel());
    if (!w->force_position()) {
      if (w->modal()) {
        [cw center];
      } else if (w->non_modal()) {
        [cw center];
      } else {
        static NSPoint delta = NSZeroPoint;
        delta = [cw cascadeTopLeftFromPoint:delta];
      }
    }
    if(w->menu_window()) { // make menu windows slightly transparent
      [cw setAlphaValue:0.97];
    }
    // Install DnD handlers 
    [myview registerForDraggedTypes:[NSArray arrayWithObjects:utf8_format,  NSFilenamesPboardType, nil]];
    if ( ! Fl_X::first->next ) {	
      // if this is the first window, we need to bring the application to the front
      [NSApp activateIgnoringOtherApps:YES];
    }
    
    if (w->size_range_set) w->size_range_();
    
    if ( w->border() || (!w->modal() && !w->tooltip_window()) ) {
      Fl_Tooltip::enter(0);
    }

    if (w->modal()) Fl::modal_ = w; 

    w->set_visible();
    if ( w->border() || (!w->modal() && !w->tooltip_window()) ) Fl::handle(FL_FOCUS, w);
    Fl::first_window(w);
    [cw setDelegate:[FLWindowDelegate createOnce]];
    if (fl_show_iconic) { 
      fl_show_iconic = 0;
      [cw miniaturize:nil];
    } else {
      [cw makeKeyAndOrderFront:nil];
    }
    
    crect = [[cw contentView] frame];
    w->w(int(crect.size.width));
    w->h(int(crect.size.height));
    crect = [cw frame];
    w->x(int(crect.origin.x));
    w->y(int(main_screen_height - (crect.origin.y + w->h())));
    
    int old_event = Fl::e_number;
    w->handle(Fl::e_number = FL_SHOW);
    Fl::e_number = old_event;
    
    // if (w->modal()) { Fl::modal_ = w; fl_fix_focus(); }
  }
}


/*
 * Tell the OS what window sizes we want to allow
 */
void Fl_Window::size_range_() {
  int bx, by, bt;
  get_window_frame_sizes(bx, by, bt);
  size_range_set = 1;
  NSSize minSize = { minw, minh + bt };
  NSSize maxSize = { maxw?maxw:32000, maxh?maxh + bt:32000 };
  if (i && i->xid) {
    [i->xid setMinSize:minSize];
    [i->xid setMaxSize:maxSize];
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
  if (!shown()) {
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
  }
}


/*
 * resize a window
 */
void Fl_Window::resize(int X,int Y,int W,int H) {
  if (W<=0) W = 1; // OS X does not like zero width windows
  if (H<=0) H = 1;
  int is_a_resize = (W != w() || H != h());
  //  printf("Fl_Window::resize(X=%d, Y=%d, W=%d, H=%d), is_a_resize=%d, resize_from_system=%p, this=%p\n",
  //         X, Y, W, H, is_a_resize, resize_from_system, this);
  if (X != x() || Y != y()) set_flag(FORCE_POSITION);
  else if (!is_a_resize) {
    resize_from_system = 0;
    return;
    }
  if ( (resize_from_system!=this) && (!parent()) && shown()) {
    if (is_a_resize) {
      if (resizable()) {
        if (W<minw) minw = W; // user request for resize takes priority
        if (maxw && W>maxw) maxw = W; // over a previously set size_range
        if (H<minh) minh = H;
        if (maxh && H>maxh) maxh = H;
        size_range(minw, minh, maxw, maxh);
      } else {
        size_range(W, H, W, H);
      }
      int bx, by, bt;
      if ( ! this->border() ) bt = 0;
      else get_window_frame_sizes(bx, by, bt);
      NSRect dim;
      dim.origin.x = X;
      dim.origin.y = main_screen_height - (Y + H);
      dim.size.width = W;
      dim.size.height = H + bt;
      [i->xid setFrame:dim display:YES]; // calls windowDidResize
    } else {
      NSPoint pt; 
      pt.x = X; 
      pt.y = main_screen_height - (Y + h());
      [i->xid setFrameOrigin:pt]; // calls windowDidMove
    }
    return;
  }
  resize_from_system = 0;
  if (is_a_resize) {
    Fl_Group::resize(X,Y,W,H);
    if (shown()) { 
      redraw(); 
    }
  } else {
    x(X); y(Y); 
  }
}


/*
 * make all drawing go into this window (called by subclass flush() impl.)
 
 This can be called in 3 different instances:
 
 1) When a window is created, resized, or deminiaturized. 
 The system sends the drawRect: message to the window's view after having prepared the current graphics context 
 to draw to this view. Variable through_drawRect is YES, and fl_x_to_redraw is NULL. Processing of drawRect: calls 
 handleUpdateEvent() that calls Fl_X::flush() for the window and its subwindows. Fl_X::flush() calls 
 Fl_Window::flush() that calls Fl_Window::make_current() that only needs to identify the graphics port of the 
 current graphics context. The window's draw() function is then executed.
 
 2) At each round of the FLTK event loop.
 Fl::flush() is called, that calls Fl_X::flush() on each window that needs drawing. Fl_X::flush() sets 
 fl_x_to_redraw to this and sends the displayIfNeededIgnoringOpacity message to the window's view. 
 This message makes the system prepare the current graphics context adequately for drawing to this view, and 
 send it the drawRect: message which sets through_drawRect to YES. Processing of the drawRect: message calls 
 Fl_X::flush() for the window which proceeds as in 1) above.
 
 3) An FLTK application can call Fl_Window::make_current() at any time before it draws to a window.
 This occurs for instance in the idle callback function of the mandelbrot test program. Variable through_drawRect is NO,
 so Fl_Window::make_current() creates a new graphics context adequate for the window. 
 Subsequent drawing requests go to this window. CAUTION: it's not possible to call Fl::wait(), Fl::check()
 nor Fl::ready() while in the draw() function of a widget. Use an idle callback instead.
 
 */
void Fl_Window::make_current() 
{
  if (make_current_counts > 1) return;
  if (make_current_counts) make_current_counts++;
  Fl_X::q_release_context();
  fl_window = i->xid;
  current_ = this;
  
  int xp = 0, yp = 0;
  Fl_Window *win = this;
  while ( win ) {
    if ( !win->window() )
      break;
    xp += win->x();
    yp += win->y();
    win = (Fl_Window*)win->window();
  }
  NSGraphicsContext *nsgc = through_drawRect ? [NSGraphicsContext currentContext] : 
					       [NSGraphicsContext graphicsContextWithWindow:fl_window];
  i->gc = (CGContextRef)[nsgc graphicsPort];
  fl_gc = i->gc;
  Fl_Region fl_window_region = XRectangleRegion(0,0,w(),h());
  if ( ! this->window() ) {
    for ( Fl_X *cx = i->xidChildren; cx; cx = cx->xidNext ) {	// clip-out all sub-windows
      Fl_Window *cw = cx->w;
      Fl_Region from = fl_window_region;
      fl_window_region = MacRegionMinusRect(from, cw->x(), cw->y(), cw->w(), cw->h() );
      XDestroyRegion(from);
    }
  }
  
  // antialiasing must be deactivated because it applies to rectangles too
  // and escapes even clipping!!!
  // it gets activated when needed (e.g., draw text)
  CGContextSetShouldAntialias(fl_gc, false);  
  CGFloat hgt = [[fl_window contentView] frame].size.height;
  CGContextTranslateCTM(fl_gc, 0.5, hgt-0.5f);
  CGContextScaleCTM(fl_gc, 1.0f, -1.0f); // now 0,0 is top-left point of the window
  win = this;
  while(win && win->window()) { // translate to subwindow origin if this is a subwindow context
    CGContextTranslateCTM(fl_gc, win->x(), win->y());
    win = win->window();
  }
  //apply window's clip
  CGContextClipToRects(fl_gc, fl_window_region->rects, fl_window_region->count );
  XDestroyRegion(fl_window_region);
// this is the context with origin at top left of (sub)window clipped out of its subwindows if any
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
    size_t hgt = CGBitmapContextGetHeight(fl_gc);
    CGContextTranslateCTM(fl_gc, 0.5, hgt-0.5f);
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
  CGContextRestoreGState(fl_gc); // KEEP IT: matches the CGContextSaveGState of make_current
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
  CGContextTranslateCTM(fl_gc, rect.origin.x - cx - 0.5, rect.origin.y - cy + h - 0.5);
  CGContextScaleCTM(fl_gc, 1, -1);
  rect.origin.x = rect.origin.y = 0;
  rect.size.width = w;
  rect.size.height = h;
}

void Fl_X::q_end_image() {
  CGContextRestoreGState(fl_gc);
}


////////////////////////////////////////////////////////////////
// Copy & Paste fltk implementation.
////////////////////////////////////////////////////////////////

static void convert_crlf(char * s, size_t len)
{
  // turn all \r characters into \n:
  for (size_t x = 0; x < len; x++) if (s[x] == '\r') s[x] = '\n';
}

// fltk 1.3 clipboard support constant definitions:
static NSString *calc_utf8_format(void)
{
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_6
#define NSPasteboardTypeString @"public.utf8-plain-text"
#endif
  if (fl_mac_os_version >= 100600) return NSPasteboardTypeString;
  return NSStringPboardType;
}

// clipboard variables definitions :
char *fl_selection_buffer[2] = {NULL, NULL};
int fl_selection_length[2] = {0, 0};
static int fl_selection_buffer_length[2];

static PasteboardRef allocatePasteboard(void) 
{
  PasteboardRef clip;
  PasteboardCreate(kPasteboardClipboard, &clip); // requires Mac OS 10.3
  return clip;
}
static PasteboardRef myPasteboard = allocatePasteboard();

extern void fl_trigger_clipboard_notify(int source);

void fl_clipboard_notify_change() {
  // No need to do anything here...
}

static void clipboard_check(void)
{
  PasteboardSyncFlags flags;
  
  flags = PasteboardSynchronize(myPasteboard); // requires Mac OS 10.3

  if (!(flags & kPasteboardModified))
    return;
  if (flags & kPasteboardClientIsOwner)
    return;

  fl_trigger_clipboard_notify(1);
}

/*
 * create a selection
 * stuff: pointer to selected data
 * len: size of selected data
 * type: always "plain/text" for now
 */
void Fl::copy(const char *stuff, int len, int clipboard, const char *type) {
  if (!stuff || len<0) return;
  if (len+1 > fl_selection_buffer_length[clipboard]) {
    delete[] fl_selection_buffer[clipboard];
    fl_selection_buffer[clipboard] = new char[len+100];
    fl_selection_buffer_length[clipboard] = len+100;
  }
  memcpy(fl_selection_buffer[clipboard], stuff, len);
  fl_selection_buffer[clipboard][len] = 0; // needed for direct paste
  fl_selection_length[clipboard] = len;
  if (clipboard) {
    CFDataRef text = CFDataCreate(kCFAllocatorDefault, (UInt8*)fl_selection_buffer[1], len);
    if (text==NULL) return; // there was a pb creating the object, abort.
    NSPasteboard *clip = [NSPasteboard generalPasteboard];
    [clip declareTypes:[NSArray arrayWithObject:utf8_format] owner:nil];
    [clip setData:(NSData*)text forType:utf8_format];
    CFRelease(text);
  }
}

static int get_plain_text_from_clipboard(char **buffer, int previous_length)
{
  NSInteger length = 0;
  NSPasteboard *clip = [NSPasteboard generalPasteboard];
  NSString *found = [clip availableTypeFromArray:[NSArray arrayWithObjects:utf8_format, @"public.utf16-plain-text", @"com.apple.traditional-mac-plain-text", nil]];
  if (found) {
    NSData *data = [clip dataForType:found];
    if (data) {
      NSInteger len;
      char *aux_c = NULL;
      if (![found isEqualToString:utf8_format]) {
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
      if ( len >= previous_length ) {
	length = len;
	delete[] *buffer;
	*buffer = new char[len];
      }
      if (![found isEqualToString:utf8_format]) {
	strcpy(*buffer, aux_c);
	free(aux_c);
      }
      else {
	[data getBytes:*buffer];
      }
      (*buffer)[len - 1] = 0;
      length = len - 1;
      convert_crlf(*buffer, len - 1); // turn all \r characters into \n:
      Fl::e_clipboard_type = Fl::clipboard_plain_text;
    }
  }    
  return length;
}

static Fl_Image* get_image_from_clipboard()
{
  Fl_RGB_Image *image = NULL;
  uchar *imagedata;
  NSBitmapImageRep *bitmap;
  NSPasteboard *clip = [NSPasteboard generalPasteboard];
  NSArray *present = [clip types]; // types in pasteboard in order of decreasing preference
  NSArray  *possible = [NSArray arrayWithObjects:@"com.adobe.pdf", @"public.tiff", @"com.apple.pict", nil];
  NSString *found = nil;
  NSUInteger rank;
  for (rank = 0; rank < [present count]; rank++) { // find first of possible types present in pasteboard
    for (NSUInteger i = 0; i < [possible count]; i++) {
      if ([[present objectAtIndex:rank] isEqualToString:[possible objectAtIndex:i]]) {
	found = [present objectAtIndex:rank];
	goto after_loop;
      }
    }
  }
after_loop: 
  if (found) {
    NSData *data = [clip dataForType:found];
    if (data) {
      if ([found isEqualToString:@"public.tiff"]) {
	bitmap = [NSBitmapImageRep imageRepWithData:data];
	int bpp = [bitmap bytesPerPlane];
	int bpr = [bitmap bytesPerRow];
	int depth = [bitmap samplesPerPixel], w = bpr/depth, h = bpp/bpr;
	imagedata = new uchar[w * h * depth];
	memcpy(imagedata, [bitmap bitmapData], w * h * depth);
	image = new Fl_RGB_Image(imagedata, w, h, depth);
	image->alloc_array = 1;
      }
      else if ([found isEqualToString:@"com.adobe.pdf"] || [found isEqualToString:@"com.apple.pict"]) {
	NSRect rect;
	NSImageRep *vectorial;
	NSAffineTransform *dilate = [NSAffineTransform transform];
	if ([found isEqualToString:@"com.adobe.pdf"] ) {
	  vectorial = [NSPDFImageRep imageRepWithData:data];
	  rect = [(NSPDFImageRep*)vectorial bounds]; // in points =  1/72 inch
	  Fl_Window *win = Fl::first_window();
	  int screen_num = win ? Fl::screen_num(win->x(), win->y(), win->w(), win->h()) : 0;
	  float hr, vr;
	  Fl::screen_dpi(hr, vr, screen_num); // 1 inch = hr pixels = 72 points -> hr/72 pixel/point	  
	  CGFloat scale = hr/72;
	  [dilate scaleBy:scale];
	  rect.size.width *= scale;
	  rect.size.height *= scale;
	  rect = NSIntegralRect(rect);
	  }
	else {
	  vectorial = [NSPICTImageRep imageRepWithData:data];
	  rect = [(NSPICTImageRep*)vectorial boundingBox]; // in pixel, no scaling required
	  }
	imagedata = new uchar[(int)(rect.size.width * rect.size.height) * 4];
	memset(imagedata, -1, (int)(rect.size.width * rect.size.height) * 4);
	bitmap = [[NSBitmapImageRep alloc]  initWithBitmapDataPlanes:&imagedata
							  pixelsWide:rect.size.width
							  pixelsHigh:rect.size.height
						       bitsPerSample:8
						     samplesPerPixel:3
							    hasAlpha:NO
							    isPlanar:NO
						      colorSpaceName:NSDeviceRGBColorSpace
							 bytesPerRow:rect.size.width*4
							bitsPerPixel:32];
	NSDictionary *dict = [NSDictionary dictionaryWithObject:bitmap 
							 forKey:NSGraphicsContextDestinationAttributeName];
	NSGraphicsContext *oldgc = [NSGraphicsContext currentContext];
	[NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithAttributes:dict]];
	[dilate concat];
	[vectorial draw];
	[NSGraphicsContext setCurrentContext:oldgc];
	[bitmap release];
	image = new Fl_RGB_Image(imagedata, rect.size.width, rect.size.height, 4);
	image->alloc_array = 1;
      }
      Fl::e_clipboard_type = Fl::clipboard_image;
    }
  }
  return image;
}

// Call this when a "paste" operation happens:
void Fl::paste(Fl_Widget &receiver, int clipboard, const char *type) {
  if (type[0] == 0) type = Fl::clipboard_plain_text;
  if (clipboard) {
    Fl::e_clipboard_type = "";
   if (strcmp(type, Fl::clipboard_plain_text) == 0) {
      fl_selection_length[1] = get_plain_text_from_clipboard( &fl_selection_buffer[1],  fl_selection_length[1]);   
      }
    else if (strcmp(type, Fl::clipboard_image) == 0) {
      Fl::e_clipboard_data = get_image_from_clipboard( );
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
    found = [[NSPasteboard generalPasteboard] availableTypeFromArray:[NSArray arrayWithObjects:utf8_format, @"public.utf16-plain-text", @"com.apple.traditional-mac-plain-text", nil]];
    }
  else if (strcmp(type, Fl::clipboard_image) == 0) {
    found = [[NSPasteboard generalPasteboard] availableTypeFromArray:[NSArray arrayWithObjects:@"public.tiff", @"com.adobe.pdf", @"com.apple.pict", nil]];
    }
  return found != nil;
}

int Fl_X::unlink(Fl_X *start) {
  if (start) {
    Fl_X *pc = start;
    while (pc) {
      if (pc->xidNext == this) {
        pc->xidNext = xidNext;
        return 1;
      }
      if (pc->xidChildren) {
        if (pc->xidChildren == this) {
          pc->xidChildren = xidNext;
          return 1;
        }
        if (unlink(pc->xidChildren))
          return 1;
      }
      pc = pc->xidNext;
    }
  } else {
    for ( Fl_X *pc = Fl_X::first; pc; pc = pc->next ) {
      if (unlink(pc))
        return 1;
    }
  }  
  return 0;
}

void Fl_X::relink(Fl_Window *w, Fl_Window *wp) {
  Fl_X *x = Fl_X::i(w);
  Fl_X *p = Fl_X::i(wp);
  if (!x || !p) return;
  // first, check if 'x' is already registered as a child of 'p'
  for (Fl_X *i = p->xidChildren; i; i=i->xidNext) {
    if (i == x) return;
  }
  // now add 'x' as the first child of 'p'
  x->xidNext = p->xidChildren;
  p->xidChildren = x;
}

void Fl_X::destroy() {
  // subwindows share their xid with their parent window, so should not close it
  if (!subwindow && w && !w->parent() && xid) {
    [xid close];
  }
}

void Fl_X::map() {
  if (w && xid) {
    [xid orderFront:nil];
  }
  //+ link to window list
  if (w && w->parent()) {
    Fl_X::relink(w, w->window() );
    w->redraw();
  }
  if (cursor) {
    [(NSCursor*)cursor release];
    cursor = NULL;
  }
}

void Fl_X::unmap() {
  if (w && !w->parent() && xid) {
    [xid orderOut:nil];
  }
  if (w && Fl_X::i(w)) 
    Fl_X::i(w)->unlink();
}


// removes x,y,w,h rectangle from region r and returns result as a new Fl_Region
static Fl_Region MacRegionMinusRect(Fl_Region r, int x,int y,int w,int h)
{
  Fl_Region outr = (Fl_Region)malloc(sizeof(*outr));
  outr->rects = (CGRect*)malloc(4 * r->count * sizeof(CGRect));
  outr->count = 0;
  CGRect rect = fl_cgrectmake_cocoa(x, y, w, h);
  for( int i = 0; i < r->count; i++) {
    CGRect A = r->rects[i];
    CGRect test = CGRectIntersection(A, rect);
    if (CGRectIsEmpty(test)) {
      outr->rects[(outr->count)++] = A;
    }
    else {
      const CGFloat verylarge = 100000.;
      CGRect side = CGRectMake(0,0,rect.origin.x,verylarge);// W side
      test = CGRectIntersection(A, side);
      if ( ! CGRectIsEmpty(test)) {
        outr->rects[(outr->count)++] = test;
      }
      side = CGRectMake(0,rect.origin.y + rect.size.height,verylarge,verylarge);// N side
      test = CGRectIntersection(A, side);
      if ( ! CGRectIsEmpty(test)) {
        outr->rects[(outr->count)++] = test;
      }
      side = CGRectMake(rect.origin.x + rect.size.width, 0, verylarge, verylarge);// E side
      test = CGRectIntersection(A, side);
      if ( ! CGRectIsEmpty(test)) {
        outr->rects[(outr->count)++] = test;
      }
      side = CGRectMake(0, 0, verylarge, rect.origin.y);// S side
      test = CGRectIntersection(A, side);
      if ( ! CGRectIsEmpty(test)) {
        outr->rects[(outr->count)++] = test;
      }
    }
  }
  if (outr->count == 0) {
    free(outr->rects);
    free(outr);
    outr = XRectangleRegion(0,0,0,0);
  }
  else outr->rects = (CGRect*)realloc(outr->rects, outr->count * sizeof(CGRect));
  return outr;
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
      unsigned char *pdata = (unsigned char *)CGBitmapContextGetData(c);
      NSBitmapImageRep *imagerep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&pdata
									   pixelsWide:CGBitmapContextGetWidth(c)
									   pixelsHigh:CGBitmapContextGetHeight(c)
									bitsPerSample:8
								      samplesPerPixel:4
									     hasAlpha:YES
									     isPlanar:NO
								       colorSpaceName:NSDeviceRGBColorSpace
									  bytesPerRow:CGBitmapContextGetBytesPerRow(c)
									 bitsPerPixel:CGBitmapContextGetBitsPerPixel(c)];
      image = [[NSImage alloc] initWithData: [imagerep TIFFRepresentation]];
      [imagerep release];
    }
  return [image autorelease];
}


CFDataRef Fl_X::CGBitmapContextToTIFF(CGContextRef c)
{ // the returned value is autoreleased
  unsigned char *pdata = (unsigned char *)CGBitmapContextGetData(c);
  NSBitmapImageRep *imagerep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&pdata
								       pixelsWide:CGBitmapContextGetWidth(c)
								       pixelsHigh:CGBitmapContextGetHeight(c)
								    bitsPerSample:8
								  samplesPerPixel:3
									 hasAlpha:NO
									 isPlanar:NO
								   colorSpaceName:NSDeviceRGBColorSpace
								      bytesPerRow:CGBitmapContextGetBytesPerRow(c)
								     bitsPerPixel:CGBitmapContextGetBitsPerPixel(c)];
  NSData* tiff = [imagerep TIFFRepresentation];
  [imagerep release];
  return (CFDataRef)tiff;
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
    i += image->ld();
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
- (void)showPanel;
- (void)printPanel;
@end
@implementation FLaboutItemTarget
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
  printer.end_page();
  printer.end_job();
  fl_unlock_function();
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
    [appleMenu setAutoenablesItems:NO];
    [menuItem setEnabled:YES];
    [appleMenu addItem:[NSMenuItem separatorItem]];
    }
  if (fl_mac_os_version >= 100400) { // services+hide+quit already in menu in OS 10.3
    // Services Menu
    services = [[NSMenu alloc] init];
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
    [appleMenu addItemWithTitle:title 
			 action:@selector(terminate:) 
		  keyEquivalent:@"q"];
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
  Fl_Offscreen off = Fl_Quartz_Graphics_Driver::create_offscreen_with_alpha(width, height);
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

int Fl::dnd(void)
{
  CFDataRef text = CFDataCreate(kCFAllocatorDefault, (UInt8*)fl_selection_buffer[0], fl_selection_length[0]);
  if (text==NULL) return false;
  NSAutoreleasePool *localPool;
  localPool = [[NSAutoreleasePool alloc] init]; 
  NSPasteboard *mypasteboard = [NSPasteboard pasteboardWithName:NSDragPboard];
  [mypasteboard declareTypes:[NSArray arrayWithObject:utf8_format] owner:nil];
  [mypasteboard setData:(NSData*)text forType:utf8_format];
  CFRelease(text);
  Fl_Widget *w = Fl::pushed();
  Fl_Window *win = w->top_window();
  NSView *myview = [Fl_X::i(win)->xid contentView];
  NSEvent *theEvent = [NSApp currentEvent];
  
  int width, height;
  NSImage *image;
  if ( dynamic_cast<Fl_Input_*>(w) != NULL ||  dynamic_cast<Fl_Text_Display*>(w) != NULL) {
    fl_selection_buffer[0][ fl_selection_length[0] ] = 0;
    image = imageFromText(fl_selection_buffer[0], &width, &height);
  } else {
    image = defaultDragImage(&width, &height);
  }
  
  static NSSize offset={0,0};
  NSPoint pt = [theEvent locationInWindow];
  pt.x -= width/2;
  pt.y -= height/2;
  [myview dragImage:image  at:pt  offset:offset 
              event:theEvent  pasteboard:mypasteboard  
             source:myview  slideBack:YES];
  if ( w ) {
    int old_event = Fl::e_number;
    w->handle(Fl::e_number = FL_RELEASE);
    Fl::e_number = old_event;
    Fl::pushed( 0 );
  }
  [localPool release];
  return true;
}

static NSBitmapImageRep* rect_to_NSBitmapImageRep(Fl_Window *win, int x, int y, int w, int h)
// the returned value is autoreleased
{
  NSRect rect;
  NSView *winview = nil;
  while (win->window()) {
    x += win->x();
    y += win->y();
    win = win->window();
  }
  if ( through_drawRect ) {
    CGFloat epsilon = 0;
    if (fl_mac_os_version >= 100600) epsilon = 0.5; // STR #2887
    rect = NSMakeRect(x - epsilon, y - epsilon, w, h);
    }
  else {
    rect = NSMakeRect(x, win->h()-(y+h), w, h);
    // lock focus to win's view
    winview = [fl_xid(win) contentView];
    [winview lockFocus];
    }
  NSBitmapImageRep *bitmap = [[[NSBitmapImageRep alloc] initWithFocusedViewRect:rect] autorelease];
  if ( !through_drawRect ) [winview unlockFocus];
  return bitmap;
}

unsigned char *Fl_X::bitmap_from_window_rect(Fl_Window *win, int x, int y, int w, int h, int *bytesPerPixel)
/* Returns a capture of a rectangle of a mapped window as a pre-multiplied RGBA array of bytes.
 Alpha values are always 1 (except for the angles of a window title bar)
 so pre-multiplication can be ignored. 
 *bytesPerPixel is always set to the value 4 upon return.
 delete[] the returned pointer after use
 */
{
  NSBitmapImageRep *bitmap = rect_to_NSBitmapImageRep(win, x, y, w, h);
  if (bitmap == nil) return NULL;
  *bytesPerPixel = [bitmap bitsPerPixel]/8;
  int bpp = (int)[bitmap bytesPerPlane];
  int bpr = (int)[bitmap bytesPerRow];
  int hh = bpp/bpr; // sometimes hh = h-1 for unclear reason
  int ww = bpr/(*bytesPerPixel); // sometimes ww = w-1
  unsigned char *data = new unsigned char[w * h *  *bytesPerPixel];
  if (w == ww) {
    memcpy(data, [bitmap bitmapData], w * hh *  *bytesPerPixel);
  } else {
    unsigned char *p = [bitmap bitmapData];
    unsigned char *q = data;
    for(int i = 0;i < hh; i++) {
      memcpy(q, p, *bytesPerPixel * ww);
      p += bpr;
      q += w * *bytesPerPixel;
      }
  }
  return data;
}

static void imgProviderReleaseData (void *info, const void *data, size_t size)
{
  delete[] (unsigned char *)data;
}

CGImageRef Fl_X::CGImage_from_window_rect(Fl_Window *win, int x, int y, int w, int h)
// CFRelease the returned CGImageRef after use
{
  CGImageRef img;
  if (fl_mac_os_version >= 100500) {
    NSBitmapImageRep *bitmap = rect_to_NSBitmapImageRep(win, x, y, w, h);
    img = (CGImageRef)[bitmap performSelector:@selector(CGImage)]; // requires Mac OS 10.5
    CGImageRetain(img);
    }
  else {
    int bpp;
    unsigned char *bitmap = bitmap_from_window_rect(win, x, y, w, h, &bpp);
    if (!bitmap) return NULL;
    CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, bitmap, w*h*bpp, imgProviderReleaseData);
    img = CGImageCreate(w, h, 8, 8*bpp, w*bpp, lut,
			bpp == 3 ? kCGImageAlphaNone : kCGImageAlphaPremultipliedLast,
			provider, NULL, false, kCGRenderingIntentDefault);
    CGColorSpaceRelease(lut);
    CGDataProviderRelease(provider);
   }
  return img;
}

WindowRef Fl_X::window_ref()
{
  return (WindowRef)[xid windowRef];
}

// so a CGRect matches exactly what is denoted x,y,w,h for clipping purposes
CGRect fl_cgrectmake_cocoa(int x, int y, int w, int h) {
  return CGRectMake(x, y, w > 0 ? w - 0.9 : 0, h > 0 ? h - 0.9 : 0);
}

Window fl_xid(const Fl_Window* w)
{
  Fl_X *temp = Fl_X::i(w);
  return temp ? temp->xid : 0;
}

int Fl_Window::decorated_w()
{
  if (!shown() || parent() || !border() || !visible()) return w();
  int bx, by, bt;
  get_window_frame_sizes(bx, by, bt);
  return w() + 2 * bx;
}

int Fl_Window::decorated_h()
{
  if (!shown() || parent() || !border() || !visible()) return h();
  int bx, by, bt;
  get_window_frame_sizes(bx, by, bt);
  return h() + bt + by;
}

void Fl_Paged_Device::print_window(Fl_Window *win, int x_offset, int y_offset)
{
  if (!win->shown() || win->parent() || !win->border() || !win->visible()) {
    this->print_widget(win, x_offset, y_offset);
    return;
  }
  int bx, by, bt, bpp;
  get_window_frame_sizes(bx, by, bt);
  Fl_Display_Device::display_device()->set_current(); // send win to front and make it current
  const char *title = win->label();
  win->label(""); // temporarily set a void window title
  win->show();
  fl_gc = NULL;
  Fl::check();
  // capture the window title bar with no title
  unsigned char *bitmap = Fl_X::bitmap_from_window_rect(win, 0, -bt, win->w(), bt, &bpp);
  win->label(title); // put back the window title
  // and print it
  this->set_current(); // back to the Fl_Paged_Device
  Fl_RGB_Image *rgb = new Fl_RGB_Image(bitmap, win->w(), bt, bpp);
  rgb->draw(x_offset, y_offset);
  delete rgb;
  delete[] bitmap;
  if (title) { // print the window title
    const int skip = 68; // approx width of the zone of the 3 window control buttons
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
    if (fl_mac_os_version >= 100400 && dynamic_cast<Fl_Printer*>(this)) { // use Cocoa string drawing with exact title bar font
      NSGraphicsContext *current = [NSGraphicsContext currentContext];
      [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithGraphicsPort:fl_gc flipped:YES]];//10.4
      NSDictionary *attr = [NSDictionary dictionaryWithObject:[NSFont titleBarFontOfSize:0] 
						       forKey:NSFontAttributeName];
      NSString *title_s = [fl_xid(win) title];
      NSSize size = [title_s sizeWithAttributes:attr];
      int x = x_offset + win->w()/2 - size.width/2;
      if (x < x_offset+skip) x = x_offset+skip;
      NSRect r = {{x, y_offset+bt/2+4}, {win->w() - skip, bt}};
      [[NSGraphicsContext currentContext] setShouldAntialias:YES];
      [title_s drawWithRect:r options:(NSStringDrawingOptions)0 attributes:attr]; // 10.4
      [[NSGraphicsContext currentContext] setShouldAntialias:NO];
      [NSGraphicsContext setCurrentContext:current];
    }
    else
#endif
    {
      fl_font(FL_HELVETICA, 14); // the exact font is LucidaGrande 13 pts
      fl_color(FL_BLACK);
      int x = x_offset + win->w()/2 - fl_width(title)/2;
      if (x < x_offset+skip) x = x_offset+skip;
      fl_push_clip(x_offset, y_offset, win->w(), bt);
      fl_draw(title, x, y_offset+bt/2+4);
      fl_pop_clip();
    }
  }
  this->print_widget(win, x_offset, y_offset + bt); // print the window inner part
}


/* Returns the address of a Carbon function after dynamically loading the Carbon library if needed.
 Supports old Mac OS X versions that may use a couple of Carbon calls:
 GetKeys used by OS X 10.3 or before (in Fl::get_key())
 PMSessionPageSetupDialog and PMSessionPrintDialog used by 10.4 or before (in Fl_Printer::start_job())
 GetWindowPort used by 10.4 or before (in Fl_Gl_Choice.cxx)
 */
void *Fl_X::get_carbon_function(const char *function_name) {
  static void *carbon = NULL;
  void *f = NULL;
  if (!carbon) {
    carbon = dlopen("/System/Library/Frameworks/Carbon.framework/Carbon", RTLD_LAZY);
  }
  if (carbon) {
    f = dlsym(carbon, function_name);
  }
  return f;
}
  
/* Returns the version of the running Mac OS as an int such as 100802 for 10.8.2
 */
static int calc_mac_os_version() {
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
  return M*10000 + m*100 + b;
}

#endif // __APPLE__

//
// End of "$Id$".
//
