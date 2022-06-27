//
// "$Id$"
//
// MacOS font utilities for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2015 by Bill Spitzak and others.
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

#include <config.h>

// #inclde <SFNTTypes.h>


// Bug: older versions calculated the value for *ap as a side effect of
// making the name, and then forgot about it. To avoid having to change
// the header files I decided to store this value in the last character
// of the font name array.
#define ENDOFBUFFER  sizeof(fl_fonts->fontname)-1

// turn a stored font name into a pretty name:
const char* Fl::get_font_name(Fl_Font fnum, int* ap) {
  if (!fl_fonts) fl_fonts = Fl_X::calc_fl_fonts();
  Fl_Fontdesc *f = fl_fonts + fnum;
  if (!f->fontname[0]) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
    if (fl_mac_os_version >= Fl_X::CoreText_threshold) {
      CFStringRef cfname = CFStringCreateWithCString(NULL, f->name, kCFStringEncodingUTF8);
      CTFontRef ctfont = cfname ? CTFontCreateWithName(cfname, 0, NULL) : NULL;
      if (cfname) { CFRelease(cfname); cfname = NULL; }
      if (ctfont) {
        cfname = CTFontCopyFullName(ctfont);
        CFRelease(ctfont);
        if (cfname) {
          CFStringGetCString(cfname, f->fontname, ENDOFBUFFER, kCFStringEncodingUTF8);
          CFRelease(cfname);
        }
      }
      if (!cfname) strlcpy(f->fontname, f->name, ENDOFBUFFER);
    }
    else 
#endif
      strlcpy(f->fontname, f->name, ENDOFBUFFER);
    const char* p = f->name;
    if (!p || !*p) {if (ap) *ap = 0; return "";}
    int type = 0;
    if (strstr(f->name, "Bold")) type |= FL_BOLD;
    if (strstr(f->name, "Italic") || strstr(f->name, "Oblique")) type |= FL_ITALIC;
    f->fontname[ENDOFBUFFER] = (char)type;
  }
  if (ap) *ap = f->fontname[ENDOFBUFFER];
  return f->fontname;
}

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5

static char *skip(char *p, int& derived)
{
  if (memcmp(p, "-BoldItalic", 11) == 0) { p += 11; derived = 3; }
  else if (memcmp(p, "-BoldOblique", 12) == 0) { p += 12; derived = 3; }
  else if (memcmp(p, "-Bold", 5) == 0) {p += 5; derived = 1; }
  else if (memcmp(p, "-Italic", 7) == 0) {p += 7; derived = 2; }
  else if (memcmp(p, "-Oblique", 8) == 0) {p += 8; derived = 2; }
  else if (memcmp(p, "-Regular", 8) == 0) {p += 8; }
  else if (memcmp(p, "-Roman", 6) == 0) {p += 6; }
  return p;
}

static int name_compare(const void *a, const void *b)
{
  /* Compare PostScript font names. 
   First compare font family names ignoring bold, italic and oblique qualifiers.
   When families are identical, order them according to regular, bold, italic, bolditalic.
   */
  char *n1 = *(char**)a;
  char *n2 = *(char**)b;
  int derived1 = 0;
  int derived2 = 0;
  while (true) {
    if (*n1 == '-') n1 = skip(n1, derived1);
    if (*n2 == '-') n2 = skip(n2, derived2);
    if (*n1 < *n2) return -1;
    if (*n1 > *n2) return +1;
    if (*n1 == 0) {
      return derived1 - derived2;
      }
    n1++; n2++;
    }
}
#endif

static int fl_free_font = FL_FREE_FONT;

// This function fills in the fltk font table with all the fonts that
// are found on the X server.  It tries to place the fonts into families
// and to sort them so the first 4 in a family are normal, bold, italic,
// and bold italic.

Fl_Font Fl::set_fonts(const char* xstarname) {
#pragma unused ( xstarname )
if (fl_free_font > FL_FREE_FONT) return (Fl_Font)fl_free_font; // if already called

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
if(fl_mac_os_version >= Fl_X::CoreText_threshold) {
  int value[1] = {1};
  CFDictionaryRef dict = CFDictionaryCreate(NULL, 
					    (const void **)kCTFontCollectionRemoveDuplicatesOption, 
					    (const void **)&value, 1, NULL, NULL);
  CTFontCollectionRef fcref = CTFontCollectionCreateFromAvailableFonts(dict);
  CFRelease(dict);
  CFArrayRef arrayref = CTFontCollectionCreateMatchingFontDescriptors(fcref);
  CFRelease(fcref);
  CFIndex count = CFArrayGetCount(arrayref);
  CFIndex i;
  char **tabfontnames = new char*[count];
  for (i = 0; i < count; i++) {
	CTFontDescriptorRef fdesc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(arrayref, i);
	CTFontRef font = CTFontCreateWithFontDescriptor(fdesc, 0., NULL);
        CFStringRef cfname = CTFontCopyPostScriptName(font);
	CFRelease(font);
	static char fname[200];
	CFStringGetCString(cfname, fname, sizeof(fname), kCFStringEncodingUTF8);
	tabfontnames[i] = strdup(fname); // never free'ed
	CFRelease(cfname);
	}
  CFRelease(arrayref);
  qsort(tabfontnames, count, sizeof(char*), name_compare);
  for (i = 0; i < count; i++) {
    Fl::set_font((Fl_Font)(fl_free_font++), tabfontnames[i]);
    }
  delete[] tabfontnames;
  return (Fl_Font)fl_free_font;
}
else {
#endif
#if (!defined(__LP64__) || !__LP64__) && MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_11
  ItemCount oFontCount, oCountAgain;
  ATSUFontID *oFontIDs;
  // How many fonts?
  ATSUFontCount (&oFontCount);
  // now allocate space for them...
  oFontIDs = (ATSUFontID *)malloc((oFontCount+1) * sizeof(ATSUFontID));
  ATSUGetFontIDs (oFontIDs, oFontCount, &oCountAgain);
  // Now oFontIDs should contain a list of all the available Unicode fonts
  // Iterate through the list to get each font name
  for (ItemCount idx = 0; idx < oFontCount; idx++)
  {
//  ByteCount actualLength = 0;
//	Ptr oName;
    // How to get the name - Apples docs say call this twice, once to get the length, then again 
	// to get the actual name...
//    ATSUFindFontName (oFontIDs[idx], kFontFullName, kFontMacintoshPlatform, kFontRomanScript, kFontEnglishLanguage,
//                      0, NULL, &actualLength, NULL);
    // Now actualLength tells us the length of buffer we need
//	oName = (Ptr)malloc(actualLength + 8);
// But who's got time for that nonsense? Let's just hard code a fixed buffer (urgh!)
    ByteCount actualLength = 511;
	char oName[512];
    ATSUFindFontName (oFontIDs[idx], kFontFullName, kFontMacintoshPlatform, kFontRomanScript, kFontEnglishLanguage,
                      actualLength, oName, &actualLength, &oCountAgain);
    // bounds check and terminate the returned name
    if(actualLength > 511)
      oName[511] = 0;
    else
      oName[actualLength] = 0;
	Fl::set_font((Fl_Font)(fl_free_font++), strdup(oName));
//	free(oName);
  }
  free(oFontIDs);
  return (Fl_Font)fl_free_font;
#endif //__LP64__
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
  }
#endif
  return 0;
}

static int array[128];
int Fl::get_font_sizes(Fl_Font fnum, int*& sizep) {
  if (!fl_fonts) fl_fonts = Fl_X::calc_fl_fonts();
  Fl_Fontdesc *s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // empty slot in table, use entry 0
  int cnt = 0;

  // ATS supports all font size 
  array[0] = 0;
  sizep = array;
  cnt = 1;

  return cnt;
}

//
// End of "$Id$".
//
