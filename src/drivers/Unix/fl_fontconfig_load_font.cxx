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

/* Makes a font available to fontconfig and returns its FLTK name, or NULL.
 With \p pango_name, the name is in Pango format ("Family, Bold Italic"),
 otherwise it is the family name with the FLTK style prefix.
 */
const char *fl_fontconfig_load_font(const char *filename, const unsigned char *data,
                                    size_t size, bool pango_name) {
  char family[128], psname[128];
  int style = Fl_Graphics_Driver::font_file_info(data, size, family, psname, sizeof(family));
  if (style < 0) return NULL;
  if (filename) {
    if (!FcConfigAppFontAddFile(NULL, (const FcChar8 *)filename)) return NULL;
  } else if (!add_font_data(data, size)) {
    return NULL;
  }
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
  std::string name;
  if (pango_name) { // the comma keeps Pango from parsing words of the family name as style
    name = std::string(family) + ",";
    if (style & FL_BOLD) name += " Bold";
    if (style & FL_ITALIC) name += " Italic";
  } else {
    name = std::string(1, " BIP"[style]) + family;
  }
  return fl_strdup(name.c_str());
}
