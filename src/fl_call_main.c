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
 * Because of problems with the Microsoft Visual C++ header files
 * and/or compiler, you cannot have a WinMain function in a DLL.
 * I don't know why.  Thus, this nifty feature is only available
 * if you link to the static library.
 *
 * Currently the debug version of this library will create a
 * console window for your application so you can put printf()
 * statements for debugging or informational purposes.  Ultimately
 * we want to update this to always use the parent's console,
 * but at present we have not identified a function or API in
 * Microsoft(r) Windows(r) that allows for it.
 */

/*
 * This file is compiled only on Windows platforms (since FLTK 1.4.0).
 * Therefore we don't need to test the _WIN32 macro anymore.
 * The _MSC_VER macro is tested to compile it only for Visual Studio
 * platforms because GNU platforms (MinGW, MSYS) don't need it.
*/
#if !defined(FL_DLL) && !defined (__GNUC__)

#include <FL/fl_utf8.h>
#include <FL/fl_string_functions.h>

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
  char strbuf[2048];

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

  /* Convert the command line arguments to UTF-8 */
  LPWSTR *wideArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
  argv = (char **)malloc((argc + 1) * sizeof(char *));
  for (i = 0; i < argc; i++) {
    int ret = WideCharToMultiByte(CP_UTF8,        /* CodePage          */
                                  0,              /* dwFlags           */
                                  wideArgv[i],    /* lpWideCharStr     */
                                  -1,             /* cchWideChar       */
                                  strbuf,         /* lpMultiByteStr    */
                                  sizeof(strbuf), /* cbMultiByte       */
                                  NULL,           /* lpDefaultChar     */
                                  NULL);          /* lpUsedDefaultChar */
    argv[i] = fl_strdup(strbuf);
  }
  argv[argc] = NULL; // required by C standard at end of list

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
