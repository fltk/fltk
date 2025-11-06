//
// Definition of macOS Cocoa Pen/Tablet event driver.
//
// Copyright 2025 by Bill Spitzak and others.
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


#include <config.h>
#include <FL/platform.H>
#include <FL/core/pen_events.H>
#include <FL/Fl.H>

#import <Cocoa/Cocoa.h>

class Fl_Widget;

/*

 - [ ] Add a class "subscriber" that watches the subscribed widget.
 - [ ] Cocoa event handler must forward Pen events here.
 - [ ] Find out in which widget this event happens.
 - [ ] Create FLTK events.
 - [ ] Write test program to verify functionality

 */


namespace Fl {

namespace Pen {

// double e_pressure_;

} // namespace Pen

} // namespace Fl


using namespace Fl::Pen;


#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_9
/*
 Return true if we handled this event, or false if we want cocoaMouseHandler
 to handle this event.

 \note The caller must call fl_lock_function() before calling this handler
 and must also call fl_unlock_function() when we return.
 */
bool fl_cocoa_tablet_handler(NSEvent *theEvent) {

//  Fl_Window *window = (Fl_Window*)[(FLWindow*)[theEvent window] getFl_Window];
//  if (!window || !window->shown() ) {
//    fl_unlock_function();
//    return;
//  }
//
//  NSPoint pos = [theEvent locationInWindow];
//  float s = Fl::screen_driver()->scale(0);
//  pos.x /= s; pos.y /= s;
//  pos.y = window->h() - pos.y;

  // https://developer.apple.com/documentation/appkit/nsevent/pointingdevicetype-swift.property?language=objc
  static int e = 0;
  if (theEvent.type == NSEventTypeTabletPoint) {
    printf("Some Tablet Event: NSEventTypeTabletPoint %d\n", e);
  } else if (theEvent.type == NSEventTypeTabletProximity) {
    printf("Some Tablet Event: NSEventTypeTabletProximity %d\n", e);
  } else if ([theEvent subtype] == NSEventSubtypeTabletProximity) {
    printf("Some Tablet Event: NSEventSubtypeTabletProximity (%lu) %d\n", static_cast<unsigned long>([theEvent type]), e);
  } else if ([theEvent subtype] == NSEventSubtypeTabletPoint) {
    if (theEvent.type == NSEventTypeMouseMoved /* 5 */) return false;
    if (theEvent.type == NSEventTypeLeftMouseDragged /* 6 */) return false;
    printf("Some Tablet Event: NSEventSubtypeTabletPoint (%lu) %d\n", static_cast<unsigned long>([theEvent type]), e);
  } else {
    printf("Some Tablet Event: HUH?\n");
  }
  if ((theEvent.type == NSEventTypeTabletProximity) || ([theEvent subtype] == NSEventSubtypeTabletProximity)) {
    //    Event come in from anywhere on the tablet, but only if our window is on top
    // type proximity:
    //    deviceID 29949, vendorID, tabletID, pointingDeviceID, systemTabletID, vendorPointingDeviceType all 0
    //    capabilityMask: 0x000005c7
    //    pointingDeviceType: 1 for tip, 3 for eraser (not reliable), 0 = unknown, 2 = cursor
    //    isEnteringProximity: 1
    // then type 5, subtype Proximity, twice, NSEventTypeMouseMoved, only if pen entered over window
    // enum pointerType
    // -- mouse
    // NSPoint locationInWindow
    // NSUInteger deviceID
    printf("  deviceID: %ld\n", static_cast<long>([theEvent deviceID]));
    // NSUInteger vendorID
    printf("  vendorID: %ld\n", static_cast<long>([theEvent vendorID]));
    // NSUInteger tabletID
    printf("  tabletID: %ld\n", static_cast<long>([theEvent tabletID]));
    // NSUInteger pointingDeviceID
    printf("  pointingDeviceID: %ld\n", static_cast<long>([theEvent pointingDeviceID]));
    // NSUInteger systemTabletID
    printf("  systemTabletID: %ld\n", static_cast<long>([theEvent systemTabletID]));
    // NSUInteger vendorPointingDeviceType
    printf("  vendorPointingDeviceType: %ld\n", static_cast<long>([theEvent vendorPointingDeviceType]));
    // NSUInteger pointingDeviceSerialNumber
    // unsigned long long uniqueID
    // NSUInteger capabilityMask
    printf("  capabilityMask: 0x%08lx\n", static_cast<unsigned long>([theEvent capabilityMask]));
    // NSPointingDeviceType pointingDeviceType
    printf("  pointingDeviceType: %ld\n", static_cast<long>([theEvent pointingDeviceType]));
    // BOOL isEnteringProximity
    printf("  isEnteringProximity: %ld\n", static_cast<long>([theEvent isEnteringProximity]));
    printf("  Button Mask: 0x%08lx\n", static_cast<unsigned long>([theEvent buttonMask]));
    printf("  Button Number: %ld\n", static_cast<long>([theEvent buttonNumber]));

  } else if ((theEvent.type == NSEventTypeTabletPoint) || (theEvent.subtype == NSEventSubtypeTabletPoint)) {
    //    Events come only in when inside window, no need to be in the front, but only for visible areas
    // type 5 when hovering, 6 when touching
    //    pressure goes from 0..1
    //    tilt x from -.7 to +.7 (pen left to right), : pen to user to pen to screen
    //    button mask is 1 for tip and eraser.
    //    positionInWindow is set, absoluteXYZ is not
    // NSEventTypeLeftMouseDown             = 1,
    //    buttonMask is already 1, rest as above, same for eraser
    // NSEventTypeLeftMouseUp               = 2,
    //    buttonMask is already 0, rest as above, same for eraser
    // With front barrel button: all is right mouse instead (3, 4, 7)
    //    NSEventTypeRightMouseDown            = 3,
    //    NSEventTypeRightMouseUp              = 4,
    //    NSEventTypeRightMouseDragged         = 7,
    // With rear barrel button
    //    NSEventTypeOtherMouseDown            = 25,
    //    NSEventTypeOtherMouseUp              = 26,
    // holding the front barrel button causes type 7 and buttonMask 2
    // holding the rear barrel button causes type 27 and buttonMask 4

    // enum buttonMask
    printf("  Button Mask: 0x%08lx\n", static_cast<unsigned long>([theEvent buttonMask]));
    printf("  Button Number: %ld\n", static_cast<long>([theEvent buttonNumber]));
    //printf("  pointingDeviceType: %ld\n", static_cast<long>([theEvent pointingDeviceType])); // not valid here
    // modifierFlags
    // mouseLocation
    // clickCount
    // float pressure
    printf("  Pressure: %g\n", [theEvent pressure]);
    // float tangentialPressure
    printf("  Tangential Pressure: %g\n", [theEvent tangentialPressure]);
    // NSUInteger deviceID
    printf("  Device ID: 0x%08lx\n", static_cast<unsigned long>([theEvent deviceID]));
    // float rotation
    printf("  Rotation: %g\n", [theEvent rotation]);
    // NSInteger absoluteX/Y/Z "at full tablet resolution"
    printf("  Absolute X Y Z: %ld %ld %ld\n", [theEvent absoluteX], [theEvent absoluteY], [theEvent absoluteZ]);
    // NSPoint tilt
    printf("  Tilt: %g %g\n", [theEvent tilt].x, [theEvent tilt].y);
    // window
    // locationInWindow
    printf("  In Window: %f %f\n", [theEvent locationInWindow].x, [theEvent locationInWindow].y);
    // pointingDeviceType, pointingDeviceID, deltaX/Y/Z, data
  } else {
  }
  e++;
  return false;
}

#endif


Trait Fl::Pen::driver_traits() { return static_cast<Trait>(0); }

Trait Fl::Pen::pen_traits(Fl_Window *window) { return static_cast<Trait>(0); }

void Fl::Pen::subscribe(Fl_Widget* widget) { }

void Fl::Pen::unsubscribe(Fl_Widget* widget) { }

void Fl::Pen::grab(Fl_Widget* widget) { }

void Fl::Pen::release() { }

double Fl::Pen::event_x() { return 0.0; }

double Fl::Pen::event_y() { return 0.0; }

double Fl::Pen::event_x_root() { return 0.0; }

double Fl::Pen::event_y_root() { return 0.0; }

int Fl::Pen::event_id() { return 0; }

double Fl::Pen::event_pressure() { return 1.0; }

double Fl::Pen::event_barrel_pressure() { return 0.0; }

double Fl::Pen::event_tilt_x() { return 0.0; }

double Fl::Pen::event_tilt_y() { return 0.0; }

double Fl::Pen::event_twist() { return 0.0; }

double Fl::Pen::event_proximity() { return 0.0; }

State Fl::Pen::event_state();

State Fl::Pen::event_trigger();


#if 0

Notes:

// For system-wide event monitoring:
id eventMonitor = [NSEvent addGlobalMonitorForEventsMatchingMask:
                   NSEventMaskTabletPoint | NSEventMaskTabletProximity
                                                         handler:^(NSEvent *event);
// Remove when done
[NSEvent removeMonitor:eventMonitor];

// CHeck if a driver supports these values (check again when pen changes?)
[NSEvent respondsToSelector:@selector(pressure)] // tilt, etc.

#endif



