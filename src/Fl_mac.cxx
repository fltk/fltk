//
// "$Id: Fl_mac.cxx,v 1.1.2.8 2001/12/20 05:27:14 matthiaswm Exp $"
//
// MacOS specific code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2001 by Bill Spitzak and others.
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

/**
 * From the inner edge of a MetroWerks CodeWarrior CD:
 * (without permission)
 *
 * Three Compiles for 68Ks under the sky,
 * Seven Compiles for PPCs in their fragments of code,
 * Nine Compiles for Mortal Carbon doomed to die,
 * One Compile for Mach-O Cocoa on its Mach-O throne,
 * in the Land of MacOS X where the Drop-Shadows lie.
 * 
 * One Compile to link them all, One Compile to merge them,
 * One Compile to copy them all and in the bundle bind them,
 * in the Land of MacOS X where the Drop-Shadows lie.
 */


// we don't need the following definition because we deliver only
// true mouse moves.  On very slow systems however, this flag may
// still be useful.
#define CONSOLIDATE_MOTION 0

#include <config.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef TARGET_API_MAC_CARBON
// use the above define if you want to use full Carbon API
// - this will change event handling to Carbon Events
#endif

static unsigned short macKeyLookUp[];
static Fl_Window* resize_from_system;
Fl_Window* fl_find(Window);
int fl_handle(const EventRecord &event);
void handleUpdateEvent( WindowPtr xid );

int fl_screen;
Handle fl_system_menu;
Fl_Sys_Menu_Bar *fl_sys_menu_bar = 0;
CursHandle fl_default_cursor;
static CursPtr default_cursor_ptr;
static Cursor default_cursor;
 
#if CONSOLIDATE_MOTION
static Fl_Window* send_motion;
extern Fl_Window* fl_xmousewin;
#endif

/**
 * we need these as temporary regions for correct subwindow clipping
 */
static RgnHandle flmFullRgn = 0L, flmSubRgn = 0L;
static Window flmFullXid = 0;
static SysEnvRec MacWorld;


/**
 * Performance timer start
 * - not finished, don't use
 */
static UnsignedWide _perfStart;
static void _startPerf()
{
  Microseconds( &_perfStart );
}

/**
 * Performance timer end
 * - not finished, don't use
 */
static unsigned int _endPerf()
{
  UnsignedWide _perfEnd;
  Microseconds( &_perfEnd );
  // display the difference somehow - probably averaged
  //			if (msStart.hi==msEnd.hi)
  //			  s1->value( (msEnd.lo-msStart.lo)/100.0 );
  
  // get delta:
  if ( _perfEnd.lo >= _perfStart.lo )
    _perfEnd.hi = _perfEnd.hi - _perfStart.hi;
  else
    _perfEnd.hi = (_perfEnd.hi - 1) - _perfStart.hi;
  _perfEnd.lo = _perfEnd.lo - _perfStart.lo;
  
  // convert to double in micro seconds
  
//  static double kTwoPower32 = 4294967296.0;
//  double t = (((double) _perfEnd.hi) * kTwoPower32) + _perfEnd.lo;
//  UpTime();
/*
AbsoluteTime   startTime;
AbsoluteTime   endTime;
AbsoluteTime   elapsedTime;
Nanoseconds    elapsedNanoseconds;   // This is an UnsignedWide integer 

startTime = UpTime();
DoMyOperation();
endTime = UpTime();
elapsedTime = SubAbsoluteFromAbsolute(endTime, startTime);
elapsedNanoseconds = AbsoluteToNanoseconds(elapsedTime);
*/
  return 0;
}
 
 
/**
 * \todo This funtion is not yet implemented!
 */
void Fl::add_fd( int n, int events, void (*cb)(int, void*), void *v ) 
{
#pragma unused ( n )
#pragma unused ( events )
#pragma unused ( cb )
#pragma unused ( v )
}


/**
 * \todo This funtion is not yet implemented!
 */
void Fl::add_fd(int fd, void (*cb)(int, void*), void* v) 
{
#pragma unused ( fd )
#pragma unused ( cb )
#pragma unused ( v )
}


/**
 * \todo This funtion is not yet implemented!
 */
void Fl::remove_fd(int n, int events) 
{
#pragma unused ( n )
#pragma unused ( events )
}


/**
 * \todo This funtion is not yet implemented!
 */
void Fl::remove_fd(int n) 
{
  remove_fd(n, -1);
}


/**
 * \todo check if there is actually a message pending!
 */
int fl_ready() 
{
  return 1;
}


/**
 */
void printMacEvent( const EventRecord &ev )
{
printf("Event: w:0x%04x m:0x%08x mod:0x%04x flags:%08x x:%d, y:%d\n", ev.what, ev.message, ev.modifiers, 0, ev.where.h, ev.where.v );
}



WindowRef fl_capture = 0; // we need this to compensate for a missing(?) mouse capture
WindowRef fl_os_capture = 0; // the dispatch handler will redirect mose move and drag events to these windows

/**
 * We can make every event pass through this function
 * - mouse events need to be manipulated to use a mouse focus window
 * - keyboard, mouse and some window  events need to quit the Apple Event Loop
 *   so FLTK can continue its own management
 */
pascal OSStatus carbonDispatchHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  OSStatus ret = eventNotHandledErr;
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
  }
  if ( ret == eventNotHandledErr )
    ret = CallNextEventHandler( nextHandler, event ); // let the OS handle the activation, but continue to get a click-through effect
  QuitApplicationEventLoop();
  return ret;
}

/**
 * this callback simply quits the main event loop handler, so FLTK can do its magic
 */
static void timerProcCB( EventLoopTimerRef, void* )
{
  QuitApplicationEventLoop();
}

/**
 * This function iss the central event handler.
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
    //++ SystemEventMask ( MouseUp )
    been_here = 1;
  }
#ifdef STRICTLY_CARBON  
  EventRef ev;
  //static int evn = 0, evnn = 0;
  //printf( "do events %d %g\n", evn++, time );
  //if (time>0.1) time=0.1;
  time = 0.1; // TODO: cheat
  for (;;) 
  {
    OSStatus status = ReceiveNextEvent(0, NULL, time, true, &ev);
    if ( status==eventLoopTimedOutErr )
      break;
    // TODO: status is 'eventLoopTimedOutErr' if we didn't receive an event in time
    // It is (against previous documentation) 0 whenever we receive an event
    //printf( "  status 0x%08x\n", status );
    //printf( "  events %d\n", evnn++ );
    //if ( status!=0 ) break;
    fl_handle(ev); //: handle the nullEvent to get mouse up events
    break; // TODO: cheat
//    SetRectRgn(rgn, ev.where.h, ev.where.v, ev.where.h+1, ev.where.v+1 );
  }
#elif defined(TARGET_API_MAC_CARBON)
  OSStatus ret;
  EventRef ev;
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
        { kEventClassMouse, kEventMouseDragged } };
    ret = InstallEventHandler( target, dispatchHandler, 14, dispatchEvents, 0, 0L );
    ret = InstallEventLoopTimer( GetMainEventLoop(), 0, 0, NewEventLoopTimerUPP( timerProcCB ), 0, &timer );
  }
  
  // InstallEventLoopTimer(); SetEventLoopNextFireTime();
  if ( time > 0.0 ) 
    SetEventLoopTimerNextFireTime( timer, time );
  RunApplicationEventLoop();
  // ;;;; printf("Left Event Loop!\n");
  //RunCurrentEventLoop(0.1);
  /*
  ret = ReceiveNextEvent( 0, NULL, time, true, &ev );
  if ( ret == noErr )
  {
    ret = SendEventToEventTarget( ev, target );
    if ( ret == eventNotHandledErr )
    {
      EventRecord er;
      if ( ConvertEventRefToEventRecord( ev, &er ) )
        fl_handle( er );
    }
    ReleaseEvent( ev );
  }
  */
#else
  EventRecord ev;
  unsigned long ticks = (int)(time*60.0); // setting ticks to 7fffffff will wait forever
  if ( WaitNextEvent(everyEvent, &ev, ticks, rgn) )
  {
    fl_handle(ev); //: handle the nullEvent to get mouse up events
    SetRectRgn(rgn, ev.where.h, ev.where.v, ev.where.h+1, ev.where.v+1 );
  }
#endif
  
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
 * 'time' secods for an event. It returns the same time that was given
 * in the argument!
 *
 * \TODO: there is no socket handling in this code whatsoever
 */
double fl_wait( double time ) 
{
  return do_queued_events( time );
}


/**
 * \todo not yet implemented!
 */
static void fd_callback(int,void *) 
{
  do_queued_events();
}


/**
 * Handle the mac typical topline menubar
 */
static void HandleMenu( long mResult )
{
  short item, menu ;
  MenuHandle mHandle ;
//  Str255 itemName;
  UInt32 ref;
  
  item = LoWord( mResult ) ; 
  menu = HiWord( mResult ) ;
  mHandle = GetMenuHandle( menu ) ;

  switch (menu)
  {
    case 0:
      break;
    case 1: // Apple (this menu should be defined in the resource of your application)
//      GetMenuItemText( GetMenuHandle( 1 ), item, itemName );
//      OpenDeskAcc( itemName );
      break;
    default: 
      if ( !item ) break;
      GetMenuItemRefCon( mHandle, item, &ref );
      if ( ref && fl_sys_menu_bar ) 
      {
        Fl_Menu_Item *m = (Fl_Menu_Item*)ref;
        fl_sys_menu_bar->picked( m );
  		if ( m->flags & FL_MENU_TOGGLE )
          SetItemMark( mHandle, item, ( m->flags & FL_MENU_VALUE ) ? 0x12 : 0 );
        if ( m->flags & FL_MENU_RADIO )
        {
          Fl_Menu_Item* j = m;
          int i = item;
          for (;;) {
            if (j->flags & FL_MENU_DIVIDER) break; // stop on divider lines
            j++; i++;
            if (!j->text || !j->radio()) break; // stop after group
            SetItemMark( mHandle, i, ( j->flags & FL_MENU_VALUE ) ? 0x13 : 0 );
          }
          j = m-1; i = item-1;
          for (;i>0;j--,i--) {
            if (!j->text || (j->flags&FL_MENU_DIVIDER) || !j->radio()) break;
            SetItemMark( mHandle, i, ( j->flags & FL_MENU_VALUE ) ? 0x13 : 0 );
          }
          SetItemMark( mHandle, item, ( m->flags & FL_MENU_VALUE ) ? 0x13 : 0 );
        }
      }
  }
  HiliteMenu( 0 );
}

static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    // call 'close' for every window. If any window returns unclosed, don't exit to the shell!
    // Fl::handle(FL_CLOSE, fl_find(xid));
    ExitToShell();
    return noErr;
}


//        { kEventClassWindow, kEventWindowDrawContent } };
/**
 * Carbon Window handler
 * This needs to be linked into all new window event handlers
 */
pascal OSStatus carbonWindowHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  UInt32 kind = GetEventKind( event );
  OSStatus ret = eventNotHandledErr;
  Fl_Window *window = (Fl_Window*)userData;

  Rect currentBounds, originalBounds;
  
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
    if ( !window->parent() ) Fl::handle( FL_SHOW, window);
    break;
  case kEventWindowHidden:
    if ( !window->parent() ) Fl::handle( FL_HIDE, window);
    break;
  case kEventWindowActivated:
    if ( !window->parent() ) Fl::handle(FL_FOCUS, window);
    break;
  case kEventWindowDeactivated:
    if ( !window->parent() ) Fl::handle(FL_UNFOCUS, window);
    break;
  case kEventWindowClose:
    Fl::handle( FL_CLOSE, window ); // this might or might not close the window
      // if there are no more windows, send a high-level quit event
    if (!Fl_X::first) QuitAppleEventHandler( 0, 0, 0 );
    ret = noErr; // returning noErr tells Carbon to stop following up on this event
    break;
  }
  
  return ret;
}


/**
 * Carbon Mousewheel handler
 * This needs to be linked into all new window event handlers
 */
pascal OSStatus carbonMousewheelHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  Fl_Window *window = (Fl_Window*)userData;
  EventMouseWheelAxis axis;
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
  else return eventNotHandledErr;
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
pascal OSStatus carbonMouseHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  static int keysym[] = { 0, FL_Button+1, FL_Button+3, FL_Button+2 };
  static int px, py;
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
    // ;;;; printf("Carb-win-hdlr: mouse down (x:%d, y:d)\n", pos.h, pos.v );
    part = FindWindow( pos, &tempXid );
    if ( part != inContent )
      return CallNextEventHandler( nextHandler, event ); // let the OS handle this for us
    if ( !IsWindowActive( xid ) )
      CallNextEventHandler( nextHandler, event ); // let the OS handle the activation, but continue to get a click-through effect
    // normal handling of mouse-down follows
    fl_os_capture = xid;
    sendEvent = FL_PUSH;
    Fl::e_is_click = 1; px = pos.h; py = pos.v;
    Fl::e_clicks = clickCount-1;
    // fall through
  case kEventMouseUp:
    // ;;;; if ( !sendEvent ) printf("Carb-win-hdlr: mouse up (x:%d, y:d)\n", pos.h, pos.v );
    if ( !window ) break;
    if ( !sendEvent ) sendEvent = FL_RELEASE; 
    Fl::e_keysym = keysym[ btn ];
    // fall through
  case kEventMouseMoved:
    // ;;;; if ( !sendEvent ) printf("Carb-win-hdlr: mouse moved (x:%d, y:d)\n", pos.h, pos.v );
    if ( !sendEvent ) { sendEvent = FL_MOVE; chord = 0; }
    // fall through
  case kEventMouseDragged:
    // ;;;; if ( !sendEvent ) printf("Carb-win-hdlr: mouse dragged (x:%d, y:d)\n", pos.h, pos.v );
    if ( !sendEvent ) {
      sendEvent = FL_DRAG;
      if (abs(pos.h-px)>5 || abs(pos.v-py)>5) Fl::e_is_click = 0;
    }
    chord_to_e_state( chord );
    SetPort( GetWindowPort(xid) ); SetOrigin(0, 0);
    Fl::e_x_root = pos.h;
    Fl::e_y_root = pos.v;
    GlobalToLocal( &pos );
    Fl::e_x = pos.h;
    Fl::e_y = pos.v;
    Fl::handle( sendEvent, window );
    break;
  }
  return noErr;
}

/**
 * convert the current mouse chord into the FLTK modifier state
 */
static void mods_to_e_state( UInt32 mods )
{
  long state = 0;
  if ( mods & kEventKeyModifierNumLockMask ) state |= FL_NUM_LOCK;
  if ( mods & cmdKey ) state |= FL_CTRL;
  if ( mods & (optionKey|rightOptionKey) ) state |= FL_ALT;
  if ( mods & (controlKey|rightControlKey) ) state |= FL_META;
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
  if ( mods & cmdKey ) Fl::e_keysym = FL_Control_L;
  //else if ( mods & kEventKeyModifierNumLockMask ) Fl::e_keysym = FL_Num_Lock;
  else if ( mods & optionKey ) Fl::e_keysym = FL_Alt_L;
  else if ( mods & rightOptionKey ) Fl::e_keysym = FL_Alt_R;
  else if ( mods & controlKey ) Fl::e_keysym = FL_Meta_L;
  else if ( mods & rightControlKey ) Fl::e_keysym = FL_Meta_R;
  else if ( mods & shiftKey ) Fl::e_keysym = FL_Shift_L;
  else if ( mods & rightShiftKey ) Fl::e_keysym = FL_Shift_R;
  else if ( mods & alphaLock ) Fl::e_keysym = FL_Caps_Lock;
  else Fl::e_keysym = 0;
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
  static UInt32 prevMods = 0xdeadbeef;
  GetEventParameter( event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &mods );
  if ( prevMods == 0xdeadbeef ) prevMods = mods;
  UInt32 keyCode;
  GetEventParameter( event, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode );
  char key;
  GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &key );
  // ;;;; printf( "kb: %08x %08x %02x %04x\n", mods, keyCode, key, GetEventKind( event ) );
  switch ( GetEventKind( event ) )
  {
  case kEventRawKeyDown:
  case kEventRawKeyRepeat:
    sendEvent = FL_KEYBOARD;
    // fall through
  case kEventRawKeyUp:
    if ( !sendEvent ) sendEvent = FL_KEYUP;
    Fl::e_keysym = macKeyLookUp[ keyCode & 0x7f ];
    if ( key=='\t' || key==27 || ( key>=32 && key!=0x7f )  ) {
      buffer[0] = key;
      Fl::e_length = 1;
    } else if ( key==3 || key==0x0d ) {
      buffer[0] = 0x0d;
      Fl::e_length = 1;
    } else {
      buffer[0] = 0;
      Fl::e_length = 0;
    }
    Fl::e_text = buffer;
    // insert UnicodeHandling right here!
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
  if (sendEvent && Fl::handle(sendEvent,window)) return noErr; // return noErr if FLTK handled the event
  //return noErr; // for testing
  return CallNextEventHandler( nextHandler, event );;
}

/**
 * initialize the Mac toolboxes and set the default menubar
 */
void fl_open_display() {
  static char beenHereDoneThat = 0;
  if ( !beenHereDoneThat )  {
    beenHereDoneThat = 1;

    //++ open all macintosh services
//    InitGraf(&qd.thePort);	  	/* init Quickdraw and global variables		*/
//    InitFonts();
//    InitWindows();
//    InitMenus();
//    InitCursor();
//    TEInit();
    FlushEvents(everyEvent, 0);
//    InitDialogs( nil );
    
    MoreMasters();
//    MaxApplZone();
    
//    SysEnvirons( 1, &MacWorld );
    // this thing call the quit-app function which in turn either quits our app or calls 'close' on all windows?!
    // (don't know which one would be better)
    AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );

    // OK, this is just ridiculous...    
    GetQDGlobalsArrow(&default_cursor);
    default_cursor_ptr = &default_cursor;
    fl_default_cursor  = &default_cursor_ptr;

    FlushEvents(everyEvent,0);

    flmFullRgn = NewRgn(); // here we remember our overall damage
    flmSubRgn = NewRgn();  // used to clip subwindows out of the parent windows redraw
    
    // create a minimal menu bar (\todo "about app", "FLTK settings") 
    // Any FLTK application may replace this menu later with its own bar.
#if 0
    fl_system_menu = GetNewMBar( 128 );
    if ( fl_system_menu ) {
        SetMenuBar( fl_system_menu );
        /* This is used to remove the Quit menu item from the File Drop Down in 'normal' Mac Apps
        err = Gestalt(gestaltMenuMgrAttr, &response);
	if ((err == noErr) && (response & gestaltMenuMgrAquaLayoutMask))
        {
            menu = GetMenuHandle( mFile );
            DeleteMenuItem( menu, iQuit );
            DeleteMenuItem( menu, iQuitSeparator );
        }
                */
        AppendResMenu( GetMenuHandle( 1 ), 'DRVR' );
    }
#else
    ClearMenuBar();
    AppendResMenu( GetMenuHandle( 1 ), 'DRVR' );
#endif // 0

    DrawMenuBar();
  }
}


/**
 * get rid of allocated resources
 */
void fl_close_display()  {
  DisposeRgn( flmFullRgn );
  DisposeRgn( flmSubRgn );
	//++ close all mac services
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
  return r.bounds.top + 20; // 20 pixel menu bar?
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


/************************** event conversion stuff ***********************/

const EventRecord* fl_macevent; // the current mac event
ulong fl_event_time; // the last timestamp from an x event
char fl_key_vector[32]; // used by Fl::get_key()

// Record event mouse position and state from an XEvent:

static int px, py;
static ulong ptime;

/** 
 * convert Mac modifiers to FLTK
 */
static void set_shift_states(const EventRecord &macevent) 
{
  ulong state = Fl::e_state & 0xff000000;
  if (macevent.modifiers&shiftKey) state |= FL_SHIFT;
  if ( (macevent.modifiers&controlKey) && (!Button()) ) state |= FL_META; // try to fetch the right mouse button
  if (macevent.modifiers&optionKey) state |= FL_ALT;
  if (macevent.modifiers&cmdKey) state |= FL_CTRL;
  if (macevent.modifiers&alphaLock) state |= FL_CAPS_LOCK;
  state |= FL_NUM_LOCK; //++ always num keypad on Mac? - No, use Fn-F5 on iBooks and  NumLock on regular keyboards
  Fl::e_state = state;
}

/**
 * set the FLTK mouse status variables
 */
static void set_event_xy(const EventRecord &macevent) 
{
#if CONSOLIDATE_MOTION
  send_motion = 0;
#endif
  Fl::e_x_root = macevent.where.h;
  Fl::e_y_root = macevent.where.v;
  Point g = macevent.where;
  GlobalToLocal(&g);
  Fl::e_x = g.h;
  Fl::e_y = g.v;
  if (macevent.what!=osEvt) set_shift_states(macevent);
  fl_event_time = macevent.when;
  if (abs(Fl::e_x_root-px)+abs(Fl::e_y_root-py) > 3 
      || fl_event_time >= ptime+GetDblTime())
    Fl::e_is_click = 0;
}

/**
 * Mac keyboard lookup table
 */
static unsigned short macKeyLookUp[128] = 
{
 'a', 's', 'd', 'f', 'h', 'g', 'z', 'x', 
 'c', 'v', 0, 'b', 'q', 'w', 'e', 'r', 
 
 'y', 't', '1', '2', '3', '4', '6', '5', 
 '=', '9', '7', '-', '8', '0', ']', 'o', 
 
 'u', '[', 'i', 'p', FL_Enter, 'l', 'j', '\'', 
 'k', ';', '\\', ',', '/', 'n', 'm', '.', 
 
 FL_Tab, ' ', '`', FL_BackSpace, 0, FL_Escape, 0, 0, 
 0, 0, 0, 0, 0, 0, 0, 0, 
 
 0, FL_KP+'.', 0, FL_KP+'*', 0, FL_KP+'+', 0, FL_Num_Lock,
 0, 0, 0, FL_KP+'/', FL_KP_Enter, 0, FL_KP+'-', 0, 
 
 0, FL_KP+'=', FL_KP+'0', FL_KP+'1', FL_KP+'2', FL_KP+'3', FL_KP+'4', FL_KP+'5', 
 FL_KP+'6', FL_KP+'7', 0, FL_KP+'8', FL_KP+'9', 0, 0, 0, 
 
 FL_F+5, FL_F+6, FL_F+7, FL_F+3, FL_F+8, FL_F+9, 0, FL_F+11, 
 0, 0, 0, 0, 0, FL_F+10, 0, FL_F+12, 
 
 0, 0, FL_Pause, FL_Home, FL_Page_Up, FL_Delete, FL_F+4, 0, 
 FL_F+2, FL_Page_Down, FL_F+1, FL_Left, FL_Right, FL_Down, FL_Up, 0, 
};


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
 * handle double and triple clicks
 */
static inline void checkdouble() 
{
  if (Fl::e_is_click == Fl::e_keysym) 
    Fl::e_clicks++;
  else {
    Fl::e_clicks = 0;
    Fl::e_is_click = Fl::e_keysym;
  }
  px = Fl::e_x_root;
  py = Fl::e_y_root;
  ptime = fl_event_time;
}



/**
 * user is in the process of resizing the window
 */
void Fl_X::MacGrowWindow(WindowPtr xid, const EventRecord &macevent) 
{
  Fl_Window *win = fl_find(xid);
  if (!win) return;
  while ( win->window() ) win = (Fl_Window*)win->window();
  Rect limit; 
  limit.top = win->minh; limit.left = win->minw;
  limit.bottom = win->maxh?win->maxh:Fl::h(); limit.right = win->maxw?win->maxw:Fl::w();
  unsigned int grow = GrowWindow(xid, macevent.where, &limit);
  if (grow==0) return;
  win->resize(win->x(), win->y(), grow&0xffff, grow>>16);
}        
 

/**
 * user is in the process of resizing the window
 */
void Fl_X::MacDragWindow(WindowPtr xid, const EventRecord &macevent) 
{
  // copied from a Carbon sample file
  Rect tempRect;
  GetRegionBounds(GetGrayRgn(), &tempRect);
  DragWindow(xid, macevent.where, &tempRect);
  /*
  BitMap bm;
  GetQDGlobalsScreenBits(&bm);
  DragWindow(xid, macevent.where, &(bm.bounds));
  */
  Fl_Window *win = fl_find(xid);
  if (!win) return;
  Point pt; pt.h = 0; pt.v = 0;
  SetPort( GetWindowPort(xid) ); SetOrigin(0, 0); LocalToGlobal(&pt);
  win->resize( pt.h, pt.v, win->w(), win->h() );
  //++ win->x(pt.h); win->y(pt.v);    
}


//++ actually this function should be part of 'set_shift_states'
//++ because pressing down SHIFT, then another key while the event queue is full
//++ while actually generate the SHIFT event AFTER the key event!
//++ (at least no events are lost in the process)
int Fl_X::MacModifiers(const EventRecord &macevent, unsigned short prev) 
{
  Fl_Window *window = fl_find(FrontWindow());
  if (!window) return 0;
  Fl::e_length = 0;
  unsigned short now = macevent.modifiers;
  set_shift_states(macevent); //++ will 'break' if multiple keys are released between 0 events
  unsigned short m = now^prev;
  if (m&cmdKey && now&cmdKey) { Fl::e_keysym = FL_Control_L; Fl::handle(FL_KEYBOARD, window); }
  if (m&shiftKey && now&shiftKey) { Fl::e_keysym = FL_Shift_L; Fl::handle(FL_KEYBOARD, window); }
  if (m&optionKey && now&optionKey) { Fl::e_keysym = FL_Alt_L; Fl::handle(FL_KEYBOARD, window); }
  if ( ((m&controlKey)&&(m&btnState)) && ((now&controlKey)&&(now&btnState)) ) { Fl::e_keysym = FL_Meta_L; Fl::handle(FL_KEYBOARD, window); }
  //: caps lock generates keyboard event only on key-down
  if (m&alphaLock) { Fl::e_keysym = FL_Caps_Lock; Fl::handle(FL_KEYBOARD, window); }
 return 1;
}
 

/**
 * Initialize the given port for redraw and call the windw's flush() to actually draw the content
 */ 
void Fl_X::flush()
{
  w->flush();
  SetOrigin( 0, 0 );
  //QDFlushPortBuffer( GetWindowPort(xid), 0 ); // easy way out - remove!
  //printf("DBG: Fl_X::flush\n");
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
  i->wait_for_expose = 0; //++ what about this flag?!
  if ( window->damage() ) {
    if ( i->region ) {
      InvalWindowRgn( xid, i->region );
    }
  }
  if ( i->region ) { // no region, so the sytem will take the update region from the OS
    DisposeRgn( i->region );
    i->region = 0;
  }
//  BeginUpdate( xid );
  
  //DrawControls(xid);  // do we need this?
  //DrawGrowIcon(xid);  // do we need this?
  for ( Fl_X *cx = i->xidChildren; cx; cx = cx->xidNext )
  {
    cx->w->clear_damage(window->damage()|FL_DAMAGE_EXPOSE);
    cx->flush();
    cx->w->clear_damage();
  }
  window->clear_damage(window->damage()|FL_DAMAGE_EXPOSE);
  i->flush();
  window->clear_damage();

//  EndUpdate( xid );
  //QDFlushPortBuffer( GetWindowPort(xid), 0 ); // should not be needed here!
  //printf("DBG: handleUpdate::flush\n");  
  SetPort( oldPort );
}     


/**
 * dispatch all mac events
 */
#ifdef STRICTLY_CARBON  
int fl_handle(const EventRef event) 
{
  EventRecord &macevent = *event;
  UInt32 eventclass, eventkind;
  static char buffer[5];
  static unsigned short prevMod = 0;
  static WindowPtr prevMouseDownXid;
  WindowPtr xid;
//  int event = 0;
  Fl_Window *window = 0L;
  eventclass = GetEventClass(event);
  eventkind  = GetEventKind(event);
  memcpy(buffer, &eventclass, 4);
  buffer[4] = '\0';
  //printf("fl_event(): class = %s, kind = %ld\n", buffer, eventkind);
}
#else
int fl_handle(const EventRecord &macevent) 
{
  static char buffer[2];
  static unsigned short prevMod = 0;
  static WindowPtr prevMouseDownXid;
  WindowPtr xid;
  int event = 0;
  Fl_Window *window = 0L;
  //printMacEvent( macevent );
  switch (macevent.what) 
  {

  case mouseDown: {
    // handle the different mouseDown events in various areas of the screen
    int part = FindWindow(macevent.where, &xid);
    //printf("mousedown in part %d\n", part );
//    prevMouseDownXid = xid;
    switch (part) {
//    case inDesk: break;
    case inMenuBar: HandleMenu(MenuSelect(macevent.where)); break; //++ I just can't get Carbon to handle my menu events :-(
//    case inSysWindow: SystemClick(&macevent, xid); break;
/*    case inContent: {
      if (xid!=FrontWindow()) SelectWindow( xid ); //{ SelectWindow(xid); return 1; }
      window = fl_find(xid);
      if (!window) break;
      SetPort( GetWindowPort(xid) ); SetOrigin(0, 0);
      //printMacEvent( macevent );
      Fl::e_keysym = FL_Button+((macevent.modifiers&0x1000)?3:1); //++ simulate three button using modifiers
      set_event_xy(macevent); checkdouble();
	  Fl::e_state |= ((macevent.modifiers&0x1000)?FL_BUTTON3:FL_BUTTON1);
      return Fl::handle(FL_PUSH, window); }
    case inDrag: Fl_X::MacDragWindow(xid, macevent); break;
    case inGrow: Fl_X::MacGrowWindow(xid, macevent); break;
    case inGoAway:
      if (TrackGoAway(xid, macevent.where)) Fl::handle(FL_CLOSE, fl_find(xid));
      // if there are no more windows, send a high-level quit event
      if (!Fl_X::first) QuitAppleEventHandler( 0, 0, 0 );
      break;
    case inZoomIn: case inZoomOut:
//      if (TrackBox(xid, event.where, part)) DoZoomWindow(xid, part); 
      break;
*/
    } // switch part
    break; } // mouseDown
/*
  case mouseUp: {
      xid = FrontWindow();
      window = fl_find( xid );
      if (!window) break;
      SetPort( GetWindowPort(xid) ); 
      SetOrigin(0, 0);
      Fl::e_keysym = FL_Button+((Fl::e_state&FL_BUTTON1)?1:3); // macevent.modifiers ... 
      set_event_xy(macevent);
      Fl::e_state &= ~(FL_BUTTON1|FL_BUTTON3);
//    if (!Fl::grab()) ReleaseCapture();
      return Fl::handle(FL_RELEASE, window); }
      */
      /*
  case nullEvent: { //: idle events - who came up with that idea?
    if (macevent.modifiers&0xff00 == prevMod) break;
    int ret = Fl_X::MacModifiers(macevent, prevMod);
    prevMod = macevent.modifiers&0xff00;
    return ret; }
    */
    /*
  case keyUp:    
    //: bit0..7 message = keycode, 8..15 virtual, 16..23 ADB
    //:: keyup does NOT GET CALLED in CW debug mode!!
  case keyDown: 
  case autoKey: {
    window = fl_find(FrontWindow());
    if (!window) break;
    unsigned short cc = mac2fltk(macevent.message);
    unsigned char cm = macevent.message;
    Fl::e_keysym = cc;
    set_shift_states(macevent);
    if (macevent.what==keyUp) {
      Fl::e_length = 0; buffer[0] = 0;
    } else {
      Fl::e_text = buffer;
      if (cc<0x100) {
        //++ please check the Mac specific 'option+key' special characters
        //++ handle the control key to generate control characters
        //if (Fl::e_state&FL_CTRL && cm>=32 && cm<64) buffer[0] = cm-32; else 
        buffer[0] = cm;
      } else if (cc>=FL_KP && cc<=FL_KP_Last) {
        buffer[0] = cc-FL_KP; //++ remapped num keys: macevent.message;
      } else {
        buffer[0] = 0;
      }
      if (cc==FL_Escape) buffer[0]=27;
      else if (cc==FL_BackSpace) buffer[0]=0x08;
      else if (cc==FL_Tab) buffer[0]=0x09;
      Fl::e_length = (buffer[0]?1:0);
      return Fl::handle(FL_KEYBOARD, window);
    }
    break;
    }
    */ /*
  case activateEvt:
    window = fl_find((WindowPtr)(macevent.message));
    if (!window) break;
    if (macevent.modifiers & activeFlag) {
      return Fl::handle(FL_FOCUS, window);
    } else { 
      return Fl::handle(FL_UNFOCUS, window);
    }
    break; */
/*  case updateEvt: // done in Carbon
    xid = (WindowPtr)macevent.message;
    if (xid) handleUpdateEvent( xid );
    break; */
/*  case diskEvt:
    break; */
/*  case osEvt: //: contains mouse move events
    switch ( (macevent.message>>24)&0xff ) {
 	case suspendResumeMessage: 
      window = fl_find(FrontWindow());
      if (!window) break;
      SetEventMask(everyEvent); //++ currentMask | keyUpEvent
      //++ mac users will expect that all windows switch to inactive
      if (macevent.message & resumeFlag) {
        return Fl::handle(FL_FOCUS, window);
      } else { 
        return Fl::handle(FL_UNFOCUS, window);
      }
      break;
    case mouseMovedMessage:
      if (Fl::e_x_root==macevent.where.h && Fl::e_y_root==macevent.where.v) break;
      xid = FrontWindow();
      window = fl_find( xid );
      if (!window) break;
      SetPort( GetWindowPort(xid) ); SetOrigin(0, 0);
      set_event_xy(macevent);
      #if CONSOLIDATE_MOTION
        send_motion = fl_xmousewin = window;
        return 0;
      #else
        return Fl::handle( Button()?FL_DRAG:FL_MOVE, window); 
      #endif
//      if (!Fl::grab()) ReleaseCapture();
    }
    break; */
  //++ get null events to grab changes in the modifier keys (shift down, etc.)
  case kHighLevelEvent:
    //AEProcessAppleEvent(&macevent);
    break;
  }
#endif // 0
  return 1; 
}

////////////////////////////////////////////////////////////////
// This function gets the dimensions of the top/left borders and
// the title bar, if there is one, based on the FL_BORDER, FL_MODAL
// and FL_NONMODAL flags, and on the window's size range.
// It returns the following values:
//
// value | border | title bar
//   0   |  none  |   no
//   1   |  fix   |   yes
//   2   |  size  |   yes
//   3   | dialog |  dialog  (currently not used)

int Fl_X::fake_X_wm(const Fl_Window* w,int &X,int &Y, int &bt,int &bx, int &by) {
  int W, H, xoff, yoff, dx, dy;
  int ret = bx = by = bt = 0;
  if (w->border() && !w->parent()) {
    if (w->maxw != w->minw || w->maxh != w->minh) {
      ret = 2;
      bx = 6; //++ GetSystemMetrics(SM_CXSIZEFRAME);
      by = 6; //++ get Mac window frame size GetSystemMetrics(SM_CYSIZEFRAME);
    } else {
      ret = 1;
      bx = 6; //++ GetSystemMetrics(SM_CXFIXEDFRAME);
      by = 6; //++ GetSystemMetrics(SM_CYFIXEDFRAME);
    }
    bt = 22; //++ GetSystemMetrics(SM_CYCAPTION);
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


////////////////////////////////////////////////////////////////
// Innards of Fl_Window::create()

bool fl_show_iconic;		//++ Implement: true if called from iconize()
int fl_disable_transient_for;	// secret method of removing TRANSIENT_FOR
const Fl_Window* fl_modal_for;	// parent of modal() window

/**
 * go ahead, create that (sub)window
 */
void Fl_X::make(Fl_Window* w)
{
  static int xyPos = 24;
  if ( w->parent() ) // create a subwindow
  {
    Fl_Group::current(0);
//    int xp = w->x();
//    int yp = w->y();
//    int wp = w->w();
//    int hp = w->h();
    //++ now we have to do completely different stuff here!
    //++ mac has no concept of a window in a window!

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
    x->xidNext = xo->xidChildren;
    x->xidChildren = 0L;
    xo->xidChildren = x;
    x->xid = fl_xid(win);
    x->w = w; w->i = x;
    x->wait_for_expose = 0;
    x->next = Fl_X::first; // must be in the list for ::flush()
    Fl_X::first = x;
    w->set_visible();
    w->handle(FL_SHOW);
    w->redraw(); // force draw to happen
    fl_show_iconic = 0;
    //++ hmmm, this should maybe set by the activate event?!
    Fl::handle(FL_FOCUS, w);
    //++ if (w->modal()) { Fl::modal_ = w; fl_fix_focus(); }
    // ;;;; printf("Created subwindow %08x (%08x)\n", w, x->xid );
  }
  else // create a desktop window
  {
    Fl_Group::current(0);
    fl_open_display();
    int winclass = kDocumentWindowClass;
    int winattr = kWindowCloseBoxAttribute 
                | kWindowCollapseBoxAttribute 
                | kWindowLiveResizeAttribute // activate this as soon as we ported to Carbon Events!
                | kWindowStandardHandlerAttribute
                //| kWindowInWindowMenuAttribute
                ;
//    int winattr = kWindowStandardHandlerAttribute;
//    int winattr = 0;
    int xp = w->x();
    int yp = w->y();
    int wp = w->w();
    int hp = w->h();
    if (w->size_range_set) {
      winattr |= kWindowFullZoomAttribute | kWindowResizableAttribute;
    } else {
      if (w->resizable()) {
        Fl_Widget *o = w->resizable();
        int minw = o->w(); if (minw > 100) minw = 100;
        int minh = o->h(); if (minh > 100) minh = 100;
        w->size_range(w->w() - o->w() + minw, w->h() - o->h() + minh, 0, 0);
        winattr |= kWindowFullZoomAttribute | kWindowResizableAttribute;
        //winattr |= kWindowFullZoomAttribute;
      } else {
        w->size_range(w->w(), w->h(), w->w(), w->h());
      }
    }
    int xwm = xp, ywm = yp, bt, bx, by;
    if (!fake_X_wm(w, xwm, ywm, bt, bx, by)) 
      { winclass = kHelpWindowClass; winattr = 0; }
    else if (w->modal()) 
      winclass = kFloatingWindowClass; // basically fine, but not modal! The modal window however does nor show
    else if (w->non_modal()) 
      winclass = kFloatingWindowClass; // we need to call 'InitFloatingWindows for OS 8, 9
    if (by+bt) {
      //++ if (!w->non_modal()) style |= WS_SYSMENU | WS_MINIMIZEBOX;
      wp += 2*bx;
      hp += 2*by+bt;
    }
    if (!(w->flags() & Fl_Window::FL_FORCE_POSITION)) {
      w->x(xyPos+Fl::x()); w->y(xyPos+Fl::y()); //+ there is a Carbon function for default window positioning
      xyPos += 24;
      if (xyPos>200) xyPos = 24;
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
      while (w->parent()) w = w->window();
      //++parent = fl_xid(w);
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
    x->other_xid = 0; // room for doublebuffering image map //++ OS X: the OS always doublebuffers!
    x->region = 0;
    x->subRegion = 0;
    x->cursor = fl_default_cursor;
    x->xidChildren = 0;
    x->xidNext = 0;
    CreateNewWindow( winclass, winattr, &wRect, &(x->xid) );
    SetWTitle(x->xid, pTitle);
    x->w = w; w->i = x;
    x->wait_for_expose = 1;
    x->next = Fl_X::first;
    Fl_X::first = x;
    if (w->resizable()) DrawGrowIcon(x->xid);
    w->set_visible();
    // add event handlers
    #ifdef TARGET_API_MAC_CARBON
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
    }
    #endif

    if ( ! Fl_X::first->next ) // if this is the first window, we need to bring the application to the front //++ this fails if the first window is a child window...
    { 
      ProcessSerialNumber psn;
      OSErr err = GetCurrentProcess( &psn );
      if ( err==noErr ) SetFrontProcess( &psn );
      // or 'BringToFront'
    }

    w->handle(FL_SHOW);
    w->redraw(); // force draw to happen

    //TransitionWindow( x->xid, kWindowZoomTransitionEffect, kWindowShowTransitionAction, 0 );
    ShowWindow( x->xid );

    fl_show_iconic = 0;
    //++ hmmm, this should maybe set by the activate event?!
    Fl::handle(FL_FOCUS, w);
    //++ if (w->modal()) { Fl::modal_ = w; fl_fix_focus(); }
    //;;;; printf("Created top level window %08x (%08x)\n", w, x->xid );
  }
}

void Fl_Window::size_range_() {
  size_range_set = 1;
}

////////////////////////////////////////////////////////////////

//++ make this run with Unix filenames
// returns pointer to the filename, or null if name ends with ':'
const char *filename_name( const char *name ) 
{
  const char *p, *q;
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

////////////////////////////////////////////////////////////////
// Implement the virtual functions for the base Fl_Window class:

// Display can *look* faster (it isn't really faster) if X's background
// color is used to erase the window.  In fltk 2.0 the only way to
// prevent this is to set the box to FL_NO_BOX.
//
// Drawing should really be faster if FL_FRAME_ONLY is passed to the
// box drawing function, since X has already erased the interior.  But
// on XFree86 (and prehaps all X's) this has a problem if the window
// is resized while a save-behind window is atop it.  The previous
// contents are restored to the area, but this assummes the area is
// cleared to background color.  So I had to give up on this...
/*
void Fl_Window::create() {
  Fl_X::create(this);
}
*/
Window fl_window;
Fl_Window *Fl_Window::current_;


void Fl_Window::show() {
  if (!shown() || !i) {
    Fl_X::make(this);
  } else {
      if ( !parent() )
      {
        if ( IsWindowCollapsed( i->xid ) ) CollapseWindow( i->xid, false );
        //++ do we need to do grab and icon handling here?
        if (!fl_capture) //++ Do we need this? It should keep the mouse modal window in front
          BringToFront(i->xid);
      }
  }
}


void Fl_Window::resize(int X,int Y,int W,int H) {
  int is_a_resize = (W != w() || H != h());
  if (X != x() || Y != y()) set_flag(FL_FORCE_POSITION);
  else if (!is_a_resize) return;
  // change the viewport first, so children (namely OpenGL) can resize correctly
  if ( (resize_from_system!=this) && (!parent()) && shown()) {
    MoveWindow(i->xid, X, Y, 0);
    if (is_a_resize) {
      SizeWindow(i->xid, W>0 ? W : 1, H>0 ? H : 1, 1);
      Rect all; all.top=-32000; all.bottom=32000; all.left=-32000; all.right=32000;
      InvalWindowRect( i->xid, &all );    
    }
  }
  resize_from_system = 0;
  if (is_a_resize) {
    Fl_Group::resize(X,Y,W,H);
    if (shown()) { redraw(); if (!parent()) i->wait_for_expose = 1; }
  } else {
    x(X); y(Y); 
  }
}

Fl_Region fl_window_region = 0;

/**
 * make all drawing go into this window (called by subclass flush() impl.)
 */
void Fl_Window::make_current() 
{
  if ( !fl_window_region )
    fl_window_region = NewRgn();
  fl_window = i->xid;
  current_ = this;

  SetPort( GetWindowPort(i->xid) );

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
  SetOrigin( -xp, -yp ); //++ how do we handle doublebuffering here?
  
  //+++ here we should set the clip to all except the subwindows
  //++ first of all, there seems to be some leftovers from an old operation
  SetRectRgn( fl_window_region, 0, 0, w(), h() );
  // remove all subwindows from the clip
  
  //+++ for performance reasons: we don't have to create this unless the child windows moved
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
  //CopyRgn( fl_window_region, GetPortClipRegion( GetWindowPort(i->xid), 0) ); // for Fl_GL_Window
  return;
}


/**
 * This block contains a collection of ideas to improve and carbonize the Mac port
 *
 * I found a note stating that in order to receive the
 * MouseUp event I need to set some SystemEvent Mask. 
 * Although I do receive them, it might still be smart to look into this!
 *
 * I have to solve the subwindow linking problem (links
 * get corrupted when widows get reparented or closed)
 *
 * The current subwindow inking allows only one level of 
 * subwindowing (which was correct for the first layout).
 * I have to change it for arbitrary depths.
 *
 * There is supposedly a nice library out for Berkeley sockets.
 * Check out www.iis.ee.ethz.ch/~neeri/macintosh/gusi-qa.html
 * (Those Swiss people in Zuerich have the longest links :-)
 *
 * Alternative timers:
 * - GetTicks(): TickCount(): simple, unreliable, 60th of a second
 * - Microseconds(): about 1ms
 * - UpTime(): nano second resultion, only PCI machines (ah well)
 */

/* ---- sample hires timer (up to 20 microseconds!)
double MicrosecondToDouble(register const UnsignedWide *epochPtr)
{
   register double    result;

   result = (((double) epochPtr->hi) * kTwoPower32) + epochPtr->lo;
   return (result);
}

void MicrosecondDelta(register const UnsignedWide   *startPtr,
                      register const UnsignedWide   *endPtr,
                      register SignedWide           *resultPtr)
{
   if (endPtr->lo >= startPtr->lo)
      resultPtr->hi = endPtr->hi - startPtr->hi;
   else 
      resultPtr->hi = (endPtr->hi - 1) - startPtr->hi;
      
   resultPtr->lo = endPtr->lo - startPtr->lo;
}
*/

/* ---- sample UpTime() timer
AbsoluteTime   startTime;
AbsoluteTime   endTime;
AbsoluteTime   elapsedTime;
Nanoseconds    elapsedNanoseconds;

startTime = UpTime();
DoMyOperation();
endTime = UpTime();
elapsedTime = SubAbsoluteFromAbsolute(endTime, startTime);
elapsedNanoseconds = AbsoluteToNanoseconds(elapsedTime);

------ window classes:
    // classes:
    // kAlertWindowClass: small up frame - nice
    // kModalWindowClass: as above
    // kFloatingWindowClass: does not deactivate app window, but has small title bar (medium decoration)
    // kDocumentWindowClass: transparent huge upper title (large decoration) -- last standard definition
    // kUtilityWindowClass: like 'floating (small decoration)
    // kHelpWindowClass: perfect: no decoration, keeps master active, stays on top of ALL windows, not modal though
    // kSheetWindowClass: no deco, deactivates parent
    // kToolbarWindowClass: no deco, passive, under other menues
    // kPlainWindowClass: no deco, active, under
    // kOverlayWindowClass: invisible!
    // kSheetAlertWindowClass: no deco, active, under
    // kAltPlainWindowClass: no deco, active, under
    // attributes:
    // kWindowCloseBoxAttribute, HorizontalZoom, VerticalZoom, FullZoom, CollapsBox, Resizable,
    // SideTitlebar(floatin only), NoUpdates, NoActivates, Macros: StandardDocument, StandardFloating


*/

//++ when using OpenGL in a Mach-O executable and include<aglMacro.h>
//++ we MUST call aglConfigure(AGL_TARGET_OS_MAC_OSX, GL_TRUE);

//
// End of "$Id: Fl_mac.cxx,v 1.1.2.8 2001/12/20 05:27:14 matthiaswm Exp $".
//

