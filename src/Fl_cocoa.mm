//
// "$Id: Fl_cocoa.mm 6971 2009-04-13 07:32:01Z matt $"
//
// MacOS specific code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
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

// we don't need the following definition because we deliver only
// true mouse moves.  On very slow systems however, this flag may
// still be useful.
#ifndef FL_DOXYGEN

#define CONSOLIDATE_MOTION 0
extern "C" {
#include <pthread.h>
}


#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Input_.H>
#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"
#include <unistd.h>
#include <stdarg.h>

#import <Cocoa/Cocoa.h>

#ifndef NSINTEGER_DEFINED // appears with 10.5 in NSObjCRuntime.h
#if defined(__LP64__) && __LP64__
typedef long NSInteger;
typedef unsigned long NSUInteger;
#else
typedef int NSInteger;
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
extern Fl_Offscreen fl_create_offscreen_with_alpha(int w, int h);
extern CGContextRef CreateWatchImage(void);
extern CGContextRef CreateHelpImage(void);
extern CGContextRef CreateNESWImage(void);
extern CGContextRef CreateNWSEImage(void);
extern CGContextRef CreateNoneImage(void);

// forward definition of functions in this file
// converting cr lf converter function
static void convert_crlf(char * string, size_t len);
static void createAppleMenu(void);
static Fl_Region MacRegionMinusRect(Fl_Region r, int x,int y,int w,int h);
static void cocoaMouseHandler(NSEvent *theEvent);

// public variables
int fl_screen;
CGContextRef fl_gc = 0;
void *fl_system_menu;                   // this is really a NSMenu*
Fl_Sys_Menu_Bar *fl_sys_menu_bar = 0;
void *fl_default_cursor;		// this is really a NSCursor*
void *fl_capture = 0;			// (NSWindow*) we need this to compensate for a missing(?) mouse capture
//ulong fl_event_time;                  // the last timestamp from an x event
char fl_key_vector[32];                 // used by Fl::get_key()
bool fl_show_iconic;                    // true if called from iconize() - shows the next created window in collapsed state
int fl_disable_transient_for;           // secret method of removing TRANSIENT_FOR
//const Fl_Window* fl_modal_for;        // parent of modal() window
Fl_Region fl_window_region = 0;
Window fl_window;
Fl_Window *Fl_Window::current_;
//EventRef fl_os_event;		// last (mouse) event

// forward declarations of variables in this file
static int got_events = 0;
static Fl_Window* resize_from_system;
static SInt32 MACsystemVersion;

#if CONSOLIDATE_MOTION
static Fl_Window* send_motion;
extern Fl_Window* fl_xmousewin;
#endif

enum { FLTKTimerEvent = 1, FLTKDataReadyEvent };


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
static unsigned short macKeyLookUp[128] =
{
  'a', 's', 'd', 'f', 'h', 'g', 'z', 'x',
  'c', 'v', '^', 'b', 'q', 'w', 'e', 'r',
  
  'y', 't', '1', '2', '3', '4', '6', '5',
  '=', '9', '7', '-', '8', '0', ']', 'o',
  
  'u', '[', 'i', 'p', FL_Enter, 'l', 'j', '\'',
  'k', ';', '\\', ',', '/', 'n', 'm', '.',
  
  FL_Tab, ' ', '`', FL_BackSpace, 
  FL_KP_Enter, FL_Escape, 0, 0/*FL_Meta_L*/,
  0/*FL_Shift_L*/, 0/*FL_Caps_Lock*/, 0/*FL_Alt_L*/, 0/*FL_Control_L*/, 
  0/*FL_Shift_R*/, 0/*FL_Alt_R*/, 0/*FL_Control_R*/, 0,
  
  0, FL_KP+'.', FL_Right, FL_KP+'*', 0, FL_KP+'+', FL_Left, FL_Delete,
  FL_Down, 0, 0, FL_KP+'/', FL_KP_Enter, FL_Up, FL_KP+'-', 0,
  
  0, FL_KP+'=', FL_KP+'0', FL_KP+'1', FL_KP+'2', FL_KP+'3', FL_KP+'4', FL_KP+'5',
  FL_KP+'6', FL_KP+'7', 0, FL_KP+'8', FL_KP+'9', 0, 0, 0,
  
  FL_F+5, FL_F+6, FL_F+7, FL_F+3, FL_F+8, FL_F+9, 0, FL_F+11,
  0, 0/*FL_F+13*/, FL_Print, FL_Scroll_Lock, 0, FL_F+10, FL_Menu, FL_F+12,
  
  0, FL_Pause, FL_Help, FL_Home, FL_Page_Up, FL_Delete, FL_F+4, FL_End,
  FL_F+2, FL_Page_Down, FL_F+1, FL_Left, FL_Right, FL_Down, FL_Up, 0/*FL_Power*/,
};

/*
 * convert the current mouse chord into the FLTK modifier state
 */
static unsigned int mods_to_e_state( NSUInteger mods )
{
  long state = 0;
  if ( mods & NSNumericPadKeyMask ) state |= FL_NUM_LOCK;
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


/*
 * convert the current key chord into the FLTK keysym
 */

 static void mods_to_e_keysym( NSUInteger mods )
 {
 if ( mods & NSCommandKeyMask ) Fl::e_keysym = FL_Meta_L;
 else if ( mods & NSNumericPadKeyMask ) Fl::e_keysym = FL_Num_Lock;
 else if ( mods & NSAlternateKeyMask ) Fl::e_keysym = FL_Alt_L;
 else if ( mods & NSControlKeyMask ) Fl::e_keysym = FL_Control_L;
 else if ( mods & NSShiftKeyMask ) Fl::e_keysym = FL_Shift_L;
 else if ( mods & NSAlphaShiftKeyMask ) Fl::e_keysym = FL_Caps_Lock;
 else Fl::e_keysym = 0;
 //printf( "to sym 0x%08x (%04x)\n", Fl::e_keysym, mods );
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
  void *_userdata;		// thread's userdata
  
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
    _userdata = 0;
    _maxfd = 0;
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
  void StartThread(void *userdata);
  void CancelThread(const char *reason);
};

static DataReady dataready;

void DataReady::AddFD(int n, int events, void (*cb)(int, void*), void *v)
{
  RemoveFD(n, events);
  int i = nfds++;
  if (i >= fd_array_size) 
  {
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
  for (i=j=0; i<nfds; i++) {
    if (fds[i].fd == n) {
      int e = fds[i].events & ~events;
      if (!e) continue; // if no events left, delete this fd
      fds[i].events = e;
    }
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
  /*LOCK*/  if (n == _maxfd) _maxfd--;
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
  NSAutoreleasePool *localPool;
  localPool = [[NSAutoreleasePool alloc] init]; 
  while ( 1 ) {					// loop until thread cancel or error
    // Thread safe local copies of data before each select()
    self->DataLock();
    /*LOCK*/  int maxfd = self->_maxfd;
    /*LOCK*/  fd_set r = self->GetFdset(0);
    /*LOCK*/  fd_set w = self->GetFdset(1);
    /*LOCK*/  fd_set x = self->GetFdset(2);
    //    /*LOCK*/  void *userdata = self->_userdata;
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
        NSEvent *event = [NSEvent otherEventWithType:NSApplicationDefined location:pt 
				       modifierFlags:0
                                           timestamp:0
                                        windowNumber:0 context:NULL 
					     subtype:FLTKDataReadyEvent data1:0 data2:0];
        [NSApp postEvent:event atStart:NO];
        return(NULL);		// done with thread
      }
    }
  }
}

// START 'DATA READY' THREAD RUNNING, CREATE INTER-THREAD PIPE
void DataReady::StartThread(void *new_userdata)
{
  CancelThread(DEBUGTEXT("STARTING NEW THREAD\n"));
  DataLock();
  /*LOCK*/  pipe(_cancelpipe);	// pipe for sending cancel msg to thread
  /*LOCK*/  _userdata = new_userdata;
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
 * Check if there is actually a message pending!
 */
int fl_ready()
{
  NSEvent *retval = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate dateWithTimeIntervalSinceNow:0]
				    inMode:NSDefaultRunLoopMode dequeue:NO];
  return retval != nil;
}


static void processFLTKEvent(void) {
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
  return;
}


/*
 * break the current event loop
 */
static void breakMacEventLoop()
{
  fl_lock_function();
  
  NSPoint pt={0,0};
  NSEvent *event = [NSEvent otherEventWithType:NSApplicationDefined location:pt 
				 modifierFlags:0
                                     timestamp:0
                                  windowNumber:0 context:NULL 
				       subtype:FLTKTimerEvent data1:0 data2:0];
  [NSApp postEvent:event atStart:NO];
  fl_unlock_function();
}

//
// MacOS X timers
//

struct MacTimeout {
  Fl_Timeout_Handler callback;
  void* data;
  EventLoopTimerRef timer;
  EventLoopTimerUPP upp;
  char pending; 
};
static MacTimeout* mac_timers;
static int mac_timer_alloc;
static int mac_timer_used;

static void realloc_timers()
{
  if (mac_timer_alloc == 0) {
    mac_timer_alloc = 8;
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
    RemoveEventLoopTimer(t.timer);
    DisposeEventLoopTimerUPP(t.upp);
    memset(&t, 0, sizeof(MacTimeout));
  }
}

static void do_timer(EventLoopTimerRef timer, void* data)
{
  for (int i = 0;  i < mac_timer_used;  ++i) {
    MacTimeout& t = mac_timers[i];
    if (t.timer == timer  &&  t.data == data) {
      t.pending = 0;
      (*t.callback)(data);
      if (t.pending==0)
        delete_timer(t);
      break;
    }
  }
  breakMacEventLoop();
}

@interface FLWindow : NSWindow {
  Fl_Window *w;
  BOOL containsGLsubwindow;
}
- (FLWindow*)initWithFl_W:(Fl_Window *)flw 
	      contentRect:(NSRect)rect 
		styleMask:(NSUInteger)windowStyle 
		  backing:(NSBackingStoreType)bufferingType 
		    defer:(BOOL)deferCreation;
- (Fl_Window *)getFl_Window;
- (BOOL)windowShouldClose:(FLWindow *)w;
- (BOOL)containsGLsubwindow;
- (void)setContainsGLsubwindow:(BOOL)contains;
@end

@implementation FLWindow
- (FLWindow*)initWithFl_W:(Fl_Window *)flw 
	      contentRect:(NSRect)rect 
		styleMask:(NSUInteger)windowStyle 
		  backing:(NSBackingStoreType)bufferingType 
		    defer:(BOOL)deferCreation
{
  self = [super initWithContentRect:rect styleMask:windowStyle backing:bufferingType defer:deferCreation];
  if (self) {
    w = flw;
    containsGLsubwindow = NO;
  }
  return self;
}
- (Fl_Window *)getFl_Window;
{
  return w;
}
- (BOOL)windowShouldClose:(FLWindow *)fl
{
  Fl::handle( FL_CLOSE, [fl getFl_Window] ); // this might or might not close the window
  if (!Fl_X::first) return YES;
  Fl_Window *l = Fl::first_window();
  while( l != NULL && l != [fl getFl_Window]) l = Fl::next_window(l);
  return (l == NULL ? YES : NO);
}
- (BOOL)containsGLsubwindow
{
  return containsGLsubwindow;
}
- (void)setContainsGLsubwindow:(BOOL)contains
{
  containsGLsubwindow = contains;
}
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
    dataready.StartThread((void*)GetCurrentEventQueue());
  }
  
  fl_unlock_function();
	
  // necessary so that after closing a non-FLTK window (e.g., Fl_Native_File_Chooser)
  // the front window turns key again
  NSWindow *nsk = [NSApp keyWindow];
  NSWindow *nsm = [NSApp mainWindow];
  if ([nsm isMemberOfClass:[FLWindow class]] && (nsk == nil || ( ! [nsk isMemberOfClass:[FLWindow class]] &&
    ! [nsk isVisible] ) ) ) {
    [nsm makeKeyAndOrderFront:nil];
  }
  NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask 
                                      untilDate:[NSDate dateWithTimeIntervalSinceNow:time] 
                                         inMode:NSDefaultRunLoopMode dequeue:YES];  
  if (event != nil) {
    BOOL needSendEvent = YES;
    if ([event type] == NSLeftMouseDown) {
      Fl_Window *grab = Fl::grab();
      if (grab && grab != [(FLWindow *)[event window] getFl_Window]) {
	// a click event out of a menu window, so we should close this menu
	// done here to catch also clicks on window title bar/resize box 
	cocoaMouseHandler(event);
      }
    }
    else if ([event type] == NSApplicationDefined) {
      if ([event subtype] == FLTKDataReadyEvent) {
	processFLTKEvent();
      }
      needSendEvent = NO;
    }
    if (needSendEvent) [NSApp sendEvent:event]; 
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

/*
 * Cocoa Mousewheel handler
 */
void cocoaMouseWheelHandler(NSEvent *theEvent)
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
  
  if ([theEvent deltaX] != 0) {
    Fl::e_dx = (int)-[theEvent deltaX];
    Fl::e_dy = 0;
    if ( Fl::e_dx) Fl::handle( FL_MOUSEWHEEL, window );
  } else if ([theEvent deltaY] != 0) {
    Fl::e_dx = 0;
    Fl::e_dy = (int)-[theEvent deltaY];
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
  int clickCount = [theEvent clickCount];
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
      if (clickCount>1) 
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
      NSPoint screen = [[theEvent window] convertBaseToScreen:[theEvent locationInWindow]];
      Fl::e_x_root = screen.x;
      Fl::e_y_root = [[[theEvent window] screen] frame].size.height - screen.y;
      Fl::e_x = pos.x;
      Fl::e_y = pos.y;
      Fl::handle( sendEvent, window );
      }
      break;
    default:
      break;
  }
  
  fl_unlock_function();
  
  return;
}


/*
 * keycode_function for post-10.5 systems, allows more sophisticated decoding of keys
 */
/*
 static int keycodeToUnicode(
 char * uniChars, int maxChars,
 EventKind eKind,
 UInt32 keycode, UInt32 modifiers,
 UInt32 * deadKeyStatePtr,
 unsigned char,  // not used in this function
 unsigned short) // not used in this function
 {
   // first get the keyboard mapping in a post 10.2 way
   
   Ptr resource;
   TextEncoding encoding;
   static TextEncoding lastEncoding = kTextEncodingMacRoman;
   int len = 0;
   KeyboardLayoutRef currentLayout = NULL;
   static KeyboardLayoutRef lastLayout = NULL;
   SInt32 currentLayoutId = 0;
   static SInt32 lastLayoutId;
   int hasLayoutChanged = false;
   static Ptr uchr = NULL;
   static Ptr KCHR = NULL;
   // ScriptCode currentKeyScript;
   
   KLGetCurrentKeyboardLayout(&currentLayout);
    if (currentLayout) {
     KLGetKeyboardLayoutProperty(currentLayout, kKLIdentifier, (const void**)&currentLayoutId);
     if ( (lastLayout != currentLayout) || (lastLayoutId != currentLayoutId) ) {
       lastLayout = currentLayout;
       lastLayoutId = currentLayoutId;
       uchr = NULL;
       KCHR = NULL;
       if ((KLGetKeyboardLayoutProperty(currentLayout, kKLuchrData, (const void**)&uchr) == noErr) && (uchr != NULL)) {
	 // done
       } else if ((KLGetKeyboardLayoutProperty(currentLayout, kKLKCHRData, (const void**)&KCHR) == noErr) && (KCHR != NULL)) {
	 // done
       }
       // FIXME No Layout property found. Now we have a problem. 
     }
   }
   if (hasLayoutChanged) {
     // deadKeyStateUp = deadKeyStateDown = 0;
     if (KCHR != NULL) {
       // FIXME this must not happen
     } else if (uchr == NULL) {
       KCHR = (Ptr) GetScriptManagerVariable(smKCHRCache);
     }
   }
   if (uchr != NULL) {
     // this is what I expect
     resource = uchr;
   } else {
     resource = KCHR;
     encoding = lastEncoding;
     // this is actually not supported by the following code and will likely crash
   }
   
   // now apply that keyboard mapping to our keycode
   
   int action;
   //OptionBits options = 0;
   // not used yet: OptionBits options = kUCKeyTranslateNoDeadKeysMask;
   unsigned long keyboardType;
   keycode &= 0xFF;
   modifiers = (modifiers >> 8) & 0xFF;
   keyboardType = LMGetKbdType();
   OSStatus status;
   UniCharCount actuallength;
   UniChar utext[10];
   
   switch(eKind) {     
     case kEventRawKeyDown:    action = kUCKeyActionDown; break;
     case kEventRawKeyUp:      action = kUCKeyActionUp; break;
     case kEventRawKeyRepeat:  action = kUCKeyActionAutoKey; break;
     default: return 0;
   }
   
   UInt32 deadKeyState = *deadKeyStatePtr;
   if ((action==kUCKeyActionUp)&&(*deadKeyStatePtr)) {
     deadKeyStatePtr = &deadKeyState;
   }
   
   status = UCKeyTranslate((const UCKeyboardLayout *) uchr,
			   keycode, action, modifiers, keyboardType,
			   0, deadKeyStatePtr,
			   10, &actuallength, utext);
   
   if (noErr != status) {
     fprintf(stderr,"UCKeyTranslate failed: %d\n", (int) status);
     actuallength = 0;
   }
   
   // convert the list of unicode chars into utf8
   // FIXME no bounds check (see maxchars)
   unsigned i;
   for (i=0; i<actuallength; ++i) {
     len += fl_utf8encode(utext[i], uniChars+len);
   }
   uniChars[len] = 0;
   return len;
 }
 */

/*
 * keycode_function for pre-10.5 systems, this is the "historic" fltk Mac key handling
 */
static int keycode_wrap_old(char * buffer,
                            int, EventKind, UInt32, // not used in this function
                            UInt32, UInt32 *,       // not used in this function
                            unsigned char key,
                            unsigned short sym)
{
  if ( (sym >= FL_KP && sym <= FL_KP_Last) || !(sym & 0xff00) ||
        sym == FL_Tab || sym == FL_Enter) {
    buffer[0] = key;
    return 1;
  } else {
    buffer[0] = 0;
    return 0;
  }
} /* keycode_wrap_old */

/* 
 * Stub pointer to select appropriate keycode_function per operating system version. This function pointer
 * is initialised in fl_open_display, based on the runtime identification of the host OS version. This is
 * intended to allow us to utilise 10.5 services dynamically to improve Unicode handling, whilst still 
 * allowing code to run satisfactorily on older systems.
 */
static int (*keycode_function)(char*, int, EventKind, UInt32, UInt32, UInt32*, unsigned char, unsigned short) = keycode_wrap_old;


// EXPERIMENTAL!
// this gets called by CJK character palette input
OSStatus carbonTextHandler( EventHandlerCallRef nextHandler, EventRef event, void *unused )
{
  // make sure the key window is an FLTK window
  NSWindow *keywindow = [NSApp keyWindow];
  if (keywindow == nil || ![keywindow isMemberOfClass:[FLWindow class]]) return eventNotHandledErr;
  // under 10.5 this gets called only after character palette inputs
  // but under 10.6 this gets also called by interpretKeyEvents 
  // during character composition when we don't want to run it
  if ([[NSApp currentEvent] type] != NSSystemDefined) return eventNotHandledErr;
  Fl_Window *window = [(FLWindow*)keywindow getFl_Window];
  fl_lock_function();
  // int kind = GetEventKind(event);
  unsigned short buf[200];
  ByteCount size;
  GetEventParameter( event, kEventParamTextInputSendText, typeUnicodeText, 
                    NULL, 100, &size, &buf );
  //  printf("TextEvent: %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
  // FIXME: oversimplified!
  unsigned ucs = buf[0];
  char utf8buf[20];
  int len = fl_utf8encode(ucs, utf8buf); 
  
  Fl::e_length = len;
  Fl::e_text = utf8buf;
  while (window->parent()) window = window->window();
  Fl::handle(FL_KEYBOARD, window);
  fl_unlock_function();
  fl_lock_function();
  Fl::handle(FL_KEYUP, window);
  fl_unlock_function();
  // for some reason, the window does not redraw until the next mouse move or button push
  // sending a 'redraw()' or 'awake()' does not solve the issue!
  Fl::flush();
  return noErr;
}

static void processCompositionSequence(CFStringRef s, Fl_Window *window)
// composed character sequences are sent here
// they contain 2 unichars, the first comes from the deadkey and can be non ascii (e.g., diaeresis),
// the second is the character to be modified (e.g., e to be accented)
{
  // unicodes: non-ascii unicode chars produced by deadkeys
  // asciis: corresponding ascii chars expected by Fl::compose()
  //                           diaeresis acute-accent circumflex tilde ring-above 
  static UniChar unicodes[] = {0xA8,      0xB4,      0x2C6,     0x2DC, 0x2DA };
  static char asciis[] = {     ':',        '\'',      '^',       '~',    '*' };
  if (CFStringGetLength(s) == 0) return;
  char text[10];
  char buffer[10];
  UniChar unis[2];
  CFStringGetCharacters(s, CFRangeMake(0, 2), unis);
  for(unsigned int i = 0; i < sizeof(asciis)/sizeof(char); i++) {
    if (unis[0] == unicodes[i]) {
      // replace the non-ascii unicode by the corresponding ascii value
      unis[0] = (UniChar)asciis[i];
      break;
    }
  }
  CFStringRef smod = CFStringCreateWithCharacters(kCFAllocatorDefault, unis, 2);
  CFStringGetCString(smod, buffer, sizeof(buffer), kCFStringEncodingUTF8);
  CFRelease(smod);
  Fl::e_keysym = 0;
  Fl::e_state = 0;
  Fl::e_length = 1;
  Fl::e_text = text;
  Fl::e_text[0] = buffer[0];
  Fl::handle(FL_KEYBOARD, window);
  Fl::e_keysym = 0;
  Fl::e_state = 0;
  Fl::e_length = 1;
  Fl::e_text[0] = buffer[1];
  Fl::handle(FL_KEYBOARD, window);
}


/*
 * handle cocoa keyboard events
 */
OSStatus cocoaKeyboardHandler(NSEvent *theEvent)
{
  static char buffer[32];
  int sendEvent = 0;
  Fl_Window *window = (Fl_Window*)[(FLWindow*)[theEvent window] getFl_Window];
  Fl::first_window(window);
  NSUInteger mods;
  
  fl_lock_function();
  
  int kind = 0;
  
  // get the modifiers for any of the events
  mods = [theEvent modifierFlags];
  
  // get the key code only for key events
  UInt32 keyCode = 0, maskedKeyCode = 0;
  unichar key = 0;
  unsigned char keychar = 0;
  unsigned short sym = 0;
  keyCode = [theEvent keyCode];
  NSString *s = [theEvent characters];
  static BOOL compose = NO;
  static NSText *edit;
  static int countevents;
  static CFMutableStringRef sequence;	// will contain the two characters of the composition sequence
  if (compose) {	// we are in a composition sequence
    // the only benefit of sending events to the NSText object edit is that the deadkey becomes visible
    // at its keyUp event; without this, the deadkey remains invisible
    if ([s length] == 0) {	// occurs if 2 deadkeys are typed successively by error
      compose = NO;
      [edit setString:@""];
      CFRelease(sequence);
      fl_unlock_function();
      return noErr;
    }
    [edit interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
    countevents++;
    UniChar deadkey;
    CFStringGetCharacters((CFStringRef)s, CFRangeMake(0, 1), &deadkey);
    CFStringAppendCharacters(sequence, &deadkey, 1);// done for keyUp of deadkey and keyDown of next key
    if (countevents >= 3) {	// end of composition sequence
      processCompositionSequence( sequence, window );
      CFRelease(sequence);
      [edit setString:@""];	// clear the content of the edit object
      compose=NO;		// character composition is now complete
    }
    fl_unlock_function();
    return noErr;
  }
  if ([s length] == 0) {	// this is a dead key that must be combined with the next key to be pressed
    while (window->parent()) window = window->window();
    Fl::e_keysym = FL_Control_R; // first simulate pressing of the compose key (FL_Control_R)
    Fl::e_text = (char*)"";
    Fl::e_length = 0;
    Fl::handle(FL_KEYBOARD, window); 
    compose=YES;
    // then send remaining events to an object of type NSText that helps handle character composition sequences
    edit = [[theEvent window]  fieldEditor:YES forObject:nil];
    [edit interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
    countevents = 1;
    sequence = CFStringCreateMutable(NULL, 2);
    fl_unlock_function();
    return noErr;
  } else {
    char buff[10];
    CFStringGetCString((CFStringRef)s, buff, sizeof(buff), kCFStringEncodingUnicode);
    key = *(unichar*)buff;
    keychar = buff[0];
  }
  // extended keyboards can also send sequences on key-up to generate Kanji etc. codes.
  // Some observed prefixes are 0x81 to 0x83, followed by an 8 bit keycode.
  // In this mode, there seem to be no key-down codes
  // printf("%08x %08x %08x\n", keyCode, mods, key);
  maskedKeyCode = keyCode & 0x7f;
  /* output a human readable event identifier for debugging 
   * const char *ev = "";
   * switch (kind) {
   *    case kEventRawKeyDown:             ev = "kEventRawKeyDown";             break;
   *    case kEventRawKeyRepeat:           ev = "kEventRawKeyRepeat";           break;
   *    case kEventRawKeyUp:               ev = "kEventRawKeyUp";               break;
   *    case kEventRawKeyModifiersChanged: ev = "kEventRawKeyModifiersChanged"; break;
   *    default:                           ev = "unknown";                      break;
   * }
   * printf("%08x %08x %08x '%c' %s \n", mods, keyCode, key, key, ev);
   */
  switch([theEvent type]) {
    case NSKeyDown:
      sendEvent = FL_KEYBOARD;
      // fall through
    case NSKeyUp: {
      if ( !sendEvent ) {
        sendEvent = FL_KEYUP;
        Fl::e_state &= 0xbfffffff; // clear the deadkey flag
      }
      mods_to_e_state( mods ); // we process modifier keys at the same time
      // if the user pressed alt/option, event_key should have the keycap, 
      // but event_text should generate the international symbol
      sym = macKeyLookUp[maskedKeyCode];
      if ( isalpha(key) )
        sym = tolower(key);
      else if ( Fl::e_state&FL_CTRL && key<32 && sym<0xff00)
        sym = key+96;
      else if ( Fl::e_state&FL_ALT && sym<0xff00) {	// find the keycap of this key
	NSString *sim = [theEvent charactersIgnoringModifiers];
	UniChar one;
	CFStringGetCharacters((CFStringRef)sim, CFRangeMake(0, 1), &one);
	sym = one;
      }
      
      Fl::e_keysym = Fl::e_original_keysym = sym;
      // Handle FL_KP_Enter on regular keyboards and on Powerbooks
      if ( maskedKeyCode==0x4c || maskedKeyCode==0x34) key=0x0d;    
      static UInt32 deadKeyState = 0; // must be cleared when losing focus
      int l =  (*keycode_function)(buffer, 31, kind, keyCode, mods, &deadKeyState, keychar, sym);
      if (l > 0) {
        CFStringGetCString((CFStringRef)s, buffer, sizeof(buffer), kCFStringEncodingUTF8);
      }
      Fl::e_length = strlen(buffer);
      Fl::e_text = buffer;
      buffer[Fl::e_length] = 0; // just in case...
      }
      break;
    default:
      fl_unlock_function();
      return eventNotHandledErr;
  }
  while (window->parent()) window = window->window();
  if (sendEvent && Fl::handle(sendEvent,window)) {
    fl_unlock_function();  
    return noErr; // return noErr if FLTK handled the event
  } else {
    fl_unlock_function();
    return eventNotHandledErr;
  }
}



/*
 * Open callback function to call...
 */

static void	(*open_cb)(const char *) = 0;


/*
 * Install an open documents event handler...
 */
@interface FLAppleEventHandler : NSObject
{
}
- (void)handleAppleEvent:(NSAppleEventDescriptor *)event withReplyEvent: (NSAppleEventDescriptor *)replyEvent;
@end
@implementation FLAppleEventHandler
- (void)handleAppleEvent:(NSAppleEventDescriptor *)event withReplyEvent: (NSAppleEventDescriptor *)replyEvent
{
  NSAppleEventDescriptor *single = [event descriptorAtIndex:1];
  const AEDesc *document = [single aeDesc];
  long i, n;
  FSRef fileRef;
  AEKeyword keyWd;
  DescType typeCd;
  Size actSz;
  char filename[1024];
  // Lock access to FLTK in this thread...
  fl_lock_function();
  
  // Open the documents via the callback...
  if (AECountItems(document, &n) == noErr) {
    for (i = 1; i <= n; i ++) {
      AEGetNthPtr(document, i, typeFSRef, &keyWd, &typeCd,
                  (Ptr)&fileRef, sizeof(fileRef),
                  (actSz = sizeof(fileRef), &actSz));
      FSRefMakePath( &fileRef, (UInt8*)filename, sizeof(filename) );
      
      (*open_cb)(filename);
    }
  }
  
  // Unlock access to FLTK for all threads...
  fl_unlock_function();
}
@end

void fl_open_callback(void (*cb)(const char *)) {
  static NSAppleEventManager *aeventmgr = nil;
  static FLAppleEventHandler *handler;
  fl_open_display();
  if (!aeventmgr) {
    aeventmgr = [NSAppleEventManager sharedAppleEventManager];
    handler = [[FLAppleEventHandler alloc] init];
  }
  
  open_cb = cb;
  if (cb) {
    [aeventmgr setEventHandler:handler andSelector:@selector(handleAppleEvent:withReplyEvent:) 
                 forEventClass:kCoreEventClass andEventID:kAEOpenDocuments];
  } else {
    [aeventmgr removeEventHandlerForEventClass:kCoreEventClass andEventID:kAEOpenDocuments];  
  }
}


/*
 * initialize the Mac toolboxes, dock status, and set the default menubar
 */

extern "C" {
  extern OSErr CPSEnableForegroundOperation(ProcessSerialNumber *psn, UInt32 _arg2,
                                            UInt32 _arg3, UInt32 _arg4, UInt32 _arg5);
}

@interface FLDelegate : NSObject 
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
<NSWindowDelegate, NSApplicationDelegate>
#endif
{
}
- (void)windowDidMove:(NSNotification *)notif;
- (void)windowDidResize:(NSNotification *)notif;
- (void)windowDidBecomeKey:(NSNotification *)notif;
- (void)windowDidBecomeMain:(NSNotification *)notif;
- (void)windowDidDeminiaturize:(NSNotification *)notif;
- (void)windowDidMiniaturize:(NSNotification *)notif;
- (void)windowWillClose:(NSNotification *)notif;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender;

@end
@implementation FLDelegate
- (void)windowDidMove:(NSNotification *)notif
{
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  NSPoint pt, pt2; 
  pt.x = 0;
  pt.y = [[nsw contentView] frame].size.height;
  pt2 = [nsw convertBaseToScreen:pt];
  window->position((int)pt2.x, (int)([[nsw screen] frame].size.height - pt2.y));
  if ([nsw containsGLsubwindow] ) {
    [nsw display];// redraw window after moving if it contains OpenGL subwindows
  }
}
- (void)windowDidResize:(NSNotification *)notif
{
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  NSRect r = [[nsw contentView] frame];
  NSPoint pt, pt2; 
  pt.x = 0;
  pt.y = [[nsw contentView] frame].size.height;
  pt2 = [nsw convertBaseToScreen:pt];
  resize_from_system = window;
  window->resize((int)pt2.x, 
                 (int)([[nsw screen] frame].size.height - pt2.y),
		 (int)r.size.width,
		 (int)r.size.height);
}
- (void)windowDidBecomeKey:(NSNotification *)notif
{
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  Fl::handle( FL_FOCUS, window);
}
- (void)windowDidBecomeMain:(NSNotification *)notif
{
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  Fl::first_window(window);
}
- (void)windowDidDeminiaturize:(NSNotification *)notif
{
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  window->set_visible();
}
- (void)windowDidMiniaturize:(NSNotification *)notif
{
  FLWindow *nsw = (FLWindow*)[notif object];
  Fl_Window *window = [nsw getFl_Window];
  window->clear_visible();
}
- (void)windowWillClose:(NSNotification *)notif
{
  Fl_Window *w = Fl::first_window();
  if (!w) return;
  NSWindow *cw = (NSWindow*)Fl_X::i(w)->xid;
  if ( ![cw isMiniaturized] && ([cw styleMask] & NSTitledWindowMask) ) {
    if (![cw isKeyWindow]) {	// always make Fl::first_window() the key widow
      [cw makeKeyAndOrderFront:nil];
    }
    if (![cw isMainWindow]) {	// always make Fl::first_window() the main widow
      [cw makeMainWindow];
    }
  }
}
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
  fl_lock_function();
  NSApplicationTerminateReply reply = NSTerminateNow;
  while ( Fl_X::first ) {
    Fl_X *x = Fl_X::first;
    Fl::handle( FL_CLOSE, x->w );
    if ( Fl_X::first == x ) {
      reply = NSTerminateCancel; // FLTK has not closed all windows, so we return to the main program now
      break;
    }
  }
  fl_unlock_function();
  return reply;
}
@end

static FLDelegate *mydelegate;

void fl_open_display() {
  static char beenHereDoneThat = 0;
  if ( !beenHereDoneThat ) {
    beenHereDoneThat = 1;
	  
    [NSApplication sharedApplication];
    NSAutoreleasePool *localPool;
    localPool = [[NSAutoreleasePool alloc] init]; 
    mydelegate = [[FLDelegate alloc] init];
    [NSApp setDelegate:mydelegate];
    [NSApp finishLaunching];
		
    FlushEvents(everyEvent,0);
    
    fl_default_cursor = [NSCursor arrowCursor];
    Gestalt(gestaltSystemVersion, &MACsystemVersion);
    
    // bring the application into foreground without a 'CARB' resource
    Boolean same_psn;
    ProcessSerialNumber cur_psn, front_psn;
    if ( !GetCurrentProcess( &cur_psn ) && !GetFrontProcess( &front_psn ) &&
         !SameProcess( &front_psn, &cur_psn, &same_psn ) && !same_psn ) {
      // only transform the application type for unbundled apps
      CFBundleRef bundle = CFBundleGetMainBundle();
      if ( bundle ) {
      	FSRef execFs;
      	CFURLRef execUrl = CFBundleCopyExecutableURL( bundle );
      	CFURLGetFSRef( execUrl, &execFs );
        
      	FSRef bundleFs;
      	GetProcessBundleLocation( &cur_psn, &bundleFs );
        
      	if ( !FSCompareFSRefs( &execFs, &bundleFs ) )
          bundle = NULL;
        
        CFRelease(execUrl);
      }
      
      keycode_function = keycode_wrap_old; // under Cocoa we always use this one
      /*  // imm: keycode handler stub setting - use Gestalt to determine the running system version,
       *  // then set the keycode_function pointer accordingly
       *  if (MACsystemVersion >= 0x1050) {      // 10.5.0 or later
       *    keycode_function = keycodeToUnicode;
       *  }
       *  else {
       *    keycode_function = keycode_wrap_old; // pre-10.5 mechanism
       *  }
       */
      
      if ( !bundle )
      {
        // Earlier versions of this code tried to use weak linking, however it
        // appears that this does not work on 10.2.  Since 10.3 and higher provide
        // both TransformProcessType and CPSEnableForegroundOperation, the following
        // conditional code compiled on 10.2 will still work on newer releases...
        OSErr err;
#if __LP64__
        err = TransformProcessType(&cur_psn, kProcessTransformToForegroundApplication);
#else
        
#if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_2
        if (TransformProcessType != NULL) {
          err = TransformProcessType(&cur_psn, kProcessTransformToForegroundApplication);
        } else
#endif // MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_2
          err = CPSEnableForegroundOperation(&cur_psn, 0x03, 0x3C, 0x2C, 0x1103);
#endif // __LP64__
        if (err == noErr) {
          SetFrontProcess( &cur_psn );
        }
      }
    }
    createAppleMenu();
    // Install Carbon Event handler for character palette input
    static EventTypeSpec textEvents[] = {
      { kEventClassTextInput, kEventTextInputUnicodeForKeyEvent }
    };
    EventHandlerUPP textHandler = NewEventHandlerUPP( carbonTextHandler );
    InstallEventHandler(GetEventDispatcherTarget(), textHandler, 1, textEvents, NULL, 0L);
  }
}


/*
 * get rid of allocated resources
 */
void fl_close_display() {
}


// Gets the border sizes and the titlebar size
static void get_window_frame_sizes(int &bx, int &by, int &bt) {
  fl_open_display();
  NSRect inside = { {20,20}, {100,100} };
  NSRect outside = [NSWindow  frameRectForContentRect:inside styleMask:NSTitledWindowMask];
  bx = outside.origin.x - inside.origin.x;
  by = outside.origin.y - inside.origin.y;
  bt = outside.size.height - inside.size.height - by;
}

/*
 * smallest x ccordinate in screen space
 */
int Fl::x() {
  return [[NSScreen mainScreen] frame].origin.x;
}


/*
 * smallest y coordinate in screen space
 */
int Fl::y() {
  NSRect all = [[NSScreen mainScreen] frame];
  NSRect visible = [[NSScreen mainScreen] visibleFrame];
  return all.size.height - (visible.origin.y + visible.size.height);
}


/*
 * screen width (single monitor!?)
 */
int Fl::w() {
  return [[NSScreen mainScreen] visibleFrame].size.width;
}


/*
 * screen height (single monitor!?)
 */
int Fl::h() {
  int bx, by, bt;
  get_window_frame_sizes(bx, by, bt);
  return [[NSScreen mainScreen] frame].size.height - bt;
}


/*
 * get the current mouse pointer world coordinates
 */
void Fl::get_mouse(int &x, int &y) 
{
  fl_open_display();
  NSPoint pt = [NSEvent mouseLocation];
  x = pt.x;
  y = [[NSScreen mainScreen] frame].size.height - pt.y;
}


/*
 * convert Mac keystrokes to FLTK
 */
/*
 * unsigned short mac2fltk(ulong macKey) 
 * {
 *   unsigned short cc = macKeyLookUp[(macKey>>8)&0x7f];
 *   if (cc) return cc;
 *   return macKey&0xff;
 * }
 */

/*
 * Initialize the given port for redraw and call the window's flush() to actually draw the content
 */ 
void Fl_X::flush()
{
  w->flush();
  if (fl_gc) CGContextFlush(fl_gc);
}

/*
 * Gets called when a window is created, resized, or deminiaturized
 */    
static void handleUpdateEvent( Fl_Window *window ) 
{
  if ( !window ) return;
  Fl_X *i = Fl_X::i( window );
  i->wait_for_expose = 0;

  // FIXME: Matt: this is in the Carbon version. Does it need to be here?
  /* 
   * // I don't think so (MG). This function gets called only when a full
   * // redraw is needed (creation, resize, deminiaturization)
   * // and later in it we set damages to DAMAGE_ALL, so there is no
   * // point in limiting redraw to i->region
   * if ( i->xid && window->damage() ) {
   *   NSView *view = [(NSWindow*)i->xid contentView];
   *   if ( view && i->region ) {
   *     int ix;
   *     Fl_Region rgn = i->region;
   *     for (ix=0; ix<rgn->count; ix++) {
   *       NSRect rect = NSRectFromCGRect(rgn->rects[ix]);
   *       [view setNeedsDisplayInRect:rect];
   *     }
   *   }
   * }
   */ 
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
    cx->flush();
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
      get_window_frame_sizes(bx, by, bt);
    } else {
      ret = 1;
      get_window_frame_sizes(bx, by, bt);
    }
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
    cy = r.size.height - cy;
    if (    cx >= r.origin.x && cx <= r.origin.x + r.size.width
        && cy >= r.origin.y  && cy <= r.origin.y + r.size.height)
      break;
  }
  if (i < count) gd = [a objectAtIndex:i];
  
  // if the center doesn't fall on a screen, try the top left
  if (!gd) {
    for( i = 0; i < count; i++) {
      r = [[a objectAtIndex:i] frame];
      if (    X >= r.origin.x && X <= r.origin.x + r.size.width
          && r.size.height - Y >= r.origin.y  && r.size.height - Y <= r.origin.y + r.size.height)
        break;
    }
    if (i < count) gd = [a objectAtIndex:i];
  }
  // if that doesn't fall on a screen, try the top right
  if (!gd) {
    for( i = 0; i < count; i++) {
      r = [[a objectAtIndex:i] frame];
      if (    R >= r.origin.x && R <= r.origin.x + r.size.width
          && r.size.height - Y >= r.origin.y  && r.size.height - Y <= r.origin.y + r.size.height)
        break;
    }
    if (i < count) gd = [a objectAtIndex:i];
  }
  // if that doesn't fall on a screen, try the bottom left
  if (!gd) {
    for( i = 0; i < count; i++) {
      r = [[a objectAtIndex:i] frame];
      if (    X >= r.origin.x && X <= r.origin.x + r.size.width
          && Y-H >= r.origin.y  && Y-H <= r.origin.y + r.size.height)
        break;
    }
    if (i < count) gd = [a objectAtIndex:i];
  }
  // last resort, try the bottom right
  if (!gd) {
    for( i = 0; i < count; i++) {
      r = [[a objectAtIndex:i] frame];
      if (    R >= r.origin.x && R <= r.origin.x + r.size.width
          && Y-H >= r.origin.y  && Y-H <= r.origin.y + r.size.height)
        break;
    }
    if (i < count) gd = [a objectAtIndex:i];
  }
  // if we still have not found a screen, we will use the main
  // screen, the one that has the application menu bar.
  if (!gd) gd = [a objectAtIndex:0];
  if (gd) {
    r = [gd visibleFrame];
    int sh = [gd frame].size.height;
    if ( R > r.origin.x + r.size.width )  X -= R - (r.origin.x + r.size.width);
    if ( B > sh - r.origin.y ) Y -= B - (sh - r.origin.y);
    if ( X < r.origin.x )   X = r.origin.x;
    if ( Y < sh - (r.origin.y + r.size.height) )    Y = sh - (r.origin.y + r.size.height);
  }
  
  // Return the client area's top left corner in (X,Y)
  X+=xoff;
  Y+=yoff;
  
  return ret;
}


Fl_Window *fl_dnd_target_window = 0;

static void  q_set_window_title(NSWindow *nsw, const char * name ) {
  CFStringRef utf8_title = CFStringCreateWithCString(NULL, (name ? name : ""), kCFStringEncodingUTF8);
	[nsw setTitle:(NSString*)utf8_title ];
  CFRelease(utf8_title);
}


@interface FLView : NSView {
}
- (void)drawRect:(NSRect)rect;
- (BOOL)acceptsFirstResponder;
- (BOOL)acceptsFirstMouse:(NSEvent*)theEvent;
- (BOOL)performKeyEquivalent:(NSEvent*)theEvent;
- (void)mouseUp:(NSEvent *)theEvent;
- (void)rightMouseUp:(NSEvent *)theEvent;
- (void)otherMouseUp:(NSEvent *)theEvent;
- (void)mouseDown:(NSEvent *)theEvent;
- (void)rightMouseDown:(NSEvent *)theEvent;
- (void)otherMouseDown:(NSEvent *)theEvent;
- (void)mouseDragged:(NSEvent *)theEvent;
- (void)scrollWheel:(NSEvent *)theEvent;
- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;
- (void)flagsChanged:(NSEvent *)theEvent;
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender;
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
- (void)draggingExited:(id < NSDraggingInfo >)sender;
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal;
@end

@implementation FLView
- (void)drawRect:(NSRect)rect
{
  FLWindow *cw = (FLWindow*)[self window];
  Fl_Window *w = [cw getFl_Window];
  handleUpdateEvent(w);
}

- (BOOL)acceptsFirstResponder
{   
  return YES;
}
- (BOOL)performKeyEquivalent:(NSEvent*)theEvent
{   
  OSStatus err = cocoaKeyboardHandler(theEvent);
  return (err ? NO : YES);
}
- (BOOL)acceptsFirstMouse:(NSEvent*)theEvent
{   
  Fl_Window *w = [(FLWindow*)[theEvent window] getFl_Window];
  Fl_Window *first = Fl::first_window();
  return (first == w || !first->modal());
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
- (void)scrollWheel:(NSEvent *)theEvent {
  cocoaMouseWheelHandler(theEvent);
}
- (void)keyDown:(NSEvent *)theEvent {
  cocoaKeyboardHandler(theEvent);
}
- (void)keyUp:(NSEvent *)theEvent {
  cocoaKeyboardHandler(theEvent);
}
- (void)flagsChanged:(NSEvent *)theEvent {
  fl_lock_function();
  static UInt32 prevMods = mods_to_e_state( GetCurrentKeyModifiers() );
  NSUInteger mods = [theEvent modifierFlags];
  Fl_Window *window = (Fl_Window*)[(FLWindow*)[theEvent window] getFl_Window];
  UInt32 tMods = prevMods ^ mods;
  int sendEvent = 0;
  if ( tMods )
  {
    mods_to_e_keysym( tMods );
    if ( Fl::e_keysym ) 
      sendEvent = ( prevMods<mods ) ? FL_KEYBOARD : FL_KEYUP;
    Fl::e_length = 0;
    Fl::e_text = "";
    prevMods = mods;
  }
  mods_to_e_state( mods );
  while (window->parent()) window = window->window();
  if (sendEvent) Fl::handle(sendEvent,window);
  fl_unlock_function();
}
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender
{
  Fl_Window *target = [(FLWindow*)[self window] getFl_Window];
  NSPoint pt = [sender draggingLocation];
  Fl::e_x = pt.x;
  Fl::e_y = [self frame].size.height - pt.y;
  Fl::e_x_root = [[self window] frame].origin.x + Fl::e_x;
  Fl::e_y_root = [[self window] frame].origin.y + pt.y;
  Fl::e_y_root = [[[self window] screen] frame].size.height - Fl::e_y_root;
  fl_dnd_target_window = target;
  int ret = Fl::handle( FL_DND_ENTER, target );
  breakMacEventLoop();
  return ret ? NSDragOperationCopy : NSDragOperationNone;
}
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender
{
  Fl_Window *target = [(FLWindow*)[self window] getFl_Window];
  NSPoint pt = [sender draggingLocation];
  Fl::e_x = pt.x;
  Fl::e_y = [self frame].size.height - pt.y;
  Fl::e_x_root = [[self window] frame].origin.x + Fl::e_x;
  Fl::e_y_root = [[self window] frame].origin.y + pt.y;
  Fl::e_y_root = [[[self window] screen] frame].size.height - Fl::e_y_root;
  fl_dnd_target_window = target;
  int ret = Fl::handle( FL_DND_DRAG, target );
  breakMacEventLoop();
  return ret ? NSDragOperationCopy : NSDragOperationNone;
}
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender 
{
  static char *DragData = NULL;
  Fl_Window *target = [(FLWindow*)[self window] getFl_Window];
  if ( !Fl::handle( FL_DND_RELEASE, target ) ) { 
    breakMacEventLoop();
    return NO;
  }
  NSPasteboard *pboard;
  // NSDragOperation sourceDragMask;
  // sourceDragMask = [sender draggingSourceOperationMask];
  pboard = [sender draggingPasteboard];
  NSPoint pt = [sender draggingLocation];
  Fl::e_x = pt.x;
  Fl::e_y = [self frame].size.height - pt.y;
  Fl::e_x_root = [[self window] frame].origin.x + Fl::e_x;
  Fl::e_y_root = [[self window] frame].origin.y + pt.y;
  Fl::e_y_root = [[[self window] screen] frame].size.height - Fl::e_y_root;
  if (DragData) { free(DragData); DragData = NULL; }
  if ( [[pboard types] containsObject:NSFilenamesPboardType] ) {
    CFArrayRef files = (CFArrayRef)[pboard propertyListForType:NSFilenamesPboardType];
    CFStringRef all = CFStringCreateByCombiningStrings(NULL, files, CFSTR("\n"));
    int l = CFStringGetMaximumSizeForEncoding(CFStringGetLength(all), kCFStringEncodingUTF8);
    DragData = (char *)malloc(l + 1);
    CFStringGetCString(all, DragData, l + 1, kCFStringEncodingUTF8);
    CFRelease(all);
  }
  else if ( [[pboard types] containsObject:NSStringPboardType] ) {
    NSData *data = [pboard dataForType:NSStringPboardType];
    DragData = (char *)malloc([data length] + 1);
    [data getBytes:DragData];
    DragData[[data length]] = 0;
    convert_crlf(DragData, strlen(DragData));
  }
  else {
    breakMacEventLoop();
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
  return YES;
}
- (void)draggingExited:(id < NSDraggingInfo >)sender
{
  if ( fl_dnd_target_window ) {
    Fl::handle( FL_DND_LEAVE, fl_dnd_target_window );
    fl_dnd_target_window = 0;
  }
}
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal
{
  return NSDragOperationGeneric;
}
@end


/*
 * go ahead, create that (sub)window
 * \todo we should make menu windows slightly transparent for the new Mac look
 */
void Fl_X::make(Fl_Window* w)
{
  static int xyPos = 100;
  if ( w->parent() ) {		// create a subwindow
    Fl_Group::current(0);
    Rect wRect;
    wRect.top    = w->y();
    wRect.left   = w->x();
    wRect.bottom = w->y() + w->h(); if (wRect.bottom<=wRect.top) wRect.bottom = wRect.top+1;
    wRect.right  = w->x() + w->w(); if (wRect.right<=wRect.left) wRect.right = wRect.left+1;
    // our subwindow needs this structure to know about its clipping. 
    Fl_X* x = new Fl_X;
    x->other_xid = 0;
    x->region = 0;
    x->subRegion = 0;
    x->cursor = fl_default_cursor;
    x->gc = 0;			// stay 0 for Quickdraw; fill with CGContext for Quartz
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
    fl_show_iconic = 0;
  }
  else {			// create a desktop window
    NSAutoreleasePool *localPool;
    localPool = [[NSAutoreleasePool alloc] init]; 
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
        winstyle |= NSResizableWindowMask;
      }
    } else {
			if (w->resizable()) {
        Fl_Widget *o = w->resizable();
        int minw = o->w(); if (minw > 100) minw = 100;
        int minh = o->h(); if (minh > 100) minh = 100;
        w->size_range(w->w() - o->w() + minw, w->h() - o->h() + minh, 0, 0);
	winstyle |= NSResizableWindowMask;
      } else {
        w->size_range(w->w(), w->h(), w->w(), w->h());
      }
    }
    int xwm = xp, ywm = yp, bt, bx, by;
    
    if (!fake_X_wm(w, xwm, ywm, bt, bx, by)) {
      // menu windows and tooltips
      if (w->modal()||w->override()) {
        winstyle = NSBorderlessWindowMask;
        winlevel = NSMainMenuWindowLevel;
      } else {
        winstyle = NSBorderlessWindowMask;
      }
    } else if (w->modal()) {
      winstyle &= ~(NSResizableWindowMask | NSMiniaturizableWindowMask);
      // winlevel = NSModalPanelWindowLevel;
    }
    else if (w->non_modal()) {
      winlevel = NSFloatingWindowLevel;
    }
    
    if (by+bt) {
      wp += 2*bx;
      hp += 2*by+bt;
    }
    if (!(w->flags() & Fl_Window::FORCE_POSITION)) {
      // use the Carbon functions below for default window positioning
      w->x(xyPos+Fl::x());
      w->y(xyPos+Fl::y());
      xyPos += 25;
      if (xyPos>200) xyPos = 100;
    } else {
      if (!Fl::grab()) {
        xp = xwm; yp = ywm;
        w->x(xp);w->y(yp);
      }
      xp -= bx;
      yp -= by+bt;
    }
    
    if (w->non_modal() && Fl_X::first && !fl_disable_transient_for) {
      // find some other window to be "transient for":
      Fl_Window* w = Fl_X::first->w;
      while (w->parent()) w = w->window(); // todo: this code does not make any sense! (w!=w??)
    }
    
    Rect wRect;
    wRect.top    = w->y();
    wRect.left   = w->x();
    wRect.bottom = w->y() + w->h(); if (wRect.bottom<=wRect.top) wRect.bottom = wRect.top+1;
    wRect.right  = w->x() + w->w(); if (wRect.right<=wRect.left) wRect.right = wRect.left+1;
    
    const char *name = w->label();
    
    Fl_X* x = new Fl_X;
    x->other_xid = 0; // room for doublebuffering image map. On OS X this is only used by overlay windows
    x->region = 0;
    x->subRegion = 0;
    x->cursor = fl_default_cursor;
    x->xidChildren = 0;
    x->xidNext = 0;
    x->gc = 0;
	  
    NSRect srect = [[NSScreen mainScreen] frame];
    NSRect crect;
    crect.origin.x = w->x(); 
    crect.origin.y = srect.size.height - (w->y() + w->h());
    crect.size.width=w->w(); 
    crect.size.height=w->h();
    FLWindow *cw = [[FLWindow alloc] initWithFl_W:w 
				      contentRect:crect  
					styleMask:winstyle  
					  backing:NSBackingStoreBuffered 
					    defer:NO];
    [cw setAcceptsMouseMovedEvents:YES];
    x->xid = cw;
    FLView *myview = [[FLView alloc] init];
    [cw setContentView:myview];
    [cw setLevel:winlevel];
    
    q_set_window_title(cw, name);
    if (!(w->flags() & Fl_Window::FORCE_POSITION)) {
      if (w->modal()) {
        [cw center];
      } else if (w->non_modal()) {
        [cw center];
      } else {
        static NSPoint delta = NSZeroPoint;
        delta = [cw cascadeTopLeftFromPoint:delta];
      }
    }
    x->w = w; w->i = x;
    x->wait_for_expose = 1;
    x->next = Fl_X::first;
    Fl_X::first = x;
    // Install DnD handlers 
    [myview registerForDraggedTypes:[NSArray arrayWithObjects:
                                     NSStringPboardType,  NSFilenamesPboardType, nil]];
    if ( ! Fl_X::first->next ) {	// if this is the first window, we need to bring the application to the front
      ProcessSerialNumber psn;
      OSErr err = GetCurrentProcess( &psn );
      if ( err==noErr ) SetFrontProcess( &psn );
    }
    
    if (w->size_range_set) w->size_range_();
    
    if (winlevel != NSMainMenuWindowLevel) {
      Fl_Tooltip::enter(0);
    }
    [cw makeKeyAndOrderFront:nil];
    Fl::first_window(w);
    [cw setDelegate:mydelegate];
    if (fl_show_iconic) { 
      fl_show_iconic = 0;
      [cw miniaturize:nil];
    } else {
      w->set_visible();
    }
    
    crect = [[cw contentView] frame];
    w->w(crect.size.width);
    w->h(crect.size.height);
    crect = [cw frame];
    w->x(crect.origin.x);
    srect = [[cw screen] frame];
    w->y(srect.size.height - (crect.origin.y + w->h()));
    
    int old_event = Fl::e_number;
    w->handle(Fl::e_number = FL_SHOW);
    Fl::e_number = old_event;
    
    if (w->modal()) { Fl::modal_ = w; fl_fix_focus(); }
    [localPool release];
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
    [(NSWindow*)i->xid setMinSize:minSize];
    [(NSWindow*)i->xid setMaxSize:maxSize];
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
 * set the window title bar
 * \todo make the titlebar icon work!
 */
void Fl_Window::label(const char *name,const char */*iname*/) {
  Fl_Widget::label(name);
  if (shown() || i) {
    q_set_window_title((NSWindow*)i->xid, name);
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
  if (!shown() || !i) {
    Fl_X::make(this);
  } else {
    if ( !parent() ) {
      if ([(NSWindow*)i->xid isMiniaturized]) {
	i->w->redraw();
	[(NSWindow*)i->xid deminiaturize:nil];
      }
      if (!fl_capture) {
	[(NSWindow*)i->xid makeKeyAndOrderFront:nil];
      }
    }
  }
}


/*
 * resize a window
 */
void Fl_Window::resize(int X,int Y,int W,int H) {
  int bx, by, bt;
  if ( ! this->border() ) bt = 0;
  else get_window_frame_sizes(bx, by, bt);
  if (W<=0) W = 1; // OS X does not like zero width windows
  if (H<=0) H = 1;
  int is_a_resize = (W != w() || H != h());
  //  printf("Fl_Window::resize(X=%d, Y=%d, W=%d, H=%d), is_a_resize=%d, resize_from_system=%p, this=%p\n",
  //         X, Y, W, H, is_a_resize, resize_from_system, this);
  if (X != x() || Y != y()) set_flag(FORCE_POSITION);
  else if (!is_a_resize) return;
  if ( (resize_from_system!=this) && (!parent()) && shown()) {
    if (is_a_resize) {
      if (resizable()) {
        if (W<minw) minw = W; // user request for resize takes priority
        if (W>maxw) maxw = W; // over a previously set size_range
        if (H<minh) minh = H;
        if (H>maxh) maxh = H;
        size_range(minw, minh, maxw, maxh);
      } else {
        size_range(W, H, W, H);
      }
      NSRect dim;
      dim.origin.x = X;
      dim.origin.y = [[(NSWindow*)i->xid screen] frame].size.height - (Y + H);
      dim.size.width = W;
      dim.size.height = H + bt;
      [(NSWindow*)i->xid setFrame:dim display:YES];
    } else {
      NSPoint pt; 
      pt.x = X; 
      pt.y = [[(NSWindow*)i->xid screen] frame].size.height - (Y + h());
      [(NSWindow*)i->xid setFrameOrigin:pt];
    }
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
 */
void Fl_Window::make_current() 
{
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
  
  [[NSView focusView] unlockFocus];
  [[(NSWindow*)i->xid contentView]  lockFocus];
  i->gc = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
  fl_gc = i->gc;
  if ( fl_window_region ) XDestroyRegion(fl_window_region);
  if (this->window()) {
    fl_window_region = XRectangleRegion(0,0,w(),h());
  } else {
    fl_window_region = XRectangleRegion(0, 0, w(), h());
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
  // this is the native view context with origin at bottom left
  CGContextSaveGState(fl_gc); 
#if defined(USE_CAIRO)
  if (Fl::cairo_autolink_context()) Fl::cairo_make_current(this); // capture gc changes automatically to update the cairo context adequately
#endif
  fl_clip_region( 0 );
  
#if defined(USE_CAIRO)
  // update the cairo_t context
  if (Fl::cairo_autolink_context()) Fl::cairo_make_current(this);
#endif
}

// helper function to manage the current CGContext fl_gc
extern Fl_Color fl_color_;
extern class Fl_Font_Descriptor *fl_fontsize;
extern void fl_font(class Fl_Font_Descriptor*);
extern void fl_quartz_restore_line_style_();

// FLTK has only one global graphics state. This function copies the FLTK state into the
// current Quartz context
void Fl_X::q_fill_context() {
  if (!fl_gc) return;
  int hgt = 0;
  if (fl_window) {
    hgt = [[(NSWindow*)Fl_X::i(Fl_Window::current_)->xid contentView] frame].size.height;
  } else {
    hgt = CGBitmapContextGetHeight(fl_gc);
  }
  CGContextTranslateCTM(fl_gc, 0.5, hgt-0.5f);
  CGContextScaleCTM(fl_gc, 1.0f, -1.0f); // now 0,0 is top-left point of the context
  fl_font(fl_fontsize);
  fl_color(fl_color_);
  fl_quartz_restore_line_style_();
  if (fl_window) {			// translate to subwindow origin if this is a subwindow context
    Fl_Window *w = Fl_Window::current_;
    while(w && w->window()) {
      CGContextTranslateCTM(fl_gc, w->x(), w->y());
      w = w->window();
    }
  }
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
  CGContextRestoreGState(fl_gc); // matches the CGContextSaveGState of make_current
  fl_gc = 0;
#if defined(USE_CAIRO)
  if (Fl::cairo_autolink_context()) Fl::cairo_make_current((Fl_Window*) 0); // capture gc changes automatically to update the cairo context adequately
#endif
}

void Fl_X::q_begin_image(CGRect &rect, int cx, int cy, int w, int h) {
  CGContextSaveGState(fl_gc);
  CGAffineTransform mx = CGContextGetCTM(fl_gc);
  CGRect r2 = rect;
  r2.origin.x -= 0.5f;
  r2.origin.y -= 0.5f;
  CGContextClipToRect(fl_gc, r2);
  mx.d = -1.0; mx.tx = -mx.tx;
  CGContextConcatCTM(fl_gc, mx);
  rect.origin.x = -(mx.tx+0.5f) + rect.origin.x     - cx;
  rect.origin.y =  (mx.ty+0.5f) - rect.origin.y - h + cy;
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
const CFStringRef	flavorNames[] = {
  CFSTR("public.utf16-plain-text"), 
  CFSTR("public.utf8-plain-text"),
  CFSTR("com.apple.traditional-mac-plain-text") };
const CFStringEncoding encodings[] = { 
  kCFStringEncodingUnicode, 
  kCFStringEncodingUTF8, 
  kCFStringEncodingMacRoman};
const size_t handledFlavorsCount = sizeof(encodings)/sizeof(CFStringEncoding);

// clipboard variables definitions :
Fl_Widget *fl_selection_requestor = 0;
char *fl_selection_buffer[2];
int fl_selection_length[2];
static int fl_selection_buffer_length[2];

static PasteboardRef myPasteboard = 0;
static void allocatePasteboard() {
  if (!myPasteboard)
    PasteboardCreate(kPasteboardClipboard, &myPasteboard);
}


/*
 * create a selection
 * owner: widget that created the selection
 * stuff: pointer to selected data
 * size of selected data
 */
void Fl::copy(const char *stuff, int len, int clipboard) {
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
    allocatePasteboard();
    OSStatus err = PasteboardClear(myPasteboard);
    if (err!=noErr) return; // clear did not work, maybe not owner of clipboard.
    PasteboardSynchronize(myPasteboard);
    CFDataRef text = CFDataCreate(kCFAllocatorDefault, (UInt8*)fl_selection_buffer[1], len);
    if (text==NULL) return; // there was a pb creating the object, abort.
    err=PasteboardPutItemFlavor(myPasteboard, (PasteboardItemID)1, CFSTR("public.utf8-plain-text"), text, 0);
    CFRelease(text);
  }
}

// Call this when a "paste" operation happens:
void Fl::paste(Fl_Widget &receiver, int clipboard) {
  if (clipboard) {
    // see if we own the selection, if not go get it:
    fl_selection_length[1] = 0;
    OSStatus err = noErr;
    Boolean found = false;
    CFDataRef flavorData = NULL;
    CFStringEncoding encoding = 0;
    
    allocatePasteboard();
    PasteboardSynchronize(myPasteboard);
    ItemCount nFlavor = 0, i, j;
    err = PasteboardGetItemCount(myPasteboard, &nFlavor);
    if (err==noErr) {
      for (i=1; i<=nFlavor; i++) {
        PasteboardItemID itemID = 0;
        CFArrayRef flavorTypeArray = NULL;
        found = false;
        err = PasteboardGetItemIdentifier(myPasteboard, i, &itemID);
        if (err!=noErr) continue;
        err = PasteboardCopyItemFlavors(myPasteboard, itemID, &flavorTypeArray);
        if (err!=noErr) {
          if (flavorTypeArray) {CFRelease(flavorTypeArray); flavorTypeArray = NULL;}
          continue;
        }
        CFIndex flavorCount = CFArrayGetCount(flavorTypeArray);
        for (j = 0; j < handledFlavorsCount; j++) {
          for (CFIndex flavorIndex=0; flavorIndex<flavorCount; flavorIndex++) {
            CFStringRef flavorType = (CFStringRef)CFArrayGetValueAtIndex(flavorTypeArray, flavorIndex);
            if (UTTypeConformsTo(flavorType, flavorNames[j])) {
              err = PasteboardCopyItemFlavorData( myPasteboard, itemID, flavorNames[j], &flavorData );
              if (err != noErr) continue;
              encoding = encodings[j];
              found = true;
              break;
            }
          }
          if (found) break;
        }
        if (flavorTypeArray) {CFRelease(flavorTypeArray); flavorTypeArray = NULL;}
        if (found) break;
      }
      if (found) {
        CFIndex len = CFDataGetLength(flavorData);
        CFStringRef mycfs = CFStringCreateWithBytes(NULL, CFDataGetBytePtr(flavorData), len, encoding, false);
        CFRelease(flavorData);
        len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(mycfs), kCFStringEncodingUTF8) + 1;
        if ( len >= fl_selection_buffer_length[1] ) {
          fl_selection_buffer_length[1] = len;
          delete[] fl_selection_buffer[1];
          fl_selection_buffer[1] = new char[len];
        }
        CFStringGetCString(mycfs, fl_selection_buffer[1], len, kCFStringEncodingUTF8);
        CFRelease(mycfs);
        len = strlen(fl_selection_buffer[1]);
        fl_selection_length[1] = len;
        convert_crlf(fl_selection_buffer[1],len); // turn all \r characters into \n:
      }
    }
  }
  Fl::e_text = fl_selection_buffer[clipboard];
  Fl::e_length = fl_selection_length[clipboard];
  if (!Fl::e_text) Fl::e_text = (char *)"";
  receiver.handle(FL_PASTE);
}

void Fl::add_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
  // check, if this timer slot exists already
  for (int i = 0; i < mac_timer_used; ++i) {
    MacTimeout& t = mac_timers[i];
    // if so, simply change the fire interval
    if (t.callback == cb  &&  t.data == data) {
      SetEventLoopTimerNextFireTime(t.timer, (EventTimerInterval)time);
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
  EventTimerInterval fireDelay = (EventTimerInterval)time;
  EventLoopTimerUPP  timerUPP = NewEventLoopTimerUPP(do_timer);
  EventLoopTimerRef  timerRef = 0;
  OSStatus err = InstallEventLoopTimer(GetMainEventLoop(), fireDelay, 0, timerUPP, data, &timerRef);
  if (err == noErr) {
    t.callback = cb;
    t.data     = data;
    t.timer    = timerRef;
    t.upp      = timerUPP;
    t.pending  = 1;
  } else {
    if (timerRef) 
      RemoveEventLoopTimer(timerRef);
    if (timerUPP)
      DisposeEventLoopTimerUPP(timerUPP);
  }
}

void Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
  // currently, repeat_timeout does not subtract the trigger time of the previous timer event as it should.
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
  breakMacEventLoop();
}

int MacUnlinkWindow(Fl_X *ip, Fl_X *start) {
  if (!ip) return 0;
  if (start) {
    Fl_X *pc = start;
    while (pc) {
      if (pc->xidNext == ip) {
        pc->xidNext = ip->xidNext;
        return 1;
      }
      if (pc->xidChildren) {
        if (pc->xidChildren == ip) {
          pc->xidChildren = ip->xidNext;
          return 1;
        }
        if (MacUnlinkWindow(ip, pc->xidChildren))
          return 1;
      }
      pc = pc->xidNext;
    }
  } else {
    for ( Fl_X *pc = Fl_X::first; pc; pc = pc->next ) {
      if (MacUnlinkWindow(ip, pc))
        return 1;
    }
  }  
  return 0;
}

static void MacRelinkWindow(Fl_X *x, Fl_X *p) {
  if (!x || !p) return;
  // first, check if 'x' is already registered as a child of 'p'
  for (Fl_X *i = p->xidChildren; i; i=i->xidNext) {
    if (i == x) return;
  }
  // now add 'x' as the first child of 'p'
  x->xidNext = p->xidChildren;
  p->xidChildren = x;
}

void MacDestroyWindow(Fl_Window *w, void *p) {
  MacUnmapWindow(w, p);
  if (w && !w->parent() && p) {
    [[(NSWindow *)p contentView] release];
    [(NSWindow *)p close];
  }
}

void MacMapWindow(Fl_Window *w, void *p) {
  if (w && p) {
    [(NSWindow *)p orderFront:nil];
  }
  //+ link to window list
  if (w && w->parent()) {
    MacRelinkWindow(Fl_X::i(w), Fl_X::i(w->window()));
    w->redraw();
  }
}

void MacUnmapWindow(Fl_Window *w, void *p) {
  if (w && !w->parent() && p) {
    [(NSWindow *)p orderOut:nil];
  }
  if (w && Fl_X::i(w)) 
    MacUnlinkWindow(Fl_X::i(w));
}

static Fl_Region MacRegionMinusRect(Fl_Region r, int x,int y,int w,int h)
/* removes x,y,w,h rectangle from region r and returns result as a new Fl_Region
 */
{
  Fl_Region outr = (Fl_Region)malloc(sizeof(*outr));
  outr->rects = (CGRect*)malloc(4 * r->count * sizeof(CGRect));
  outr->count = 0;
  CGRect rect = FL_CGRECTMAKE_COCOA(x, y, w, h);
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

Fl_Region MacRectRegionIntersect(Fl_Region current, int x,int y,int w, int h)
/* intersects current and x,y,w,h rectangle and returns result as a new Fl_Region
 */
{
  if (current == NULL) return XRectangleRegion(x,y,w,h);
  CGRect r = FL_CGRECTMAKE_COCOA(x, y, w, h);
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

void MacCollapseWindow(Window w)
{
  [(NSWindow*)w miniaturize:nil];
}

static NSImage *CGBitmapContextToNSImage(CGContextRef c)
// the returned NSImage is autoreleased
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
  NSImage* image = [[NSImage alloc] initWithData: [imagerep TIFFRepresentation]];
  [imagerep release];
  return [image autorelease];
}

static NSCursor *PrepareCursor(NSCursor *cursor, CGContextRef (*f)() )
{
  if (cursor == nil) {
    CGContextRef c = f();
    NSImage *image = CGBitmapContextToNSImage(c);
    fl_delete_offscreen( (Fl_Offscreen)c ); 
    NSPoint pt = {[image size].width/2, [image size].height/2};
    cursor = [[NSCursor alloc] initWithImage:image hotSpot:pt];
  }
  return cursor;
}

void *MACSetCursor(Fl_Cursor c)
{
  NSCursor *icrsr;
  switch (c) {
    case FL_CURSOR_CROSS:  icrsr = [NSCursor crosshairCursor]; break;
    case FL_CURSOR_WAIT:
      static NSCursor *watch = nil;
      watch = PrepareCursor(watch,  CreateWatchImage);
      icrsr = watch;
      break;
    case FL_CURSOR_INSERT: icrsr = [NSCursor IBeamCursor]; break;
    case FL_CURSOR_N:      icrsr = [NSCursor resizeUpCursor]; break;
    case FL_CURSOR_S:      icrsr = [NSCursor resizeDownCursor]; break;
    case FL_CURSOR_NS:     icrsr = [NSCursor resizeUpDownCursor]; break;
    case FL_CURSOR_HELP:   
      static NSCursor *help = nil;
      help = PrepareCursor(help,  CreateHelpImage);
      icrsr = help;
      break;
    case FL_CURSOR_HAND:   icrsr = [NSCursor pointingHandCursor]; break;
    case FL_CURSOR_MOVE:   icrsr = [NSCursor openHandCursor]; break;
    case FL_CURSOR_NE:
    case FL_CURSOR_SW:
    case FL_CURSOR_NESW:   
      static NSCursor *nesw = nil;
      nesw = PrepareCursor(nesw,  CreateNESWImage);
      icrsr = nesw;
      break;
    case FL_CURSOR_E:      icrsr = [NSCursor resizeRightCursor]; break;
    case FL_CURSOR_W:      icrsr = [NSCursor resizeLeftCursor]; break;
    case FL_CURSOR_WE:     icrsr = [NSCursor resizeLeftRightCursor]; break;
    case FL_CURSOR_SE:
    case FL_CURSOR_NW:
    case FL_CURSOR_NWSE:   
      static NSCursor *nwse = nil;
      nwse = PrepareCursor(nwse,  CreateNWSEImage);
      icrsr = nwse;
      break;
    case FL_CURSOR_NONE:   
      static NSCursor *none = nil;
      none = PrepareCursor(none,  CreateNoneImage);
      icrsr = none; 
      break;
    case FL_CURSOR_ARROW:
    case FL_CURSOR_DEFAULT:
    default:			   icrsr = [NSCursor arrowCursor];
      break;
  }
  [icrsr set];
  return icrsr;
}

int MACscreen_init(XRectangle screens[])
{
  NSAutoreleasePool *localPool;
  localPool = [[NSAutoreleasePool alloc] init]; 
  NSArray *a = [NSScreen screens]; 
  int count = (int)[a count]; 
  NSRect r; 
  int i, num_screens = 0;
  for( i = 0; i < count; i++) {
    r = [[a objectAtIndex:i] frame];
    screens[num_screens].x      = r.origin.x;
    screens[num_screens].y      = r.size.height - (r.origin.y + r.size.height);
    screens[num_screens].width  = r.size.width;
    screens[num_screens].height = r.size.height;
    num_screens ++;
    if (num_screens >= 16) break;
  }
  [localPool release];
  return num_screens;
}

@interface FLaboutItemTarget : NSObject 
{
}
- (void)showPanel;
@end
@implementation FLaboutItemTarget
- (void)showPanel
{
    NSDictionary *options;
    options = [NSDictionary dictionaryWithObjectsAndKeys:
                	     [NSString stringWithFormat:@" FLTK %d.%d Cocoa", FL_MAJOR_VERSION,
                              FL_MINOR_VERSION ], @"Copyright",
                	     nil];
    [NSApp  orderFrontStandardAboutPanelWithOptions:options];
  }
@end

static NSMenu *appleMenu;
static void createAppleMenu(void)
{
  static BOOL donethat = NO;
  if (donethat) return;
  donethat = YES;
  NSMenu *mainmenu, *services;
  NSMenuItem *menuItem;
  NSString *title;
  CFStringRef nsappname;
  
  ProcessSerialNumber psn;
  GetCurrentProcess(&psn);
  CopyProcessName(&psn, &nsappname);
  appleMenu = [[NSMenu alloc] initWithTitle:@""];
  /* Add menu items */
  title = [@"About " stringByAppendingString:(NSString*)nsappname];
  [appleMenu addItemWithTitle:title action:@selector(showPanel) keyEquivalent:@""];
  FLaboutItemTarget *about = [[FLaboutItemTarget alloc] init];
  [[appleMenu itemAtIndex:0] setTarget:about];
  [appleMenu addItem:[NSMenuItem separatorItem]];
  // Services Menu
  services = [[NSMenu alloc] init];
  [appleMenu addItemWithTitle:@"Services" action:nil keyEquivalent:@""];
  [appleMenu setSubmenu: services forItem: [appleMenu itemWithTitle: @"Services"]];
  // Hide AppName
  title = [@"Hide " stringByAppendingString:(NSString*)nsappname];
  [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];
  // Hide Others
  menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) 
                                         keyEquivalent:@"h"];
  [menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];
  // Show All
  [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
  [appleMenu addItem:[NSMenuItem separatorItem]];
  // Quit AppName
  title = [@"Quit " stringByAppendingString:(NSString*)nsappname];
  [appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];
  /* Put menu into the menubar */
  menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
  [menuItem setSubmenu:appleMenu];
  mainmenu = [[NSMenu alloc] initWithTitle:@""];
  [mainmenu addItem:menuItem];
  if (MACsystemVersion < 0x1060) {
    //	[NSApp setAppleMenu:appleMenu];
    //	to avoid compiler warning raised by use of undocumented setAppleMenu	:
  [NSApp performSelector:@selector(setAppleMenu:) withObject:appleMenu];
  }
  [NSApp setServicesMenu:services];
  [NSApp setMainMenu:mainmenu];
  CFRelease(nsappname);
  [services release];
  [mainmenu release];
  [appleMenu release];
  [menuItem release];
  fl_system_menu = [NSApp mainMenu];
}

@interface FLMenuItem : NSMenuItem {
}
- (void) doCallback:(id)unused;
- (void) directCallback:(id)unused;
@end
@implementation FLMenuItem
- (void) doCallback:(id)unused
{
  int flRank = [self tag];
  const Fl_Menu_Item *items = fl_sys_menu_bar->Fl_Menu_::menu();
  const Fl_Menu_Item *item = items + flRank;
  if (item) {
    fl_sys_menu_bar->picked(item);
    if ( item->flags & FL_MENU_TOGGLE ) {	// update the menu toggle symbol
      [self setState:(item->value() ? NSOnState : NSOffState)];
    }
    else if ( item->flags & FL_MENU_RADIO ) {	// update the menu radio symbols
      int from = flRank;
      while( from > 0 && items[from - 1].label() && (items[from - 1].flags & FL_MENU_RADIO) &&
            !(items[from - 1].flags & FL_MENU_DIVIDER) ) {
        from--;
      }
      int to = flRank;
      while( !(items[to].flags & FL_MENU_DIVIDER) && items[to + 1].label() && 
            (items[to + 1].flags & FL_MENU_RADIO) ) {
        to++;
      }
      NSMenu *nsmenu = [self menu];
      int nsrank = (int)[nsmenu indexOfItem:self];
      for(int i =  from - flRank + nsrank ; i <= to - flRank + nsrank; i++) {
        NSMenuItem *nsitem = [nsmenu itemAtIndex:i];
        if (nsitem != self) [nsitem setState:NSOffState];
        else [nsitem setState:(item->value() ? NSOnState : NSOffState) ];
      }
    }
  }
}
- (void) directCallback:(id)unused
{
  Fl_Menu_Item *item = (Fl_Menu_Item *)[(NSData*)[self representedObject] bytes];
  if ( item && item->callback() ) item->do_callback(NULL);
}
@end


/** 
 * Mac OS: attaches a callback to the "About myprog" item of the system application menu.
 * \note  #include <FL/x.H>
 *
 * \author Manolo Gouy
 *
 * \param[in] cb   a callback that will be called by "About myprog" menu item
 *		   with NULL 1st argument.
 * \param[in] user_data   a pointer transmitted as 2nd argument to the callback.
 * \param[in] shortcut    optional shortcut to attach to the "About myprog" menu item (e.g., FL_META+'a')
 */
void fl_mac_set_about( Fl_Callback *cb, void *user_data, int shortcut) 
{
  NSAutoreleasePool *localPool;
  localPool = [[NSAutoreleasePool alloc] init]; 
  fl_open_display();
  Fl_Menu_Item aboutItem;
  memset(&aboutItem, 0, sizeof(Fl_Menu_Item));
  aboutItem.callback(cb);
  aboutItem.user_data(user_data);
  aboutItem.shortcut(shortcut);
  CFStringRef cfname = CFStringCreateCopy(NULL, (CFStringRef)[[appleMenu itemAtIndex:0] title]);
  [appleMenu removeItemAtIndex:0];
  FLMenuItem *item = [[[FLMenuItem alloc] initWithTitle:(NSString*)cfname 
						 action:@selector(directCallback:) 
					  keyEquivalent:@""] autorelease];
  if (aboutItem.shortcut()) {
    Fl_Sys_Menu_Bar::doMenuOrItemOperation(Fl_Sys_Menu_Bar::setKeyEquivalent, item, aboutItem.shortcut() & 0xff);
    Fl_Sys_Menu_Bar::doMenuOrItemOperation(Fl_Sys_Menu_Bar::setKeyEquivalentModifierMask, item, aboutItem.shortcut() );
  }
  NSData *pointer = [NSData dataWithBytes:&aboutItem length:sizeof(Fl_Menu_Item)];
  [item setRepresentedObject:pointer];
  [appleMenu insertItem:item atIndex:0];
  CFRelease(cfname);
  [item setTarget:item];
  [localPool release];
}

static char *remove_ampersand(const char *s)
{
  char *ret = strdup(s);
  const char *p = s;
  char *q = ret;
  while(*p != 0) {
    if (p[0]=='&') {
      if (p[1]=='&') {
        *q++ = '&'; p+=2;
      } else {
        p++;
      }
    } else {
      *q++ = *p++;
    }
  }
  *q = 0;
  return ret;
}

void *Fl_Sys_Menu_Bar::doMenuOrItemOperation(Fl_Sys_Menu_Bar::menuOrItemOperation operation, ...)
/* these operations apply to menus, submenus, or menu items
 */
{
  NSAutoreleasePool *localPool;
  localPool = [[NSAutoreleasePool alloc] init]; 
  NSMenu *menu;
  NSMenuItem *item;
  int value;
  void *pter;
  void *retval = NULL;
  va_list ap;
  va_start(ap, operation);
  
  if (operation == Fl_Sys_Menu_Bar::itemAtIndex) {	// arguments: NSMenu*, int. Returns the item
    menu = va_arg(ap, NSMenu*);
    value = va_arg(ap, int);
    retval = (void *)[menu itemAtIndex:value];
  }
  else if (operation == Fl_Sys_Menu_Bar::setKeyEquivalent) {	// arguments: NSMenuItem*, int
    item = va_arg(ap, NSMenuItem*);
    value = va_arg(ap, int);
    char key = value;
    NSString *equiv = [[NSString alloc] initWithBytes:&key length:1 encoding:NSASCIIStringEncoding];
    [item setKeyEquivalent:equiv];
    [equiv release];
  }
  else if (operation == Fl_Sys_Menu_Bar::setKeyEquivalentModifierMask) {		// arguments: NSMenuItem*, int
    item = va_arg(ap, NSMenuItem*);
    value = va_arg(ap, int);
    NSUInteger macMod = 0;
    if ( value & FL_META ) macMod = NSCommandKeyMask;
    if ( value & FL_SHIFT || isupper(value) ) macMod |= NSShiftKeyMask;
    if ( value & FL_ALT ) macMod |= NSAlternateKeyMask;
    if ( value & FL_CTRL ) macMod |= NSControlKeyMask;
    [item setKeyEquivalentModifierMask:macMod];
  }
  else if (operation == Fl_Sys_Menu_Bar::setState) {	// arguments: NSMenuItem*, int
    item = va_arg(ap, NSMenuItem*);
    value = va_arg(ap, int);
    [item setState:(value ? NSOnState : NSOffState)];
  }
  else if (operation == Fl_Sys_Menu_Bar::initWithTitle) {	// arguments: const char*title. Returns the newly created menu
                                                                // creates a new (sub)menu
    char *ts = remove_ampersand(va_arg(ap, char *));
    CFStringRef title = CFStringCreateWithCString(NULL, ts, kCFStringEncodingUTF8);
    free(ts);
    NSMenu *menu = [[NSMenu alloc] initWithTitle:(NSString*)title];
    CFRelease(title);
    [menu setAutoenablesItems:NO];
    retval = (void *)menu;
  }
  else if (operation == Fl_Sys_Menu_Bar::numberOfItems) {	// arguments: NSMenu *menu, int *pcount
                                                                // upon return, *pcount is set to menu's item count
    menu = va_arg(ap, NSMenu*);
    pter = va_arg(ap, void *);
    *(int*)pter = [menu numberOfItems];
  }
  else if (operation == Fl_Sys_Menu_Bar::setSubmenu) {		// arguments: NSMenuItem *item, NSMenu *menu
                                                        	// sets 'menu' as submenu attached to 'item'
    item = va_arg(ap, NSMenuItem*);
    menu = va_arg(ap, NSMenu*);
    [item setSubmenu:menu];
    [menu release];
  }
  else if (operation == Fl_Sys_Menu_Bar::setEnabled) {		// arguments: NSMenuItem*, int
    item = va_arg(ap, NSMenuItem*);
    value = va_arg(ap, int);
    [item setEnabled:(value ? YES : NO)];
  }
  else if (operation == Fl_Sys_Menu_Bar::addSeparatorItem) {	// arguments: NSMenu*
    menu = va_arg(ap, NSMenu*);
    [menu addItem:[NSMenuItem separatorItem]];
  }
  else if (operation == Fl_Sys_Menu_Bar::setTitle) {		// arguments: NSMenuItem*, const char *
    item = va_arg(ap, NSMenuItem*);
    char *ts = remove_ampersand(va_arg(ap, char *));
    CFStringRef title = CFStringCreateWithCString(NULL, ts, kCFStringEncodingUTF8);
    free(ts);
    [item setTitle:(NSString*)title];
    CFRelease(title);
  }
  else if (operation == Fl_Sys_Menu_Bar::removeItem) {		// arguments: NSMenu*, int
    menu = va_arg(ap, NSMenu*);
    value = va_arg(ap, int);
    [menu removeItem:[menu itemAtIndex:value]];
  }
  else if (operation == Fl_Sys_Menu_Bar::addNewItem) {		// arguments: NSMenu *menu, int flrank, int *prank
    // creates a new menu item at the end of 'menu'
    // attaches the item of rank flrank (counted in Fl_Menu_) of fl_sys_menu_bar to it
    // upon return, puts the rank (counted in NSMenu) of the new item in *prank unless prank is NULL
    menu = va_arg(ap, NSMenu*);
    int flRank = va_arg(ap, int);
    char *name = remove_ampersand( (fl_sys_menu_bar->Fl_Menu_::menu() + flRank)->label());
    int *prank = va_arg(ap, int*);
    CFStringRef cfname = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
    free(name);
    FLMenuItem *item = [[FLMenuItem alloc] initWithTitle:(NSString*)cfname 
						  action:@selector(doCallback:) 
					   keyEquivalent:@""];
    [item setTag:flRank];
    [menu addItem:item];
    CFRelease(cfname);
    [item setTarget:item];
    if (prank != NULL) *prank = [menu indexOfItem:item];
    [item release];
  }
  else if (operation == Fl_Sys_Menu_Bar::renameItem) {		// arguments: int rank, const char *newname
    // renames the system menu item numbered rank in fl_sys_menu_bar->menu()
    int rank = va_arg(ap, int);
    char *newname = remove_ampersand( va_arg(ap, const char *) );
    int countmenus = [(NSMenu*)fl_system_menu numberOfItems];
    bool found = NO;
    NSMenuItem *macitem = 0;
    for(int i = 1; (!found) && i < countmenus; i++) {
      NSMenuItem *item = [(NSMenu*)fl_system_menu itemAtIndex:i];
      NSMenu *submenu = [item submenu];
      if (submenu == nil) continue;
      int countitems = [submenu numberOfItems];
      for(int j = 0; j < countitems; j++) {
	macitem = [submenu itemAtIndex:j];
	if ([macitem tag] == rank) { found = YES; break; }
      }
    }
    if (found) {
      [macitem setTitle:[[[NSString alloc] initWithUTF8String:newname] autorelease]];
    }
    free(newname);
  }
  va_end(ap);
  [localPool release];
  return retval;
}

void MACsetkeywindow(void *nsw)
{
  [(NSWindow*)nsw makeKeyAndOrderFront:nil];
}

static NSImage *imageFromText(const char *text, int *pwidth, int *pheight)
{
  const char *p, *q;
  int width = 0, height, w2, ltext = strlen(text);
  fl_font(FL_HELVETICA, 10);
  p = text;
  int nl = 0;
  while((q=strchr(p, '\n')) != NULL) { 
    nl++; 
    w2 = fl_width(p, q - p);
    if (w2 > width) width = w2;
    p = q + 1; 
  }
  if (text[ ltext - 1] != '\n') {
    nl++;
    w2 = fl_width(p);
    if (w2 > width) width = w2;
  }
  height = nl * fl_height() + 3;
  width += 6;
  Fl_Offscreen off = fl_create_offscreen_with_alpha(width, height);
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
  const int width = 16, height = 16;
  Fl_Offscreen off = fl_create_offscreen_with_alpha(width, height);
  fl_begin_offscreen(off);
  CGContextSetRGBFillColor( (CGContextRef)off, 0,0,0,0);
  fl_rectf(0,0,width,height);
  CGContextSetRGBStrokeColor( (CGContextRef)off, 0,0,0,0.6);
  fl_rect(0,0,width,height);
  fl_rect(2,2,width-4,height-4);
  fl_end_offscreen();
  NSImage* image = CGBitmapContextToNSImage( (CGContextRef)off );
  fl_delete_offscreen( off );
  *pwidth = width;
  *pheight = height;
  return image;
}

int MACpreparedrag(void)
{
  CFDataRef text = CFDataCreate(kCFAllocatorDefault, (UInt8*)fl_selection_buffer[0], fl_selection_length[0]);
  if (text==NULL) return false;
  NSAutoreleasePool *localPool;
  localPool = [[NSAutoreleasePool alloc] init]; 
  NSPasteboard *mypasteboard = [NSPasteboard pasteboardWithName:NSDragPboard];
  [mypasteboard declareTypes:[NSArray arrayWithObjects:@"public.utf8-plain-text", nil] owner:nil];
  [mypasteboard setData:(NSData*)text forType:@"public.utf8-plain-text"];
  CFRelease(text);
  Fl_Widget *w = Fl::pushed();
  Fl_Window *win = w->window();
  if (win == NULL) {
    win = (Fl_Window*)w;
  } else { 
    while(win->window()) win = win->window();
  }
  NSView *myview = [(NSWindow*)Fl_X::i(win)->xid contentView];
  NSEvent *theEvent = [NSApp currentEvent];
  
  int width, height;
  NSImage *image;
  if ( dynamic_cast<Fl_Input_*>(w) != NULL) {
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

unsigned char *MACbitmapFromRectOfWindow(Fl_Window *win, int x, int y, int w, int h, int *bytesPerPixel)
// delete the returned pointer after use
{
  while(win->window()) {
    x += win->x();
    y += win->y();
    win = win->window();
  }
  CGFloat epsilon = 0;
  if (MACsystemVersion >= 0x1060) epsilon = 0.001;
  // The epsilon offset is absolutely necessary under 10.6. Without it, the top pixel row and
  // left pixel column are not read, and bitmap is read shifted by one pixel in both directions. 
  // Under 10.5, we want no offset.
  NSRect rect = NSMakeRect(x - epsilon, y - epsilon, w, h);
  NSBitmapImageRep *bitmap = [[NSBitmapImageRep alloc] initWithFocusedViewRect:rect];
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
  [bitmap release];
  return data;
}

void imgProviderReleaseData (void *info, const void *data, size_t size)
{
  delete (unsigned char *)data;
}

CGImageRef MAC_CGImageFromRectOfWindow(Fl_Window *win, int x, int y, int w, int h)
// CFRelease the returned CGImageRef after use
{
  int bpp;
  unsigned char *bitmap = MACbitmapFromRectOfWindow(win, x, y, w, h, &bpp);
  CGImageRef img;
  CGColorSpaceRef lut = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, bitmap, w*h*bpp, imgProviderReleaseData);
  img = CGImageCreate(w, h, 8, 8*bpp, w*bpp, lut,
                      bpp == 3 ? kCGImageAlphaNone : kCGImageAlphaLast,
                      provider, NULL, false, kCGRenderingIntentDefault);
  CGColorSpaceRelease(lut);
  CGDataProviderRelease(provider);
  return img;
}

void MACsetContainsGLsubwindow(Fl_Window *w) 
{
  [(FLWindow*)Fl_X::i(w)->xid setContainsGLsubwindow:YES];
}

WindowRef MACwindowRef(Fl_Window *w)
{
  return (WindowRef)[(FLWindow*)Fl_X::i(w)->xid windowRef];
}
#endif // FL_DOXYGEN

//
// End of "$Id: Fl_cocoa.mm 6971 2009-04-13 07:32:01Z matt $".
//
