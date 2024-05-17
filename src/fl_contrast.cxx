//
// Color contrast functions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

/**
  \file fl_contrast.cxx
  \brief Color contrast handling

  Implementation of fl_contrast() and its variants.
*/

#include <FL/Fl.H>
#include <math.h>

#ifndef DEBUG_CONTRAST_LEGACY
#define DEBUG_CONTRAST_LEGACY 0
#endif // DEBUG_CONTRAST_LEGACY

// Initial values of global/static variables defined by fl_contrast_* functions.

// This defines the default contrast mode since FLTK 1.4.0
static int fl_contrast_mode_ = FL_CONTRAST_CIELAB;

// This defines the default contrast level per contrast mode
static int fl_contrast_level_[10] = {
         0,                     // 0 = FL_CONTRAST_NONE
        50,                     // 1 = FL_CONTRAST_LEGACY
        39,                     // 2 = FL_CONTRAST_CIELAB
         0,                     // 3 = FL_CONTRAST_CUSTOM
         0                      // 4-9 = not yet defined
};

// There is no default custom contrast function
static Fl_Contrast_Function *fl_contrast_function_ = 0;


// The following function is (and must be!) the same as Fl::get_color() but
// can be inlined. We need this additional implementation because all contrast
// related functions have been moved from fl_color.cxx to fl_contrast.cxx
// or have been directly implemented in fl_contrast.cxx (new functions).
// Inlining will hopefully prevent an extra function call.

extern unsigned fl_cmap[256]; // defined in fl_color.cxx

inline unsigned get_color(Fl_Color i) { // see Fl::get_color() !
  if (i & 0xffffff00) return (i);
  else return fl_cmap[i];
}


/** \addtogroup  fl_attributes
 \{ */

/**
  Return the raw / physical luminance of a color.

  This function calculates the physical luminance of Fl_Color \p color.

  The returned luminance value (aka \p Y) is the physical luminance of the
  Fl_Color \p color.

  The result is in the range 0.0 (black) to 1.0 (white).

  \note This is probably not what you want if you are interested in perceived
    contrast or lightness calculation because the luminance \p Y is \b not linear
    with respect to human perception.

  See fl_lightness(Fl_Color) for a function that returns the perceived lightness
  of a color which can be used directly for contrast calculation.

  \param[in]  color   Fl_Color value
  \return             Raw (physical) luminance (0.0 .. 1.0)

  \since 1.4.0

  \see fl_lightness(Fl_Color)
*/
double fl_luminance(Fl_Color color) {

  // Get the sRGB (0xrrggbb) components of the FLTK color
  unsigned col = get_color(color) >> 8;

  int r = (col & 0xFF0000) >> 16;
  int g = (col & 0x00FF00) >> 8;
  int b = (col & 0x0000FF);

  return (0.2126729 * pow(r/255.0, 2.4) +
          0.7151522 * pow(g/255.0, 2.4) +
          0.0721750 * pow(b/255.0, 2.4));
}


/**
  Return the perceived lightness of a color.

  This function calculates the perceived lightness of Fl_Color \p color.

  The returned lightness value \p Lstar according to the CIELAB (L*a*b*)
  color model is almost linear with respect to human perception. It is in
  the range 0 (black) to 100 (white).

  The result values of two colors can be compared directly and the difference
  is their perceived contrast.

  \param[in]  color   Fl_Color value
  \return             perceived lightness (0 .. 100)

  \since 1.4.0
*/
double fl_lightness(Fl_Color color) {

  // compute the raw luminance Y (0.0 .. 1.0)

  double Y = fl_luminance(color);

  // return the perceived lightness L* (Lstar)

  if (Y <= (216/24389.))
    return Y * (24389/27.);
  else
    return pow(Y, (1/3.)) * 116 - 16;
}


/**
  Set the contrast level (sensitivity) of the fl_contrast() method.

  This can be used to tune the legacy fl_contrast() function to achieve
  slightly better results. The default value is defined per contrast mode
  (see below). Values between 50 and 70 may be useful for the legacy contrast
  mode but you can raise it up to 100. Lower values than 50 are probably not
  useful.

  The contrast \p level affects not only the legacy (1.3.x) fl_contrast()
  function but also the new CIELAB contrast mode which is the default since
  FLTK 1.4.0. See default value below.

  Other contrast modes are currently not affected by the contrast level.

  You may use the contrast level if you define your own custom contrast
  function in mode FL_CONTRAST_CUSTOM.

  \note All contrast modes store their own contrast level because the
    behavior is slightly different. You must change the contrast mode
    fl_contrast_mode() \b before you set or get the contrast level.

  The default contrast level is
    - 50 in mode FL_CONTRAST_LEGACY (compatible with FLTK 1.3.x)
    - 39 in mode FL_CONTRAST_CIELAB (similar threshold as in FLTK 1.3.x)
    -  0 (undefined) for all other modes

  See the description of fl_contrast_mode(int mode) for more information about
  the contrast level per mode.

  Example:
  \code
    fl_contrast_mode(FL_CONTRAST_LEGACY);
    fl_contrast_level(60);
  \endcode

  A \p level greater than 50 (probably best in the range 50 to 70) may achieve
  better results of the legacy fl_contrast() function in some border cases of
  low contrast between foreground and background colors but we recommend to
  use the new default algorithm \c FL_CONTRAST_CIELAB unless you need strict
  backwards compatibility or use a CPU constrained embedded system.

  \param[in]  level  valid range is 0 to 100

  \since 1.4.0
*/
void fl_contrast_level(int level) {
  if (level < 0) level = 0;
  else if (level > 100) level = 100;
  fl_contrast_level_[fl_contrast_mode_] = level;
}

/**
  Get the contrast level (sensitivity) of the fl_contrast() method.

  This returns the level of the currently selected contrast mode.

  \return  The current contrast level.

  \see fl_contrast_level(int level)
  \see fl_contrast_mode(int mode)

  \since 1.4.0
*/
int fl_contrast_level() {
  return fl_contrast_level_[fl_contrast_mode_];
}

/**
  Set the contrast algorithm (mode).

  You can use one of

  - FL_CONTRAST_NONE   (not recommended: returns the foreground color)
  - FL_CONTRAST_LEGACY (same as in FLTK 1.3.x)
  - FL_CONTRAST_CIELAB (better, this is the default since FLTK 1.4.0)
  - FL_CONTRAST_CUSTOM (you must define your own contrast algorithm)

  If you set FL_CONTRAST_CUSTOM you must also register your custom
  contrast function by calling fl_contrast_function().

  You may set the contrast level fl_contrast_level(int) after setting
  the contrast mode. This affects the contrast algorithm as described
  below:

  - FL_CONTRAST_LEGACY: default level is 50 which is compatible with
    FLTK 1.3.x and older. This mode is no longer the default and is
    not recommended because it doesn't take human contrast perception
    into account and doesn't properly handle sRGB color values. You
    may get better contrasts if you set the level higher than 50.
    Values in the range 50 to 70 may be useful. Higher values result
    in higher contrast, i.e. the algorithm switches "earlier" to
    black or white mode.

  - FL_CONTRAST_CIELAB: defaut level is 39 which appears to be a good
    value. The higher the level is, the more contrast is to be expected.
    Values in the range below 39 accept lower contrast and values above
    39 switch "earlier" to black or white. Values between 36 and 46 may
    yield usable contrast experience.

  \note The goal of fl_contrast() is to achieve a "sufficient" contrast
    between text and background. Level 39 in CIELAB mode means that the
    accepted contrast is about 39% of the lightness difference between
    both colors. This can be perceived as very low contrast in some cases,
    but the text should at least be readable. Note that the highest possible
    contrast value on a medium gray background is 50% (either black or white).
    Bill Spitzak wrote on May 16, 2024 in fltk.general in thread "FLTK 1.4
    Menu Bar Style": <i>"I would certainly aim for a function that does not
    alter  color combinations where it is physically possible to read the
    text, even if squinting is needed."</i>\n
    See https://groups.google.com/g/fltkgeneral/c/EkWI4HTHSLA/m/rsZunZ1vAwAJ

  \param[in]  mode  if invalid, FL_CONTRAST_CIELAB will be selected

  \since 1.4.0

  \see fl_contrast_function(Fl_Contrast_Function *)
  \see fl_contrast_level(int)
*/
void fl_contrast_mode(int mode) {
  if (mode >= 0 && mode < FL_CONTRAST_LAST)
    fl_contrast_mode_ = mode;
  else
    fl_contrast_mode_ = FL_CONTRAST_CIELAB;
}

/**
  Return the current contrast algorithm (mode).

  \return  Contrast algorithm (mode).

  \since 1.4.0

  \see fl_contrast_mode(int)
*/
int fl_contrast_mode() {
  return fl_contrast_mode_;
}

/**
  Register a custom contrast function.

  Your custom contrast function will be called when fl_contrast() is
  called if and only if you previously registered your function and
  called fl_contrast_mode(FL_CONTRAST_CUSTOM) .

  Your custom contrast function must provide the signature
  \code
    Fl_Color my_contrast_function(Fl_Color fg, Fl_Color bg, int context, int size)
  \endcode

  The arguments are the same as for the full fl_contrast() function since FLTK 1.4.
  You can use the supplied \p size to modify the result. Depending on the
  caller the \p size parameter can be 0 (default) or a valid size. In the context
  of text, i.e. \p context == 0, the \p size parameter is the fontsize.

  The \p context parameter is not yet used and will always be 0 unless included in
  a call to fl_contrast(). The value 0 must be interpreted as text.
  In the future the \p context argument will be used to supply a different context
  than text (small icons, large icons, etc.). The exact usage is not yet specified.

  Your function may also use fl_contrast_level() to modify the result accordingly.

  \since 1.4.0

  \see fl_contrast_mode(int)
  \see fl_contrast_level(int)
  \see fl_contrast()
*/
void fl_contrast_function(Fl_Contrast_Function *f) {
  fl_contrast_function_ = f;
}

/*
  Returns a color that contrasts with the background color.

  This is functionally identical to the algorithm used in FLTK 1.3.x,
  modified only to utilize fl_contrast_level() (since 1.4.0).

  *** This function is intentionally not public and not documented.
  *** Do not change this except for level adjustment (backwards compatibility).

  Note: this is fast but *inaccurate* WRT human contrast perception.
  The default since FLTK 1.4 is to use fl_contrast_cielab().

  \param[in] fg,bg        foreground and background colors
  \param[in] fs,context   fontsize and context (unused)
  \return                 contrasting color
*/
static Fl_Color fl_contrast_legacy(Fl_Color fg, Fl_Color bg, int context, int size) {

  (void) context; // currently ignored
  (void) size;    // currently ignored

  // internal static variables, recalculated only if fl_contrast_level() is changed

  static int level =  50;  // default, compatible with FLTK 1.3.x
  static int tc    =  99;  // sufficient contrast threshold (99 <=> 38.82 %)
  static int tbw   = 127;  // black/white threshold        (127 <=> 49.80 %)

  // new in FLTK 1.4: adjust thresholds if fl_contrast_level() was changed

  if (fl_contrast_level() != level) {
    level = fl_contrast_level();
    if (level == 100)
      tc = 256;
    else if (level == 0)
      tc = 0;
    else if (level > 50)
      tc = 99 + ((level - 50) * (255 - 99) / 50);
    else
      tc = 99 - ((50 - level) * 99 / 50);
  }

  // Get the real sRGB values for each color...
  unsigned cfg = get_color(fg);
  unsigned cbg = get_color(bg);

  // Compute the luminance for each color (0 .. 255)
  // Note: FLTK 1.3 compatible, don't change this!

  int lfg = ((cfg >> 24) * 30 + ((cfg >> 16) & 255) * 59 + ((cfg >> 8) & 255) * 11) / 100;
  int lbg = ((cbg >> 24) * 30 + ((cbg >> 16) & 255) * 59 + ((cbg >> 8) & 255) * 11) / 100;

  int lc = lfg - lbg;                   // calculated contrast (-255 .. 255)

#if DEBUG_CONTRAST_LEGACY

  const char *rv = "?";                 // return value as text (init)
  if (lc > tc || lc < -tc) rv = "fg";   // sufficient contrast
  else if (lbg > tbw) rv = "BLACK";     // light background
  else rv = "WHITE";                    // dark background

  printf("fl_contrast_legacy: lfg %4d (%7.2f)  lbg %4d (%7.2f)  lc %4d (%7.2f)  => %s\n",
        lfg, lfg/255.*100, lbg, lbg/255.*100, lc, lc/255.*100, rv);

#endif // DEBUG_CONTRAST_LEGACY

  // Compare and return the contrasting color...

  if (lc > tc || lc < -tc) return fg;   // sufficient contrast
  if (lbg > tbw) return FL_BLACK;       // light background
  return FL_WHITE;                      // dark background
}

/*
  Returns a color that contrasts with the background color.

  ** This function is intentionally not public and not documented. **

  This is an improved algorithm compared to the one used in FLTK 1.3.x.
  It is slower (uses floating point and pow()) but is much more
  accurate WRT human contrast perception.

  \param[in] fg,bg        foreground and background colors
  \param[in] fs,context   unused: fontsize and context
  \return                 contrasting color
*/
static Fl_Color fl_contrast_cielab(Fl_Color fg, Fl_Color bg, int context, int size) {

  (void) context; // currently ignored
  (void) size;    // currently ignored

  double tc  = (double)fl_contrast_level(); // sufficient contrast threshold
  double tbw = 50.;                         // black/white threshold

  // Compute the perceived lightness L* (Lstar) and the contrast

  double lfg = fl_lightness(fg);
  double lbg = fl_lightness(bg);
  double lc  = lfg - lbg;

  // Compare and return the contrasting color...
  if (lc >= tc || lc <= -tc) return fg;     // sufficient contrast
  if (lbg > tbw) return (Fl_Color)FL_BLACK; // black
  return (Fl_Color)FL_WHITE;                // white
}


/**
  Returns a color that contrasts with the background color.

  This will be the foreground color if it contrasts sufficiently with the
  background color. Otherwise, returns \p FL_WHITE or \p FL_BLACK depending
  on which color provides the best contrast.

  FLTK 1.4.0 uses a different default contrast function than earlier releases
  (1.3.x) but you can use the old "legacy" contrast function by calling
  \code
    fl_contrast_mode(FL_CONTRAST_LEGACY);
  \endcode
  early in your main program.

  \note It is a known issue that static initialization using fl_contrast() may already
    have been executed before you call this function in main(). You should be aware of
    this and, if necessary, write your own (static) contrast initialization function.
    This should rarely be necessary.

  You can change the behavior of fl_contrast() in several ways:

  - Change the "level" (sensitivity) for contrast calculation, see fl_contrast_level().
    Valid levels are 0 - 100, the default "medium" value depends on the contrast mode.
    If you raise the level above the default value the overall contrast will generally
    be higher, i.e. the required contrast to return the foreground color is raised and
    therefore the calculated color switches "earlier" to either black or white.
    In other words, using the following values:
    - 0 always uses the foreground color
    - the default, unmodified algorithm allows a sufficient contrast such that the text
      is readable
    - 100 will always use black or white

    Changing the \p level is particularly useful and intended for the "legacy mode"
    to improve the results partially. Values slightly above 50 (50 - 70) will likely
    return the best results (50 is the default, as used in FLTK 1.3.x).

    \note Different contrast modes (algorithms) can use their own values and
      defaults of fl_contrast_level().

  - Change the used contrast calculation function. You can either use the old
    (FLTK 1.3.x) function or use the better but slower function based on the
    CIELAB (L*a*b*) color model, or you can define your own custom contrast
    function if you need even better contrast results.

  The following contrast functions are available:

  - FL_CONTRAST_LEGACY, the old FLTK 1.3.x compatible function. This is the
    fastest function (using integer arithmetic) but it provides worse results
    in border cases. You may want to use this on embedded or otherwise CPU
    constrained systems or if you need strict backwards compatibility.
    For slightly better results you may utilize the new fl_contrast_level(int)
    function (since 1.4.0) to increase the contrast sensitivity. This will
    provide slightly better results than FLTK 1.3.x and earlier but we recommend
    to use the new default function:

  - FL_CONTRAST_CIELAB, based on the CIELAB (L*a*b*) color model. This function
    is superior regarding the human contrast perception but may be slightly
    slower - which should not matter on a modern CPU. The default contrast
    level in this mode is 39 which results in a very similar experience as the
    old contrast function but avoids unreadable border cases.
    <b>This is the default since FLTK 1.4.0.</b>

  - FL_CONTRAST_CUSTOM, your own contrast calculation function.

  In the future we \b may provide even more (and superior) contrast algorithms.

  The new parameters \p context and \p size (since 1.4.0) are defined for
  future extensions and are currently not used. Default values are 0.
  - The \p context is intended to differentiate text and other kinds
    of objects, e.g. radio buttons, check marks, or icon types.
  - The \p size parameter is an unspecified (object) size that may be used to
    calculate the required contrast. In text mode this must be the font size.
    Rule: the larger the object (font), the lower the required contrast.

  \note These new optional parameters must be provided in the custom contrast
  function which is the reason why they are added now. In the future we may use
  the (font) size to adjust the calculated contrast, and users defining their
  own contrast functions may use them in their functions.

  \param[in]  fg      foreground (text) color
  \param[in]  bg      background color
  \param[in]  context graphical context (optional, default = 0 == text)
  \param[in]  size    unspecified size (optional, default = 0 == undefined)

  \return     contrasting color: \p fg, \p FL_BLACK, or \p FL_WHITE

  \see fl_contrast_level(int)
  \see fl_contrast_mode(int)
  \see fl_contrast_function()
*/
Fl_Color fl_contrast(Fl_Color fg, Fl_Color bg, int context, int size) {

  switch (fl_contrast_mode_) {

    case FL_CONTRAST_LEGACY:
      return fl_contrast_legacy(fg, bg, context, size);

    case FL_CONTRAST_CUSTOM:
      if (fl_contrast_function_)
        return (fl_contrast_function_)(fg, bg, context, size);

      // FALLTHROUGH

    case FL_CONTRAST_CIELAB:
      return fl_contrast_cielab(fg, bg, context, size);

    default: // unknown (none): return fg
      break;
  }
  return fg;

} // fl_contrast()

/**
 \}
 */
