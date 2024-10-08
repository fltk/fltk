//
// More font utilities for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#ifndef FL_DOXYGEN

#include "../../flstring.h"
#include "Fl_Xlib_Graphics_Driver.H"
#if USE_PANGO
#  include "../Cairo/Fl_Cairo_Graphics_Driver.H"
#endif
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/fl_string_functions.h>  // fl_strdup()
#include <FL/platform.H>
#include <FL/fl_utf8.h>
#include "Fl_Font.H"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <X11/Xft/Xft.h>
#include <X11/Xft/XftCompat.h>


Fl_XFont_On_Demand fl_xfont = 0;


#if ! USE_PANGO

// The predefined fonts that FLTK has with Xft but without Pango:
static Fl_Fontdesc built_in_table[] = {
#if 1
  {" sans"},
  {"Bsans"},
  {"Isans"},
  {"Psans"},
  {" mono"},
  {"Bmono"},
  {"Imono"},
  {"Pmono"},
  {" serif"},
  {"Bserif"},
  {"Iserif"},
  {"Pserif"},
  {" symbol"},
  {" screen"},
  {"Bscreen"},
  {" zapf dingbats"},
#else
  {" helvetica"},
  {"Bhelvetica"},
  {"Ihelvetica"},
  {"Phelvetica"},
  {" courier"},
  {"Bcourier"},
  {"Icourier"},
  {"Pcourier"},
  {" times"},
  {"Btimes"},
  {"Itimes"},
  {"Ptimes"},
  {" symbol"},
  {" lucidatypewriter"},
  {"Blucidatypewriter"},
  {" zapf dingbats"},
#endif
};

FL_EXPORT Fl_Fontdesc* fl_fonts = (Fl_Fontdesc*)built_in_table;

#endif // ! USE_PANGO

static void fl_xft_font(Fl_Xlib_Graphics_Driver *driver, Fl_Font fnum, Fl_Fontsize size, int angle);

// For some reason Xft produces errors if you destroy a window whose id
// still exists in an XftDraw structure. It would be nice if this is not
// true, a lot of junk is needed to try to stop this:

XftDraw* Fl_Xlib_Graphics_Driver::draw_ = 0;
Window Fl_Xlib_Graphics_Driver::draw_window = (Window)0;

Fl_Fontsize Fl_Xlib_Graphics_Driver::size_unscaled() {
  return (Fl_Fontsize)(size_);
}

static void correct_extents (float s, int &dx, int &dy, int &w, int &h) {
  if (int(s) == s) { // correct for extents non divisible by integral s
    int delta = dx - int(dx/s)*s;
    if (delta) {
      dx -= delta; w += delta;
    }
    delta = -dy - int((-dy)/s)*s;
    if (delta) {
      dy -= delta; h += delta;
    }
    delta = h - int(h/s)*s;
    if (delta) {
      h += delta;
    }
    delta = w - int(w/s)*s;
    if (delta) {
      w += delta;
    }
  }
}

#if ! USE_PANGO

///////////////////////////////////////////////////////////
#define LOCAL_RAW_NAME_MAX 256

extern "C" {
// sort returned fontconfig font names
static int name_sort(const void *aa, const void *bb) {
  // What should we do here? Just do a string compare for now...
  // NOTE: This yeilds some oddities - in particular a Blah Bold font will be
  // listed before Blah...
  // Also - the fontconfig listing returns some faces that are effectively duplicates
  // as far as fltk is concerned, e.g. where there are ko or ja variants that we
  // can't distinguish (since we are not yet fully UTF-*) - should we strip them here?
  return fl_ascii_strcasecmp(*(char**)aa, *(char**)bb);
} // end of name_sort
} // end of extern C section


// Read the "pretty" name we have derived from fontconfig then convert
// it into the format fltk uses internally for Xft names...
// This is just a mess - I should have tokenised the strings and gone from there,
// but I really thought this would be easier!
static void make_raw_name(char *raw, char *pretty)
{
  // Input name will be "Some Name:style = Bold Italic" or whatever
  // The plan is this:
  // - the first char in the "raw" name becomes either I, B, P or " " for
  //   italic, bold, bold italic or normal - this seems to be the fltk way...

  char *style = strchr(pretty, ':');

  if (style)
  {
    *style = 0; // Terminate "name" string
    style ++;   // point to start of style section
  }

  // It is still possible that the "pretty" name has multiple comma separated entries
  // I've seen this often in CJK fonts, for example... Keep only the first one... This
  // is not ideal, the CJK fonts often have the name in utf8 in several languages. What
  // we ought to do is use fontconfig to query the available languages and pick one... But which?
#if 0 // loop to keep the LAST name entry...
  char *nm1 = pretty;
  char *nm2 = strchr(nm1, ',');
  while(nm2) {
    nm1 = nm2 + 1;
    nm2 = strchr(nm1, ',');
  }
  raw[0] = ' '; raw[1] = 0; // Default start of "raw name" text
  strncat(raw, nm1, LOCAL_RAW_NAME_MAX-1); // only copy MAX-1 chars, we have already set cell 0
  // Ensure raw is terminated, just in case the given name is infeasibly long...
  raw[LOCAL_RAW_NAME_MAX-1] = 0;
#else // keep the first remaining name entry
  char *nm2 = strchr(pretty, ',');
  if(nm2) *nm2 = 0; // terminate name after first entry
  raw[0] = ' '; raw[1] = 0; // Default start of "raw name" text
  strncat(raw, pretty, LOCAL_RAW_NAME_MAX-1); // only copy MAX-1 chars, we have already set cell 0
  // Ensure raw is terminated, just in case the given name is infeasibly long...
  raw[LOCAL_RAW_NAME_MAX-1] = 0;
#endif
  // At this point, the name is "marked" as regular...
  if (style)
  {
#define PLAIN   0
#define BOLD    1
#define ITALIC  2
#define BITALIC (BOLD | ITALIC)

    int mods = PLAIN;
    char *last = style + strlen(style) - 2;

    // Now try and parse the style string - look for the "=" sign
    style = strchr(style, '=');
    while ((style) && (style < last))
    {
      int type;
      while ((*style == '=') || (*style == ' ') || (*style == '\t') || (*style == ','))
      {
        style++; // Start of Style string
        if ((style >= last) || (*style == 0)) continue;
      }
      type = toupper(style[0]);
      switch (type)
      {
      // Things we might see: Regular Normal Bold Italic Oblique (??what??) Medium
      // Roman Light Demi Sans SemiCondensed SuperBold Book... etc...
      // Things we actually care about: Bold Italic Oblique SuperBold - Others???
      case 'I':
        if (strncasecmp(style, "Italic", 6) == 0)
        {
          mods |= ITALIC;
        }
        goto NEXT_STYLE;

      case 'B':
        if (strncasecmp(style, "Bold", 4) == 0)
        {
          if (!strstr(raw, " Demi Bold")) mods |= BOLD;
        }
        else if (strncasecmp(style, "Black", 5) == 0)
          {
            if (!strstr(raw, " Black")) strcat(raw, " Black");
          }
        goto NEXT_STYLE;

        case 'D':
          if (strncasecmp(style, "Demi Bold", 7) == 0)
          {
            if (!strstr(raw, " Demi Bold")) strcat(raw, " Demi Bold");
          }
          goto NEXT_STYLE;

      case 'O':
        if (strncasecmp(style, "Oblique", 7) == 0)
        {
          mods |= ITALIC;
        }
        goto NEXT_STYLE;

      case 'S':
        if (strncasecmp(style, "SuperBold", 9) == 0)
        {
          mods |= BOLD;
        }
        goto NEXT_STYLE;

        case 'L':
          if (strncasecmp(style, "Light", 5) == 0)
          {
            if (!strstr(raw, " Light")) strcat(raw, " Light");
          }
          goto NEXT_STYLE;

        case 'M':
          if (strncasecmp(style, "Medium", 6) == 0)
          {
            if (!strstr(raw, " Medium")) strcat(raw, " Medium");
          }
          goto NEXT_STYLE;

      default: // find the next gap
        goto NEXT_STYLE;
      } // switch end
NEXT_STYLE:
      while ((*style != ' ') && (*style != '\t') && (*style != ','))
      {
        style++;
        if ((style >= last) || (*style == 0)) goto STYLE_DONE;
      }
    }
STYLE_DONE:
    // Set the "modifier" character in the raw string
    switch(mods)
    {
    case BOLD: raw[0] = 'B';
      break;
    case ITALIC: raw[0] = 'I';
      break;
    case BITALIC: raw[0] = 'P';
      break;
    default: raw[0] = ' ';
      break;
    }
  }
} // make_raw_name

///////////////////////////////////////////////////////////

static int fl_free_font = FL_FREE_FONT;

// This function fills in the fltk font table with all the fonts that
// are found on the X server.  It tries to place the fonts into families
// and to sort them so the first 4 in a family are normal, bold, italic,
// and bold italic.

// Uses the fontconfig lib to construct a list of all installed fonts.
// I tried using XftListFonts for this, but the API is tricky - and when
// I looked at the XftList* code, it calls the Fc* functions anyway, so...
//
// Also, for now I'm ignoring the "pattern_name" and just getting everything...
// AND I don't try and skip the fonts we've already loaded in the defaults.
// Blimey! What a hack!
Fl_Font Fl_Xlib_Graphics_Driver::set_fonts(const char* pattern_name)
{
  FcFontSet  *fnt_set;     // Will hold the list of fonts we find
  FcPattern   *fnt_pattern; // Holds the generic "match all names" pattern
  FcObjectSet *fnt_obj_set = 0; // Holds the generic "match all objects"

  int j; // loop iterator variable
  int font_count; // Total number of fonts found to process
  char **full_list; // The list of font names we build

  if (fl_free_font > FL_FREE_FONT) // already been here
    return (Fl_Font)fl_free_font;

  fl_open_display(); // Just in case...

  // Make sure fontconfig is ready... is this necessary? The docs say it is
  // safe to call it multiple times, so just go for it anyway!
  if (!FcInit())
  {
    // What to do? Just return defaults...
    return FL_FREE_FONT;
  }

  // Create a search pattern that will match every font name - I think this
  // does the Right Thing, but am not certain...
  //
  // This could possibly be "enhanced" to pay attention to the requested
  // "pattern_name"?
  fnt_pattern = FcPatternCreate();
  fnt_obj_set = FcObjectSetBuild(FC_FAMILY, FC_STYLE, (void *)0);

  // Hopefully, this is a set of all the fonts...
  fnt_set = FcFontList(0, fnt_pattern, fnt_obj_set);

  // We don't need the fnt_pattern and fnt_obj_set any more, release them
  FcPatternDestroy(fnt_pattern);
  FcObjectSetDestroy(fnt_obj_set);

  // Now, if we got any fonts, iterate through them...
  if (fnt_set)
  {
    char *stop;
    char *start;
    char *first;

    font_count = fnt_set->nfont; // How many fonts?

    // Allocate array of char*'s to hold the name strings
    full_list = (char **)malloc(sizeof(char *) * font_count);

    // iterate through all the font patterns and get the names out...
      for (j = 0; j < font_count; j++)
      {
      // NOTE: FcChar8 is a typedef of "unsigned char"...
      FcChar8 *font; // String to hold the font's name

      // Convert from fontconfig internal pattern to human readable name
      // NOTE: This WILL malloc storage, so we need to free it later...
      font = FcNameUnparse(fnt_set->fonts[j]);

      // The returned strings look like this...
      // Century Schoolbook:style=Bold Italic,fed kursiv,Fett Kursiv,...
      // So the bit we want is up to the first comma - BUT some strings have
      // more than one name, separated by, guess what?, a comma...
      stop = start = first = 0;
      stop = strchr((char *)font, ',');
      start = strchr((char *)font, ':');
      if ((stop) && (start) && (stop < start))
      {
        first = stop + 1; // discard first version of name
        // find first comma *after* the end of the name
        stop = strchr((char *)start, ',');
      }
      else
      {
        first = (char *)font; // name is just what was returned
      }
      // Truncate the name after the (english) modifiers description
      // Matt: Actually, there is no guarantee that the *first* description is the English one.
      // Matt: So we keep the entire description, just in case.
      //if (stop)
      //{
      //  *stop = 0; // Terminate the string at the first comma, if there is one
      //}

      // Copy the font description into our list
      if (first == (char *)font)
      { // The listed name is still OK
        full_list[j] = (char *)font;
      }
      else
      { // The listed name has been modified
        full_list[j] = fl_strdup(first);
        // Free the font name storage
        free (font);
      }
      // replace "style=Regular" so strcmp sorts it first
      if (start) {
        char *reg = strstr(full_list[j], "=Regular");
        if (reg) reg[1]='.';
      }
    }

    // Release the fnt_set - we don't need it any more
    FcFontSetDestroy(fnt_set);

    // Sort the list into alphabetic order
    qsort(full_list, font_count, sizeof(*full_list), name_sort);

    // Now let us add the names we got to fltk's font list...
    for (j = 0; j < font_count; j++)
    {
      if (full_list[j])
      {
        char xft_name[LOCAL_RAW_NAME_MAX];
        char *stored_name;
        // Parse the strings into FLTK-XFT style..
        make_raw_name(xft_name, full_list[j]);
        // NOTE: This just adds on AFTER the default fonts - no attempt is made
        // to identify already loaded fonts. Is this bad?
        stored_name = fl_strdup(xft_name);
        Fl::set_font((Fl_Font)(j + FL_FREE_FONT), stored_name);
        fl_free_font ++;

        free(full_list[j]); // release that name from our internal array
      }
    }
    // Now we are done with the list, release it fully
    free(full_list);
  }
  return (Fl_Font)fl_free_font;
} // ::set_fonts

////////////////////////////////////////////////////////////////

//
// Draw fonts using Keith Packard's Xft library to provide anti-
// aliased text. Yow!
//
// Many thanks to Carl for making the original version of this.
//
// This font code only requires libXft to work.  Contrary to popular
// belief there is no need to have FreeType, or the Xrender extension
// available to use this code.  You will just get normal Xlib fonts
// (Xft calls them "core" fonts) The Xft algorithms for choosing
// these is about as good as the FLTK ones (I hope to fix it so it is
// exactly as good...), plus it can cache its results and share them
// between programs, so using this should be a win in all cases. Also
// it should be obvious by comparing this file and fl_font_x.cxx that
// it is a lot easier to program with Xft than with Xlib.
//
// Also, Xft supports UTF-8 text rendering directly, which will allow
// us to support UTF-8 on all platforms more easily.
//
// To actually get antialiasing you need the following:
//
//     1. You have XFree86 4
//     2. You have the XRender extension
//     3. Your X device driver supports the render extension
//     4. You have libXft
//     5. Your libXft has FreeType2 support compiled in
//     6. You have the FreeType2 library
//
// Distributions that have XFree86 4.0.3 or later should have all of this...
//
// Unlike some other Xft packages, I tried to keep this simple and not
// to work around the current problems in Xft by making the "patterns"
// complicated. I believe doing this defeats our ability to improve Xft
// itself. You should edit the ~/.xftconfig file to "fix" things, there
// are several web pages of information on how to do this.
//

//static const char* fl_encoding_ = "iso8859-1";
static const char* fl_encoding_ = "iso10646-1";


void Fl_Xlib_Graphics_Driver::font_unscaled(Fl_Font fnum, Fl_Fontsize size) {
  fl_xft_font(this, fnum, size, 0);
}

static XftFont* fontopen(const char* name, /*Fl_Fontsize*/double size, bool core, int angle) {
  // Check: does it look like we have been passed an old-school XLFD fontname?
  bool is_xlfd = false;
  int hyphen_count = 0;
  int comma_count = 0;
  unsigned len = strlen(name);
  if (len > 512) len = 512; // ensure we are not passed an unbounded font name
  for(unsigned idx = 0; idx < len; idx++) {
    if(name[idx] == '-') hyphen_count++; // check for XLFD hyphens
    if(name[idx] == ',') comma_count++;  // are there multiple names?
  }
  if(hyphen_count >= 14) is_xlfd = true; // Not a robust check, but good enough?

  fl_open_display();

  if(!is_xlfd) { // Not an XLFD - open as a XFT style name
    XftFont *the_font = NULL; // the font we will return;
    XftPattern *fnt_pat = XftPatternCreate(); // the pattern we will use for matching
    int slant = XFT_SLANT_ROMAN;
    int weight = XFT_WEIGHT_MEDIUM;

    /* This "converts" FLTK-style font names back into "regular" names, extracting
     * the BOLD and ITALIC codes as it does so - all FLTK font names are prefixed
     * by 'I' (italic) 'B' (bold) 'P' (bold italic) or ' ' (regular) modifiers.
     * This gives a fairly limited font selection ability, but is retained for
     * compatibility reasons. If you really need a more complex choice, you are best
     * calling Fl::set_fonts(*) then selecting the font by font-index rather than by
     * name anyway. Probably.
     * If you want to load a font who's name does actually begin with I, B or P, you
     * MUST use a leading space OR simply use lowercase for the name...
     */
    /* This may be efficient, but it is non-obvious. */
    switch (*name++) {
    case 'I': slant = XFT_SLANT_ITALIC; break; // italic
    case 'P': slant = XFT_SLANT_ITALIC;        // bold-italic (falls-through)
    case 'B': weight = XFT_WEIGHT_BOLD; break; // bold
    case ' ': break;                           // regular
    default: name--;                           // no prefix, restore name
    }

    if(comma_count) { // multiple comma-separated names were passed
      char *local_name = fl_strdup(name); // duplicate the full name so we can edit the copy
      char *curr = local_name; // points to first name in string
      char *nxt; // next name in string
      do {
        nxt = strchr(curr, ','); // find comma separator
        if (nxt) {
          *nxt = 0; // terminate first name
          nxt++; // first char of next name
        }

        // Add the current name to the match pattern
        XftPatternAddString(fnt_pat, XFT_FAMILY, curr);

        if(nxt) curr = nxt; // move onto next name (if it exists)
        // Now do a cut-down version of the FLTK name conversion.
        // NOTE: we only use the slant and weight of the first name,
        // subsequent names we ignore this for... But we still need to do the check.
        switch (*curr++) {
        case 'I': break; // italic
        case 'P':        // bold-italic (falls-through)
        case 'B': break; // bold
        case ' ': break; // regular
        default: curr--; // no prefix, restore name
        }

        comma_count--; // decrement name sections count
      } while (comma_count >= 0);
      free(local_name); // release our local copy of font names
    }
    else { // single name was passed - add it directly
      XftPatternAddString(fnt_pat, XFT_FAMILY, name);
    }

    // Construct a match pattern for the font we want...
    XftPatternAddInteger(fnt_pat, XFT_WEIGHT, weight);
    XftPatternAddInteger(fnt_pat, XFT_SLANT, slant);
    XftPatternAddDouble (fnt_pat, XFT_PIXEL_SIZE, (double)size);
    XftPatternAddString (fnt_pat, XFT_ENCODING, fl_encoding_);

    // rotate font if angle!=0
    if (angle !=0) {
      XftMatrix m;
      XftMatrixInit(&m);
      XftMatrixRotate(&m,cos(M_PI*angle/180.),sin(M_PI*angle/180.));
      XftPatternAddMatrix (fnt_pat, XFT_MATRIX,&m);
    }

    if (core) {
      XftPatternAddBool(fnt_pat, XFT_CORE, FcTrue);
      XftPatternAddBool(fnt_pat, XFT_RENDER, FcFalse);
    }

    XftPattern *match_pat;  // the best available match on the system
    XftResult match_result; // the result of our matching attempt

    // query the system to find a match for this font
    match_pat = XftFontMatch(fl_display, fl_screen, fnt_pat, &match_result);

#if 0 // the XftResult never seems to get set to anything... abandon this code?
    switch(match_result) { // how good a match is this font for our request?
      case XftResultMatch:
        puts("Object exists with the specified ID");
        break;

      case XftResultTypeMismatch:
        puts("Object exists, but the type does not match");
        break;

      case XftResultNoId:
        puts("Object exists, but has fewer values than specified");
        break;

      case FcResultOutOfMemory:
        puts("FcResult: Malloc failed");
        break;

      case XftResultNoMatch:
        puts("Object does not exist at all");
        break;

      default:
        printf("Invalid XftResult status %d \n", match_result);
        break;
    }
#endif

#if 0 // diagnostic to print the "full name" of the font we matched. This works.
    FcChar8 *picked_name =  FcNameUnparse(match_pat);
    printf("Match: %s\n", picked_name);
    free(picked_name);
#endif

    // open the matched font
    if (match_pat) the_font = XftFontOpenPattern(fl_display, match_pat);

    if (!match_pat || !the_font) {
      // last chance, just open any font in the right size
      the_font = XftFontOpen (fl_display, fl_screen,
                        XFT_FAMILY, XftTypeString, "sans",
                        XFT_SIZE, XftTypeDouble, (double)size,
                        NULL);
      XftPatternDestroy(fnt_pat);
      if (!the_font) {
        Fl::error("Unable to find fonts. Check your FontConfig configuration.\n");
        exit(1);
      }
      return the_font;
    }

#if 0 // diagnostic to print the "full name" of the font we actually opened. This works.
    FcChar8 *picked_name2 =  FcNameUnparse(the_font->pattern);
    printf("Open : %s\n", picked_name2);
    free(picked_name2);
#endif

    XftPatternDestroy(fnt_pat);
//  XftPatternDestroy(match_pat); // FontConfig will destroy this resource for us. We must not!

    return the_font;
  }
  else { // We were passed a font name in XLFD format
    /* OksiD's X font code could handle being passed a comma separated list
     * of XLFD's. It then attempted to find which font was "best" from this list.
     * But XftFontOpenXlfd can not do this, so if a list is passed, we just
     * terminate it at the first comma.
     * A "better" solution might be to use XftXlfdParse() on each of the passed
     * XLFD's to construct a "super-pattern" that incorporates attributes from all
     * XLFD's and use that to perform a XftFontMatch(). Maybe...
     */
    char *local_name = fl_strdup(name);
    if(comma_count) { // This means we were passed multiple XLFD's
      char *pc = strchr(local_name, ',');
      *pc = 0; // terminate the XLFD at the first comma
    }
    XftFont *the_font = XftFontOpenXlfd(fl_display, fl_screen, local_name);
    free(local_name);
#if 0 // diagnostic to print the "full name" of the font we actually opened. This works.
puts("Font Opened"); fflush(stdout);
    FcChar8 *picked_name2 =  FcNameUnparse(the_font->pattern);
    printf("Open : %s\n", picked_name2); fflush(stdout);
    free(picked_name2);
#endif
   return the_font;
  }
} // end of fontopen

Fl_Xlib_Font_Descriptor::Fl_Xlib_Font_Descriptor(const char* name, Fl_Fontsize fsize, int fangle) : Fl_Font_Descriptor(name, fsize) {
//  encoding = fl_encoding_;
  angle = fangle;
  font = fontopen(name, fsize, false, angle);
}


/* decodes the input UTF-8 string into a series of wchar_t characters.
 n is set upon return to the number of characters.
 Don't deallocate the returned memory.
 */
static const wchar_t *utf8reformat(const char *str, int& n)
{
  static const wchar_t empty[] = {0};
  static wchar_t *buffer;
  static int lbuf = 0;
  int newn;
  if (n == 0) return empty;
  newn = fl_utf8towc(str, n, (wchar_t*)buffer, lbuf);
  if (newn >= lbuf) {
    lbuf = newn + 100;
    if (buffer) free(buffer);
    buffer = (wchar_t*)malloc(lbuf * sizeof(wchar_t));
    n = fl_utf8towc(str, n, (wchar_t*)buffer, lbuf);
  } else {
    n = newn;
  }
  return buffer;
}

static void utf8extents(Fl_Xlib_Font_Descriptor *desc, const char *str, int n, XGlyphInfo *extents)
{
  memset(extents, 0, sizeof(XGlyphInfo));
  const wchar_t *buffer = utf8reformat(str, n);
#ifdef __CYGWIN__
    XftTextExtents16(fl_display, desc->font, (XftChar16 *)buffer, n, extents);
#else
    XftTextExtents32(fl_display, desc->font, (XftChar32 *)buffer, n, extents);
#endif
}

int Fl_Xlib_Graphics_Driver::height_unscaled() {
  if (font_descriptor()) return ((Fl_Xlib_Font_Descriptor*)font_descriptor())->font->ascent + ((Fl_Xlib_Font_Descriptor*)font_descriptor())->font->descent;
  else return -1;
}

int Fl_Xlib_Graphics_Driver::descent_unscaled() {
  if (font_descriptor()) return ((Fl_Xlib_Font_Descriptor*)font_descriptor())->font->descent;
  else return -1;
}

double Fl_Xlib_Graphics_Driver::width_unscaled(const char* str, int n) {
  if (!font_descriptor()) return -1.0;
  XGlyphInfo i;
  utf8extents((Fl_Xlib_Font_Descriptor*)font_descriptor(), str, n, &i);
  return i.xOff;
}

static double fl_xft_width(Fl_Font_Descriptor *desc, FcChar32 *str, int n) {
  if (!desc) return -1.0;
  XGlyphInfo i;
  XftTextExtents32(fl_display, ((Fl_Xlib_Font_Descriptor*)desc)->font, str, n, &i);
  return i.xOff;
}

double Fl_Xlib_Graphics_Driver::width_unscaled(unsigned int c) {
  if (!font_descriptor()) return -1.0;
  return fl_xft_width(font_descriptor(), (FcChar32 *)(&c), 1);
}

void Fl_Xlib_Graphics_Driver::text_extents_unscaled(const char *c, int n, int &dx, int &dy, int &w, int &h) {
  if (!font_descriptor()) {
    w = h = 0;
    dx = dy = 0;
    return;
  }
  XGlyphInfo gi;
  utf8extents((Fl_Xlib_Font_Descriptor*)font_descriptor(), c, n, &gi);

  w = gi.width;
  h = gi.height;
  dx = -gi.x ;
  dy = -gi.y ;
  correct_extents(scale(), dx, dy, w, h);
}

void Fl_Xlib_Graphics_Driver::draw_unscaled(const char *str, int n, int x, int y) {

  // transform coordinates and clip if outside 16-bit space (STR 2798)

  int x1 = x + floor(offset_x_) ;
  if (x1 < clip_min() || x1 > clip_max()) return;

  int y1 = y + floor(offset_y_) ;
  if (y1 < clip_min() || y1 > clip_max()) return;

  if (!draw_)
    draw_ = XftDrawCreate(fl_display, draw_window = fl_window,
                         fl_visual->visual, fl_colormap);
  else //if (draw_window != fl_window)
    XftDrawChange(draw_, draw_window = fl_window);

  Region region = (Region)fl_clip_region();
  if (!(region && XEmptyRegion(region))) {
    XftDrawSetClip(draw_, region);

    // Use fltk's color allocator, copy the results to match what
    // XftCollorAllocValue returns:
    XftColor color;
    color.pixel = fl_xpixel(Fl_Graphics_Driver::color());
    uchar r,g,b; Fl::get_color(Fl_Graphics_Driver::color(), r,g,b);
    color.color.red   = ((int)r)*0x101;
    color.color.green = ((int)g)*0x101;
    color.color.blue  = ((int)b)*0x101;
    color.color.alpha = 0xffff;

    const wchar_t *buffer = utf8reformat(str, n);
#ifdef __CYGWIN__
    XftDrawString16(draw_, &color, ((Fl_Xlib_Font_Descriptor*)font_descriptor())->font, x1, y1, (XftChar16 *)buffer, n);
#else
    XftDrawString32(draw_, &color, ((Fl_Xlib_Font_Descriptor*)font_descriptor())->font, x1, y1, (XftChar32 *)buffer, n);
#endif
  }
}

void Fl_Xlib_Graphics_Driver::draw_unscaled(int angle, const char *str, int n, int x, int y) {
  fl_xft_font(this, this->Fl_Graphics_Driver::font(), this->size_unscaled(), angle);
  this->draw_unscaled(str, n, x, y);
  fl_xft_font(this, this->Fl_Graphics_Driver::font(), this->size_unscaled(), 0);
}

void Fl_Xlib_Graphics_Driver::drawUCS4(const void *str, int n, int x, int y) {
  if (!draw_)
    draw_ = XftDrawCreate(fl_display, draw_window = fl_window,
                         fl_visual->visual, fl_colormap);
  else //if (draw_window != fl_window)
    XftDrawChange(draw_, draw_window = fl_window);

  Region region = (Region)fl_clip_region();
  if (region && XEmptyRegion(region)) return;
  XftDrawSetClip(draw_, region);

  // Use fltk's color allocator, copy the results to match what
  // XftCollorAllocValue returns:
  XftColor color;
  color.pixel = fl_xpixel(this->color());
  uchar r,g,b; Fl::get_color(this->color(), r,g,b);
  color.color.red   = ((int)r)*0x101;
  color.color.green = ((int)g)*0x101;
  color.color.blue  = ((int)b)*0x101;
  color.color.alpha = 0xffff;

  XftDrawString32(draw_, &color, ((Fl_Xlib_Font_Descriptor*)font_descriptor())->font, x+floor(offset_x_), y+floor(offset_y_), (FcChar32 *)str, n);
}


void Fl_Xlib_Graphics_Driver::rtl_draw_unscaled(const char* c, int n, int x, int y) {

  // clip if outside 16-bit space (STR 2798)

  if (x < clip_min() || x > clip_max()) return;
  if (y < clip_min() || y > clip_max()) return;

#if defined(__GNUC__)
// FIXME: warning Need to improve this XFT right to left draw function
#endif /*__GNUC__*/

// This actually draws LtoR, but aligned to R edge with the glyph order reversed...
// but you can't just byte-rev a UTF-8 string, that isn't valid.
// You can reverse a UCS4 string though...
  int num_chars, wid, utf_len = strlen(c);
  FcChar8 *u8 = (FcChar8 *)c;
  FcBool valid = FcUtf8Len(u8, utf_len, &num_chars, &wid);
  if (!valid)
  {
    // badly formed Utf-8 input string
    return;
  }
  if (num_chars < n) n = num_chars; // limit drawing to usable characters in input array
  FcChar32 *ucs_txt = new FcChar32[n+1];
  FcChar32* pu;
  int out, sz;
  ucs_txt[n] = 0;
  out = n-1;
  while ((out >= 0) && (utf_len > 0))
  {
    pu = &ucs_txt[out];
    sz = FcUtf8ToUcs4(u8, pu, utf_len);
    utf_len = utf_len - sz;
    u8 = u8 + sz;
    out = out - 1;
  }
  // Now we have a UCS4 version of the input text, reversed, in ucs_txt
  int offs = (int)fl_xft_width(font_descriptor(), ucs_txt, n);
  drawUCS4(ucs_txt, n, (x-offs), y);

  delete[] ucs_txt;
}


extern "C" {
  static int int_sort(const void *aa, const void *bb) {
    return (*(int*)aa)-(*(int*)bb);
  }
}

////////////////////////////////////////////////////////////////

// Return all the point sizes supported by this font:
// Suprisingly enough Xft works exactly like fltk does and returns
// the same list. Except there is no way to tell if the font is scalable.
int Fl_Xlib_Graphics_Driver::get_font_sizes(Fl_Font fnum, int*& sizep) {
  Fl_Fontdesc *s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // empty slot in table, use entry 0

  fl_open_display();
  XftFontSet* fs = XftListFonts(fl_display, fl_screen,
                                XFT_FAMILY, XftTypeString, s->name+1,
                                (void *)0,
                                XFT_PIXEL_SIZE,
                                (void *)0);
  static int* array = 0;
  static int array_size = 0;
  if (fs->nfont >= array_size) {
    delete[] array;
    array = new int[array_size = fs->nfont+1];
  }
  array[0] = 0; int j = 1; // claim all fonts are scalable
  for (int i = 0; i < fs->nfont; i++) {
    double v;
    if (XftPatternGetDouble(fs->fonts[i], XFT_PIXEL_SIZE, 0, &v) == XftResultMatch) {
      array[j++] = int(v);
    }
  }
  qsort(array+1, j-1, sizeof(int), int_sort);
  XftFontSetDestroy(fs);
  sizep = array;
  return j;
}

#endif // !USE_PANGO

float Fl_Xlib_Graphics_Driver::scale_font_for_PostScript(Fl_Font_Descriptor *desc, int s) {
  // Xft font height is sometimes larger than the required size (see STR 2566).
  // Increase the PostScript font size by 15% without exceeding the display font height
  int max = height_unscaled();
  float ps_size = s * 1.15;
  if (ps_size > max) ps_size = max;
  return ps_size;
}

// Bug: older versions calculated the value for *ap as a side effect of
// making the name, and then forgot about it. To avoid having to change
// the header files I decided to store this value in the last character
// of the font name array.
#define ENDOFBUFFER 127 // sizeof(Fl_Font.fontname)-1

// turn a stored font name in "fltk format" into a pretty name:
const char* Fl_Xlib_Graphics_Driver::get_font_name(Fl_Font fnum, int* ap) {
  Fl_Fontdesc *f = fl_fonts + fnum;
  if (!f->fontname[0]) {
    const char* p = f->name;
    int type;
#if USE_PANGO
    type = 0;
    if (strstr(p, " Bold")) type = FL_BOLD;
    if (strstr(p, " Italic") || strstr(p, " Oblique")) type += FL_ITALIC;
    strlcpy(f->fontname, p, ENDOFBUFFER);
#else
    switch (p[0]) {
      case 'B': type = FL_BOLD; break;
      case 'I': type = FL_ITALIC; break;
      case 'P': type = FL_BOLD | FL_ITALIC; break;
      default:  type = 0; break;
    }
  // NOTE: This can cause duplications in fonts that already have Bold or Italic in
  // their "name". Maybe we need to find a cleverer way?
    strlcpy(f->fontname, p+1, ENDOFBUFFER);
    if (type & FL_BOLD) strlcat(f->fontname, " bold", ENDOFBUFFER);
    if (type & FL_ITALIC) strlcat(f->fontname, " italic", ENDOFBUFFER);
#endif // USE_PANGO
    f->fontname[ENDOFBUFFER] = (char)type;
  }
  if (ap) *ap = f->fontname[ENDOFBUFFER];
  return f->fontname;
}

Fl_Xlib_Font_Descriptor::~Fl_Xlib_Font_Descriptor() {
  if (this == fl_graphics_driver->font_descriptor()) fl_graphics_driver->font_descriptor(NULL);
  //  XftFontClose(fl_display, font);
#if USE_PANGO
  if (width) for (int i = 0; i < 64; i++) delete[] width[i];
  delete[] width;
#endif
}


void Fl_Xlib_Graphics_Driver::destroy_xft_draw(Window id) {
  if (id == draw_window)
    XftDrawChange(draw_, draw_window = fl_message_window);
}

void *fl_xftfont = 0; // always 0 under Pango
static void fl_xft_font(Fl_Xlib_Graphics_Driver *driver, Fl_Font fnum, Fl_Fontsize size, int angle) {
  if (fnum==-1) { // special case to stop font caching
    driver->Fl_Graphics_Driver::font(0, 0);
    return;
  }
  Fl_Xlib_Font_Descriptor* f = (Fl_Xlib_Font_Descriptor*)driver->font_descriptor();
  if (fnum == driver->Fl_Graphics_Driver::font() && size == driver->size_unscaled() && f && f->angle == angle)
    return;
  driver->Fl_Graphics_Driver::font(fnum, size);
  Fl_Fontdesc *font = fl_fonts + fnum;
  // search the fontsizes we have generated already
  for (f = (Fl_Xlib_Font_Descriptor*)font->first; f; f = (Fl_Xlib_Font_Descriptor*)f->next) {
    if (f->size == size && f->angle == angle)// && !strcasecmp(f->encoding, fl_encoding_))
      break;
  }
  if (!f) {
    f = new Fl_Xlib_Font_Descriptor(font->name, size, angle);
    f->next = font->first;
    font->first = f;
  }
  driver->font_descriptor(f);
#if XFT_MAJOR < 2 && ! USE_PANGO
  fl_xfont    = f->font->u.core.font;
#else
  fl_xfont    = NULL; // invalidate
#endif // XFT_MAJOR < 2
#if USE_PANGO
  fl_xftfont = NULL;
#else
  fl_xftfont = (void*)f->font;
#endif
}

/* This code is used (mainly by opengl) to get a bitmapped font. The
 * original XFT-1 code used XFT's "core" fonts methods to load an XFT
 * font that was actually a X-bitmap font, that could then be readily
 * used with GL.  But XFT-2 does not provide that ability, and there
 * is no easy method to use an XFT font directly with GL. So...
 */
#  if XFT_MAJOR > 1 || USE_PANGO
// This function attempts, on XFT2 systems, to find a suitable "core" Xfont
// for GL or other bitmap font needs (we dont have an XglUseXftFont(...) function.)
// There's probably a better way to do this. I can't believe it is this hard...
// Anyway... This code attempts to make an XLFD out of the fltk-style font
// name it is passed, then tries to load that font. Surprisingly, this quite
// often works - boxes that have XFT generally also have a fontserver that
// can serve TTF and other fonts to X, and so the font name that fltk makes
// from the XFT name often also "exists" as an "core" X font...
// If this code fails to load the requested font, it falls back through a
// series of tried 'n tested alternatives, ultimately resorting to what the
// original fltk code did.
// NOTE: On my test boxes (FC6, FC7, FC8, ubuntu8.04, 9.04, 9.10) this works
//       well for the fltk "built-in" font names.
static XFontStruct* load_xfont_for_xft2(Fl_Graphics_Driver *driver) {
  XFontStruct* xgl_font = 0;
  int size = ((Fl_Xlib_Graphics_Driver*)driver)->size_unscaled();
  int fnum = driver->font();
  const char *wt_med = "medium";
  const char *wt_bold = "bold";
  const char *weight = wt_med; // no specifc weight requested - accept any
  char slant = 'r';   // regular non-italic by default
  char xlfd[128];     // we will put our synthetic XLFD in here
  char *pc = fl_strdup(fl_fonts[fnum].name); // what font were we asked for?
#if USE_PANGO
  char *p = pc + 1;
  while (*p) { *p = tolower(*p); p++; }
#endif // USE_PANGO
  const char *name = pc;    // keep a handle to the original name for freeing later
  // Parse the "fltk-name" of the font
  switch (*name++) {
    case 'I': slant = 'i'; break;       // italic
    case 'P': slant = 'i';              // bold-italic (falls-through)
    case 'B': weight = wt_bold; break;  // bold
    case ' ': break;                    // regular
    default: name--;                    // no prefix, restore name
  }

  // first, we do a query with no prefered size, to see if the font exists at all
  snprintf(xlfd, 128, "-*-%s-%s-%c-*--*-*-*-*-*-*-*-*", name, weight, slant); // make up xlfd style name
  xgl_font = XLoadQueryFont(fl_display, xlfd);
  if(xgl_font) { // the face exists, but can we get it in a suitable size?
    XFreeFont(fl_display, xgl_font); // release the non-sized version
    snprintf(xlfd, 128, "-*-%s-%s-%c-*--*-%d-*-*-*-*-*-*", name, weight, slant, (size*10));
    xgl_font = XLoadQueryFont(fl_display, xlfd); // attempt to load the font at the right size
  }
//puts(xlfd);

  // try alternative names
  if (!xgl_font) {
    if (!strcmp(name, "sans")) {
      name = "helvetica";
      if (slant == 'i') slant = 'o';
    } else if (!strcmp(name, "mono") || !strcmp(name, "monospace")) {
      name = "courier";
    } else if (!strcmp(name, "serif")) {
      name = "times";
    } else if (!strcmp(name, "screen")) {
      name = "lucidatypewriter";
    } else if (!strcmp(name, "dingbats")) {
      name = "zapf dingbats";
    }
    snprintf(xlfd, 128, "-*-*%s*-%s-%c-*--*-%d-*-*-*-*-*-*", name, weight, slant, (size*10));
    xgl_font = XLoadQueryFont(fl_display, xlfd);
  }
  free(pc); // release our copy of the font name

  // if we have nothing loaded, try a generic proportional font
  if(!xgl_font) {
    snprintf(xlfd, 128, "-*-helvetica-*-%c-*--*-%d-*-*-*-*-*-*", slant, (size*10));
    xgl_font = XLoadQueryFont(fl_display, xlfd);
  }
  // If that still didn't work, try courier with resquested weight and slant
  if(!xgl_font && weight != wt_med) {
    snprintf(xlfd, 128, "-*-courier*-%s-%c-*--*-%d-*-*-*-*-*-*", weight, slant, (size*10));
    xgl_font = XLoadQueryFont(fl_display, xlfd);
  }
  // If that still didn't work, try this instead
  if(!xgl_font) {
    snprintf(xlfd, 128, "-*-courier*-medium-%c-*--*-%d-*-*-*-*-*-*", slant, (size*10));
    xgl_font = XLoadQueryFont(fl_display, xlfd);
  }
//printf("glf: %d\n%s\n%s\n", size, xlfd, fl_fonts[fl_font_].name);
//if(xgl_font) puts("ok");

  // Last chance fallback - this usually loads something...
  if (!xgl_font) xgl_font = XLoadQueryFont(fl_display, "fixed");

  return xgl_font;
} // end of load_xfont_for_xft2
#  endif

static XFontStruct* fl_xxfont(Fl_Graphics_Driver *driver) {
#  if XFT_MAJOR > 1 || USE_PANGO
  // kludge! XFT 2 and later does not provide core fonts for us to use with GL
  // try to load a bitmap X font instead
  static XFontStruct* xgl_font = 0;
  static int glsize = 0;
  static int glfont = -1;
  // Do we need to load a new font?
  if ((!xgl_font) || (glsize != ((Fl_Xlib_Graphics_Driver*)driver)->size_unscaled()) || (glfont != driver->font())) {
    // create a dummy XLFD for some font of the appropriate size...
    if (xgl_font) XFreeFont(fl_display, xgl_font); // font already loaded, free it - this *might* be a Bad Idea
    glsize = ((Fl_Xlib_Graphics_Driver*)driver)->size_unscaled(); // record current font size
    glfont = driver->font(); // and face
    xgl_font = load_xfont_for_xft2(driver);
  }
  return xgl_font;
#  else // XFT-1 provides a means to load a "core" font directly
  if (driver->font_descriptor()->font->core) {
    return driver->font_descriptor()->font->u.core.font; // is the current font a "core" font? If so, use it.
  }
  static XftFont* xftfont;
  if (xftfont) XftFontClose (fl_display, xftfont);
  xftfont = fontopen(fl_fonts[driver->font()].name, ((Fl_Xlib_Graphics_Driver*)driver)->size_unscaled(), true, 0); // else request XFT to load a suitable "core" font instead.
  return xftfont->u.core.font;
#  endif // XFT_MAJOR > 1
}

XFontStruct* Fl_XFont_On_Demand::value() {
  if (!ptr) ptr = fl_xxfont(fl_graphics_driver);
  return ptr;
}

#if USE_PANGO

#include <pango/pangoxft.h>
#include <pango/pango.h>
#if ! PANGO_VERSION_CHECK(1,8,0)
#error "Requires Pango 1.8 or higher"
#endif

PangoFontMap *Fl_Xlib_Graphics_Driver::pfmap_ = 0;
PangoContext *Fl_Xlib_Graphics_Driver::pctxt_ = 0;
PangoLayout *Fl_Xlib_Graphics_Driver::playout_ = 0;

PangoContext *Fl_Xlib_Graphics_Driver::context() {
  if (fl_display && !pctxt_) {
    pfmap_ = pango_xft_get_font_map(fl_display, fl_screen); // 1.2
#if PANGO_VERSION_CHECK(1,22,0)
    pctxt_ = pango_font_map_create_context(pfmap_); // 1.22
#else
    pctxt_ = pango_xft_get_context(fl_display, fl_screen); // deprecated since 1.22
#endif
    playout_ = pango_layout_new(pctxt_);
  }
  return pctxt_;
}


void Fl_Xlib_Graphics_Driver::font_unscaled(Fl_Font fnum, Fl_Fontsize size) {
  if (!size) return;
  if (size < 0) {
    Fl_Graphics_Driver::font(0, 0);
    return;
  }
  if (this->Fl_Graphics_Driver::font() == fnum && this->size_unscaled() == size && this->font_descriptor()) {
    pango_font_description_set_absolute_size(pfd_array[fnum], size*PANGO_SCALE); // 1.8
    return;
  }
  fl_xft_font(this, fnum, size, 0);
  init_built_in_fonts();
  if (fnum >= pfd_array_length) {
    int new_length = fnum + 10;
    PangoFontDescription **data = (PangoFontDescription**)calloc(new_length, sizeof(PangoFontDescription*));
    memcpy(data, pfd_array, pfd_array_length*sizeof(PangoFontDescription*));
    free(pfd_array);
    pfd_array = data;
    pfd_array_length = new_length;
  }
  if (!pfd_array[fnum]) {
    pfd_array[fnum] = pango_font_description_from_string(Fl::get_font_name(fnum));
  }
  pango_font_description_set_absolute_size(pfd_array[fnum], size*PANGO_SCALE); // 1.8
  if (!pctxt_) context();
  Fl_Xlib_Font_Descriptor *fd = (Fl_Xlib_Font_Descriptor*)font_descriptor();
  if (!fd->height_) {
    PangoFont *pfont = pango_font_map_load_font(pfmap_, pctxt_, pfd_array[fnum]);
    PangoRectangle logical_rect;
    pango_font_get_glyph_extents(pfont, /*PangoGlyph glyph*/'p', NULL, &logical_rect);
    fd->descent_ = PANGO_DESCENT(logical_rect)/PANGO_SCALE;
    fd->height_ = logical_rect.height/PANGO_SCALE;
    if (fd->height_ == 0) { // this occurs with some fonts
      PangoFontMetrics *pfmt = pango_font_get_metrics(pfont, pango_script_get_sample_language(PANGO_SCRIPT_LATIN) /* 1.4 */);
      fd->descent_ = pango_font_metrics_get_descent(pfmt)/PANGO_SCALE;
      fd->height_ = (pango_font_metrics_get_ascent(pfmt) + pango_font_metrics_get_descent(pfmt))/PANGO_SCALE ;
      pango_font_metrics_unref(pfmt);
    }
  }
}

void Fl_Xlib_Graphics_Driver::draw_unscaled(const char *str, int n, int x, int y) {
  do_draw(0, str, n, x + floor(offset_x_), y + floor(offset_y_));
}

void Fl_Xlib_Graphics_Driver::draw_unscaled(int angle, const char *str, int n, int x, int y) {
  PangoMatrix mat = PANGO_MATRIX_INIT; // 1.6
  pango_matrix_translate(&mat, x + floor(offset_x_), y + floor(offset_y_)); // 1.6
  double l = width_unscaled(str, n);
  pango_matrix_rotate(&mat, angle); // 1.6
  pango_context_set_matrix(pctxt_, &mat); // 1.6
  str = Fl_Cairo_Graphics_Driver::clean_utf8(str, n);
  pango_layout_set_text(playout_, str, n);
  int w, h;
  pango_layout_get_pixel_size(playout_, &w, &h);
  pango_matrix_scale(&mat, l/w, l/w); // 1.6
  pango_context_set_matrix(pctxt_, &mat); // 1.6
  do_draw(0, str, n, 0, 0);
  pango_context_set_matrix(pctxt_, NULL); // 1.6
}

void Fl_Xlib_Graphics_Driver::rtl_draw_unscaled(const char* str, int n, int x, int y) {
  do_draw(1, str, n, x+floor(offset_x_), y+floor(offset_y_));
}

/* Compute dx, dy, w, h so that fl_rect(x+dx, y+dy, w, h) is the bounding box
 of text drawn by fl_draw(str, n, x, y).
 Also, compute y_correction to be used to correct the text's y coordinate to make sure
 drawn text does not extend below the bottom of the line of text.
 */
static void fl_pango_layout_get_pixel_extents(PangoLayout *layout, int &dx, int &dy, int &w, int &h, int desc, int lheight, int &y_correction) {
  PangoRectangle ink_rect;
  pango_layout_get_pixel_extents(layout, &ink_rect, NULL);
  dx = ink_rect.x;
  dy = ink_rect.y - lheight + desc;
  w = ink_rect.width;
  h = ink_rect.height;
  int y = ink_rect.y + ink_rect.height;
  y_correction = (y > lheight ? y - lheight : 0);
}

void Fl_Xlib_Graphics_Driver::do_draw(int from_right, const char *str, int n, int x, int y) {
  if (!fl_display || n == 0) return;
  Region region = (Region)clip_region();
  if (region && XEmptyRegion(region)) return;
  if (!playout_) context();

  char *str2 = NULL;
  const char *tmpv = (const char *)memchr(str, '\n', n);
  if (tmpv == str + n - 1) { // ignore final '\n'
    if (--n == 0) return;
    tmpv = NULL;
  }
  pango_layout_set_font_description(playout_, pfd_array[font_]);
  if (tmpv) { // replace newlines by spaces in a copy of str
    str2 = (char*)malloc(n);
    memcpy(str2, str, n);
    do {
      str2[tmpv - str] = ' ';
      if (tmpv >= str + n - 1) break;
      tmpv = (const char *)memchr(tmpv + 1, '\n', n - (tmpv - str + 1));
    }
    while (tmpv);
    str = str2;
  }
  const char *old = 0;
  if (!str2) old = pango_layout_get_text(playout_);
  str = Fl_Cairo_Graphics_Driver::clean_utf8(str, n);
  if (!old || (int)strlen(old) != n || memcmp(str, old, n)) // do not re-set text if equal to text already in layout
        pango_layout_set_text(playout_, str, n);
  if (str2) free(str2);

  XftColor color;
  Fl_Color c = this->color();
  color.pixel = fl_xpixel(c);
  uchar r,g,b; Fl::get_color(c, r,g,b);
  color.color.red   = ((int)r)*0x101;
  color.color.green = ((int)g)*0x101;
  color.color.blue  = ((int)b)*0x101;
  color.color.alpha = 0xffff;
  if (!draw_)
    draw_ = XftDrawCreate(fl_display, draw_window = fl_window, fl_visual->visual, fl_colormap);
  else
    XftDrawChange(draw_, draw_window = fl_window);
  XftDrawSetClip(draw_, region);

  int  dx, dy, w, h, y_correction, desc = descent_unscaled(), lheight = height_unscaled();
  fl_pango_layout_get_pixel_extents(playout_, dx, dy, w, h, desc, lheight, y_correction);
  if (from_right) {
    x -= w;
  }
  pango_xft_render_layout(draw_, &color, playout_, x * PANGO_SCALE,
                          (y - y_correction  - lheight + desc) * PANGO_SCALE ); // 1.8
  }

// cache the widths of single Unicode characters
double Fl_Xlib_Graphics_Driver::width_unscaled(unsigned int utf32) {
  unsigned r=0;
  Fl_Xlib_Font_Descriptor *desc = NULL;
  if (utf32 <= 0xFFFF) {
    desc = (Fl_Xlib_Font_Descriptor*)font_descriptor();
    r = (utf32 & 0xFC00) >> 10;
    if (!desc->width) {
      desc->width = (int**)new int*[64];
      memset(desc->width, 0, 64*sizeof(int*));
    }
    if (!desc->width[r]) {
      desc->width[r] = (int*)new int[0x0400];
      for (int i = 0; i < 0x0400; i++) desc->width[r][i] = -1;
    } else {
      if ( desc->width[r][utf32&0x03FF] >= 0 ) { // already cached
        return double(desc->width[r][utf32 & 0x03FF]);
      }
    }
  }
  char buf4[4];
  int n = fl_utf8encode(utf32, buf4);
  double width = do_width_unscaled_(buf4, n);
  if (utf32 <= 0xFFFF) {
    desc->width[r][utf32 & 0x03FF] = (int)width;
  }
  return width;
}

double Fl_Xlib_Graphics_Driver::width_unscaled(const char* str, int n) {
  if ((str == NULL) || (n == 0)) return 0.;
  if (n == fl_utf8len(*str)) { // str contains a single unicode character
    int l;
    unsigned c = fl_utf8decode(str, str+n, &l);
    return width_unscaled(c); // that character's width may have been cached
  }
  return do_width_unscaled_(str, n); // full width computation for multi-char strings
}

double Fl_Xlib_Graphics_Driver::do_width_unscaled_(const char* str, int n) {
  if (!n) return 0;
  if (!fl_display || size_ == 0) return -1;
  if (!playout_) context();
  int width, height;
  pango_layout_set_font_description(playout_, pfd_array[font_]);
  str = Fl_Cairo_Graphics_Driver::clean_utf8(str, n);
  pango_layout_set_text(playout_, str, n);
  pango_layout_get_pixel_size(playout_, &width, &height);
  return (double)width;
}

void Fl_Xlib_Graphics_Driver::text_extents_unscaled(const char *str, int n, int &dx, int &dy, int &w, int &h) {
  if (!playout_) context();
  pango_layout_set_font_description(playout_, pfd_array[font_]);
  str = Fl_Cairo_Graphics_Driver::clean_utf8(str, n);
  pango_layout_set_text(playout_, str, n);
  int y_correction;
  fl_pango_layout_get_pixel_extents(playout_, dx, dy, w, h, descent_unscaled(), height_unscaled(), y_correction);
  dy -= y_correction;
  correct_extents(scale(), dx, dy, w, h);
}

int Fl_Xlib_Graphics_Driver::height_unscaled() {
  if (font_descriptor())  return ((Fl_Xlib_Font_Descriptor*)font_descriptor())->height_;
  else return -1;
}

int Fl_Xlib_Graphics_Driver::descent_unscaled() {
  if (font_descriptor()) return ((Fl_Xlib_Font_Descriptor*)font_descriptor())->descent_;
  else return -1;
}

// FIXME: this (static) function should likely be in Fl_Graphics_Driver.
// The code is the same as in src/drivers/Cairo/Fl_Cairo_Graphics_Driver.cxx

static int font_name_process(const char *name, char &face) {
  int l = strlen(name);
  face = ' ';
  if      (l >  8 && !memcmp(name + l -  8, " Regular", 8)) l -= 8;
  else if (l >  6 && !memcmp(name + l -  6, " Plain", 6)) l -= 6;
  else if (l > 12 && !memcmp(name + l - 12, " Bold Italic", 12)) {l -= 12; face = 'P';}
  else if (l >  7 && !memcmp(name + l -  7, " Italic", 7)) {l -= 7; face = 'I';}
  else if (l >  5 && !memcmp(name + l -  5, " Bold", 5)) {l -= 5; face = 'B';}
  return l;
}

typedef int (*sort_f_type)(const void *aa, const void *bb);

static int font_sort(Fl_Fontdesc *fa, Fl_Fontdesc *fb) {
  char face_a, face_b;
  int la = font_name_process(fa->name, face_a);
  int lb = font_name_process(fb->name, face_b);
  int c = strncasecmp(fa->name, fb->name, la >= lb ? lb : la);
  return (c == 0 ? face_a - face_b : c);
}

Fl_Font Fl_Xlib_Graphics_Driver::set_fonts(const char* pattern_name)
{
  fl_open_display();
  int n_families, count = 0;
  PangoFontFamily **families;
  char *saved_lang = fl_getenv("LANG");
  const char *Clang = "LANG=C";
  if (saved_lang && strcmp(saved_lang, Clang)) {
    // Force LANG=C to prevent pango_font_face_get_face_name() below from returning
    // translated versions of Bold, Italic, etc… (see issue #732).
    // Unfortunately, using setlocale() doesn't do the job.
    char *p = saved_lang;
    saved_lang = (char*)malloc(strlen(p) + 6);
    memcpy(saved_lang, "LANG=", 5);
    strcpy(saved_lang + 5, p);
    fl_putenv(Clang);
  } else saved_lang = NULL;
  Fl_Xlib_Graphics_Driver::context();
  Fl_Xlib_Graphics_Driver::init_built_in_fonts();
  pango_font_map_list_families(Fl_Xlib_Graphics_Driver::pfmap_, &families, &n_families);
  for (int fam = 0; fam < n_families; fam++) {
    PangoFontFace **faces;
    int n_faces;
    const char *fam_name = pango_font_family_get_name (families[fam]);
    int lfam = strlen(fam_name);
    pango_font_family_list_faces(families[fam], &faces, &n_faces);
    for (int j = 0; j < n_faces; j++) {
      const char *p = pango_font_face_get_face_name(faces[j]);
      // Remove " Regular" suffix from font names
      if (!strcasecmp(p, "regular")) p = NULL;
      // build the font's FLTK name
      int lfont = lfam + (p ? strlen(p) + 2 : 1);
      char *q = new char[lfont];
      if (p) snprintf(q, lfont, "%s %s", fam_name, p);
      else strcpy(q, fam_name);
      Fl::set_font((Fl_Font)(count++ + FL_FREE_FONT), q);
    }
    /*g_*/free(faces); // glib source code shows that g_free is equivalent to free
  }
  /*g_*/free(families);
  if (saved_lang) {
    fl_putenv(saved_lang);
    free(saved_lang);
  }
  // Sort the list into alphabetic order
  qsort(fl_fonts + FL_FREE_FONT, count, sizeof(Fl_Fontdesc), (sort_f_type)font_sort);
  return FL_FREE_FONT + count;
}


void Fl_Xlib_Graphics_Driver::init_built_in_fonts() {
  static int i = 0;
  if (!i) {
    while (i < FL_FREE_FONT) {
      i++;
      Fl::set_font((Fl_Font)i-1, fl_fonts[i-1].name);
    }
  }
}


int Fl_Xlib_Graphics_Driver::get_font_sizes(Fl_Font fnum, int*& sizep) {
  static int array[1] = {0};
  sizep = array;
  return 1;
}

Fl_Xlib_Font_Descriptor::Fl_Xlib_Font_Descriptor(const char* name, Fl_Fontsize fsize, int fangle) : Fl_Font_Descriptor(name, fsize) {
  fl_open_display();
  angle = fangle;
  height_ = 0;
  descent_ = 0;
  width = NULL;
}

#endif // USE_PANGO

#endif // FL_DOXYGEN
