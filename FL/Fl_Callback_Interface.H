//
// Callback template header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 2026 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file. If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#ifndef Fl_Callback_Interface_H
#define Fl_Callback_Interface_H

#include "Fl.H"

#include <functional>


#ifndef Fl_Callback_User_Data_H
#define Fl_Callback_User_Data_H

/** Callback function type that takes a Fl_Callback_User_Data pointer as its argument. */
typedef void (Fl_Callback_User_Data_Dtor)(class Fl_Callback_User_Data*);

/** A class prototype that allows for additional data in callbacks.

 Users can derive this class and pass objects of such derived classes to widget callbacks.
 Widgets can take ownership of the callback data, deleting the data when the widget
 itself is deleted.

 The destructor of this class is virtual, allowing for additional code to
 deallocate resources when the user data is deleted.

 \see FL_FUNCTION_CALLBACK_3, FL_METHOD_CALLBACK_1, FL_INLINE_CALLBACK_2
 \see Fl_Widget::callback(Fl_Callback*, Fl_Callback_User_Data*, bool)
 \see Fl_Widget::user_data(Fl_Callback_User_Data*, bool)
 */
class FL_EXPORT Fl_Callback_User_Data {
protected:
  Fl_Callback_User_Data() {} ///< Protected constructor forces user to derive from this class
public:
  static void default_dtor_callback(Fl_Callback_User_Data* self) {
    delete self;
  }
  virtual ~Fl_Callback_User_Data() { } ///< Destructor
};

#endif // Fl_Callback_User_Data_H


/**
 Add callback functionality to the class T.

 `callback_is_stdf()`, `set_callback_stdf()`, and `clear_callback_stdf()`.

 \param T The class to add callback functionality to.
 */
template <typename T>
class Fl_Callback_Interface
{
  // -- types
  typedef void (Callback_voidp)(T*, void*);
  typedef void (Callback_long)(T*, long);
  typedef void (Callback_empty)(T*);
  typedef void (Callback_flud)(T*, Fl_Callback_User_Data*);

  // -- implementation calls to avoid naming conflicts
  class Callback_Interface {
  public:
    Callback_voidp* callback_ = nullptr;
    Fl_Callback_User_Data_Dtor* dtor_callback_ = nullptr;
    void* user_data_ = nullptr;

    Callback_Interface() = default;
    ~Callback_Interface() {
      clear_user_data();
    }
    void do_dtor_callback() {
      if (dtor_callback_)
        dtor_callback_((Fl_Callback_User_Data*)user_data_);
    }
    void clear_user_data() {
      do_dtor_callback();
      dtor_callback_ = nullptr;
      user_data_ = nullptr;
    }
  };
  Callback_Interface callback_interface_;

protected:
  // -- ctor and dtor
  Fl_Callback_Interface() = default;
  ~Fl_Callback_Interface() = default;

public:
  // -- public interface
  void callback(Callback_voidp* cb, void* p) {
    callback_interface_.callback_ = cb;
    user_data(p);
  }
  void callback(Callback_voidp* cb, Fl_Callback_User_Data* p, bool auto_free) {
    callback_interface_.callback_ = cb;
    user_data(p, auto_free);
  }
  void callback(Callback_voidp* cb, Fl_Callback_User_Data* p, Fl_Callback_User_Data_Dtor* dtor) {
    callback_interface_.callback_ = cb;
    user_data(p, dtor);
  }
  void callback(Callback_voidp* cb){
    callback_interface_.callback_ = cb;
    user_data(nullptr); // original does not clear user_data!
  }
  void callback(Callback_empty* cb) {
    callback_interface_.callback_ = (Callback_voidp*)(fl_intptr_t)(cb);
    user_data(nullptr); // original does not clear user_data!
  }
  void callback(Callback_long* cb, long p = 0) {
    callback_interface_.callback_ = (Callback_voidp*)(fl_intptr_t)(cb);
    user_data(fl_voidptr(p));
  }
  void user_data(void* v) {
    callback_interface_.clear_user_data();
    callback_interface_.user_data_ = v;
  }
  void user_data(Fl_Callback_User_Data* v, bool auto_free) {
    user_data((void*)v);
    if (auto_free)
      callback_interface_.dtor_callback_ = Fl_Callback_User_Data::default_dtor_callback;
  }
  void user_data(Fl_Callback_User_Data* v, Fl_Callback_User_Data_Dtor* dtor) {
    user_data((void*)v);
    callback_interface_.dtor_callback_ = dtor;
  }
  void argument(long v) {
    user_data(fl_voidptr(v));
  }
  // -- Callback getters
  Callback_voidp* callback() const {
    return callback_interface_.callback_;
  }
  void* user_data() const {
    return callback_interface_.user_data_;
  }
  long argument() const {
    return fl_long(callback_interface_.user_data_);
  }
  // -- callback actions
  void do_callback(Fl_Callback_Reason reason=FL_REASON_UNKNOWN) {
    static_cast<T*>(this)->do_callback(static_cast<T*>(this), callback_interface_.user_data_, reason);
  }
  void do_callback(T *widget, long arg, Fl_Callback_Reason reason=FL_REASON_UNKNOWN) {
    static_cast<T*>(this)->do_callback(widget, fl_voidptr(arg), reason);
  }
  void do_callback(T *widget, void *arg = 0, Fl_Callback_Reason reason=FL_REASON_UNKNOWN) {
    // default implementation, specialisation in Fl_Widget
    Fl::callback_reason_ = reason;
    if (!callback_interface_.callback_) return;
    // Fl_Widget_Tracker wp(this);
    callback_interface_.callback_(widget, arg);
    // if (wp.deleted()) return;
    // if (callback_interface_.callback_ != T::default_callback)
    //   static_cast<T*>(this)->clear_changed();
  }
};


#endif // Fl_Callback_Interface_H
