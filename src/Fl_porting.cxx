//
// "$Id$"
//
// core code stubs for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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

#ifndef FL_DOXYGEN
#include <FL/Fl.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Paged_Device.H>
#include "flstring.h"
#include "Fl_Font.H"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

extern unsigned int fl_codepage;

Fl_Fontdesc* fl_fonts = NULL;

void *fl_gc = NULL;

Window fl_window = NULL;

void fl_reset_spot()
{
#  pragma message "FL_PORTING: implement fl_reset_spot"
}

void fl_set_spot(int font, int size, int X, int Y, int W, int H, Fl_Window *win)
{
#  pragma message "FL_PORTING: implement fl_set_spot"
}

void fl_set_status(int x, int y, int w, int h)
{
#  pragma message "FL_PORTING: implement fl_set_status"
}

void Fl::add_fd(int n, int events, void (*cb)(FL_SOCKET, void*), void *v) {
#  pragma message "FL_PORTING: implement Fl::add_fd"
}

void Fl::add_fd(int fd, void (*cb)(FL_SOCKET, void*), void* v) {
#  pragma message "FL_PORTING: implement Fl::add_fd"
}

void Fl::remove_fd(int n, int events) {
#  pragma message "FL_PORTING: implement Fl::remove_fd"
}

void Fl::remove_fd(int n) {
#  pragma message "FL_PORTING: implement Fl::remove_fd"
}

static void nothing() {}
void (*fl_lock_function)() = nothing;
void (*fl_unlock_function)() = nothing;

//void* Fl::thread_message() {
//#  pragma message "FL_PORTING: implement Fl::thread_message"
//}

int fl_wait(double time_to_wait) {
#  pragma message "FL_PORTING: implement fl_wait"
  Fl::flush();
  return 0;
}

int fl_ready() {
#  pragma message "FL_PORTING: implement fl_ready"
  return 0;
}

void fl_open_display() {
#  pragma message "FL_PORTING: implement fl_open_display()"
}

void Fl::enable_im() {
#  pragma message "FL_PORTING: implement Fl::enable_im"
}

void Fl::disable_im() {
#  pragma message "FL_PORTING: implement Fl::disable_im"
}

int Fl::x()
{
#  pragma message "FL_PORTING: implement Fl::x"
  return 0;
}

int Fl::y()
{
#  pragma message "FL_PORTING: implement Fl::y"
  return 0;
}

int Fl::h()
{
#  pragma message "FL_PORTING: implement Fl::h"
  return 600;
}

int Fl::w()
{
#  pragma message "FL_PORTING: implement Fl::w"
  return 800;
}

char *fl_selection_buffer[2];
int fl_selection_length[2];
int fl_selection_buffer_length[2];
char fl_i_own_selection[2];

void fl_update_clipboard(void) {
#  pragma message "FL_PORTING: implement fl_update_clipboard"
}

// call this when you create a selection:
void Fl::copy(const char *stuff, int len, int clipboard, const char *type) {
#  pragma message "FL_PORTING: implement Fl::copy"
}

// Call this when a "paste" operation happens:
void Fl::paste(Fl_Widget &receiver, int clipboard, const char *type) {
#  pragma message "FL_PORTING: implement Fl::paste"
}

int Fl::clipboard_contains(const char *type)
{
#  pragma message "FL_PORTING: implement Fl::clipboard_contains"
  return 0;
}

void fl_get_codepage()
{
#  pragma message "FL_PORTING: implement fl_get_codepage"
}

int Fl_X::fake_X_wm(const Fl_Window* w,int &X,int &Y, int &bt,int &bx, int &by) {
#  pragma message "FL_PORTING: don't ask. We do't know either..."
  return 0;
}

void Fl_Window::resize(int X,int Y,int W,int H) {
#  pragma message "FL_PORTING: implement Fl_Window::resize"
}

//void Fl_X::make_fullscreen(int X, int Y, int W, int H) {
//#  pragma message "FL_PORTING: implement Fl_X::make_fullscreen"
//}

void Fl_Window::fullscreen_x() {
#  pragma message "FL_PORTING: implement Fl_Window::fullscreen_x"
}

void Fl_Window::fullscreen_off_x(int X, int Y, int W, int H) {
#  pragma message "FL_PORTING: implement Fl_Window::fullscreen_off_x"
}

void fl_fix_focus(); // in Fl.cxx

char fl_show_iconic;
int fl_disable_transient_for; // secret method of removing TRANSIENT_FOR

Fl_X* Fl_X::make(Fl_Window* w) {
#  pragma message "FL_PORTING: implement Fl_X::make"
  return 0;
}

void Fl::add_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
#  pragma message "FL_PORTING: implement Fl::add_timeout"
}

void Fl::repeat_timeout(double time, Fl_Timeout_Handler cb, void* data)
{
#  pragma message "FL_PORTING: implement Fl::repeat_timeout"
}

int Fl::has_timeout(Fl_Timeout_Handler cb, void* data)
{
#  pragma message "FL_PORTING: implement Fl::has_timeout"
  return 0;
}

void Fl::remove_timeout(Fl_Timeout_Handler cb, void* data)
{
#  pragma message "FL_PORTING: implement Fl::remove_timeout"
}

void Fl_Window::size_range_() {
  size_range_set = 1;
}

#include <FL/filename.H> // need so FL_EXPORT fl_filename_name works

// returns pointer to the filename, or null if name ends with '/'
const char *fl_filename_name(const char *name) {
#  pragma message "FL_PORTING: implement fl_filename_name"
  return 0;
}

void Fl_Window::label(const char *name,const char *iname) {
#  pragma message "FL_PORTING: implement Fl_Window::label"
}

void Fl_X::set_default_icons(const Fl_RGB_Image *icons[], int count) {
#  pragma message "FL_PORTING: implement Fl_X::set_defult_icons"
}

void Fl_X::set_icons() {
#  pragma message "FL_PORTING: implement Fl_X::set_icons"
}

int Fl_X::set_cursor(Fl_Cursor c) {
#  pragma message "FL_PORTING: implement Fl_X::set_cursor"
  return 1;
}

int Fl_X::set_cursor(const Fl_RGB_Image *image, int hotx, int hoty) {
#  pragma message "FL_PORTING: implement Fl_X::set_cursor"
  return 1;
}

void Fl_Window::show() {
#  pragma message "FL_PORTING: implement Fl_Window::show"
}

Fl_Window *Fl_Window::current_;

void Fl_Window::make_current() {
#  pragma message "FL_PORTING: implement Fl_Window::make_current"
}

void fl_free_fonts(void)
{
#  pragma message "FL_PORTING: implement fl_free_fonts"
}


//Fl_Region XRectangleRegion(int x, int y, int w, int h) {
//}

FL_EXPORT Window fl_xid_(const Fl_Window *w) {
  Fl_X *temp = Fl_X::i(w);
  return temp ? temp->xid : 0;
}

int Fl_Window::decorated_w()
{
  return w();
}

int Fl_Window::decorated_h()
{
  return h();
}

void Fl_Paged_Device::print_window(Fl_Window *win, int x_offset, int y_offset)
{
#  pragma message "FL_PORTING: implement Fl_Paged_Device::print_window"
}

//void Fl_Paged_Device::draw_decorated_window(Fl_Window *win, int x_offset, int y_offset, Fl_Surface_Device *toset)
//{
//}

Fl_Bitmask fl_create_bitmask(int w, int h, const uchar *data)
{
#  pragma message "FL_PORTING: implement fl_create_bitmask"
  return 0;
}

//Fl_Bitmask fl_create_alphamask(int w, int h, int d, int ld, const uchar *data)
//{
//#  pragma message "FL_PORTING: implement fl_create_alphamask"
//  return 0;
//}

void fl_delete_bitmask(Fl_Bitmask bm)
{
#  pragma message "FL_PORTING: implement fl_delete_bitmask"
}

Fl_Offscreen fl_create_offscreen(int w, int h)
{
#  pragma message "FL_PORTING: implement fl_create_offscreen"
  return 0;
}

//void fl_copy_offscreen(int x,int y,int w,int h, Fl_Offscreen gWorld, int srcx,int srcy)
//{
//#  pragma message "FL_PORTING: implement fl_copy_offscreen"
//}

void fl_delete_offscreen(Fl_Offscreen gWorld)
{
#  pragma message "FL_PORTING: implement fl_delete_offscreen"
}

void fl_begin_offscreen(Fl_Offscreen gWorld)
{
#  pragma message "FL_PORTING: implement fl_begin_offscreen"
}

void fl_end_offscreen()
{
#  pragma message "FL_PORTING: implement fl_end_offscreen"
}

void fl_clipboard_notify_change() {
#  pragma message "FL_PORTING: implement fl_clipboard_notify_change"
}

Fl_Font_Descriptor::~Fl_Font_Descriptor()
{
#  pragma message "FL_PORTING: implement Fl_Font_Descriptor::~Fl_Font_Descriptor"
}

int Fl::dnd()
{
#  pragma message "FL_PORTING: implement Fl::dnd"
  return 0;
}

void Fl::get_mouse(int &x, int &y)
{
#  pragma message "FL_PORTING: implement Fl::get_mouse"
  x = 0; y = 0;
}

#endif // FL_DOXYGEN

//
// End of "$Id$".
//
