//
// "$Id: Fl_mac.cxx,v 1.1.2.39 2003/05/20 17:53:26 easysw Exp $"
//
// MacOS specific code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2003 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

//// From the inner edge of a MetroWerks CodeWarrior CD:
// (without permission)
//
// Three Compiles for 68Ks under the sky,
// Seven Compiles for PPCs in their fragments of code,
// Nine Compiles for Mortal Carbon doomed to die,
// One Compile for Mach-O Cocoa on its Mach-O throne,
// in the Land of MacOS X where the Drop-Shadows lie.
// 
// One Compile to link them all, One Compile to merge them,
// One Compile to copy them all and in the bundle bind them,
// in the Land of MacOS X where the Drop-Shadows lie.


// we don't need the following definition because we deliver only
// true mouse moves.  On very slow systems however, this flag may
// still be useful.
#define CONSOLIDATE_MOTION 0

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <stdio.h>
#include <stdlib.h>
#include "flstring.h"
#include <unistd.h>
#include <pthread.h>

// #define DEBUG_SELECT		// UNCOMMENT FOR SELECT()/THREAD DEBUGGING
#ifdef DEBUG_SELECT
#include <stdio.h>	// testing
#define DEBUGMSG(msg)		fprintf(stderr, msg);
#define DEBUGPERRORMSG(msg)	perror(msg)
#else
#define DEBUGMSG(msg)
#define DEBUGPERRORMSG(msg)
#endif /*DEBUGSELECT*/

// external functions
extern Fl_Window* fl_find(Window);
extern void fl_fix_focus();

// forward definition of functions in this file
static void handleUpdateEvent( WindowPtr xid );
//+ int fl_handle(const EventRecord &event);

// public variables
int fl_screen;
Handle fl_system_menu;
Fl_Sys_Menu_Bar *fl_sys_menu_bar = 0;
CursHandle fl_default_cursor;
WindowRef fl_capture = 0;            // we need this to compensate for a missing(?) mouse capture
ulong fl_event_time;                 // the last timestamp from an x event
char fl_key_vector[32];              // used by Fl::get_key()
bool fl_show_iconic;                 // true if called from iconize() - shows the next created window in collapsed state
int fl_disable_transient_for;        // secret method of removing TRANSIENT_FOR
const Fl_Window* fl_modal_for;       // parent of modal() window
Fl_Region fl_window_region = 0;
Window fl_window;
Fl_Window *Fl_Window::current_;
EventRef fl_os_event;		// last (mouse) event

// forward declarations of variables in this file
static int got_events = 0;
static Fl_Window* resize_from_system;
static CursPtr default_cursor_ptr;
static Cursor default_cursor;
static WindowRef fl_os_capture = 0; // the dispatch handler will redirect mose move and drag events to these windows

#if CONSOLIDATE_MOTION
static Fl_Window* send_motion;
extern Fl_Window* fl_xmousewin;
#endif

enum { kEventClassFLTK = 'fltk' };
enum { kEventFLTKBreakLoop = 1, kEventFLTKDataReady };

/**
* Mac keyboard lookup table
 */
static unsigned short macKeyLookUp[128] =
{
    'a', 's', 'd', 'f', 'h', 'g', 'z', 'x',
    'c', 'v', 0/*ISO extra ('#' on German keyboard)*/, 'b', 'q', 'w', 'e', 'r',

    'y', 't', '1', '2', '3', '4', '6', '5',
    '=', '9', '7', '-', '8', '0', ']', 'o',

    'u', '[', 'i', 'p', FL_Enter, 'l', 'j', '\'',
    'k', ';', '\\', ',', '/', 'n', 'm', '.',

    FL_Tab, ' ', '`', FL_BackSpace, 0, FL_Escape, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    0, FL_KP+'.', FL_Right, FL_KP+'*', 0, FL_KP+'+', FL_Left, FL_Num_Lock,
    FL_Down, 0, 0, FL_KP+'/', FL_KP_Enter, FL_Up, FL_KP+'-', 0,

    0, FL_KP+'=', FL_KP+'0', FL_KP+'1', FL_KP+'2', FL_KP+'3', FL_KP+'4', FL_KP+'5',
    FL_KP+'6', FL_KP+'7', 0, FL_KP+'8', FL_KP+'9', 0, 0, 0,

    FL_F+5, FL_F+6, FL_F+7, FL_F+3, FL_F+8, FL_F+9, 0, FL_F+11,
    0, 0, FL_Print, FL_Scroll_Lock, 0, FL_F+10, 0, FL_F+12,

    0, FL_Pause, FL_Help, FL_Home, FL_Page_Up, FL_Delete, FL_F+4, FL_End,
    FL_F+2, FL_Page_Down, FL_F+1, FL_Left, FL_Right, FL_Down, FL_Up, 0,
};


// these pointers are set by the Fl::lock() function:
static void nothing() {}
void (*fl_lock_function)() = nothing;
void (*fl_unlock_function)() = nothing;


//
// Select interface
//
#define POLLIN  1
#define POLLOUT 4
#define POLLERR 8
struct FD
{
  int fd;
  short events;
  void (*cb)(int, void*);
  void* arg;
};
static int nfds = 0;
static int fd_array_size = 0;
static FD *fd = 0;
static int G_pipe[2] = { 0,0 };		// work around pthread_cancel() problem
enum GetSet { GET = 1, SET = 2 };
static pthread_mutex_t select_mutex;	// global data lock

// MAXFD ACCESSOR (HANDLES LOCKING)
static void MaxFD(GetSet which, int& val)
{
  static int maxfd = 0;
  pthread_mutex_lock(&select_mutex);
  if ( which == GET ) { val = maxfd; }
  else                { maxfd = val; }
  pthread_mutex_unlock(&select_mutex);
}

// FDSET ACCESSOR (HANDLES LOCKING)
static void Fdset(GetSet which, fd_set& r, fd_set &w, fd_set &x)
{
  static fd_set fdsets[3];
  pthread_mutex_lock(&select_mutex);
  if ( which == GET ) { r = fdsets[0]; w = fdsets[1]; x = fdsets[2]; }
  else                { fdsets[0] = r; fdsets[1] = w; fdsets[2] = x; }
  pthread_mutex_unlock(&select_mutex);
}

void Fl::add_fd( int n, int events, void (*cb)(int, void*), void *v )
{
  remove_fd(n, events);

  int i = nfds++;

  if (i >= fd_array_size) {
    FD *temp;
    fd_array_size = 2*fd_array_size+1;

    if (!fd) { temp = (FD*)malloc(fd_array_size*sizeof(FD)); }
    else     { temp = (FD*)realloc(fd, fd_array_size*sizeof(FD)); }

    if (!temp) return;
    fd = temp;
  }

  fd[i].cb  = cb;
  fd[i].arg = v;
  fd[i].fd  = n;
  fd[i].events = events;

  {
    int maxfd;
    fd_set r, w, x;
    MaxFD(GET, maxfd);
    Fdset(GET, r, w, x);
    if (events & POLLIN)  FD_SET(n, &r);
    if (events & POLLOUT) FD_SET(n, &w);
    if (events & POLLERR) FD_SET(n, &x);
    if (n > maxfd) maxfd = n;
    Fdset(SET, r, w, x);
    MaxFD(SET, maxfd);
  }
}

void Fl::add_fd(int fd, void (*cb)(int, void*), void* v)
{
  Fl::add_fd(fd, POLLIN, cb, v);
}

void Fl::remove_fd(int n, int events)
{
  int i,j;

  for (i=j=0; i<nfds; i++) {

    if (fd[i].fd == n) {
      int e = fd[i].events & ~events;
      if (!e) continue; // if no events left, delete this fd
      fd[i].events = e;
    }

    // move it down in the array if necessary:
    if (j<i)
    { fd[j] = fd[i]; }

    j++;
  }

  nfds = j;

  {
    int maxfd;
    fd_set r, w, x;
    MaxFD(GET, maxfd);
    Fdset(GET, r, w, x);
    if (events & POLLIN)  FD_CLR(n, &r);
    if (events & POLLOUT) FD_CLR(n, &w);
    if (events & POLLERR) FD_CLR(n, &x);
    if (n == maxfd) maxfd--;
    Fdset(SET, r, w, x);
    MaxFD(SET, maxfd);
  }
}

void Fl::remove_fd(int n)
{
  remove_fd(n, -1);
}

/**
 * Check if there is actually a message pending!
 */
int fl_ready()
{
  if (GetNumEventsInQueue(GetCurrentEventQueue()) > 0) return 1;
  return 0;
}

// CHECK IF USER DATA READY
static int CheckDataReady(fd_set& r, fd_set& w, fd_set& x)
{
  int maxfd;
  MaxFD(GET, maxfd);
  timeval t = { 0, 1 };		// quick check
  int ret = ::select(maxfd+1, &r, &w, &x, &t);
  if ( ret == -1 )
    { DEBUGPERRORMSG("CheckDataReady(): select()"); }
  return(ret);
}

// HANDLE DATA READY CALLBACKS
static void HandleDataReady(fd_set& r, fd_set& w, fd_set& x)
{
  for (int i=0; i<nfds; i++) 
  {
    // fprintf(stderr, "CHECKING FD %d OF %d (%d)\n", i, nfds, fd[i].fd);
    int f = fd[i].fd;
    short revents = 0;
    if (FD_ISSET(f, &r)) revents |= POLLIN;
    if (FD_ISSET(f, &w)) revents |= POLLOUT;
    if (FD_ISSET(f, &x)) revents |= POLLERR;
    if (fd[i].events & revents) 
    {
      DEBUGMSG("DOING CALLBACK: ");
      fd[i].cb(f, fd[i].arg);
      DEBUGMSG("DONE\n");
    }
  }
}


/**
 * handle Apple Menu items (can be created using the Fl_Sys_Menu_Bar
 * returns eventNotHandledErr if the menu item could not be handled
 */
OSStatus HandleMenu( HICommand *cmd )
{
  OSStatus ret = eventNotHandledErr;
  // attributes, commandIDm menu.menuRef, menu.menuItemIndex
  UInt32 ref;
  OSErr rrc = GetMenuItemRefCon( cmd->menu.menuRef, cmd->menu.menuItemIndex, &ref );
  //printf( "%d, %08x, %08x, %d, %d, %8x\n", rrc, cmd->attributes, cmd->commandID, cmd->menu.menuRef, cmd->menu.menuItemIndex, rrc );
  if ( rrc==noErr && ref )
  {
    Fl_Menu_Item *m = (Fl_Menu_Item*)ref;
    //printf( "Menu: %s\n", m->label() );
    fl_sys_menu_bar->picked( m );
    if ( m->flags & FL_MENU_TOGGLE ) // update the menu toggle symbol
      SetItemMark( cmd->menu.menuRef, cmd->menu.menuItemIndex, (m->flags & FL_MENU_VALUE ) ? 0x12 : 0 );
    if ( m->flags & FL_MENU_RADIO ) // update all radio buttons in this menu
    {
      Fl_Menu_Item *j = m;
      int i = cmd->menu.menuItemIndex;
      for (;;)
      {
        if ( j->flags & FL_MENU_DIVIDER )
          break;
        j++; i++;
        if ( !j->text || !j->radio() )
          break;
        SetItemMark( cmd->menu.menuRef, i, ( j->flags & FL_MENU_VALUE ) ? 0x13 : 0 );
      }
      j = m-1; i = cmd->menu.menuItemIndex-1;
      for ( ; i>0; j--, i-- )
      {
        if ( !j->text || j->flags&FL_MENU_DIVIDER || !j->radio() )
          break;
        SetItemMark( cmd->menu.menuRef, i, ( j->flags & FL_MENU_VALUE ) ? 0x13 : 0 );
      }
      SetItemMark( cmd->menu.menuRef, cmd->menu.menuItemIndex, ( m->flags & FL_MENU_VALUE ) ? 0x13 : 0 );
    }
    ret = noErr; // done handling this event
  }
  HiliteMenu(0);
  return ret;
}


/**
 * We can make every event pass through this function
 * - mouse events need to be manipulated to use a mouse focus window
 * - keyboard, mouse and some window  events need to quit the Apple Event Loop
 *   so FLTK can continue its own management
 */
static pascal OSStatus carbonDispatchHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  OSStatus ret = eventNotHandledErr;
  HICommand cmd;

  fl_lock_function();

  got_events = 1;

  switch ( GetEventClass( event ) )
  {
  case kEventClassMouse:
    switch ( GetEventKind( event ) )
    {
    case kEventMouseUp:
    case kEventMouseMoved:
    case kEventMouseDragged:
      if ( fl_capture )
        ret = SendEventToEventTarget( event, GetWindowEventTarget( fl_capture ) );
      else if ( fl_os_capture )
        ret = SendEventToEventTarget( event, GetWindowEventTarget( fl_os_capture ) );
      break;
    }
    break;
  case kEventClassCommand:
    switch (GetEventKind( event ) )
    {
      case kEventCommandProcess:
        GetEventParameter( event, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &cmd );
        ret = HandleMenu( &cmd );
        break;
    }
    break;
  case kEventClassFLTK:
    switch ( GetEventKind( event ) )
    {
    case kEventFLTKBreakLoop:
      ret = noErr;
      break;
    case kEventFLTKDataReady:
      {
	DEBUGMSG("DATA READY EVENT: RECEIVED\n");

        // CHILD THREAD TELLS US DATA READY
	//     Check to see what's ready, and invoke user's cb's
	//
	fd_set r, w, x;
	Fdset(GET, r, w, x);
	switch ( CheckDataReady(r, w, x) )
	{
	case 0:		// NO DATA
	  break;

	case -1:	// ERROR
	  break;

	default:	// DATA READY
	  HandleDataReady(r, w, x);
	  break;
        }
      }
      ret = noErr;
      break;
    }
  }
  if ( ret == eventNotHandledErr )
    ret = CallNextEventHandler( nextHandler, event ); // let the OS handle the activation, but continue to get a click-through effect
  QuitApplicationEventLoop();

  fl_unlock_function();

  return ret;
}


/**
 * this callback simply quits the main event loop handler, so FLTK can do its magic
 */
static pascal void timerProcCB( EventLoopTimerRef, void* )
{
  fl_lock_function();

  QuitApplicationEventLoop();

  fl_unlock_function();
}


// DATA READY THREAD
//    Separate thread, watches for changes in user's file descriptors.
//    Sends a 'data ready event' to the main thread if any change.
//
static void *dataready_thread(void *userdata)
{
  EventRef drEvent;
  CreateEvent( 0, kEventClassFLTK, kEventFLTKDataReady,
               0, kEventAttributeUserEvent, &drEvent);
  EventQueueRef eventqueue = (EventQueueRef)userdata;

  // Thread safe local copy
  int maxfd;
  fd_set r, w, x;
  MaxFD(GET, maxfd);
  Fdset(GET, r, w, x);

  // TACK ON FD'S FOR 'CANCEL PIPE'
  FD_SET(G_pipe[0], &r);
  if ( G_pipe[0] > maxfd ) maxfd = G_pipe[0];

  // FOREVER UNTIL THREAD CANCEL OR ERROR
  while ( 1 )
  {
    timeval t = { 1000, 0 };	// 1000 seconds;
    int ret = ::select(maxfd+1, &r, &w, &x, &t);
    pthread_testcancel();	// OSX 10.0.4 and under: need to do this
                          // so parent can cancel us :(
    switch ( ret )
    {
      case  0:	// NO DATA
        continue;
      case -1:	// ERROR
      {
        DEBUGPERRORMSG("CHILD THREAD: select() failed");
        return(NULL);		// error? exit thread
      }
      default:	// DATA READY
      {
        DEBUGMSG("DATA READY EVENT: SENDING\n");
        PostEventToQueue(eventqueue, drEvent, kEventPriorityStandard );
        return(NULL);		// done with thread
      }
    }
  }
}


/**
 * break the current event loop
 */
static void breakMacEventLoop()
{
  EventRef breakEvent;

  fl_lock_function();

  CreateEvent( 0, kEventClassFLTK, kEventFLTKBreakLoop, 0, kEventAttributeUserEvent, &breakEvent );
  PostEventToQueue( GetCurrentEventQueue(), breakEvent, kEventPriorityStandard );
  ReleaseEvent( breakEvent );

  fl_unlock_function();
}


/**
 * This function is the central event handler.
 * It reads events from the event queue using the given maximum time
 * Funny enough, it returns the same time that it got as the argument. 
 */
static double do_queued_events( double time = 0.0 ) 
{
  static bool been_here = 0;
  static RgnHandle rgn;
  
  // initialize events and a region that enables mouse move events
  if (!been_here) {
    rgn = NewRgn();
    Point mp;
    GetMouse(&mp);
    SetRectRgn(rgn, mp.h, mp.v, mp.h, mp.v);
    SetEventMask(everyEvent);
    been_here = 1;
  }
  OSStatus ret;
  static EventTargetRef target = 0;
  static EventLoopTimerRef timer = 0;
  if ( !target ) 
  {
    target = GetEventDispatcherTarget();

    EventHandlerUPP dispatchHandler = NewEventHandlerUPP( carbonDispatchHandler ); // will not be disposed by Carbon...
    static EventTypeSpec dispatchEvents[] = {
        { kEventClassWindow, kEventWindowShown },
        { kEventClassWindow, kEventWindowHidden },
        { kEventClassWindow, kEventWindowActivated },
        { kEventClassWindow, kEventWindowDeactivated },
        { kEventClassWindow, kEventWindowClose },
        { kEventClassKeyboard, kEventRawKeyDown },
        { kEventClassKeyboard, kEventRawKeyRepeat },
        { kEventClassKeyboard, kEventRawKeyUp },
        { kEventClassKeyboard, kEventRawKeyModifiersChanged },
        { kEventClassMouse, kEventMouseDown },
        { kEventClassMouse, kEventMouseUp },
        { kEventClassMouse, kEventMouseMoved },
        { kEventClassMouse, kEventMouseWheelMoved },
        { kEventClassMouse, kEventMouseDragged },
        { kEventClassFLTK, kEventFLTKBreakLoop },
        { kEventClassFLTK, kEventFLTKDataReady } };
    ret = InstallEventHandler( target, dispatchHandler, GetEventTypeCount(dispatchEvents), dispatchEvents, 0, 0L );
    static EventTypeSpec appEvents[] = {
        { kEventClassCommand, kEventCommandProcess } };
    ret = InstallApplicationEventHandler( dispatchHandler, GetEventTypeCount(appEvents), appEvents, 0, 0L );
    ret = InstallEventLoopTimer( GetMainEventLoop(), 0, 0, NewEventLoopTimerUPP( timerProcCB ), 0, &timer );
  }

  got_events = 0;

  // START A THREAD TO WATCH FOR DATA READY
  static pthread_t dataready_tid = 0;
  if ( nfds )
  {
    void *userdata = (void*)GetCurrentEventQueue();

    // PREPARE INTER-THREAD DATA
    pthread_mutex_init(&select_mutex, NULL);

    if ( G_pipe[0] ) { close(G_pipe[0]); G_pipe[0] = 0; }
    if ( G_pipe[1] ) { close(G_pipe[1]); G_pipe[1] = 0; }
    pipe(G_pipe);

    DEBUGMSG("*** START THREAD\n");
    pthread_create(&dataready_tid, NULL, dataready_thread, userdata);
  }

  fl_unlock_function();

  SetEventLoopTimerNextFireTime( timer, time );
  RunApplicationEventLoop(); // will return after the previously set time
  if ( dataready_tid != 0 )
  {
      DEBUGMSG("*** CANCEL THREAD: ");
      pthread_cancel(dataready_tid);		// cancel first
      write(G_pipe[1], "x", 1);		// then wakeup thread from select
      pthread_join(dataready_tid, NULL);	// wait for thread to finish
      if ( G_pipe[0] ) { close(G_pipe[0]); G_pipe[0] = 0; }
      if ( G_pipe[1] ) { close(G_pipe[1]); G_pipe[1] = 0; }
      dataready_tid = 0;
      DEBUGMSG("OK\n");
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


/**
 * This public function handles all events. It wait a maximum of 
 * 'time' secods for an event. This version returns 1 if events
 * other than the timeout timer were processed.
 *
 * \todo there is no socket handling in this code whatsoever
 */
int fl_wait( double time ) 
{
  do_queued_events( time );
  return (got_events);
}


/**
 * event handler for Apple-Q key combination
 * this is also called from the Carbon Window handler after all windows were closed
 */
static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
  fl_lock_function();

  while ( Fl_X::first ) {
    Fl_X *x = Fl_X::first;
    Fl::handle( FL_CLOSE, x->w );
    if ( Fl_X::first == x ) {
      fl_unlock_function();
      return noErr; // FLTK has not close all windows, so we return to the main program now
    }
  }
  ExitToShell();

  fl_unlock_function();

  return noErr;
}


/**
 * Carbon Window handler
 * This needs to be linked into all new window event handlers
 */
static pascal OSStatus carbonWindowHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  UInt32 kind = GetEventKind( event );
  OSStatus ret = eventNotHandledErr;
  Fl_Window *window = (Fl_Window*)userData;

  Rect currentBounds, originalBounds;
  WindowClass winClass;
  static Fl_Window *activeWindow = 0;
  
  fl_lock_function();
  
  switch ( kind )
  {
  case kEventWindowDrawContent:
    handleUpdateEvent( fl_xid( window ) );
    ret = noErr;
    break;
  case kEventWindowBoundsChanged: {
    GetEventParameter( event, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &currentBounds );
    GetEventParameter( event, kEventParamOriginalBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &originalBounds );
    int X = currentBounds.left, W = currentBounds.right-X;
    int Y = currentBounds.top, H = currentBounds.bottom-Y;
    resize_from_system = window;
    window->resize( X, Y, W, H );
    if ( ( originalBounds.right - originalBounds.left != W ) 
      || ( originalBounds.bottom - originalBounds.top != H ) )
    {
      if ( window->shown() ) 
        handleUpdateEvent( fl_xid( window ) );
    } 
    break; }
  case kEventWindowShown:
    if ( !window->parent() )
    {
      GetWindowClass( fl_xid( window ), &winClass );
      if ( winClass != kHelpWindowClass ) {	// help windows can't get the focus!
        Fl::handle( FL_FOCUS, window);
        activeWindow = window;
      }
      Fl::handle( FL_SHOW, window);
    }
    break;
  case kEventWindowHidden:
    if ( !window->parent() ) Fl::handle( FL_HIDE, window);
    break;
  case kEventWindowActivated:
    if ( window!=activeWindow ) 
    {
      GetWindowClass( fl_xid( window ), &winClass );
      if ( winClass != kHelpWindowClass ) {	// help windows can't get the focus!
        Fl::handle( FL_FOCUS, window);
        activeWindow = window;
      }
    }
    break;
  case kEventWindowDeactivated:
    if ( window==activeWindow ) 
    {
      Fl::handle( FL_UNFOCUS, window);
      activeWindow = 0;
    }
    break;
  case kEventWindowClose:
    Fl::handle( FL_CLOSE, window ); // this might or might not close the window
    // if there are no more windows, send a high-level quit event
    if (!Fl_X::first) QuitAppleEventHandler( 0, 0, 0 );
    ret = noErr; // returning noErr tells Carbon to stop following up on this event
    break;
  }

  fl_unlock_function();

  return ret;
}


/**
 * Carbon Mousewheel handler
 * This needs to be linked into all new window event handlers
 */
static pascal OSStatus carbonMousewheelHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  Fl_Window *window = (Fl_Window*)userData;
  EventMouseWheelAxis axis;

  fl_lock_function();
  
  GetEventParameter( event, kEventParamMouseWheelAxis, typeMouseWheelAxis, NULL, sizeof(EventMouseWheelAxis), NULL, &axis );
  long delta;
  GetEventParameter( event, kEventParamMouseWheelDelta, typeLongInteger, NULL, sizeof(long), NULL, &delta );
  if ( axis == kEventMouseWheelAxisX )
  {
    Fl::e_dx = delta;
    if ( Fl::e_dx) Fl::handle( FL_MOUSEWHEEL, window );
  }
  else if ( axis == kEventMouseWheelAxisY )
  {
    Fl::e_dy = -delta;
    if ( Fl::e_dy) Fl::handle( FL_MOUSEWHEEL, window );
  }
  else {
    fl_unlock_function();

    return eventNotHandledErr;
  }

  fl_unlock_function();
  
  return noErr;
}


/**
 * convert the current mouse chord into the FLTK modifier state
 */
static void chord_to_e_state( UInt32 chord )
{
  static ulong state[] = 
  { 
    0, FL_BUTTON1, FL_BUTTON3, FL_BUTTON1|FL_BUTTON3, FL_BUTTON2,
    FL_BUTTON2|FL_BUTTON1, FL_BUTTON2|FL_BUTTON3, FL_BUTTON2|FL_BUTTON1|FL_BUTTON3
  };
  Fl::e_state = ( Fl::e_state & 0xff0000 ) | state[ chord & 0x07 ];
}


/**
 * Carbon Mouse Button Handler
 */
static pascal OSStatus carbonMouseHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  static int keysym[] = { 0, FL_Button+1, FL_Button+3, FL_Button+2 };
  static int px, py;

  fl_lock_function();
  
  fl_os_event = event;
  Fl_Window *window = (Fl_Window*)userData;
  Point pos;
  GetEventParameter( event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &pos );
  EventMouseButton btn;
  GetEventParameter( event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &btn );
  UInt32 clickCount;
  GetEventParameter( event, kEventParamClickCount, typeUInt32, NULL, sizeof(UInt32), NULL, &clickCount );
  UInt32 chord;
  GetEventParameter( event, kEventParamMouseChord, typeUInt32, NULL, sizeof(UInt32), NULL, &chord );
  WindowRef xid = fl_xid(window), tempXid;
  int sendEvent = 0, part;
  switch ( GetEventKind( event ) )
  {
  case kEventMouseDown:
    part = FindWindow( pos, &tempXid );
    if ( part != inContent ) {
      fl_unlock_function();
      return CallNextEventHandler( nextHandler, event ); // let the OS handle this for us
    }
    if ( !IsWindowActive( xid ) )
      CallNextEventHandler( nextHandler, event ); // let the OS handle the activation, but continue to get a click-through effect
    // normal handling of mouse-down follows
    fl_os_capture = xid;
    sendEvent = FL_PUSH;
    Fl::e_is_click = 1; px = pos.h; py = pos.v;
    Fl::e_clicks = clickCount-1;
    // fall through
  case kEventMouseUp:
    if ( !window ) break;
    if ( !sendEvent ) {
      sendEvent = FL_RELEASE; 
    }
    Fl::e_keysym = keysym[ btn ];
    // fall through
  case kEventMouseMoved:
    if ( !sendEvent ) { 
      sendEvent = FL_MOVE; chord = 0; 
    }
    // fall through
  case kEventMouseDragged:
    if ( !sendEvent ) {
      sendEvent = FL_MOVE; // Fl::handle will convert into FL_DRAG
      if (abs(pos.h-px)>5 || abs(pos.v-py)>5) 
        Fl::e_is_click = 0;
    }
    chord_to_e_state( chord );
    GrafPtr oldPort;
    GetPort( &oldPort );
    SetPort( GetWindowPort(xid) ); // \todo replace this! There must be some GlobalToLocal call that has a port as an argument
    SetOrigin(0, 0);
    Fl::e_x_root = pos.h;
    Fl::e_y_root = pos.v;
    GlobalToLocal( &pos );
    Fl::e_x = pos.h;
    Fl::e_y = pos.v;
    SetPort( oldPort );
    Fl::handle( sendEvent, window );
    break;
  }

  fl_unlock_function();
  
  return noErr;
}


/**
 * convert the current mouse chord into the FLTK modifier state
 */
static void mods_to_e_state( UInt32 mods )
{
  long state = 0;
  if ( mods & kEventKeyModifierNumLockMask ) state |= FL_NUM_LOCK;
  if ( mods & cmdKey ) state |= FL_META;
  if ( mods & (optionKey|rightOptionKey) ) state |= FL_ALT;
  if ( mods & (controlKey|rightControlKey) ) state |= FL_CTRL;
  if ( mods & (shiftKey|rightShiftKey) ) state |= FL_SHIFT;
  if ( mods & alphaLock ) state |= FL_CAPS_LOCK;
  Fl::e_state = ( Fl::e_state & 0xff000000 ) | state;
  //printf( "State 0x%08x (%04x)\n", Fl::e_state, mods );
}


/**
 * convert the current mouse chord into the FLTK keysym
 */
static void mods_to_e_keysym( UInt32 mods )
{
  if ( mods & cmdKey ) Fl::e_keysym = FL_Meta_L;
  else if ( mods & kEventKeyModifierNumLockMask ) Fl::e_keysym = FL_Num_Lock;
  else if ( mods & optionKey ) Fl::e_keysym = FL_Alt_L;
  else if ( mods & rightOptionKey ) Fl::e_keysym = FL_Alt_R;
  else if ( mods & controlKey ) Fl::e_keysym = FL_Control_L;
  else if ( mods & rightControlKey ) Fl::e_keysym = FL_Control_R;
  else if ( mods & shiftKey ) Fl::e_keysym = FL_Shift_L;
  else if ( mods & rightShiftKey ) Fl::e_keysym = FL_Shift_R;
  else if ( mods & alphaLock ) Fl::e_keysym = FL_Caps_Lock;
  else Fl::e_keysym = 0;
}

/**
 * convert the keyboard return code into the symbol on the keycaps
 */
static unsigned short keycode_to_sym( UInt32 keyCode, UInt32 mods, unsigned short deflt )
{
  static Ptr map = 0;
  UInt32 state = 0;
  if (!map) {
    map = (Ptr)GetScriptManagerVariable(smKCHRCache);
    if (!map) {
      long kbID = GetScriptManagerVariable(smKeyScript);
      map = *GetResource('KCHR', kbID);
    }
  }
  if (map)
    return KeyTranslate(map, keyCode|mods, &state );
  return deflt;
}

/**
 * handle carbon keyboard events
 */
pascal OSStatus carbonKeyboardHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  static char buffer[5];
  int sendEvent = 0;
  Fl_Window *window = (Fl_Window*)userData;
  UInt32 mods;
  static UInt32 prevMods = 0xffffffff;

  fl_lock_function();
  
  GetEventParameter( event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &mods );
  if ( prevMods == 0xffffffff ) prevMods = mods;
  UInt32 keyCode;
  GetEventParameter( event, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode );
  unsigned char key;
  GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &key );
  unsigned short sym;

  switch ( GetEventKind( event ) )
  {
  case kEventRawKeyDown:
  case kEventRawKeyRepeat:
    sendEvent = FL_KEYBOARD;
    // fall through
  case kEventRawKeyUp:
    if ( !sendEvent ) sendEvent = FL_KEYUP;
    // if the user pressed alt/option, event_key should have the keycap, but event_text should generate the international symbol
    if ( isalpha(key) )
      sym = tolower(key);
    else if ( Fl::e_state&FL_CTRL && key<32 )
      sym = key+96;
    else if ( Fl::e_state&FL_ALT )
      sym = keycode_to_sym( keyCode & 0x7f, 0, macKeyLookUp[ keyCode & 0x7f ] ); // find the keycap of this key
    else
      sym = macKeyLookUp[ keyCode & 0x7f ];
    Fl::e_keysym = sym;
    if ( keyCode==0x4c ) key=0x0d;
    if ( ( (sym>=FL_KP) && (sym<=FL_KP_Last) ) || ((sym&0xff00)==0) || (sym==FL_Tab) ) {
      buffer[0] = key;
      Fl::e_length = 1;
    } else {
      buffer[0] = 0;
      Fl::e_length = 0;
    }
    Fl::e_text = buffer;
    // insert UnicodeHandling here!
    break;
  case kEventRawKeyModifiersChanged: {
    UInt32 tMods = prevMods ^ mods;
    if ( tMods )
    {
      mods_to_e_keysym( tMods );
      if ( Fl::e_keysym ) 
        sendEvent = ( prevMods<mods ) ? FL_KEYBOARD : FL_KEYUP;
      Fl::e_length = 0;
      buffer[0] = 0;
      prevMods = mods;
    }
    mods_to_e_state( mods );
    break; }
  }
  while (window->parent()) window = window->window();
  if (sendEvent && Fl::handle(sendEvent,window)) {
    fl_unlock_function();
  
    return noErr; // return noErr if FLTK handled the event
  } else {
    fl_unlock_function();
  
    return CallNextEventHandler( nextHandler, event );;
  }
}


/**
 * initialize the Mac toolboxes and set the default menubar
 */
void fl_open_display() {
  static char beenHereDoneThat = 0;
  if ( !beenHereDoneThat )  {
    beenHereDoneThat = 1;
    
    FlushEvents(everyEvent,0);

    MoreMasters(); // \todo Carbon suggests MoreMasterPointers()
    AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );

    // create the Mac Handle for the default cursor (a pointer to a pointer)
    GetQDGlobalsArrow(&default_cursor);
    default_cursor_ptr = &default_cursor;
    fl_default_cursor  = &default_cursor_ptr;
    
    ClearMenuBar();
    AppendResMenu( GetMenuHandle( 1 ), 'DRVR' );
    DrawMenuBar();
  }
}


/**
 * get rid of allocated resources
 */
void fl_close_display()  {
}


/**
 * smallest x ccordinate in screen space
 */
int Fl::x() {
  BitMap r;
  GetQDGlobalsScreenBits(&r);
  return r.bounds.left;
}


/**
 * smallest y ccordinate in screen space
 */
int Fl::y() {
  BitMap r;
  GetQDGlobalsScreenBits(&r);
  return r.bounds.top + 20; // \todo 20 pixel menu bar?
}


/**
 * screen width (single monitor!?)
 */
int Fl::w() {
  BitMap r;
  GetQDGlobalsScreenBits(&r);
  return r.bounds.right - r.bounds.left;
}


/**
 * screen height (single monitor!?)
 */
int Fl::h() {
  BitMap r;
  GetQDGlobalsScreenBits(&r);
  return r.bounds.bottom - r.bounds.top - 20;
}


/**
 * get the current mouse pointer world coordinates
 */
void Fl::get_mouse(int &x, int &y) 
{
  fl_open_display();
  Point loc; 
  GetMouse( &loc );
  LocalToGlobal( &loc );
  x = loc.h;
  y = loc.v;
}


/**
 * convert Mac keystrokes to FLTK
 */
unsigned short mac2fltk(ulong macKey) 
{
  unsigned short cc = macKeyLookUp[(macKey>>8)&0x7f];
  if (cc) return cc;
  return macKey&0xff;
}


/**
 * Initialize the given port for redraw and call the windw's flush() to actually draw the content
 */ 
void Fl_X::flush()
{
  w->flush();
  SetOrigin( 0, 0 );
  //QDFlushPortBuffer( GetWindowPort(xid), 0 ); // \todo do we need this?
}


/**
 * Handle all clipping and redraw for the given port
 * There are two different callers for this event:
 * 1: the OS can request a redraw and provides all clipping itself
 * 2: Fl::flush() wants all redraws now
 */    
void handleUpdateEvent( WindowPtr xid ) 
{
  Fl_Window *window = fl_find( xid );
  if ( !window ) return;
  GrafPtr oldPort;
  GetPort( &oldPort );
  SetPort( GetWindowPort(xid) );
  Fl_X *i = Fl_X::i( window );
  i->wait_for_expose = 0;
  if ( window->damage() ) {
    if ( i->region ) {
      InvalWindowRgn( xid, i->region );
    }
  }
  if ( i->region ) { // no region, so the sytem will take the update region from the OS
    DisposeRgn( i->region );
    i->region = 0;
  }
  for ( Fl_X *cx = i->xidChildren; cx; cx = cx->xidNext )
  {
    cx->w->clear_damage(window->damage()|FL_DAMAGE_EXPOSE);
    cx->flush();
    cx->w->clear_damage();
  }
  window->clear_damage(window->damage()|FL_DAMAGE_EXPOSE);
  i->flush();
  window->clear_damage();
  SetPort( oldPort );
}     


/**
 * \todo this is a leftover from OS9 times. Please check how much applies to Carbon!
 */
int Fl_X::fake_X_wm(const Fl_Window* w,int &X,int &Y, int &bt,int &bx, int &by) {
  int W, H, xoff, yoff, dx, dy;
  int ret = bx = by = bt = 0;
  if (w->border() && !w->parent()) {
    if (w->maxw != w->minw || w->maxh != w->minh) {
      ret = 2;
      bx = 6; // \todo Mac : GetSystemMetrics(SM_CXSIZEFRAME);
      by = 6; // \todo Mac : get Mac window frame size GetSystemMetrics(SM_CYSIZEFRAME);
    } else {
      ret = 1;
      bx = 6; // \todo Mac : GetSystemMetrics(SM_CXFIXEDFRAME);
      by = 6; // \todo Mac : GetSystemMetrics(SM_CYFIXEDFRAME);
    }
    bt = 22; // \todo Mac : GetSystemMetrics(SM_CYCAPTION);
  }
  //The coordinates of the whole window, including non-client area
  xoff = bx;
  yoff = by + bt;
  dx = 2*bx;
  dy = 2*by + bt;
  X = w->x()-xoff;
  Y = w->y()-yoff;
  W = w->w()+dx;
  H = w->h()+dy;

  //Proceed to positioning the window fully inside the screen, if possible
  //Make border's lower right corner visible
  if (Fl::w() < X+W) X = Fl::w() - W;
  if (Fl::h() < Y+H) Y = Fl::h() - H;
  //Make border's upper left corner visible
  if (X<0) X = 0;
  if (Y<0) Y = 0;
  //Make client area's lower right corner visible
  if (Fl::w() < X+dx+ w->w()) X = Fl::w() - w->w() - dx;
  if (Fl::h() < Y+dy+ w->h()) Y = Fl::h() - w->h() - dy;
  //Make client area's upper left corner visible
  if (X+xoff < 0) X = -xoff;
  if (Y+yoff < 0) Y = -yoff;
  //Return the client area's top left corner in (X,Y)
  X+=xoff;
  Y+=yoff;

  return ret;
}

/**
 * convert a Mac FSSpec structure into a Unix filename 
 */
static int FSSpec2UnixPath( FSSpec *fs, char *dst )
{
  FSRef fsRef;
  FSpMakeFSRef( fs, &fsRef );
  FSRefMakePath( &fsRef, (UInt8*)dst, 1024 );
  return strlen(dst);
/* keep the code below. The above function is only implemented in OS X, so we might need the other code for OS 9 and friends
  short offset = 0;
  if ( fs->parID != fsRtParID )
  {
    FSSpec parent;
    OSErr ret = FSMakeFSSpec( fs->vRefNum, fs->parID, 0, &parent );
    if ( ret != noErr ) return 0;
    offset = FSSpec2UnixPath( &parent, dst );
  }

  if ( fs->parID == fsRtParID && fs->vRefNum == -100 ) //+ bad hack: we assume that volume -100 is mounted as root
  {
    memcpy( dst, "/", 2 );
    return 1; // don't add anything to the filename - we are fine already
  }

  short len = fs->name[0];
  if ( fs->parID == fsRtParID ) { // assume tat all other volumes are in this directory (international name WILL vary!)
    memcpy( dst, "/Volumes", 8 );
    offset = 8;
  }
  
  if ( offset!=1 ) dst[ offset++ ] = '/'; // avoid double '/'
  memcpy( dst+offset, fs->name+1, len );
  dst[ len+offset ] = 0;
  return len+offset;
*/
}
 
Fl_Window *fl_dnd_target_window = 0;
#include <FL/fl_draw.H>
/**
 * Drag'n'drop tracking handler
 */
static pascal OSErr dndTrackingHandler( DragTrackingMessage msg, WindowPtr w, void *userData, DragReference dragRef )
{
  Fl_Window *target = (Fl_Window*)userData;
  Point mp;
  static int px, py;
  
  switch ( msg )
  {
  case kDragTrackingEnterWindow:
    // check if 'TEXT' is available
    GetDragMouse( dragRef, &mp, 0 );
    Fl::e_x_root = px = mp.h;
    Fl::e_y_root = py = mp.v;
    Fl::e_x = px - target->x();
    Fl::e_y = py - target->y();
    fl_dnd_target_window = target;
    if ( Fl::handle( FL_DND_ENTER, target ) )
      fl_cursor( FL_CURSOR_HAND ); //ShowDragHilite( ); // modify the mouse cursor?!
    else
      fl_cursor( FL_CURSOR_DEFAULT ); //HideDragHilite( dragRef );
    breakMacEventLoop();
    return noErr;
  case kDragTrackingInWindow:
    GetDragMouse( dragRef, &mp, 0 );
    if ( mp.h==px && mp.v==py )
      break;	//+ return previous condition for dnd hiliting
    Fl::e_x_root = px = mp.h;
    Fl::e_y_root = py = mp.v;
    Fl::e_x = px - target->x();
    Fl::e_y = py - target->y();
    fl_dnd_target_window = target;
    if ( Fl::handle( FL_DND_DRAG, target ) )
      fl_cursor( FL_CURSOR_HAND ); //ShowDragHilite( ); // modify the mouse cursor?!
    else
      fl_cursor( FL_CURSOR_DEFAULT ); //HideDragHilite( dragRef );
    breakMacEventLoop();
    return noErr;
    break;
  case kDragTrackingLeaveWindow:
    // HideDragHilite()
    fl_cursor( FL_CURSOR_DEFAULT ); //HideDragHilite( dragRef );
    if ( fl_dnd_target_window )
    {
      Fl::handle( FL_DND_LEAVE, fl_dnd_target_window );
      fl_dnd_target_window = 0;
    }
    breakMacEventLoop();
    return noErr;
  }
  return noErr;
}


/**
 * Drag'n'drop receive handler
 */
static pascal OSErr dndReceiveHandler( WindowPtr w, void *userData, DragReference dragRef )
{
  Point mp;
  OSErr ret;
  
  Fl_Window *target = fl_dnd_target_window = (Fl_Window*)userData;
  GetDragMouse( dragRef, &mp, 0 );
  Fl::e_x_root = mp.h;
  Fl::e_y_root = mp.v;
  Fl::e_x = Fl::e_x_root - target->x();
  Fl::e_y = Fl::e_y_root - target->y();
  if ( !Fl::handle( FL_DND_RELEASE, target ) )
    return userCanceledErr;
    
  // get the ASCII text
  UInt16 i, nItem;
  ItemReference itemRef;
  FlavorFlags flags;
  Size itemSize, size = 0;
  CountDragItems( dragRef, &nItem );
  for ( i = 1; i <= nItem; i++ )
  {
    GetDragItemReferenceNumber( dragRef, i, &itemRef );
    ret = GetFlavorFlags( dragRef, itemRef, 'TEXT', &flags );
    if ( ret == noErr )
    {
      GetFlavorDataSize( dragRef, itemRef, 'TEXT', &itemSize );
      size += itemSize;
    }
    ret = GetFlavorFlags( dragRef, itemRef, 'hfs ', &flags );
    if ( ret == noErr )
    {
      size += 1024; //++ ouch! We should create the full pathname and figure out its length
    }
  }
  
  if ( !size )
    return userCanceledErr;
  
  Fl::e_length = size + nItem - 1;
  char *dst = Fl::e_text = (char*)malloc( size+nItem );;
  
  for ( i = 1; i <= nItem; i++ )
  {
    GetDragItemReferenceNumber( dragRef, i, &itemRef );
    ret = GetFlavorFlags( dragRef, itemRef, 'TEXT', &flags );
    if ( ret == noErr )
    {
      GetFlavorDataSize( dragRef, itemRef, 'TEXT', &itemSize );
      GetFlavorData( dragRef, itemRef, 'TEXT', dst, &itemSize, 0L );
      dst += itemSize;
      *dst++ = '\n'; // add our element seperator
    }
    ret = GetFlavorFlags( dragRef, itemRef, 'hfs ', &flags );
    if ( ret == noErr )
    {
      HFSFlavor hfs; itemSize = sizeof( hfs );
      GetFlavorData( dragRef, itemRef, 'hfs ', &hfs, &itemSize, 0L );
      itemSize = FSSpec2UnixPath( &hfs.fileSpec, dst );
      dst += itemSize;
      if ( itemSize>1 && ( hfs.fileType=='fold' || hfs.fileType=='disk' ) ) 
        *dst++ = '/';
      *dst++ = '\n'; // add our element seperator
    }
  }
  
  dst[-1] = 0;
//  if ( Fl::e_text[Fl::e_length-1]==0 ) Fl::e_length--; // modify, if trailing 0 is part of string
  Fl::e_length = dst - Fl::e_text - 1;
  target->handle(FL_PASTE);
  free( Fl::e_text );
  
  fl_dnd_target_window = 0L;
  breakMacEventLoop();
  return noErr;
}


/**
 * go ahead, create that (sub)window
 * \todo we should make menu windows slightly transparent for the new Mac look
 */
void Fl_X::make(Fl_Window* w)
{
  static int xyPos = 50;
  if ( w->parent() ) // create a subwindow
  {
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
    Fl_Window *win = w->window();
    Fl_X *xo = Fl_X::i(win);
    w->set_visible();
    if (xo) {
      x->xidNext = xo->xidChildren;
      x->xidChildren = 0L;
      xo->xidChildren = x;
      x->xid = fl_xid(win);
      x->w = w; w->i = x;
      x->wait_for_expose = 0;
      x->next = Fl_X::first; // must be in the list for ::flush()
      Fl_X::first = x;
      w->handle(FL_SHOW);
      w->redraw(); // force draw to happen
    }
    fl_show_iconic = 0;
  }
  else // create a desktop window
  {
    Fl_Group::current(0);
    fl_open_display();
    int winclass = kDocumentWindowClass;
    int winattr = kWindowStandardHandlerAttribute | kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute;
    int xp = w->x();
    int yp = w->y();
    int wp = w->w();
    int hp = w->h();
    if (w->size_range_set) {
      if ( w->minh != w->maxh || w->minw != w->maxw)
        winattr |= kWindowFullZoomAttribute | kWindowResizableAttribute | kWindowLiveResizeAttribute;
    } else {
      if (w->resizable()) {
        Fl_Widget *o = w->resizable();
        int minw = o->w(); if (minw > 100) minw = 100;
        int minh = o->h(); if (minh > 100) minh = 100;
        w->size_range(w->w() - o->w() + minw, w->h() - o->h() + minh, 0, 0);
        winattr |= kWindowFullZoomAttribute | kWindowResizableAttribute | kWindowLiveResizeAttribute;
      } else {
        w->size_range(w->w(), w->h(), w->w(), w->h());
      }
    }
    int xwm = xp, ywm = yp, bt, bx, by;
    if (!fake_X_wm(w, xwm, ywm, bt, bx, by)) 
      { winclass = kHelpWindowClass; winattr = 0; } // menu windows and tooltips
    else if (w->modal())
      winclass = kMovableModalWindowClass;
    if (by+bt) {
      wp += 2*bx;
      hp += 2*by+bt;
    }
    if (!(w->flags() & Fl_Window::FL_FORCE_POSITION)) {
      w->x(xyPos+Fl::x()); w->y(xyPos+Fl::y()); // use the Carbon functions below for default window positioning
      xyPos += 25;
      if (xyPos>200) xyPos = 25;
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
    Str255 pTitle; 
    if (name) { pTitle[0] = strlen(name); memcpy(pTitle+1, name, pTitle[0]); }
    else pTitle[0]=0;

    Fl_X* x = new Fl_X;
    x->other_xid = 0; // room for doublebuffering image map. On OS X this is only used by overlay windows
    x->region = 0;
    x->subRegion = 0;
    x->cursor = fl_default_cursor;
    x->xidChildren = 0;
    x->xidNext = 0;

    winattr &= GetAvailableWindowAttributes( winclass );	// make sure that the window will open
    CreateNewWindow( winclass, winattr, &wRect, &(x->xid) );
    SetWTitle(x->xid, pTitle);
    MoveWindow(x->xid, wRect.left, wRect.top, 1);	// avoid Carbon Bug on old OS
    if (w->non_modal() && !w->modal())
      SetWindowClass(x->xid, kFloatingWindowClass );	// Major kludge: this is to have the regular look, but stay above the document windows
    if (!(w->flags() & Fl_Window::FL_FORCE_POSITION))
    {
      WindowRef pw = Fl_X::first ? Fl_X::first->xid : 0 ;
      if ( w->modal() )
        RepositionWindow( x->xid, pw, kWindowAlertPositionOnParentWindowScreen );
      else if ( w->non_modal() )
        RepositionWindow( x->xid, pw, kWindowCenterOnParentWindowScreen );
      else
        RepositionWindow( x->xid, pw, kWindowCascadeOnParentWindowScreen );
    }
    x->w = w; w->i = x;
    x->wait_for_expose = 1;
    x->next = Fl_X::first;
    Fl_X::first = x;
    if (w->resizable()) DrawGrowIcon(x->xid);
    w->set_visible();
    { // Install Carbon Event handlers 
      OSStatus ret;
      EventHandlerUPP mousewheelHandler = NewEventHandlerUPP( carbonMousewheelHandler ); // will not be disposed by Carbon...
      static EventTypeSpec mousewheelEvents[] = {
        { kEventClassMouse, kEventMouseWheelMoved } };
      ret = InstallWindowEventHandler( x->xid, mousewheelHandler, 1, mousewheelEvents, w, 0L );
      EventHandlerUPP mouseHandler = NewEventHandlerUPP( carbonMouseHandler ); // will not be disposed by Carbon...
      static EventTypeSpec mouseEvents[] = {
        { kEventClassMouse, kEventMouseDown },
        { kEventClassMouse, kEventMouseUp },
        { kEventClassMouse, kEventMouseMoved },
        { kEventClassMouse, kEventMouseDragged } };
      ret = InstallWindowEventHandler( x->xid, mouseHandler, 4, mouseEvents, w, 0L );
      EventHandlerUPP keyboardHandler = NewEventHandlerUPP( carbonKeyboardHandler ); // will not be disposed by Carbon...
      static EventTypeSpec keyboardEvents[] = {
        { kEventClassKeyboard, kEventRawKeyDown },
        { kEventClassKeyboard, kEventRawKeyRepeat },
        { kEventClassKeyboard, kEventRawKeyUp },
        { kEventClassKeyboard, kEventRawKeyModifiersChanged } };
      ret = InstallWindowEventHandler( x->xid, keyboardHandler, 4, keyboardEvents, w, 0L );
      EventHandlerUPP windowHandler = NewEventHandlerUPP( carbonWindowHandler ); // will not be disposed by Carbon...
      static EventTypeSpec windowEvents[] = {
        { kEventClassWindow, kEventWindowDrawContent },
        { kEventClassWindow, kEventWindowShown },
        { kEventClassWindow, kEventWindowHidden },
        { kEventClassWindow, kEventWindowActivated },
        { kEventClassWindow, kEventWindowDeactivated },
        { kEventClassWindow, kEventWindowClose },
        { kEventClassWindow, kEventWindowBoundsChanged } };
      ret = InstallWindowEventHandler( x->xid, windowHandler, 7, windowEvents, w, 0L );
      ret = InstallTrackingHandler( dndTrackingHandler, x->xid, w );
      ret = InstallReceiveHandler( dndReceiveHandler, x->xid, w );
    }

    if ( ! Fl_X::first->next ) // if this is the first window, we need to bring the application to the front
    { 
      ProcessSerialNumber psn;
      OSErr err = GetCurrentProcess( &psn );
      if ( err==noErr ) SetFrontProcess( &psn );
    }
    
    if (fl_show_iconic) { 
      fl_show_iconic = 0;
      CollapseWindow( x->xid, true ); // \todo Mac ; untested
    } else if (winclass != kHelpWindowClass) {
      Fl_Tooltip::enter(0);
    }

    ShowWindow(x->xid);

    w->handle(FL_SHOW);
    w->redraw(); // force draw to happen
    w->set_visible();
    
    if (w->modal()) { Fl::modal_ = w; fl_fix_focus(); }
  }
}


/**
 * this is a leftover from X Windows
 */
void Fl_Window::size_range_() {
  size_range_set = 1;
}


/**
 * returns pointer to the filename, or null if name ends with ':'
 */
const char *fl_filename_name( const char *name ) 
{
  const char *p, *q;
  if (!name) return (0);
  for ( p = q = name ; *p ; ) 
  {
    if ( ( p[0] == ':' ) && ( p[1] == ':' ) ) 
    {
      q = p+2;
      p++;
    }
    else if (p[0] == '/')
      q = p + 1;
    p++;
  }
  return q;
}


/**
 * set the window title bar
 * \todo make the titlebar icon work!
 */
void Fl_Window::label(const char *name,const char */*iname*/) {
  Fl_Widget::label(name);
  Str255 pTitle;

  if (name) { pTitle[0] = strlen(name); memcpy(pTitle+1, name, pTitle[0]); }
  else pTitle[0] = 0;

  if (shown() || i) SetWTitle(fl_xid(this), pTitle);
}


/**
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
  if (!shown() || !i) {
    Fl_X::make(this);
  } else {
      if ( !parent() )
      {
        if ( IsWindowCollapsed( i->xid ) ) CollapseWindow( i->xid, false );
        if (!fl_capture) {
          BringToFront(i->xid);
          SelectWindow(i->xid);
        }
      }
  }
}


/**
 * resize a window
 */
void Fl_Window::resize(int X,int Y,int W,int H) {
  int is_a_resize = (W != w() || H != h());
  if (X != x() || Y != y()) set_flag(FL_FORCE_POSITION);
  else if (!is_a_resize) return;
  if ( (resize_from_system!=this) && (!parent()) && shown()) {
    MoveWindow(i->xid, X, Y, 0);
    if (is_a_resize) {
      SizeWindow(i->xid, W>0 ? W : 1, H>0 ? H : 1, 1);
      Rect all; all.top=-32000; all.bottom=32000; all.left=-32000; all.right=32000;
      InvalWindowRect( i->xid, &all );    
    }
  } else if (resize_from_system == this && size_range_set && !parent() && shown()) {
    if (size_range_set) {
      if (W < minw) W = minw;
      else if (W > maxw && maxw) W = maxw;
      if (H < minh) H = minh;
      else if (H > maxh && maxh) H = maxh;
    }
    SizeWindow(i->xid, W>0 ? W : 1, H>0 ? H : 1, 1);
  }
  resize_from_system = 0;
  if (is_a_resize) {
    Fl_Group::resize(X,Y,W,H);
    if (shown()) { redraw(); if (!parent()) i->wait_for_expose = 1; }
  } else {
    x(X); y(Y); 
  }
}


/**
 * make all drawing go into this window (called by subclass flush() impl.)
 */
void Fl_Window::make_current() 
{
  if ( !fl_window_region )
    fl_window_region = NewRgn();
  fl_window = i->xid;
  current_ = this;

  SetPort( GetWindowPort(i->xid) ); // \todo check for the handling of doublebuffered windows

  int xp = 0, yp = 0;
  Fl_Window *win = this;
  while ( win ) 
  {
    if ( !win->window() )
      break;
    xp += win->x();
    yp += win->y();
    win = (Fl_Window*)win->window();
  }
  SetOrigin( -xp, -yp );
  
  SetRectRgn( fl_window_region, 0, 0, w(), h() );
  
  // \todo for performance reasons: we don't have to create this unless the child windows moved
  for ( Fl_X *cx = i->xidChildren; cx; cx = cx->xidNext )
  {
    Fl_Region r = NewRgn();
    Fl_Window *cw = cx->w;
    SetRectRgn( r, cw->x() - xp, cw->y() - yp, 
                   cw->x() + cw->w() - xp, cw->y() + cw->h() - yp );
    DiffRgn( fl_window_region, r, fl_window_region );
    DisposeRgn( r );
  }
  
  fl_clip_region( 0 );
  SetPortClipRegion( GetWindowPort(i->xid), fl_window_region );
  return;
}

////////////////////////////////////////////////////////////////
// Cut & paste.

Fl_Widget *fl_selection_requestor = 0;
char *fl_selection_buffer[2];
int fl_selection_length[2];
int fl_selection_buffer_length[2];
static ScrapRef myScrap = 0;

/**
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
    ClearCurrentScrap();
    OSStatus ret = GetCurrentScrap( &myScrap );
    if ( ret != noErr ) {
      myScrap = 0;
      return;
    }
    // Previous version changed \n to \r before sending the text, but I would
    // prefer to leave the local buffer alone, so a copied buffer may be
    // needed. Check to see if this is necessary on OS/X.
    PutScrapFlavor( myScrap, kScrapFlavorTypeText, 0,
		    len, fl_selection_buffer[1] );
  }
}

// Call this when a "paste" operation happens:
void Fl::paste(Fl_Widget &receiver, int clipboard) {
  if (clipboard) {
    // see if we own the selection, if not go get it:
    ScrapRef scrap = 0;
    Size len = 0;
    if (GetCurrentScrap(&scrap) == noErr && scrap != myScrap &&
	GetScrapFlavorSize(scrap, kScrapFlavorTypeText, &len) == noErr) {
      if ( len > fl_selection_buffer_length[1] ) {
	fl_selection_buffer_length[1] = len + 32;
	delete[] fl_selection_buffer[1];
	fl_selection_buffer[1] = new char[len];
      }
      GetScrapFlavorData( scrap, kScrapFlavorTypeText, &len,
			  fl_selection_buffer[1] );
      fl_selection_length[1] = len;
      // turn all \r characters into \n:
      for (int x = 0; x < len; x++) {
	if (fl_selection_buffer[1][x] == '\r')
	  fl_selection_buffer[1][x] = '\n';
      }
    }
  }
  Fl::e_text = fl_selection_buffer[clipboard];
  Fl::e_length = fl_selection_length[clipboard];
  receiver.handle(FL_PASTE);
  return;
}


//
// End of "$Id: Fl_mac.cxx,v 1.1.2.39 2003/05/20 17:53:26 easysw Exp $".
//

