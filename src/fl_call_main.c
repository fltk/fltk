/*
 * Copyright 1998-2023 by Bill Spitzak and others.
 *
 * fl_call_main() calls main() for you Windows people.  Needs to be done in C
 * because Borland C++ won't let you call main() from C++.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     https://www.fltk.org/COPYING.php
 *
 * Please see the following page on how to report bugs and issues:
 *
 *     https://www.fltk.org/bugs.php
 */

/*
 * This WinMain() function can be overridden by an application and
 * is provided for compatibility with programs written for other
 * operating systems that conform to the ANSI standard entry point
 * "main()".  This will allow you to build a Windows Application
 * without any special settings.
 *
 * You cannot have this WinMain() function in a DLL because it would have
 * to call \c main() outside the DLL.  Thus, this nifty feature is only
 * available if you link to the static library.
 *
 * However, it is possible to build this module separately so you can
 * use it in progams that link to the shared library.
 *
 * Currently the debug version of this library will create a console window
 * for your application so you can put printf() statements for debugging or
 * informational purposes.  Ultimately we want to update this to always use
 * the parent's console, but at present we have not identified a function
 * or API in Microsoft(r) Windows(r) that allows for it.
 */

/*
 * Notes for FLTK developers:
 *
 * 1) Since FLTK 1.4.0 this file is compiled only on Windows, hence we don't
 *    need to test the _WIN32 macro.
 * 2) This file must not call any FLTK library functions because this would
 *    not work with /both/ the DLL /and/ the static library (linkage stuff).
 * 3) Converting the commandline arguments to UTF-8 is therefore implemented
 *    here *and* in the library but this seems to be an acceptable compromise.
 * 4) (Unless someone finds a better solution, of course. Albrecht)
 * 5) The condition "!defined(FL_DLL)" prevents building this in the shared
 *    library, i.e. "WinMain()" will not be defined in the shared lib (DLL).
 * 6) The condition "!defined (__GNUC__)" prevents compilation of this
 *    module with MinGW, MSYS, and Cygwin which don't use WinMain().
 * 7) It is unclear if there are other build systems on Windows that need a
 *    WinMain() entry point. Earlier comments and code seem to indicate that
 *    Borland C++ would require it.
*/

#if !defined(FL_DLL) && !defined (__GNUC__)

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <shellapi.h>

extern int main(int, char *[]);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  int rc;
  int i;
  int argc = 0;
  char** argv = NULL;

 /*
  * If we are compiling in debug mode, open a console window so
  * we can see any printf's, etc...
  *
  * While we can detect if the program was run from the command-line -
  * look at the CMDLINE environment variable, it will be "WIN" for
  * programs started from the GUI - the shell seems to run all Windows
  * applications in the background anyways...
  */

#ifdef _DEBUG
  AllocConsole();
  freopen("conin$", "r", stdin);
  freopen("conout$", "w", stdout);
  freopen("conout$", "w", stderr);
#endif /* _DEBUG */

  /* Get the command line arguments as Windows Wide Character strings */
  LPWSTR *wideArgv = CommandLineToArgvW(GetCommandLineW(), &argc);

  /* Allocate an array of 'argc + 1' string pointers */
  argv = (char **)malloc((argc + 1) * sizeof(char *));

  /* Convert the command line arguments to UTF-8 */
  for (i = 0; i < argc; i++) {
    /* find the required size of the buffer */
    int u8size = WideCharToMultiByte(CP_UTF8,     /* CodePage */
                                     0,           /* dwFlags */
                                     wideArgv[i], /* lpWideCharStr */
                                     -1,          /* cchWideChar */
                                     NULL,        /* lpMultiByteStr */
                                     0,           /* cbMultiByte */
                                     NULL,        /* lpDefaultChar */
                                     NULL);       /* lpUsedDefaultChar */
    if (u8size > 0) {
      char *strbuf = (char *)malloc(u8size);
      int ret = WideCharToMultiByte(CP_UTF8,     /* CodePage */
                                    0,           /* dwFlags */
                                    wideArgv[i], /* lpWideCharStr */
                                    -1,          /* cchWideChar */
                                    strbuf,      /* lpMultiByteStr */
                                    u8size,      /* cbMultiByte */
                                    NULL,        /* lpDefaultChar */
                                    NULL);       /* lpUsedDefaultChar */
      if (ret) {
        argv[i] = strbuf;
      } else {
        argv[i] = _strdup("");
        free(strbuf);
      }
    } else {
      argv[i] = _strdup("");
    }
  }
  argv[argc] = NULL; /* required by C standard at end of list */

  /* Free the wide character string array */
  LocalFree(wideArgv);

  /* Call the program's entry point main() */
  rc = main(argc, argv);

  /* Cleanup allocated memory for argv */
  for (int i = 0; i < argc; ++i) {
    free((void *)argv[i]);
  }
  free((void *)argv);

  /* Close the console in debug mode */

#ifdef _DEBUG
  fclose(stdin);
  fclose(stdout);
  fclose(stderr);
#endif /* _DEBUG */

  return rc;
}

#else

/* STR# 2973: solves "empty translation unit" error */
typedef int dummy;

#endif /* !defined(FL_DLL) && !defined (__GNUC__) */
