//
// "$Id$"
//
// Hello, World! program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/filename.H>
#include <FL/Fl_File_Chooser.H>

#include "flink_ui.h"


void showAboutWindow()
{
  fl_message("%s",
             "Flink creates all files needed to compile FLTK for Android.\n\n"
             "Flink was written for FLTK 1.4 and tested with\n"
             "AndroidStudio 3.5 ."
             );
}


void selectSourceFolder()
{
  const char *dir = fl_dir_chooser("Select the FLTK root folder", wSourceFolder->value(), 0);
  if (dir) {
    wSourceFolder->value(dir);
  }
}


void selectProjectFolder()
{
  char oldDir[FL_PATH_MAX];
  fl_getcwd(oldDir, FL_PATH_MAX);
  fl_chdir(wSourceFolder->value());
  const char *dir = fl_dir_chooser("Select the AndroidStudio subfolder", wProjectFolder->value(), 1);
  fl_chdir(oldDir);
  if (dir) {
    wProjectFolder->value(dir);
  }
}


void createFile(const char *dir, const char *name, const char *text)
{
  char filename[FL_PATH_MAX];
  strcpy(filename, dir);
  strcat(filename, name);
  fl_make_path_for_file(filename);
  FILE *f = fl_fopen(filename, "wb");
  fwrite(text, strlen(text), 1, f);
  fclose(f);
}


void createProjectFolder()
{
  char projectDir[FL_PATH_MAX];
  fl_chdir(wSourceFolder->value());
  fl_filename_absolute(projectDir, FL_PATH_MAX, wProjectFolder->value());
  int n = strlen(projectDir);
  if (projectDir[n]!='/') strcat(projectDir, "/");
  fl_make_path(projectDir);

  //fl_message("Can't create project. Not yet implemented.");

  createFile(projectDir, "FL/abi-version.h", "/* #undef FL_ABI_VERSION */\n");
  createFile(projectDir, "build.gradle",
             "buildscript {\n"
             "    repositories {\n"
             "        jcenter()\n"
             "        google()\n"
             "    }\n"
             "    dependencies {\n"
             "        classpath 'com.android.tools.build:gradle:3.5.3'\n"
             "    }\n"
             "}\n\n"
             "allprojects {\n"
             "    repositories {\n"
             "        jcenter()\n"
             "        google()\n"
             "    }\n"
             "}\n");
  createFile(projectDir, "settings.gradle",
             "include ':fltk'\n"
             "include ':adjuster'\n");
  createFile(projectDir, "config.h",
             "#define FLTK_DATADIR \"/usr/local/share/fltk\"\n"
             "#define FLTK_DOCDIR \"/usr/local/share/doc/fltk\"\n"
             "#define BORDER_WIDTH 2\n"
             "#define HAVE_GL 0\n"
             "#define HAVE_GL_GLU_H 0\n"
             "/* #undef HAVE_GLXGETPROCADDRESSARB */\n"
             "#define USE_COLORMAP 1\n"
             "#define HAVE_XINERAMA 0\n"
             "#define USE_XFT 0\n"
             "#define USE_PANGO 0\n"
             "#define HAVE_XDBE 0\n"
             "#define USE_XDBE HAVE_XDBE\n"
             "#define HAVE_XFIXES 0\n"
             "#define HAVE_XCURSOR 0\n"
             "#define HAVE_XRENDER 0\n"
             "#define HAVE_X11_XREGION_H 0\n"
             "/* #undef __APPLE_QUARTZ__ */\n"
             "/* #undef USE_X11 */\n"
             "/* #undef USE_SDL */\n"
             "#define HAVE_OVERLAY 0\n"
             "#define HAVE_GL_OVERLAY HAVE_OVERLAY\n"
             "#define WORDS_BIGENDIAN 0\n"
             "#define U16 unsigned short\n"
             "#define U32 unsigned\n"
             "#define U64 unsigned long\n"
             "#define HAVE_DIRENT_H 1\n"
             "#define HAVE_SCANDIR 1\n"
             "#define HAVE_SCANDIR_POSIX 1\n"
             "#define HAVE_VSNPRINTF 1\n"
             "#define HAVE_SNPRINTF 1\n"
             "#define HAVE_STRINGS_H 1\n"
             "#define HAVE_STRCASECMP 1\n"
             "#define HAVE_STRLCAT 1\n"
             "#define HAVE_STRLCPY 1\n"
             "#define HAVE_LOCALE_H 1\n"
             "#define HAVE_LOCALECONV 1\n"
             "#define HAVE_SYS_SELECT_H 1\n"
             "/* #undef HAVE_SYS_STDTYPES_H */\n"
             "#define USE_POLL 0\n"
             "#define HAVE_LIBPNG 1\n"
             "#define HAVE_LIBZ 1\n"
             "#define HAVE_LIBJPEG 1\n"
             "/* #undef FLTK_USE_CAIRO */\n"
             "/* #undef FLTK_HAVE_CAIRO */\n"
             "#define HAVE_PNG_H 1\n"
             "/* #undef HAVE_LIBPNG_PNG_H */\n"
             "#define HAVE_PNG_GET_VALID 1\n"
             "#define HAVE_PNG_SET_TRNS_TO_ALPHA 1\n"
             "#define FLTK_USE_NANOSVG 1\n"
             "#define HAVE_PTHREAD 1\n"
             "#define HAVE_PTHREAD_H 1\n"
             "/* #undef HAVE_ALSA_ASOUNDLIB_H */\n"
             "#define HAVE_LONG_LONG 1\n"
             "#define FLTK_LLFMT \"%lld\"\n"
             "#define FLTK_LLCAST (long long)\n"
             "#define HAVE_DLFCN_H 1\n"
             "#define HAVE_DLSYM 1\n"
             "#define FL_NO_PRINT_SUPPORT 1\n"
             "/* #undef FL_CFG_NO_FILESYSTEM_SUPPORT */\n");

/*
 fltk adjuster 

 ../AndroidStudio.min//adjuster:
 build.gradle  src

 ../AndroidStudio.min//adjuster/src:
 main

 ../AndroidStudio.min//adjuster/src/main:
 AndroidManifest.xml  assets      cpp      res

 ../AndroidStudio.min//adjuster/src/main/assets:
 fonts

 ../AndroidStudio.min//adjuster/src/main/assets/fonts:
 Roboto-Regular.ttf

 ../AndroidStudio.min//adjuster/src/main/cpp:
 CMakeLists.txt

 ../AndroidStudio.min//adjuster/src/main/res:
 mipmap-hdpi  mipmap-mdpi  mipmap-xhdpi  mipmap-xxhdpi  values

 ../AndroidStudio.min//adjuster/src/main/res/mipmap-hdpi:
 ic_launcher.png

 ../AndroidStudio.min//adjuster/src/main/res/mipmap-mdpi:
 ic_launcher.png

 ../AndroidStudio.min//adjuster/src/main/res/mipmap-xhdpi:
 ic_launcher.png

 ../AndroidStudio.min//adjuster/src/main/res/mipmap-xxhdpi:
 ic_launcher.png

 ../AndroidStudio.min//adjuster/src/main/res/values:
 strings.xml

 ../AndroidStudio.min//fltk:
 build.gradle  src

 ../AndroidStudio.min//fltk/src:
 main

 ../AndroidStudio.min//fltk/src/main:
 AndroidManifest.xml  cpp      res

 ../AndroidStudio.min//fltk/src/main/cpp:
 CMakeLists.txt

 ../AndroidStudio.min//fltk/src/main/res:
 values

 ../AndroidStudio.min//fltk/src/main/res/values:
 strings.xml

 ../AndroidStudio.min//gradle:
 wrapper

 ../AndroidStudio.min//gradle/wrapper:
 gradle-wrapper.jar    gradle-wrapper.properties
 WonkoBook:Xcode matt$

 */
}


int main(int argc, char **argv) {
  fl_message_title_default("Flink");
  Fl_Window *window = createMainWindow();

  char pathToFLTK[FL_PATH_MAX];
  strcpy(pathToFLTK, __FILE__);
  char *name = (char*)fl_filename_name(pathToFLTK);
  if (name && name>pathToFLTK) name[-1] = 0;
  name = (char*)fl_filename_name(pathToFLTK);
  if (name && name>pathToFLTK) name[-1] = 0;
  wSourceFolder->value(pathToFLTK);

  wProjectFolder->value("build/AndroidStudio");

  wDeleteExistingProject->value(1);

  window->show(argc, argv);
  return Fl::run();
}

//
// End of "$Id$".
//

