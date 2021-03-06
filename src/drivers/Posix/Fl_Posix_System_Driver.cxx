//
// Definition of Posix system driver (used by both the X11 and macOS platforms).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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
#include "Fl_Posix_System_Driver.H"
#include "../../flstring.h"
#include <FL/Fl_File_Icon.H>
#include <FL/filename.H>
#include <FL/fl_string.h>
#include <FL/Fl.H>
#include <locale.h>
#include <stdio.h>
#if HAVE_DLFCN_H
#  include <dlfcn.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

//
// Define missing POSIX/XPG4 macros as needed...
//
#ifndef S_ISDIR
#  define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#  define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#  define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#  define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif /* !S_ISDIR */


#if HAVE_DLFCN_H
void *Fl_Posix_System_Driver::load(const char *filename) {
  return ::dlopen(filename, RTLD_LAZY);
}
#endif

int Fl_Posix_System_Driver::file_type(const char *filename)
{
  int filetype;
  struct stat fileinfo;         // Information on file
  if (!::stat(filename, &fileinfo))
  {
    if (S_ISDIR(fileinfo.st_mode))
      filetype = Fl_File_Icon::DIRECTORY;
#  ifdef S_ISFIFO
    else if (S_ISFIFO(fileinfo.st_mode))
      filetype = Fl_File_Icon::FIFO;
#  endif // S_ISFIFO
#  if defined(S_ISCHR) && defined(S_ISBLK)
    else if (S_ISCHR(fileinfo.st_mode) || S_ISBLK(fileinfo.st_mode))
      filetype = Fl_File_Icon::DEVICE;
#  endif // S_ISCHR && S_ISBLK
#  ifdef S_ISLNK
    else if (S_ISLNK(fileinfo.st_mode))
      filetype = Fl_File_Icon::LINK;
#  endif // S_ISLNK
    else
      filetype = Fl_File_Icon::PLAIN;
  }
  else
    filetype = Fl_File_Icon::PLAIN;
  return filetype;
}

const char *Fl_Posix_System_Driver::getpwnam(const char *login) {
  struct passwd *pwd;
  pwd = ::getpwnam(login);
  return pwd ? pwd->pw_dir : NULL;
}


void Fl_Posix_System_Driver::gettime(time_t *sec, int *usec) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  *sec = tv.tv_sec;
  *usec = tv.tv_usec;
}

// Run the specified program, returning 1 on success and 0 on failure
int Fl_Posix_System_Driver::run_program(const char *program, char **argv, char *msg, int msglen) {
  pid_t pid;                            // Process ID of first child
  int status;                           // Exit status from first child
  sigset_t set, oldset;                 // Signal masks


  // Block SIGCHLD while we run the program...
  //
  // Note that I only use the POSIX signal APIs, however older operating
  // systems may either not support POSIX signals or have side effects.
  // IRIX, for example, provides three separate and incompatible signal
  // APIs, so it is possible that an application setting a signal handler
  // via signal() or sigset() will not have its SIGCHLD signals blocked...

  sigemptyset(&set);
  sigaddset(&set, SIGCHLD);
  sigprocmask(SIG_BLOCK, &set, &oldset);

  // Create child processes that actually run the program for us...
  if ((pid = fork()) == 0) {
    // First child comes here, fork a second child and exit...
    if (!fork()) {
      // Second child comes here, redirect stdin/out/err to /dev/null...
      close(0);
      ::open("/dev/null", O_RDONLY);

      close(1);
      ::open("/dev/null", O_WRONLY);

      close(2);
      ::open("/dev/null", O_WRONLY);

      // Detach from the current process group...
      setsid();

      // Run the program...
      execv(program, argv);
      _exit(0);
    } else {
      // First child gets here, exit immediately...
      _exit(0);
    }
  } else if (pid < 0) {
    // Restore signal handling...
    sigprocmask(SIG_SETMASK, &oldset, NULL);

    // Return indicating failure...
    return 0;
  }

  // Wait for the first child to exit...
  while (waitpid(pid, &status, 0) < 0) {
    if (errno != EINTR) {
      // Someone else grabbed the child status...
      if (msg) snprintf(msg, msglen, "waitpid(%ld) failed: %s", (long)pid,
                        strerror(errno));

      // Restore signal handling...
      sigprocmask(SIG_SETMASK, &oldset, NULL);

      // Return indicating failure...
      return 0;
    }
  }

  // Restore signal handling...
  sigprocmask(SIG_SETMASK, &oldset, NULL);

  // Return indicating success...
  return 1;
}

////////////////////////////////////////////////////////////////
// POSIX threading...
#if defined(HAVE_PTHREAD)
#  include <unistd.h>
#  include <fcntl.h>
#  include <pthread.h>

// Pipe for thread messaging via Fl::awake()...
static int thread_filedes[2];

// Mutex and state information for Fl::lock() and Fl::unlock()...
static pthread_mutex_t fltk_mutex;
static pthread_t owner;
static int counter;

static void lock_function_init_std() {
  pthread_mutex_init(&fltk_mutex, NULL);
}

static void lock_function_std() {
  if (!counter || owner != pthread_self()) {
    pthread_mutex_lock(&fltk_mutex);
    owner = pthread_self();
  }
  counter++;
}

static void unlock_function_std() {
  if (!--counter) pthread_mutex_unlock(&fltk_mutex);
}

#  ifdef PTHREAD_MUTEX_RECURSIVE
static bool lock_function_init_rec() {
  pthread_mutexattr_t attrib;
  pthread_mutexattr_init(&attrib);
  if (pthread_mutexattr_settype(&attrib, PTHREAD_MUTEX_RECURSIVE)) {
    pthread_mutexattr_destroy(&attrib);
    return true;
  }

  pthread_mutex_init(&fltk_mutex, &attrib);
  return false;
}

static void lock_function_rec() {
  pthread_mutex_lock(&fltk_mutex);
}

static void unlock_function_rec() {
  pthread_mutex_unlock(&fltk_mutex);
}
#  endif // PTHREAD_MUTEX_RECURSIVE

void Fl_Posix_System_Driver::awake(void* msg) {
  if (thread_filedes[1]) {
    if (write(thread_filedes[1], &msg, sizeof(void*))==0) { /* ignore */ }
  }
}

static void* thread_message_;
void* Fl_Posix_System_Driver::thread_message() {
  void* r = thread_message_;
  thread_message_ = 0;
  return r;
}

static void thread_awake_cb(int fd, void*) {
  if (read(fd, &thread_message_, sizeof(void*))==0) {
    /* This should never happen */
  }
  Fl_Awake_Handler func;
  void *data;
  while (Fl::get_awake_handler_(func, data)==0) {
    (*func)(data);
  }
}

// These pointers are in Fl_x.cxx:
extern void (*fl_lock_function)();
extern void (*fl_unlock_function)();

int Fl_Posix_System_Driver::lock() {
  if (!thread_filedes[1]) {
    // Initialize thread communication pipe to let threads awake FLTK
    // from Fl::wait()
    if (pipe(thread_filedes)==-1) {
      /* this should not happen */
    }

    // Make the write side of the pipe non-blocking to avoid deadlock
    // conditions (STR #1537)
    fcntl(thread_filedes[1], F_SETFL,
          fcntl(thread_filedes[1], F_GETFL) | O_NONBLOCK);

    // Monitor the read side of the pipe so that messages sent via
    // Fl::awake() from a thread will "wake up" the main thread in
    // Fl::wait().
    Fl::add_fd(thread_filedes[0], FL_READ, thread_awake_cb);

    // Set lock/unlock functions for this system, using a system-supplied
    // recursive mutex if supported...
#  ifdef PTHREAD_MUTEX_RECURSIVE
    if (!lock_function_init_rec()) {
      fl_lock_function   = lock_function_rec;
      fl_unlock_function = unlock_function_rec;
    } else {
#  endif // PTHREAD_MUTEX_RECURSIVE
      lock_function_init_std();
      fl_lock_function   = lock_function_std;
      fl_unlock_function = unlock_function_std;
#  ifdef PTHREAD_MUTEX_RECURSIVE
    }
#  endif // PTHREAD_MUTEX_RECURSIVE
  }

  fl_lock_function();
  return 0;
}

void Fl_Posix_System_Driver::unlock() {
  fl_unlock_function();
}

// Mutex code for the awake ring buffer
static pthread_mutex_t *ring_mutex;

void Fl_Posix_System_Driver::unlock_ring() {
  pthread_mutex_unlock(ring_mutex);
}

void Fl_Posix_System_Driver::lock_ring() {
  if (!ring_mutex) {
    ring_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(ring_mutex, NULL);
  }
  pthread_mutex_lock(ring_mutex);
}

#else // ! HAVE_PTHREAD

void Fl_Posix_System_Driver::awake(void*) {}
int Fl_Posix_System_Driver::lock() { return 1; }
void Fl_Posix_System_Driver::unlock() {}
void* Fl_Posix_System_Driver::thread_message() { return NULL; }

//void lock_ring() {}
//void unlock_ring() {}

#endif // HAVE_PTHREAD
