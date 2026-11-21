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
#include <new>
#include <cstddef>


#ifndef Fl_Callback_User_Data_H
#define Fl_Callback_User_Data_H

/** Callback function type for a user defined Fl_Callback_User_Data destructor. */
typedef void (Fl_Callback_User_Data_Dtor)(class Fl_Callback_User_Data*);
/** Callback function type to free `void*` based user data. */
typedef void (Fl_Callback_Free)(void*);

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
 An interface template class that adds callback functionality to the class T.

 This interface adds modern C++ and classic FLTK callback functionality to any
 class T. Use this interface by adding it as one base class in the declaration
 of class T with T itself as the template parameter.

 ```cpp
 class MyClass : public Fl_Callback_Interface<MyClass> {
    // ...
 };
 ```
 The interface adds member functions set a type-safe std::function callback
 or classic FLTK callback functions and user data that require type casting.

 Fl_Callback_Interface needs to store a single bit of infomation in the derived
 class. To do this, `MyClass` must implement the following functions:

 ```cpp
 bool callback_is_stdf() const;
 void set_callback_stdf();
 void clear_callback_stdf();
 ```

 Classes that use this interface must implement the following functions that
 simply get, set, and clear a bit, must be false initially:
 `callback_is_stdf()`, `set_callback_stdf()`, and `clear_callback_stdf()`.

 \param T The class to add callback functionality to.
 */
template <typename T>
class Fl_Callback_Interface
{
  // -- types
  typedef void (Callback_T_voidp)(T*, void*);
  typedef void (Callback_T_long)(T*, long);
  typedef void (Callback_T_empty)(T*);
  typedef void (Callback_voidp)(void*);
  typedef void (Callback_long)(long);
  // typedef void (Callback_empty)(); // Would be ambiguous with std::function

  // -- implementation calls to avoid naming conflicts
  class Callback_Data
  {
  public:
    static void dummy_callback(T*, void*) {}
    // The following structure looks large, but only takes 32 bytes due to the use of unions.
    union {
      struct {
        union {
          // Store the callback function pointer in the matching member of the
          // union and mark cb_type_ accordingly.
          Callback_T_voidp* callback_t_voidp_;  // (1): void(T*, void*)
          Callback_T_long*  callback_t_long_;   // (2): void(T*, long)
          Callback_T_empty* callback_t_empty_;  // (3): void(T*)
          Callback_voidp*   callback_voidp_;    // (4): void(void*)
          Callback_long*    callback_long_;     // (5): void(long)
        };
        union {
          // Store the user data destructor function pointer in the matching
          // member of the union and mark cb_type_ accordingly.
          Fl_Callback_User_Data_Dtor* dtor_callback_;
          Fl_Callback_Free* free_callback_;
        };
        // We use void* user_data_ for all types because we can safely cast
        // the void* to long and Fl_Callback_User_Data*.
        void* user_data_;
        // Remember which type of callback is currently stored in the union.
        uint8_t cb_type_;   // 0 = none, 1 = callback_t_voidp_, etc.
        // Remember which type of user data dtor is currently stored in the union.
        uint8_t dtor_type_; // 0 = none, 1 = dtor_callback_, 2 = free_callback_
      };
      std::function<void()> stdf_callback_;
    };
    // Constructor, members of unions must be explicitly initialized
    Callback_Data()
      : callback_t_voidp_(nullptr)
      , dtor_callback_(nullptr) // union: also clears free_callback_
      , user_data_(nullptr)
      , dtor_type_(0)
      , cb_type_(0)
    {
    }
    // Can't copy, because we can't duplicate user_data_ safely.
    Callback_Data(const Callback_Data&) = delete;
    Callback_Data& operator=(const Callback_Data&) = delete;
    // Don't allowing moving either because we can't control the stdf flag.
    Callback_Data(Callback_Data&&) = delete;
    Callback_Data& operator=(Callback_Data&&) = delete;
    // Not enough information. Let Fl_Callback_Interface handle destruction.
    ~Callback_Data() {};
    // Run the user data destructor callback if it exists.
    void do_dtor_callback(T* t) {
      if (!t->callback_is_stdf()) {
        if (dtor_type_ == 1) { // dtor_callback_
          if (dtor_callback_)
            dtor_callback_((Fl_Callback_User_Data*)user_data_);
        } else if (dtor_type_ == 2) { // free_callback_
          if (free_callback_)
            free_callback_(user_data_);
        }
      }
    }
    // Clear the user data and reset the destructor callback.
    void clear_user_data(T* t) {
      if (!t->callback_is_stdf()) {
        do_dtor_callback(t);
        dtor_callback_ = nullptr; // union: also clears free_callback_
        user_data_ = nullptr;
        dtor_type_ = 0;
      }
    }
    // Clear all callback data, including the std::function callback if active.
    void clear(T* t) {
      if (t->callback_is_stdf()) {
        // Explicity clear the std::function object to avoid memory leaks
        stdf_callback_.~function();
        // Clear the bit that indicates stdf_callback_ is active
        t->clear_callback_stdf();
      } else {
        clear_user_data(t);
      }
      // Restore the union to a safe state by calling the ctor in place.
      new(&callback_t_voidp_) Callback_Data();
    }
  };

protected:
  // Storage for the callback data, including user data and std::function callback.
  Callback_Data callback_data_;

  // -- ctor and dtor
  // Constructor has no extra jobs.
  Fl_Callback_Interface() = default;
  // Destructor clears the callback data to avoid memory leaks.
  ~Fl_Callback_Interface() {
    callback_data_.clear(static_cast<T*>(this));
  }

public:
  // -- non-copyable and non-movable
  // Not copyable, can't copy callback_data_.
  Fl_Callback_Interface(const Fl_Callback_Interface&) = delete;
  Fl_Callback_Interface& operator=(const Fl_Callback_Interface&) = delete;
  // Not movable either, because we can't control the stdf flag safely.
  Fl_Callback_Interface(Fl_Callback_Interface&&) = delete;
  Fl_Callback_Interface& operator=(Fl_Callback_Interface&& other) = delete;

  // -- public interface
  void callback(nullptr_t) {
    callback_data_.clear(static_cast<T*>(this));
  }
  void callback(Callback_T_voidp* cb, void* p=nullptr, Fl_Callback_Free* free_cb = nullptr) {
    callback_data_.clear(static_cast<T*>(this));
    callback_data_.callback_t_voidp_ = cb;
    callback_data_.cb_type_ = 1;
    user_data(p, free_cb);
  }
  void callback(Callback_T_voidp* cb, Fl_Callback_User_Data* p, bool auto_free) {
    callback_data_.clear(static_cast<T*>(this));
    callback_data_.callback_t_voidp_ = cb;
    callback_data_.cb_type_ = 1;
    user_data(p, auto_free);
  }
  void callback(Callback_T_long* cb, long p = 0) {
    callback_data_.clear(static_cast<T*>(this));
    callback_data_.callback_t_long_ = cb; // stored and later called through its own type -- no cast
    callback_data_.cb_type_ = 2;
    user_data(fl_voidptr(p));
  }
  void callback(Callback_T_empty* cb) {
    callback_data_.clear(static_cast<T*>(this));
    callback_data_.callback_t_empty_ = cb; // stored and later called through its own type -- no cast
    callback_data_.cb_type_ = 3;
    user_data(nullptr); // original does not clear user_data!
  }
  void callback(Callback_voidp* cb, void* p=nullptr, Fl_Callback_Free* free_cb = nullptr) {
    callback_data_.clear(static_cast<T*>(this));
    callback_data_.callback_voidp_ = cb;
    callback_data_.cb_type_ = 4;
    user_data(p, free_cb);
  }
  void callback(Callback_voidp* cb, Fl_Callback_User_Data* p, bool auto_free) {
    callback_data_.clear(static_cast<T*>(this));
    callback_data_.callback_voidp_ = cb;
    callback_data_.cb_type_ = 4;
    user_data(p, auto_free);
  }
  void callback(Callback_long* cb, long p = 0) {
    callback_data_.clear(static_cast<T*>(this));
    callback_data_.callback_long_ = cb; // stored and later called through its own type -- no cast
    callback_data_.cb_type_ = 5;
    user_data(fl_voidptr(p));
  }
  void callback(std::function<void()> cb) {
    callback_data_.clear(static_cast<T*>(this));
    static_cast<T*>(this)->set_callback_stdf();
    new (&callback_data_.stdf_callback_) std::function<void()>(std::move(cb));
  }
  void user_data(void* v, Fl_Callback_Free* free_cb = nullptr) {
    // user data has no meaning while the union's active member is
    // stdf_callback_; skip so we don't write over the live std::function.
    if (static_cast<T*>(this)->callback_is_stdf()) return;
    callback_data_.clear_user_data(static_cast<T*>(this));
    callback_data_.user_data_ = v;
    if (free_cb) {
      callback_data_.free_callback_ = free_cb;
      callback_data_.dtor_type_ = 2;
    }
  }
  void user_data(Fl_Callback_User_Data* v, bool auto_free) {
    if (static_cast<T*>(this)->callback_is_stdf()) return;
    callback_data_.clear_user_data(static_cast<T*>(this));
    callback_data_.user_data_ = v;
    if (auto_free) {
      callback_data_.dtor_callback_ = Fl_Callback_User_Data::default_dtor_callback;
      callback_data_.dtor_type_ = 1;
    }
  }
  void argument(long v) {
    user_data(fl_voidptr(v));
  }
  // -- Callback getters
  // The return value of this method should only ever be compared to nullptr!
  Callback_T_voidp* callback() const {
    if (static_cast<const T*>(this)->callback_is_stdf()) {
      // For std::function, only a comparison to nullptr is meaningful.
      if (!callback_data_.stdf_callback_) {
        return nullptr;
      } else {
        return callback_data_.dummy_callback;
      }
    } else {
      // Returned for backward compatibility.
      return callback_data_.callback_t_voidp_;
    }
  }
  void* user_data() const {
    if (static_cast<const T*>(this)->callback_is_stdf()) {
      return nullptr;
    } else {
      return callback_data_.user_data_;
    }
  }
  long argument() const {
    return fl_long(user_data());
  }
  // -- callback actions
  void do_callback(Fl_Callback_Reason reason=FL_REASON_UNKNOWN) {
    T* self = static_cast<T*>(this);
    self->do_callback(self, user_data(), reason);
  }
  void do_callback(T *widget, long arg, Fl_Callback_Reason reason=FL_REASON_UNKNOWN) {
    T* self = static_cast<T*>(this);
    self->do_callback(widget, fl_voidptr(arg), reason);
  }
  void do_callback(T *widget, void *arg = 0, Fl_Callback_Reason reason=FL_REASON_UNKNOWN) {
    // default implementation, specialisation in Fl_Widget
    T* self = static_cast<T*>(this);
    if (self->callback_is_stdf()) {
      if (!callback_data_.stdf_callback_) return;
      Fl::callback_reason_ = reason;
      callback_data_.stdf_callback_();
    } else {
      // Return early if the callback is nullptr. Legal, because all callback
      // pointers are the same element in a union.
      if (!callback_data_.callback_t_voidp_) return;
      Fl::callback_reason_ = reason;
      // Dispatch strictly by cb_type_ to avoid C++ undefined behavior.
      switch (callback_data_.cb_type_) {
        case 1: callback_data_.callback_t_voidp_(widget, arg); break;
        case 2: callback_data_.callback_t_long_(widget, fl_long(arg)); break;
        case 3: callback_data_.callback_t_empty_(widget); break;
        case 4: callback_data_.callback_voidp_(arg); break;
        case 5: callback_data_.callback_long_(fl_long(arg)); break;
        default: break; // Ignore.
      }
    }
  }
};


#endif // Fl_Callback_Interface_H
