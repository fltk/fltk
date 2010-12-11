//
// $Id$
//
// Miscellaneous files for developers:
//

Developer files for testing UTF-8 character sets and functions:
================================================================================

Open these files in test/editor and compare the contents.

Test scrolling, move the cursor over the "special" characters (> U+007F),
watch cursor movement, drawing artefacts, etc..

CAUTION: Do not 'cat' these files unless you know what you're doing.
Especially the cp1252* files can contain control characters that may freeze
your screen (xterm etc.).

Detailed file description:

The "native" encoded files contain 8-bit characters with binary values that
can be seen at the top and left borders, resp. (add both). They should look
like the image file (cp1252.png) on Windows only (if the current codepage is
"Western, CP 1252", a superset of ISO-8859-1). They will look different with
other codepages or on Linux, Unix, or Mac OS X. Use these files with FLTK 1.1
or an editor with the corresponding codepage or locale.

The UTF-8 encoded files should be usable with FLTK 1.3, FLTK 2, or FLTK 3
(future). They contain UTF-8 encoded characters at the table positions,
where the corresponding ISO-8859-1, Windows copepage 1252, or Mac OS Roman
characters would be. They should look identical as the image file (cp1252.png)
or MacRoman_utf-8.png, resp., on all UTF-8 capable systems.

The files with names iso-8859-1_* contain only the ISO-8859-1 subset, i.e.
the colums with character codes 0x80 - 0x9F (U+0080 - U+009F) are empty.

Mac OS Roman is the Apple Mac native character set before it became Unicode
by default (OS X). The corresponding files are ordered as they would appear
with the native character set / encoding.

The *.html files contain a character encoding meta tag, so that they should
be viewable with all standard browsers. They can be used for cut'n'paste and
drag'n'drop tests from a 'native' application (browser) to FLTK, e.g.
test/editor or test/input.

Special Notes:

  0x22 (U+0022: "quotation mark") is doubled intentionally. This has been
  done for better results in test/editor, because otherwise the rest of the
  file would be shown in blue color (as a comment).

  0x98 (U+02DC: "small tilde") may not display correctly on Windows systems.
  I saw the same effect with other Windows editors as well. The following
  characters may be displayed "shifted left" by one position. Seems to work
  correctly on Linux (with UTF-8 encoding).
  
  Mac OS Roman: 0xF0 (U+F8FF: "Apple logo") may not be printable, depending
  on installed fonts on other systems.

  There may currently be drawing artefacts when moving the cursor forwards
  and/or backwards over some "special" characters.

References:

  http://unicode.org/Public/MAPPINGS/ISO8859/8859-1.TXT
  http://unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1252.TXT
  http://unicode.org/Public/MAPPINGS/VENDORS/APPLE/ROMAN.TXT
  http://en.wikipedia.org/wiki/Mac_OS_Roman

Files:

The first two files are images of how it should look:

  cp1252.png		full Windows Codepage 1252 (Western)
  MacRoman_utf-8.png	full Mac OS Roman character set (w/o Apple logo)

The following four files contain the full Windows Codepage 1252:

  cp1252.txt		native (Windows) encoding, suitable for FLTK 1.1
  cp1252_utf-8.txt	UTF-8 encoding, suitable for FLTK 1.3 and greater
  cp1252.html		native (Windows) encoding, use with any browser
  cp1252_utf-8.html	UTF-8 encoding, use with any browser

The following two files contain only the ISO-8859-1 subset:

  iso-8859-1.txt	native (Windows) encoding, suitable for FLTK 1.1
  iso-8859-1_utf-8.txt	UTF-8 encoding, suitable for FLTK 1.3 and greater

The following file contains the full Mac OS Roman character set:

  MacRoman_utf-8.txt	UTF-8 encoding, suitable for FLTK 1.3 and greater
			(use cp1252.txt with FLTK 1.1 on Mac OS instead)

Other developer support files:
================================================================================

  doxystar.cxx		Use this to reformat doxygen comments (test only).

	Compile with:	g++ -o doxystar doxystar.cxx

	Usage:		cat file | doxystar

  DO NOT USE this for current code development!

  It is intended for testing of future code reformatting !

//
// End of $Id$.
//
