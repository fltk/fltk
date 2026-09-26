//
// Font loading with fontconfig for the Fast Light Tool Kit (FLTK).
//
// Copyright 2026 by Bill Spitzak and others.
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

// Shared by the Xlib (Xft, Pango) and Cairo (X11, Wayland) graphics drivers.

#include <config.h>
#include <FL/Fl_Graphics_Driver.H>
#include <FL/fl_string_functions.h>
#include <fontconfig/fontconfig.h>
#if USE_PANGO
#  include <pango/pangocairo.h>
#  include <pango/pangofc-fontmap.h>
#endif
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>

// Temporary files holding fonts loaded from memory, removed at exit
static std::vector<std::string> temp_files;

static void remove_temp_files() {
  for (size_t i = 0; i < temp_files.size(); i++) unlink(temp_files[i].c_str());
}

// fontconfig can only read fonts from files, so copy the font to a private file
static bool add_font_data(const unsigned char *data, size_t size) {
  const char *dir = getenv("XDG_RUNTIME_DIR");
  if (!dir || !*dir) dir = getenv("TMPDIR");
  if (!dir || !*dir) dir = "/tmp";
  std::string path = std::string(dir) + "/fltk-font-XXXXXX";
  int fd = mkstemp(&path[0]);
  if (fd < 0) return false;
  size_t done = 0;
  while (done < size) {
    ssize_t n = write(fd, data + done, size - done);
    if (n <= 0) break;
    done += n;
  }
  close(fd);
  if (done < size || !FcConfigAppFontAddFile(NULL, (const FcChar8 *)path.c_str())) {
    unlink(path.c_str());
    return false;
  }
  if (temp_files.empty()) atexit(remove_temp_files);
  temp_files.push_back(path);
  return true;
}

/* Makes a face of a font available to fontconfig and returns its FLTK name, or NULL.
 The face gets a unique additional family name ("PostScriptName#n"), so that it
 is selected exactly, even if other faces or installed fonts have the same family
 name and style. With \p pango_name, the name is in Pango format ("Name,"),
 otherwise it has the FLTK style prefix (" Name").
 */
const char *fl_fontconfig_load_font(const char *filename, const unsigned char *data,
                                    size_t size, int face, bool pango_name) {
  char psname[128];
  if (Fl_Graphics_Driver::font_file_info(data, size, face, psname, sizeof(psname)) < 0)
    return NULL;
  FcFontSet *set = FcConfigGetFonts(NULL, FcSetApplication);
  int first = set ? set->nfont : 0;
  if (filename) {
    if (!FcConfigAppFontAddFile(NULL, (const FcChar8 *)filename)) return NULL;
  } else if (!add_font_data(data, size)) {
    return NULL;
  }
  static int count = 0;
  std::string family = std::string(psname) + "#" + std::to_string(++count);
  // the new faces were appended to the application font set
  bool found = false;
  set = FcConfigGetFonts(NULL, FcSetApplication);
  for (int i = first; set && i < set->nfont && !found; i++) {
    int index;
    if (FcPatternGetInteger(set->fonts[i], FC_INDEX, 0, &index) == FcResultMatch && index == face)
      found = FcPatternAddString(set->fonts[i], FC_FAMILY, (const FcChar8 *)family.c_str());
  }
  if (!found) return NULL;
#if USE_PANGO
  // Pango caches the fonts it found, make it look again
  PangoFontMap *fontmap = pango_cairo_font_map_get_default();
  if (PANGO_IS_FC_FONT_MAP(fontmap)) {
#  if PANGO_VERSION_CHECK(1,38,0)
    pango_fc_font_map_config_changed(PANGO_FC_FONT_MAP(fontmap));
#  else
    pango_fc_font_map_cache_clear(PANGO_FC_FONT_MAP(fontmap));
#  endif
  }
#endif
  // the comma keeps Pango from reading words of the name as style
  std::string name = pango_name ? family + "," : " " + family;
  return fl_strdup(name.c_str());
}
