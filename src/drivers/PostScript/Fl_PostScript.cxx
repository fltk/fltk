//
// Classes Fl_PostScript_File_Device and Fl_PostScript_Graphics_Driver for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2024 by Bill Spitzak and others.
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
#if !defined(FL_NO_PRINT_SUPPORT)
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <stdio.h>
#include "Fl_PostScript_Graphics_Driver.H"
#include <FL/Fl_PDF_File_Surface.H>
#include <FL/Fl_PostScript.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Native_File_Chooser.H>
#include "../../Fl_System_Driver.H"
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/fl_string_functions.h>
#include <FL/fl_callback_macros.H>
#include <FL/platform.H>
#include <stdarg.h>
#include <time.h>
#if USE_PANGO
#include <FL/math.h> // for M_PI
#include <pango/pangocairo.h>
#include <cairo/cairo-ps.h>
#include <cairo/cairo-pdf.h>
#include <FL/Fl_Preferences.H>
#  if ! PANGO_VERSION_CHECK(1,10,0)
#    error "Requires Pango 1.10 or higher"
#  endif
#endif

const char *Fl_PostScript_File_Device::file_chooser_title = "Select a .ps file";

Fl_PostScript_File_Device::Fl_PostScript_File_Device(void)
{
  Fl_Surface_Device::driver( new Fl_PostScript_Graphics_Driver() );
}

FILE *Fl_PostScript_File_Device::file() {
  return driver()->file();
}

int Fl_PostScript_File_Device::begin_job (int pagecount, enum Fl_Paged_Device::Page_Format format,
                                          enum Fl_Paged_Device::Page_Layout layout)
{
  Fl_Native_File_Chooser fnfc;
  fnfc.title(Fl_PostScript_File_Device::file_chooser_title);
  fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  fnfc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);
  fnfc.filter("PostScript\t*.ps\n");
  // Show native chooser
  if ( fnfc.show() ) return 1;
  Fl_PostScript_Graphics_Driver *ps = driver();
  ps->output = fl_fopen(fnfc.filename(), "w");
  if(ps->output == NULL) return 2;
  ps->ps_filename_ = fl_strdup(fnfc.filename());
  ps->start_postscript(pagecount, format, layout);
  return 0;
}

extern "C" {
  static int dont_close(FILE *f)
  {
    return 0;
  }
}

int Fl_PostScript_File_Device::begin_job (FILE *ps_output, int pagecount,
    enum Fl_Paged_Device::Page_Format format, enum Fl_Paged_Device::Page_Layout layout)
{
  Fl_PostScript_Graphics_Driver *ps = driver();
  ps->output = ps_output;
  ps->ps_filename_ = NULL;
  ps->start_postscript(pagecount, format, layout);
  ps->close_command(dont_close); // so that end_job() doesn't close the file
  return 0;
}

int Fl_PostScript_File_Device::begin_job(int pagecount, int* from, int* to, char **)
{
  return 1;
}

Fl_PostScript_File_Device::~Fl_PostScript_File_Device() {
  Fl_PostScript_Graphics_Driver *ps = driver();
  if (ps) delete ps;
}

void Fl_PostScript_File_Device::set_current() {
  Fl_Graphics_Driver& driver = Fl_Graphics_Driver::default_driver();
  display_font_ = driver.font();
  display_size_ = driver.size();
  Fl_Paged_Device::set_current();
}

void Fl_PostScript_File_Device::end_current() {
  if (display_font_ >= 0 && display_size_ > 0)  {
    Fl_Graphics_Driver& driver = Fl_Graphics_Driver::default_driver();
    driver.font(display_font_, display_size_);
  }
  Fl_Paged_Device::end_current();
}

/**
 \cond DriverDev
 \addtogroup DriverDeveloper
 \{
 */

#if ! USE_PANGO
static const int dashes_flat[5][7]={
{-1,0,0,0,0,0,0},
{3,1,-1,0,0,0,0},
{1,1,-1,0,0,0,0},
{3,1,1,1,-1,0,0},
{3,1,1,1,1,1,-1}
};

//yeah, hack...
static const double dashes_cap[5][7]={
{-1,0,0,0,0,0,0},
{2,2,-1,0,0,0,0},
{0.01,1.99,-1,0,0,0,0},
{2,2,0.01,1.99,-1,0,0},
{2,2,0.01,1.99,0.01,1.99,-1}
};
#endif

/**
 \brief The constructor.
 */
Fl_PostScript_Graphics_Driver::Fl_PostScript_Graphics_Driver(void)
{
  close_cmd_ = 0;
#if ! USE_PANGO
  //lang_level_ = 3;
  lang_level_ = 2;
  mask = 0;
  bg_r = bg_g = bg_b = 255;
  clip_ = NULL;
  scale_x = scale_y = 1.;
#endif
  ps_filename_ = NULL;
  nPages = 0;
}

/** \brief The destructor. */
Fl_PostScript_Graphics_Driver::~Fl_PostScript_Graphics_Driver() {
  if(ps_filename_) free(ps_filename_);
}


#if ! USE_PANGO
static const char *_fontNames[] = {
"Helvetica2B",
"Helvetica-Bold2B",
"Helvetica-Oblique2B",
"Helvetica-BoldOblique2B",
"Courier2B",
"Courier-Bold2B",
"Courier-Oblique2B",
"Courier-BoldOblique2B",
"Times-Roman2B",
"Times-Bold2B",
"Times-Italic2B",
"Times-BoldItalic2B",
"Symbol",
"Courier2B",
"Courier-Bold2B",
"ZapfDingbats"
};
#endif

void Fl_PostScript_Graphics_Driver::font(int f, int s) {
  Fl_Graphics_Driver& driver = Fl_Graphics_Driver::default_driver();
  driver.font(f,s); // Use display fonts for font measurement
  Fl_Graphics_Driver::font(f, s);
  Fl_Font_Descriptor *desc = driver.font_descriptor();
  this->font_descriptor(desc);
#if ! USE_PANGO
  if (f < FL_FREE_FONT) {
    fprintf(output, "/%s SF\n" , _fontNames[f]);
    float ps_size = driver.scale_font_for_PostScript(desc, s);
    clocale_printf("%.1f FS\n", ps_size);
  }
#endif
}

Fl_Font Fl_PostScript_Graphics_Driver::font() { return Fl_Graphics_Driver:: font(); }

double Fl_PostScript_Graphics_Driver::width(const char *s, int n) {
  return Fl_Graphics_Driver::default_driver().width(s, n);
}

double Fl_PostScript_Graphics_Driver::width(unsigned u) {
  return Fl_Graphics_Driver::default_driver().width(u);
}

int Fl_PostScript_Graphics_Driver::height() {
  return Fl_Graphics_Driver::default_driver().height();
}

int Fl_PostScript_Graphics_Driver::descent() {
  return Fl_Graphics_Driver::default_driver().descent();
}

void Fl_PostScript_Graphics_Driver::text_extents(const char *c, int n, int &dx, int &dy, int &w, int &h) {
  Fl_Graphics_Driver::default_driver().text_extents(c, n, dx, dy, w, h);
}

void Fl_PostScript_Graphics_Driver::point(int x, int y){
  rectf(x,y,1,1);
}

int Fl_PostScript_Graphics_Driver::not_clipped(int x, int y, int w, int h) {
  if (!clip_) return 1;
  if (clip_->w < 0) return 1;
  int X = 0, Y = 0, W = 0, H = 0;
  clip_box(x, y, w, h, X, Y, W, H);
  if (W) return 1;
  return 0;
}

int Fl_PostScript_Graphics_Driver::clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H) {
  if (!clip_) {
    X = x; Y = y; W = w; H = h;
    return 0;
  }
  if (clip_->w < 0) {
    X = x; Y = y; W = w; H = h;
    return 1;
  }
  int ret = 0;
  if (x > (X=clip_->x)) {X=x; ret=1;}
  if (y > (Y=clip_->y)) {Y=y; ret=1;}
  if ((x+w) < (clip_->x+clip_->w)) {
    W=x+w-X;

    ret=1;

  }else
    W = clip_->x + clip_->w - X;
  if(W<0){
    W=0;
    return 1;
  }
  if ((y+h) < (clip_->y+clip_->h)) {
    H=y+h-Y;
    ret=1;
  }else
    H = clip_->y + clip_->h - Y;
  if(H<0){
    W=0;
    H=0;
    return 1;
  }
  return ret;
}


#if ! USE_PANGO

int Fl_PostScript_Graphics_Driver::clocale_printf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  int retval = Fl::system_driver()->clocale_vprintf(output, format, args);
  va_end(args);
  return retval;
}

//  Prolog string

static const char * prolog =
"/L { /y2 exch def\n"
"/x2 exch def\n"
"/y1 exch def\n"
"/x1 exch def\n"
"newpath   x1 y1 moveto x2 y2 lineto\n"
"stroke}\n"
"bind def\n"


"/R { /dy exch def\n"
"/dx exch def\n"
"/y exch def\n"
"/x exch def\n"
"newpath\n"
"x y moveto\n"
"dx 0 rlineto\n"
"0 dy rlineto\n"
"dx neg 0 rlineto\n"
"closepath stroke\n"
"} bind def\n"

"/CL {\n"
"/dy exch def\n"
"/dx exch def\n"
"/y exch def\n"
"/x exch def\n"
"newpath\n"
"x y moveto\n"
"dx 0 rlineto\n"
"0 dy rlineto\n"
"dx neg 0 rlineto\n"
"closepath\n"
"clip\n"
"} bind def\n"

"/FR { /dy exch def\n"
"/dx exch def\n"
"/y exch def\n"
"/x exch def\n"
"currentlinewidth 0 setlinewidth newpath\n"
"x y moveto\n"
"dx 0 rlineto\n"
"0 dy rlineto\n"
"dx neg 0 rlineto\n"
"closepath fill setlinewidth\n"
"} bind def\n"

"/GS { gsave } bind  def\n"
"/GR { grestore } bind def\n"

"/SP { showpage } bind def\n"
"/LW { setlinewidth } bind def\n"
"/CF /Courier def\n"
"/SF { /CF exch def } bind def\n"
"/fsize 12 def\n"
"/FS { /fsize exch def fsize CF findfont exch scalefont setfont }def \n"


"/GL { setgray } bind def\n"
"/SRGB { setrgbcolor } bind def\n"

"/A85RLE { /ASCII85Decode filter /RunLengthDecode filter } bind def\n" // ASCII85Decode followed by RunLengthDecode filters

//  color images

"/CI { GS /py exch def /px exch def /sy exch def /sx exch def\n"
"translate \n"
"sx sy scale px py 8 \n"
"[ px 0 0 py neg 0 py ]\n"
"currentfile A85RLE\n false 3"
" colorimage GR\n"
"} bind def\n"

//  gray images

"/GI { GS /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale px py 8 \n"


"[ px 0 0 py neg 0 py ]\n"
"currentfile A85RLE\n"
"image GR\n"
"} bind def\n"

// single-color bitmask

"/MI { GS /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale px py true \n"
"[ px 0 0 py neg 0 py ]\n"
"currentfile A85RLE\n"
"imagemask GR\n"
"} bind def\n"


//  path

"/BFP { newpath moveto }  def\n"
"/BP { newpath } bind def \n"
"/PL { lineto } bind def \n"
"/PM { moveto } bind def \n"
"/MT { moveto } bind def \n"
"/LT { lineto } bind def \n"
"/EFP { closepath fill } bind def\n"  //was:stroke
"/ELP { stroke } bind def\n"
"/ECP { closepath stroke } bind def\n"  // Closed (loop)
"/LW { setlinewidth } bind def\n"

// ////////////////////////// misc ////////////////
"/TR { translate } bind def\n"
"/CT { concat } bind def\n"
"/RCT { matrix invertmatrix concat} bind def\n"
"/SC { scale } bind def\n"
//"/GPD { currentpagedevice /PageSize get} def\n"

// show at position with desired width
// usage:
// width (string) x y show_pos_width
"/show_pos_width {GS moveto dup dup stringwidth pop exch length 2 div dup 2 le {pop 9999} if "
"1 sub exch 3 index exch sub exch "
"div 0 2 index 1 -1 scale ashow pop pop GR} bind def\n" // spacing altered to match desired width
//"/show_pos_width {GS moveto dup stringwidth pop 3 2 roll exch div -1 matrix scale concat "
//"show GR } bind def\n" // horizontally scaled text to match desired width

;


static const char * prolog_2 =  // prolog relevant only if lang_level >1

// color image dictionaries
"/CII {GS /inter exch def /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale\n"
"/DeviceRGB setcolorspace\n"
"/IDD 8 dict def\n"
"IDD begin\n"
"/ImageType 1 def\n"
"/Width px def\n"
"/Height py def\n"
"/BitsPerComponent 8 def\n"
"/Interpolate inter def\n"
"/DataSource currentfile A85RLE def\n"
"/MultipleDataSources false def\n"
"/ImageMatrix [ px 0 0 py neg 0 py ] def\n"
"/Decode [ 0 1 0 1 0 1 ] def\n"
"end\n"
"IDD image GR} bind def\n"

// gray image dict
"/GII {GS /inter exch def /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale\n"
"/DeviceGray setcolorspace\n"
"/IDD 8 dict def\n"
"IDD begin\n"
"/ImageType 1 def\n"
"/Width px def\n"
"/Height py def\n"
"/BitsPerComponent 8 def\n"

"/Interpolate inter def\n"
"/DataSource currentfile A85RLE def\n"
"/MultipleDataSources false def\n"
"/ImageMatrix [ px 0 0 py neg 0 py ] def\n"
"/Decode [ 0 1 ] def\n"
"end\n"
"IDD image GR} bind def\n"

// Create a custom PostScript font derived from PostScript standard text fonts
// The encoding of this custom font is as follows:
// 0000-00FF  coincides with Unicode, that is to ASCII + Latin-1
// 0100-017F  coincides with Unicode, that is to Latin Extended-A
// 0180-01A6  encodes miscellaneous characters present in PostScript standard text fonts

// use ISOLatin1Encoding for all text fonts
"/ToISO { dup findfont dup length dict copy begin /Encoding ISOLatin1Encoding def currentdict end definefont pop } def\n"
"/Helvetica ToISO /Helvetica-Bold ToISO /Helvetica-Oblique ToISO /Helvetica-BoldOblique ToISO \n"
"/Courier ToISO /Courier-Bold ToISO /Courier-Oblique ToISO /Courier-BoldOblique ToISO \n"
"/Times-Roman ToISO /Times-Bold ToISO /Times-Italic ToISO /Times-BoldItalic ToISO \n"

// define LatinExtA, the encoding of Latin-extended-A + some additional characters
// see http://partners.adobe.com/public/developer/en/opentype/glyphlist.txt for their names
"/LatinExtA \n"
"[ "
" /Amacron /amacron /Abreve /abreve /Aogonek /aogonek\n" // begin of Latin Extended-A code page
" /Cacute  /cacute  /Ccircumflex  /ccircumflex  /Cdotaccent  /cdotaccent  /Ccaron  /ccaron \n"
" /Dcaron  /dcaron   /Dcroat  /dcroat\n"
" /Emacron  /emacron  /Ebreve  /ebreve  /Edotaccent  /edotaccent  /Eogonek  /eogonek  /Ecaron  /ecaron\n"
" /Gcircumflex  /gcircumflex  /Gbreve  /gbreve  /Gdotaccent  /gdotaccent  /Gcommaaccent  /gcommaaccent \n"
" /Hcircumflex /hcircumflex  /Hbar  /hbar  \n"
" /Itilde  /itilde  /Imacron  /imacron  /Ibreve  /ibreve  /Iogonek  /iogonek /Idotaccent  /dotlessi  \n"
" /IJ  /ij  /Jcircumflex  /jcircumflex\n"
" /Kcommaaccent  /kcommaaccent  /kgreenlandic  \n"
" /Lacute  /lacute  /Lcommaaccent  /lcommaaccent   /Lcaron  /lcaron  /Ldotaccent /ldotaccent   /Lslash  /lslash \n"
" /Nacute  /nacute  /Ncommaaccent  /ncommaaccent  /Ncaron  /ncaron  /napostrophe  /Eng  /eng  \n"
" /Omacron  /omacron /Obreve  /obreve  /Ohungarumlaut  /ohungarumlaut  /OE  /oe \n"
" /Racute  /racute  /Rcommaaccent  /rcommaaccent  /Rcaron  /rcaron \n"
" /Sacute /sacute  /Scircumflex  /scircumflex  /Scedilla /scedilla /Scaron  /scaron \n"
" /Tcommaaccent  /tcommaaccent  /Tcaron  /tcaron  /Tbar  /tbar \n"
" /Utilde  /utilde /Umacron /umacron  /Ubreve  /ubreve  /Uring  /uring  /Uhungarumlaut  /uhungarumlaut  /Uogonek /uogonek \n"
" /Wcircumflex  /wcircumflex  /Ycircumflex  /ycircumflex  /Ydieresis \n"
" /Zacute /zacute /Zdotaccent /zdotaccent /Zcaron /zcaron \n"
" /longs \n" // end of Latin Extended-A code page
" /florin  /circumflex  /caron  /breve  /dotaccent  /ring \n" // remaining characters from PostScript standard text fonts
" /ogonek  /tilde  /hungarumlaut  /endash /emdash \n"
" /quoteleft  /quoteright  /quotesinglbase  /quotedblleft  /quotedblright \n"
" /quotedblbase  /dagger  /daggerdbl  /bullet  /ellipsis \n"
" /perthousand  /guilsinglleft  /guilsinglright  /fraction  /Euro \n"
" /trademark /partialdiff  /Delta /summation  /radical \n"
" /infinity /notequal /lessequal /greaterequal /lozenge \n"
" /fi /fl /apple \n"
" ] def \n"
// deal with alternative PostScript names of some characters
" /mycharstrings /Helvetica findfont /CharStrings get def\n"
" /PSname2 { dup mycharstrings exch known {LatinExtA 3 -1 roll 3 -1 roll put}{pop pop} ifelse } def \n"
" 16#20 /Gdot PSname2 16#21 /gdot PSname2 16#30 /Idot PSname2 16#3F /Ldot PSname2 16#40 /ldot PSname2 16#7F /slong PSname2 \n"

// proc that gives LatinExtA encoding to a font
"/ToLatinExtA { findfont dup length dict copy begin /Encoding LatinExtA def currentdict end definefont pop } def\n"
// create Ext-versions of standard fonts that use LatinExtA encoding \n"
"/HelveticaExt /Helvetica ToLatinExtA \n"
"/Helvetica-BoldExt /Helvetica-Bold ToLatinExtA /Helvetica-ObliqueExt /Helvetica-Oblique ToLatinExtA  \n"
"/Helvetica-BoldObliqueExt /Helvetica-BoldOblique ToLatinExtA  \n"
"/CourierExt /Courier ToLatinExtA /Courier-BoldExt /Courier-Bold ToLatinExtA  \n"
"/Courier-ObliqueExt /Courier-Oblique ToLatinExtA /Courier-BoldObliqueExt /Courier-BoldOblique ToLatinExtA \n"
"/Times-RomanExt /Times-Roman ToLatinExtA /Times-BoldExt /Times-Bold ToLatinExtA  \n"
"/Times-ItalicExt /Times-Italic ToLatinExtA /Times-BoldItalicExt /Times-BoldItalic ToLatinExtA \n"

// proc to create a Type 0 font with 2-byte encoding
// that merges a text font with ISO encoding + same font with LatinExtA encoding
"/To2byte { 6 dict begin /FontType 0 def \n"
"/FDepVector 3 1 roll findfont exch findfont 2 array astore def \n"
"/FontMatrix [1  0  0  1  0  0] def /FMapType 6 def /Encoding [ 0 1 0 ] def\n"
// 100: Hexa count of ISO array; A7: hexa count of LatinExtA array
"/SubsVector < 01 0100 00A7 > def\n"
"currentdict end definefont pop } def\n"
// create Type 0 versions of standard fonts
"/Helvetica2B /HelveticaExt /Helvetica To2byte \n"
"/Helvetica-Bold2B /Helvetica-BoldExt /Helvetica-Bold To2byte \n"
"/Helvetica-Oblique2B /Helvetica-ObliqueExt /Helvetica-Oblique To2byte \n"
"/Helvetica-BoldOblique2B /Helvetica-BoldObliqueExt /Helvetica-BoldOblique To2byte \n"
"/Courier2B /CourierExt /Courier To2byte \n"
"/Courier-Bold2B /Courier-BoldExt /Courier-Bold To2byte \n"
"/Courier-Oblique2B /Courier-ObliqueExt /Courier-Oblique To2byte \n"
"/Courier-BoldOblique2B /Courier-BoldObliqueExt /Courier-BoldOblique To2byte \n"
"/Times-Roman2B /Times-RomanExt /Times-Roman To2byte \n"
"/Times-Bold2B /Times-BoldExt /Times-Bold To2byte \n"
"/Times-Italic2B /Times-ItalicExt /Times-Italic To2byte \n"
"/Times-BoldItalic2B /Times-BoldItalicExt /Times-BoldItalic To2byte \n"
;

static const char * prolog_2_pixmap =  // prolog relevant only if lang_level == 2 for pixmaps/masked color images
"/pixmap_mat {[ pixmap_sx 0 0 pixmap_sy neg 0 pixmap_sy ]}  bind def\n"

"/pixmap_dict {"
"<< /PatternType 1 "
"/PaintType 1 "
"/TilingType 2 "
"/BBox [0  0  pixmap_sx  pixmap_sy] "
"/XStep pixmap_sx "
"/YStep pixmap_sy\n"
"/PaintProc "
"{ begin "
"pixmap_w pixmap_h scale "
"pixmap_sx pixmap_sy 8 "
"pixmap_mat "
"currentfile A85RLE "
"false 3 "
"colorimage "
"end "
"} bind "
">>\n"
"} bind def\n"

"/pixmap_plot {"
"GS "
"/pixmap_sy exch def /pixmap_sx exch def\n"
"/pixmap_h exch def /pixmap_w exch def\n"
"translate\n"
"pixmap_dict matrix makepattern setpattern\n"
"pixmap_w pixmap_h scale\n"
"pixmap_sx pixmap_sy\n"
"true\n"
"pixmap_mat\n"
"currentfile A85RLE\n"
"imagemask\n"
"GR\n"
"} bind def\n"
;

static const char * prolog_3 = // prolog relevant only if lang_level >2

// masked color images
"/CIM {GS /inter exch def /my exch def /mx exch def /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale\n"
"/DeviceRGB setcolorspace\n"

"/IDD 8 dict def\n"

"IDD begin\n"
"/ImageType 1 def\n"
"/Width px def\n"
"/Height py def\n"
"/BitsPerComponent 8 def\n"
"/Interpolate inter def\n"
"/DataSource currentfile A85RLE def\n"
"/MultipleDataSources false def\n"
"/ImageMatrix [ px 0 0 py neg 0 py ] def\n"

"/Decode [ 0 1 0 1 0 1 ] def\n"
"end\n"

"/IMD 8 dict def\n"
"IMD begin\n"
"/ImageType 1 def\n"
"/Width mx def\n"
"/Height my def\n"
"/BitsPerComponent 1 def\n"
//  "/Interpolate inter def\n"
"/ImageMatrix [ mx 0 0 my neg 0 my ] def\n"
"/Decode [ 1 0 ] def\n"
"end\n"

"<<\n"
"/ImageType 3\n"
"/InterleaveType 2\n"
"/MaskDict IMD\n"
"/DataDict IDD\n"
">> image GR\n"
"} bind def\n"


//  masked gray images
"/GIM {GS /inter exch def /my exch def /mx exch def /py exch def /px exch def /sy exch def /sx exch def \n"
"translate \n"
"sx sy scale\n"
"/DeviceGray setcolorspace\n"

"/IDD 8 dict def\n"

"IDD begin\n"
"/ImageType 1 def\n"
"/Width px def\n"
"/Height py def\n"
"/BitsPerComponent 8 def\n"
"/Interpolate inter def\n"
"/DataSource currentfile A85RLE def\n"
"/MultipleDataSources false def\n"
"/ImageMatrix [ px 0 0 py neg 0 py ] def\n"

"/Decode [ 0 1 ] def\n"
"end\n"

"/IMD 8 dict def\n"

"IMD begin\n"
"/ImageType 1 def\n"
"/Width mx def\n"
"/Height my def\n"
"/BitsPerComponent 1 def\n"
"/ImageMatrix [ mx 0 0 my neg 0 my ] def\n"
"/Decode [ 1 0 ] def\n"
"end\n"

"<<\n"
"/ImageType 3\n"
"/InterleaveType 2\n"
"/MaskDict IMD\n"
"/DataDict IDD\n"
">> image GR\n"
"} bind def\n"


"\n"
;

// end prolog

int Fl_PostScript_Graphics_Driver::start_postscript (int pagecount,
    enum Fl_Paged_Device::Page_Format format, enum Fl_Paged_Device::Page_Layout layout)
//returns 0 iff OK
{
  int w, h, x;
  if (format == Fl_Paged_Device::A4) {
    left_margin = 18;
    top_margin = 18;
  }
  else {
    left_margin = 12;
    top_margin = 12;
  }
  // combine the format and layout information
  page_format_ = ((int)format | (int)layout);
  if (layout & Fl_Paged_Device::LANDSCAPE){
    ph_ = Fl_Paged_Device::page_formats[format].width;
    pw_ = Fl_Paged_Device::page_formats[format].height;
  } else {
    pw_ = Fl_Paged_Device::page_formats[format].width;
    ph_ = Fl_Paged_Device::page_formats[format].height;
  }

  fputs("%!PS-Adobe-3.0\n", output);
  fputs("%%Creator: FLTK\n", output);
  if (lang_level_>1)
    fprintf(output, "%%%%LanguageLevel: %i\n" , lang_level_);
  if ((pages_ = pagecount))
    fprintf(output, "%%%%Pages: %i\n", pagecount);
  else
    fputs("%%Pages: (atend)\n", output);
  fprintf(output, "%%%%BeginFeature: *PageSize %s\n", Fl_Paged_Device::page_formats[format].name );
  w = Fl_Paged_Device::page_formats[format].width;
  h = Fl_Paged_Device::page_formats[format].height;
  if (lang_level_ == 3 && (layout & Fl_Paged_Device::LANDSCAPE) ) { x = w; w = h; h = x; }
  fprintf(output, "<</PageSize[%d %d]>>setpagedevice\n", w, h );
  fputs("%%EndFeature\n", output);
  fputs("%%EndComments\n%%BeginProlog\n", output);
  fputs(prolog, output);
  if (lang_level_ > 1) {
    fputs(prolog_2, output);
    }
  if (lang_level_ == 2) {
    fputs(prolog_2_pixmap, output);
    }
  if (lang_level_ > 2)
    fputs(prolog_3, output);
  if (lang_level_ >= 3) {
    fputs("/CS { clipsave } bind def\n", output);
    fputs("/CR { cliprestore } bind def\n", output);
  } else {
    fputs("/CS { GS } bind def\n", output);
    fputs("/CR { GR } bind def\n", output);
  }
  page_policy_ = 1;


  fputs("%%EndProlog\n",output);
  if (lang_level_ >= 2)
    fprintf(output,"<< /Policies << /Pagesize 1 >> >> setpagedevice\n");

  reset();
  nPages=0;
  return 0;
}

int Fl_PostScript_Graphics_Driver::start_eps (int width, int height) {
  pw_ = width;
  ph_ = height;
  fputs("%!PS-Adobe-3.0 EPSF-3.0\n", output);
  fputs("%%Creator: (FLTK)\n", output);
  fprintf(output,"%%%%BoundingBox: 1 1 %d %d\n", width, height);
  if (ps_filename_) fprintf(output,"%%%%Title: (%s)\n", fl_filename_name(ps_filename_));
  time_t lt = time(NULL);
  fprintf(output,"%%%%CreationDate: %s", ctime(&lt)+4);
  lang_level_= 2;
  fprintf(output, "%%%%LanguageLevel: 2\n");
  fputs("%%Pages: 1\n%%EndComments\n", output);
  fputs("%%BeginProlog\n", output);
  fputs("%%EndProlog\n",output);
  fprintf(output, "save\n");
  fputs("/FLTK 20 dict def FLTK begin\n"
  "/x1 0 def /x2 0 def /y1 0 def /y2 0 def /x 0 def /y 0 def /dx 0 def /dy 0 def\n"
        "/px 0 def /py 0 def /sx 0 def /sy 0 def /inter 0 def\n"
        "/pixmap_sx 0 def  /pixmap_sy 0 def /pixmap_w 0 def /pixmap_h 0 def\n", output);
  fputs(prolog, output);
  fputs(prolog_2, output);
  fputs(prolog_2_pixmap, output);
  fputs("/CS { GS } bind def\n", output);
  fputs("/CR { GR } bind def\n", output);
  page_policy_ = 1;
  reset();
  nPages=0;
  fprintf(output, "GS\n");
  clocale_printf( "%g %g TR\n", (double)0, ph_);
  fprintf(output, "1 -1 SC\n");
  line_style(0);
  fprintf(output, "GS GS\n");
  return 0;
}

void Fl_PostScript_Graphics_Driver::recover(){
  color(cr_,cg_,cb_);
  line_style(linestyle_,linewidth_,linedash_);
  font(Fl_Graphics_Driver::font(), Fl_Graphics_Driver::size());
}

void Fl_PostScript_Graphics_Driver::reset(){
  gap_=1;
  clip_=0;
  cr_=cg_=cb_=0;
  Fl_Graphics_Driver::font(FL_HELVETICA, 12);
  linewidth_=0;
  linestyle_=FL_SOLID;
  strcpy(linedash_,"");
  Clip *c=clip_;   ////just not to have memory leaks for badly writen code (forgotten clip popping)

  while(c){
    clip_=clip_->prev;
    delete c;
    c=clip_;
  }

}

void Fl_PostScript_Graphics_Driver::page_policy(int p){
  page_policy_ = p;
  if(lang_level_>=2)
    fprintf(output,"<< /Policies << /Pagesize %i >> >> setpagedevice\n", p);
}

// //////////////////// paging //////////////////////////////////////////



void Fl_PostScript_Graphics_Driver::page(double pw, double ph, int media) {

  if (nPages){
    fprintf(output, "CR\nGR\nGR\nGR\nSP\nrestore\n");
  }
  ++nPages;
  fprintf(output, "%%%%Page: %i %i\n" , nPages , nPages);
  fprintf(output, "%%%%PageBoundingBox: 0 0 %d %d\n", pw > ph ? (int)ph : (int)pw , pw > ph ? (int)pw : (int)ph);
  if (pw>ph){
    fprintf(output, "%%%%PageOrientation: Landscape\n");
  }else{
    fprintf(output, "%%%%PageOrientation: Portrait\n");
  }

  fprintf(output, "%%%%BeginPageSetup\n");
  if((media & Fl_Paged_Device::MEDIA) &&(lang_level_>1)){
    int r = media & Fl_Paged_Device::REVERSED;
    if(r) r = 2;
    fprintf(output, "<< /PageSize [%i %i] /Orientation %i>> setpagedevice\n", (int)(pw+.5), (int)(ph+.5), r);
  }
  fprintf(output, "%%%%EndPageSetup\n");

/*  pw_ = pw;
  ph_ = ph;*/
  reset();

  fprintf(output, "save\n");
  fprintf(output, "GS\n");
  clocale_printf( "%g %g TR\n", (double)0 /*lm_*/ , ph_ /* - tm_*/);
  fprintf(output, "1 -1 SC\n");
  line_style(0);
  fprintf(output, "GS\n");

  if (!((media & Fl_Paged_Device::MEDIA) &&(lang_level_>1))){
    if (pw > ph) {
      if(media & Fl_Paged_Device::REVERSED) {
        fprintf(output, "-90 rotate %i 0 translate\n", int(-pw));
        }
      else {
        fprintf(output, "90 rotate -%i -%i translate\n", (lang_level_ == 2 ? int(pw - ph) : 0), int(ph));
        }
      }
      else {
        if(media & Fl_Paged_Device::REVERSED)
          fprintf(output, "180 rotate %i %i translate\n", int(-pw), int(-ph));
        }
  }
  fprintf(output, "GS\nCS\n");
}

void Fl_PostScript_Graphics_Driver::page(int format){
/*  if(format &  Fl_Paged_Device::LANDSCAPE){
    ph_=Fl_Paged_Device::page_formats[format & 0xFF].width;
    pw_=Fl_Paged_Device::page_formats[format & 0xFF].height;
  }else{
    pw_=Fl_Paged_Device::page_formats[format & 0xFF].width;
    ph_=Fl_Paged_Device::page_formats[format & 0xFF].height;
  }*/
  page(pw_,ph_,format & 0xFF00);//,orientation only;
}

void Fl_PostScript_Graphics_Driver::rect(int x, int y, int w, int h) {
  // Commented code does not work, i can't find the bug ;-(
  // fprintf(output, "GS\n");
  //  fprintf(output, "%i, %i, %i, %i R\n", x , y , w, h);
  //  fprintf(output, "GR\n");
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y);
  fprintf(output, "%i %i LT\n", x+w-1 , y);
  fprintf(output, "%i %i LT\n", x+w-1 , y+h-1);
  fprintf(output, "%i %i LT\n", x , y+h-1);
  fprintf(output, "ECP\n");
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::rectf(int x, int y, int w, int h) {
  clocale_printf( "%g %g %i %i FR\n", x-0.5, y-0.5, w, h);
}

void Fl_PostScript_Graphics_Driver::line(int x1, int y1, int x2, int y2) {
  fprintf(output, "GS\n");
  fprintf(output, "%i %i %i %i L\n", x1 , y1, x2 ,y2);
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::line(int x0, int y0, int x1, int y1, int x2, int y2) {
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x0 , y0);
  fprintf(output, "%i %i LT\n", x1 , y1);
  fprintf(output, "%i %i LT\n", x2 , y2);
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::xyline(int x, int y, int x1, int y2, int x3){
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y );
  fprintf(output, "%i %i LT\n", x1 , y );
  fprintf(output, "%i %i LT\n", x1 , y2);
  fprintf(output,"%i %i LT\n", x3 , y2);
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
}


void Fl_PostScript_Graphics_Driver::xyline(int x, int y, int x1, int y2){

  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y);
  fprintf(output,"%i %i LT\n", x1 , y);
  fprintf(output, "%i %i LT\n", x1 , y2 );
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::xyline(int x, int y, int x1){
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y);
  fprintf(output, "%i %i LT\n", x1 , y );
  fprintf(output, "ELP\n");

  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::yxline(int x, int y, int y1, int x2, int y3){
  fprintf(output, "GS\n");

  fprintf(output,"BP\n");
  fprintf(output,"%i %i MT\n", x , y);
  fprintf(output, "%i %i LT\n", x , y1 );
  fprintf(output, "%i %i LT\n", x2 , y1 );
  fprintf(output , "%i %i LT\n", x2 , y3);
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::yxline(int x, int y, int y1, int x2){
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y);
  fprintf(output, "%i %i LT\n", x , y1);
  fprintf(output, "%i %i LT\n", x2 , y1);
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::yxline(int x, int y, int y1){
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y);
  fprintf(output, "%i %i LT\n", x , y1);
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2) {
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x0 , y0);
  fprintf(output, "%i %i LT\n", x1 , y1);
  fprintf(output, "%i %i LT\n", x2 , y2);
  fprintf(output, "ECP\n");
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x0 , y0);
  fprintf(output, "%i %i LT\n", x1 , y1);
  fprintf(output, "%i %i LT\n", x2 , y2);
  fprintf(output, "%i %i LT\n", x3 , y3);
  fprintf(output, "ECP\n");
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2) {
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x0 , y0);
  fprintf(output,"%i %i LT\n", x1 , y1);
  fprintf(output, "%i %i LT\n", x2 , y2);
  fprintf(output, "EFP\n");
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x0 , y0 );
  fprintf(output, "%i %i LT\n", x1 , y1 );
  fprintf(output, "%i %i LT\n", x2 , y2 );
  fprintf(output, "%i %i LT\n", x3 , y3 );

  fprintf(output, "EFP\n");
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::line_style(int style, int width, char* dashes){
  //line_styled_=1;

  linewidth_=width;
  linestyle_=style;
  //dashes_= dashes;
  if(dashes){
    if(dashes != linedash_)
      strcpy(linedash_,dashes);

  }else
    linedash_[0]=0;
  char width0 = 0;
  if(!width){
    width=1; //for screen drawing compatibility
    width0=1;
  }

  fprintf(output, "%i setlinewidth\n", width);

  if(!style && (!dashes || !(*dashes)) && width0) //system lines
    style = FL_CAP_SQUARE;

  int cap = (style &0xf00) >> 8;
  if(cap) cap--;
  fprintf(output,"%i setlinecap\n", cap);

  int join = (style & 0xf000) >> 12;

  if(join) join--;
  fprintf(output,"%i setlinejoin\n", join);


  fprintf(output, "[");
  if(dashes && *dashes){
    while(*dashes){
      fprintf(output, "%i ", *dashes);
      dashes++;
    }
  }else{
    if(style & 0x200){ // round and square caps, dash length need to be adjusted
      const double *dt = dashes_cap[style & 0xff];
      while (*dt >= 0){
        clocale_printf("%g ",width * (*dt));
        dt++;
      }
    }else{

      const int *ds = dashes_flat[style & 0xff];
      while (*ds >= 0){
        fprintf(output, "%i ",width * (*ds));
        ds++;
      }
    }
  }
  fprintf(output, "] 0 setdash\n");
}

void Fl_PostScript_Graphics_Driver::color(Fl_Color c) {
  Fl::get_color(c, cr_, cg_, cb_);
  color(cr_, cg_, cb_);
}

Fl_Color Fl_PostScript_Graphics_Driver::color() { return Fl_Graphics_Driver::color(); }

void Fl_PostScript_Graphics_Driver::color(unsigned char r, unsigned char g, unsigned char b) {
  Fl_Graphics_Driver::color( fl_rgb_color(r, g, b) );
  cr_ = r; cg_ = g; cb_ = b;
  if (r == g && g == b) {
    double gray = r/255.0;
    clocale_printf("%g GL\n", gray);
  } else {
    double fr, fg, fb;
    fr = r/255.0;
    fg = g/255.0;
    fb = b/255.0;
    clocale_printf("%g %g %g SRGB\n", fr , fg , fb);
  }
}

void Fl_PostScript_Graphics_Driver::draw(int rotation, const char *str, int n, int x, int y)
{
  fprintf(output, "GS %d %d translate %d rotate\n", x, y, -rotation);
  this->transformed_draw(str, n, 0, 0);
  fprintf(output, "GR\n");
}


// computes the mask for the RGB image img of all pixels with color != bg
static uchar *calc_mask(uchar *img, int w, int h, Fl_Color bg)
{
  uchar red, green, blue, r, g, b;
  uchar bit, byte, *q;
  Fl::get_color(bg, red, green, blue);
  int W = (w+7)/8; // width of mask
  uchar* mask = new uchar[W * h];
  q = mask;
  while (h-- > 0) { // for each row
    bit = 0x80; // byte with last bit set
    byte = 0; // next mask byte to compute
    for (int j = 0; j < w; j++) { // for each column
      r = *img++; // the pixel color components
      g = *img++;
      b = *img++;
      // if pixel doesn't have bg color, put it in mask
      if (r != red || g != green || b != blue) byte |= bit;
      bit = bit>>1; // shift bit one step to the right
      if (bit == 0) { // single set bit has fallen out
        *q++ = byte; // enter byte in mask
        byte = 0; // reset next mask byte to zero
        bit = 0x80; // and this byte
        }
      }
    if (bit != 0x80) *q++ = byte; // enter last columns' byte in mask
    }
  return mask;
}

// write to PostScript a bitmap image of a UTF8 string
void Fl_PostScript_Graphics_Driver::transformed_draw_extra(const char* str, int n, double x, double y, int w, bool rtl)
{
  // scale for bitmask computation is set to 1 when we can't expect to have scalable fonts
  float scale = Fl_Graphics_Driver::default_driver().scale_bitmap_for_PostScript();
  Fl_Fontsize old_size = size();
  Fl_Font fontnum = Fl_Graphics_Driver::font();
  int w_scaled =  (int)(w * (scale + 0.5) + 1);
  int h = (int)(height() * scale);
  // create an offscreen image of the string
  Fl_Color text_color = Fl_Graphics_Driver::color();
  Fl_Color bg_color = fl_contrast(FL_WHITE, text_color);
  Fl_Image_Surface *off = new Fl_Image_Surface(w_scaled, (int)(h+3*scale), 1);
  Fl_Surface_Device::push_current(off);
  fl_color(bg_color);
  // color offscreen background with a shade contrasting with the text color
  fl_rectf(0, 0, w_scaled, (int)(h+3*scale) );
  fl_color(text_color);
  if (scale < 1.5) {
    // force seeing this font as new so it's applied to the offscreen graphics context
    fl_graphics_driver->font_descriptor(NULL);
    fl_font(fontnum, 0);
  }
  fl_font(fontnum, (Fl_Fontsize)(scale * old_size) );
  int w2 = (int)fl_width(str, n);
  if (w2 > w_scaled) w2 = w_scaled;
  // draw string in offscreen
  if (rtl) fl_rtl_draw(str, n, w2, (int)(h * 0.8) );
  else fl_draw(str, n, 0, (int)(h * 0.8) );
  // read (most of) the offscreen image
  uchar *img = fl_read_image(NULL, 0, 1, w2, h, 0);
  Fl_Surface_Device::pop_current();
  font(fontnum, old_size);
  delete off;
  // compute the mask of what is not the background
  uchar *img_mask = calc_mask(img, w2, h, bg_color);
  delete[] img;
  // write the string image to PostScript as a scaled bitmask
  scale = w2 / float(w);
  clocale_printf("%g %g %g %g %d %d MI\n", x, y - h*0.77/scale, w2/scale, h/scale, w2, h);
  uchar *di;
  int wmask = (w2+7)/8;
  void *rle85 = prepare_rle85();
  for (int j = h - 1; j >= 0; j--){
    di = img_mask + j * wmask;
    for (int i = 0; i < wmask; i++){
      write_rle85(*di, rle85);
      di++;
    }
  }
  close_rle85(rle85); fputc('\n', output);
  delete[] img_mask;
}

static int is_in_table(unsigned utf) {
  unsigned i;
  static unsigned extra_table_roman[] = { // unicodes/*names*/ of other characters from PostScript standard fonts
    0x192/*florin*/, 0x2C6/*circumflex*/, 0x2C7/*caron*/,
    0x2D8/*breve*/, 0x2D9/*dotaccent*/, 0x2DA/*ring*/, 0x2DB/*ogonek*/, 0x2DC/*tilde*/, 0x2DD/*hungarumlaut*/,
    0x2013/*endash*/, 0x2014/*emdash*/, 0x2018/*quoteleft*/, 0x2019/*quoteright*/,
    0x201A/*quotesinglbase*/, 0x201C/*quotedblleft*/, 0x201D/*quotedblright*/, 0x201E/*quotedblbase*/,
    0x2020/*dagger*/, 0x2021/*daggerdbl*/, 0x2022/*bullet*/,
    0x2026/*ellipsis*/, 0x2030/*perthousand*/, 0x2039/*guilsinglleft*/, 0x203A/*guilsinglright*/,
    0x2044/*fraction*/, 0x20AC/*Euro*/, 0x2122/*trademark*/,
    0x2202/*partialdiff*/, 0x2206/*Delta*/, 0x2211/*summation*/, 0x221A/*radical*/,
    0x221E/*infinity*/, 0x2260/*notequal*/, 0x2264/*lessequal*/,
    0x2265/*greaterequal*/,
    0x25CA/*lozenge*/, 0xFB01/*fi*/, 0xFB02/*fl*/,
    0xF8FF/*apple*/
  };
  for ( i = 0; i < sizeof(extra_table_roman)/sizeof(int); i++) {
    if (extra_table_roman[i] == utf) return i + 0x180;
  }
  return 0;
}

// outputs in PostScript a UTF8 string using the same width in points as on display
void Fl_PostScript_Graphics_Driver::transformed_draw(const char* str, int n, double x, double y) {
  int len, code;
  if (!n || !str || !*str) return;
  // compute display width of string
  int w = (int)width(str, n);
  if (w == 0) return;
  if (Fl_Graphics_Driver::font() >= FL_FREE_FONT) {
    transformed_draw_extra(str, n, x, y, w, false);
    return;
    }
  fprintf(output, "%d <~", w);
  void *data = prepare85();
  // transforms UTF8 encoding to our custom PostScript encoding as follows:
  // extract each unicode character
  // if unicode <= 0x17F, unicode and PostScript codes are identical
  // if unicode is one of the values listed in extra_table_roman above
  //    its PostScript code is 0x180 + the character's rank in extra_table_roman
  // if unicode is something else, draw all string as bitmap image

  const char *last = str + n;
  const char *str2 = str;
  while (str2 < last) {
    // Extract each unicode character of string.
    unsigned utf = fl_utf8decode(str2, last, &len);
    str2 += len;
    if (utf <= 0x17F) { // until Latin Extended-A
      ;
      }
    else if ( (code = is_in_table(utf)) != 0) { // other handled characters
      utf = code;
      }
    else { // unhandled character: draw all string as bitmap image
      fprintf(output, "~> pop pop\n"); // close and ignore the opened hex string
      transformed_draw_extra(str, n, x, y, w, false);
      return;
    }
    // 2 bytes per character, high-order byte first, encode that to ASCII85
    uchar c[2]; c[1] = utf & 0xFF; c[0] = (utf & 0xFF00)>>8; write85(data, c, 2);
  }
  close85(data);
  clocale_printf(" %g %g show_pos_width\n", x, y);
}

void Fl_PostScript_Graphics_Driver::rtl_draw(const char* str, int n, int x, int y) {
  int w = (int)width(str, n);
  transformed_draw_extra(str, n, x - w, y, w, true);
}

void Fl_PostScript_Graphics_Driver::concat(){
  clocale_printf("[%g %g %g %g %g %g] CT\n", m.a , m.b , m.c , m.d , m.x , m.y);
}

void Fl_PostScript_Graphics_Driver::reconcat(){
  clocale_printf("[%g %g %g %g %g %g] RCT\n" , m.a , m.b , m.c , m.d , m.x , m.y);
}

/////////////////  transformed (double) drawings ////////////////////////////////


void Fl_PostScript_Graphics_Driver::begin_points(){
  fprintf(output, "GS\n");
  concat();

  fprintf(output, "BP\n");
  gap_=1;
  what=POINTS;
}

void Fl_PostScript_Graphics_Driver::begin_line(){
  fprintf(output, "GS\n");
  concat();
  fprintf(output, "BP\n");
  gap_=1;
  what=LINE;
}

void Fl_PostScript_Graphics_Driver::begin_loop(){
  fprintf(output, "GS\n");
  concat();
  fprintf(output, "BP\n");
  gap_=1;
  what=LOOP;
}

void Fl_PostScript_Graphics_Driver::begin_polygon(){
  fprintf(output, "GS\n");
  concat();
  fprintf(output, "BP\n");
  gap_=1;
  what=POLYGON;
}

void Fl_PostScript_Graphics_Driver::vertex(double x, double y){
  if(what==POINTS){
    clocale_printf("%g %g MT\n", x , y);
    gap_=1;
    return;
  }
  if(gap_){
    clocale_printf("%g %g MT\n", x , y);
    gap_=0;
  }else
    clocale_printf("%g %g LT\n", x , y);
}

void Fl_PostScript_Graphics_Driver::curve(double x, double y, double x1, double y1, double x2, double y2, double x3, double y3){
  if(what==NONE) return;
  if(gap_)
    clocale_printf("%g %g MT\n", x , y);
  else
    clocale_printf("%g %g LT\n", x , y);
  gap_=0;

  clocale_printf("%g %g %g %g %g %g curveto \n", x1 , y1 , x2 , y2 , x3 , y3);
}


void Fl_PostScript_Graphics_Driver::circle(double x, double y, double r){
  if(what==NONE){
    fprintf(output, "GS\n");
    concat();
    //    fprintf(output, "BP\n");
    clocale_printf("%g %g %g 0 360 arc\n", x , y , r);
    reconcat();
    //    fprintf(output, "ELP\n");
    fprintf(output, "GR\n");
  }else

    clocale_printf("%g %g %g 0 360 arc\n", x , y , r);

}

void Fl_PostScript_Graphics_Driver::arc(double x, double y, double r, double start, double a){
  if(what==NONE) return;
  gap_=0;
  if(start>a)
    clocale_printf("%g %g %g %g %g arc\n", x , y , r , -start, -a);
  else
    clocale_printf("%g %g %g %g %g arcn\n", x , y , r , -start, -a);

}

void Fl_PostScript_Graphics_Driver::arc(int x, int y, int w, int h, double a1, double a2) {
  if (w <= 1 || h <= 1) return;
  fprintf(output, "GS\n");
  //fprintf(output, "BP\n");
  begin_line();
  clocale_printf("%g %g TR\n", x + w/2.0 -0.5 , y + h/2.0 - 0.5);
  clocale_printf("%g %g SC\n", (w-1)/2.0 , (h-1)/2.0 );
  arc(0,0,1,a2,a1);
  //  fprintf(output, "0 0 1 %g %g arc\n" , -a1 , -a2);
  clocale_printf("%g %g SC\n", 2.0/(w-1) , 2.0/(h-1) );
  clocale_printf("%g %g TR\n", -x - w/2.0 +0.5 , -y - h/2.0 +0.5);
  end_line();

  //  fprintf(output, "%g setlinewidth\n",  2/sqrt(w*h));
  //  fprintf(output, "ELP\n");
  //  fprintf(output, 2.0/w , 2.0/w , " SC\n";
  //  fprintf(output, (-x - w/2.0) , (-y - h/2)  , " TR\n";
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::pie(int x, int y, int w, int h, double a1, double a2) {
  fprintf(output, "GS\n");
  begin_polygon();
  clocale_printf("%g %g TR\n", x + w/2.0 -0.5 , y + h/2.0 - 0.5);
  clocale_printf("%g %g SC\n", (w-1)/2.0 , (h-1)/2.0 );
  vertex(0,0);
  arc(0.0,0.0, 1, a2, a1);
  end_polygon();
  fprintf(output, "GR\n");
}

void Fl_PostScript_Graphics_Driver::end_points(){
  gap_=1;
  reconcat();
  fprintf(output, "ELP\n"); //??
  fprintf(output, "GR\n");
  what=NONE;
}

void Fl_PostScript_Graphics_Driver::end_line(){
  gap_=1;
  reconcat();
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
  what=NONE;
}
void Fl_PostScript_Graphics_Driver::end_loop(){
  gap_=1;
  reconcat();
  fprintf(output, "ECP\n");
  fprintf(output, "GR\n");
  what=NONE;
}

void Fl_PostScript_Graphics_Driver::end_polygon(){

  gap_=1;
  reconcat();
  fprintf(output, "EFP\n");
  fprintf(output, "GR\n");
  what=NONE;
}

void Fl_PostScript_Graphics_Driver::transformed_vertex(double x, double y){
  reconcat();
  if(gap_){
    clocale_printf("%g %g MT\n", x , y);
    gap_=0;
  }else
    clocale_printf("%g %g LT\n", x , y);
  concat();
}

/////////////////////////////   Clipping /////////////////////////////////////////////

void Fl_PostScript_Graphics_Driver::push_clip(int x, int y, int w, int h) {
  Clip * c=new Clip();
  clip_box(x,y,w,h,c->x,c->y,c->w,c->h);
  c->prev=clip_;
  clip_=c;
  fprintf(output, "CR\nCS\n");
  if(lang_level_<3)
    recover();
  clocale_printf("%g %g %i %i CL\n", clip_->x-0.5 , clip_->y-0.5 , clip_->w  , clip_->h);

}

void Fl_PostScript_Graphics_Driver::push_no_clip() {
  Clip * c = new Clip();
  c->prev=clip_;
  clip_=c;
  clip_->x = clip_->y = clip_->w = clip_->h = -1;
  fprintf(output, "CR\nCS\n");
  if(lang_level_<3)
    recover();
}

void Fl_PostScript_Graphics_Driver::pop_clip() {
  if(!clip_)return;
  Clip * c=clip_;
  clip_=clip_->prev;
  delete c;
  fprintf(output, "CR\nCS\n");
  if(clip_ && clip_->w >0)
    clocale_printf("%g %g %i %i CL\n", clip_->x - 0.5, clip_->y - 0.5, clip_->w  , clip_->h);
  // uh, -0.5 is to match screen clipping, for floats there should be something beter
  if(lang_level_<3)
    recover();
}

void Fl_PostScript_Graphics_Driver::ps_origin(int x, int y)
{
  clocale_printf("GR GR GS %d %d TR  %f %f SC %d %d TR %f rotate GS\n",
    left_margin, top_margin, scale_x, scale_y, x, y, angle);
}

void Fl_PostScript_Graphics_Driver::ps_translate(int x, int y)
{
  fprintf(output, "GS %d %d translate GS\n", x, y);
}

void Fl_PostScript_Graphics_Driver::ps_untranslate(void)
{
  fprintf(output, "GR GR\n");
}

#if defined(FLTK_USE_X11) || defined(FLTK_USE_WAYLAND)

Fl_Paged_Device *Fl_PDF_File_Surface::new_platform_pdf_surface_(const char ***pfname) {
  *pfname = NULL;
  return new Fl_PostScript_File_Device;
}

int Fl_PDF_File_Surface::begin_job(const char* defaultfilename,
                                char **perr_message) {
  if (perr_message) {
    *perr_message = strdup("Class Fl_PDF_File_Surface requires PANGO to be usable.");
  }
  return 2;
}

int Fl_PDF_File_Surface::begin_document(const char* defaultfilename,
                                     enum Fl_Paged_Device::Page_Format format,
                                     enum Fl_Paged_Device::Page_Layout layout,
                                     char **perr_message) {
  return begin_job(NULL, perr_message);
}

#endif // defined(FLTK_USE_X11) || defined(FLTK_USE_WAYLAND)

# else // USE_PANGO

/* Cairo-based implementation of the PostScript graphics driver */

static cairo_status_t write_to_cairo_stream(FILE *output, unsigned char *data, unsigned int length) {
  size_t l = fwrite(data, 1, length, output);
  return (l == length ? CAIRO_STATUS_SUCCESS : CAIRO_STATUS_WRITE_ERROR);
}

static cairo_t* init_cairo_postscript(FILE* output, int w, int h) {
  cairo_surface_t* cs = cairo_ps_surface_create_for_stream(
                        (cairo_write_func_t)write_to_cairo_stream, output, w, h);
  if (cairo_surface_status(cs) != CAIRO_STATUS_SUCCESS) return NULL;
  cairo_ps_surface_restrict_to_level(cs, CAIRO_PS_LEVEL_2);
  cairo_t* cairo_ = cairo_create(cs);
  cairo_surface_destroy(cs);
  return cairo_;
}

int Fl_PostScript_Graphics_Driver::start_postscript(int pagecount,
    enum Fl_Paged_Device::Page_Format format, enum Fl_Paged_Device::Page_Layout layout)
//returns 0 iff OK
{
  if (format == Fl_Paged_Device::A4) {
    left_margin = 18;
    top_margin = 18;
  }
  else {
    left_margin = 12;
    top_margin = 12;
  }
  // combine the format and layout information
  page_format_ = ((int)format | (int)layout);
  if (layout & Fl_Paged_Device::LANDSCAPE){
    ph_ = Fl_Paged_Device::page_formats[format].width;
    pw_ = Fl_Paged_Device::page_formats[format].height;
  } else {
    pw_ = Fl_Paged_Device::page_formats[format].width;
    ph_ = Fl_Paged_Device::page_formats[format].height;
  }
  cairo_ = init_cairo_postscript(output, Fl_Paged_Device::page_formats[format].width,
                            Fl_Paged_Device::page_formats[format].height);
  if (!cairo_) return 1;
  nPages=0;
  char feature[250];
  snprintf(feature, 250, "%%%%BeginFeature: *PageSize %s\n<</PageSize[%d %d]>>setpagedevice\n%%%%EndFeature",
          Fl_Paged_Device::page_formats[format].name, Fl_Paged_Device::page_formats[format].width, Fl_Paged_Device::page_formats[format].height);
  cairo_ps_surface_dsc_comment(cairo_get_target(cairo_), feature);
  return 0;
}

int Fl_PostScript_Graphics_Driver::start_eps(int width, int height) {
  pw_ = width;
  ph_ = height;
  cairo_ = init_cairo_postscript(output, width, height);
  if (!cairo_) return 1;
  cairo_ps_surface_set_eps(cairo_get_target(cairo_), true);
  nPages=0; //useful?
  return 0;
}


void Fl_PostScript_Graphics_Driver::transformed_draw(const char* str, int n, double x, double y) {
  if (!n) return;
  if (!pango_context_) {
    PangoFontMap *def_font_map = pango_cairo_font_map_get_default(); // 1.10
#if PANGO_VERSION_CHECK(1,22,0)
    pango_context_ = pango_font_map_create_context(def_font_map); // 1.22
#else
    pango_context_ = pango_context_new();
    pango_context_set_font_map(pango_context_, def_font_map);
#endif
    pango_layout_ = pango_layout_new(pango_context_);
  }
  PangoFontDescription *pfd = Fl_Graphics_Driver::default_driver().pango_font_description();
  pango_layout_set_font_description(pango_layout_, pfd);
  int pwidth, pheight;
  cairo_save(cairo_);
  str = Fl_Cairo_Graphics_Driver::clean_utf8(str, n);
  pango_layout_set_text(pango_layout_, str, n);
  pango_layout_get_size(pango_layout_, &pwidth, &pheight);
  if (pwidth > 0) {
    double s = width(str, n);
    cairo_translate(cairo_, x, y - height() + descent());
    s = (s/pwidth) * PANGO_SCALE;
    cairo_scale(cairo_, s, s);
    pango_cairo_show_layout(cairo_, pango_layout_); // 1.10
  }
  cairo_restore(cairo_);
  check_status();
}


// =======================================================


class Fl_PDF_Pango_File_Surface : public Fl_PostScript_File_Device
{
public:
  char *doc_fname;
  Fl_PDF_Pango_File_Surface();
  ~Fl_PDF_Pango_File_Surface() { if (doc_fname) free(doc_fname); }
  int begin_job(const char *defaultname,
                char **perr_message = NULL);
  int begin_job(int, int*, int *, char **) FL_OVERRIDE {return 1;} // don't use
  int begin_document(const char* outname,
                     enum Fl_Paged_Device::Page_Format format,
                     enum Fl_Paged_Device::Page_Layout layout,
                     char **perr_message);
  int begin_page() FL_OVERRIDE;
  void end_job() FL_OVERRIDE;
};


Fl_PDF_Pango_File_Surface::Fl_PDF_Pango_File_Surface() {
  doc_fname = NULL;
  driver()->output = NULL;
}


static Fl_Paged_Device::Page_Format menu_to_size[] = {Fl_Paged_Device::A3, Fl_Paged_Device::A4,
  Fl_Paged_Device::A5, Fl_Paged_Device::B4, Fl_Paged_Device::B5, Fl_Paged_Device::EXECUTIVE,
  Fl_Paged_Device::LEGAL, Fl_Paged_Device::LETTER, Fl_Paged_Device::TABLOID
};
static int size_count = sizeof(menu_to_size) / sizeof(menu_to_size[0]);


static int update_format_layout(int rank, Fl_Paged_Device::Page_Layout layout,
                                bool &need_set_default_psize) {
  int status = -1;
  Fl_Window *modal = new Fl_Window(510, 90, Fl_PDF_File_Surface::format_dialog_title);
  modal->begin();
  Fl_Choice *psize = new Fl_Choice(140, 10, 110, 30, Fl_PDF_File_Surface::format_dialog_page_size);
  psize->when(FL_WHEN_CHANGED);
  for (int i = 0; i < size_count; i++) {
    psize->add(Fl_Paged_Device::page_formats[menu_to_size[i]].name);
  }
  psize->value(rank);
  Fl_Check_Button *default_size = new Fl_Check_Button(psize->x(), psize->y() + psize->h(),
                          psize->w(), psize->h(), Fl_PDF_File_Surface::format_dialog_default);
  default_size->value(1);
  default_size->user_data(&need_set_default_psize);
  FL_INLINE_CALLBACK_2(psize, Fl_Choice*, choice, psize,
                       Fl_Check_Button*, check_but, default_size,
                       {
                        if (check_but->value() && choice->mvalue() && choice->prev_mvalue() &&
                            choice->prev_mvalue() != choice->mvalue()) {
                          check_but->value(0);
                        }
                       });
  FL_INLINE_CALLBACK_2( modal, Fl_Window*, win, modal,
                       Fl_Check_Button*, check_but, default_size,
                       {
                        *((bool*)check_but->user_data()) = check_but->value();
                        win->hide();
                       } );
  Fl_Choice *orientation = new Fl_Choice(psize->x() + psize->w() + 120, psize->y(), 130, psize->h(),
                                         Fl_PDF_File_Surface::format_dialog_orientation);
  orientation->add("PORTRAIT|LANDSCAPE");
  orientation->value(layout == Fl_Paged_Device::PORTRAIT ? 0 : 1);
  Fl_Return_Button *ok = new Fl_Return_Button(orientation->x() + orientation->w() - 55,
                                              psize->y() + psize->h() + 10, 55, 30, fl_ok);
  FL_INLINE_CALLBACK_4( ok, Fl_Widget*, b, ok,
                       int*, pstatus, &status,
                       Fl_Choice*, psize, psize,
                       Fl_Choice*, orientation, orientation,
                       {
    *pstatus = menu_to_size[psize->value()] + 0x100 * orientation->value();
    b->window()->do_callback();
                       } );
  Fl_Button *cancel = new Fl_Button(ok->x() - 90, psize->y() + psize->h() + 10, 70, 30, fl_cancel);
  FL_INLINE_CALLBACK_1( cancel, Fl_Widget*, wid, cancel, { wid->window()->do_callback(); } );
  modal->end();
  modal->set_modal();
  modal->show();
  while (modal->shown()) Fl::wait();
  delete modal;
  return status;
}


int Fl_PDF_Pango_File_Surface::begin_job(const char *defaultname, char **perr_message) {
  static Page_Layout layout = PORTRAIT;

  Fl_Preferences print_prefs(Fl_Preferences::CORE_USER, "fltk.org", "printers");
  char *pref_format;
  print_prefs.get("PDF/page_size", pref_format, "A4");
  int rank = 1; // corresponds to A4
  for (int i = 0; i < size_count; i++) {
    if (strcmp(pref_format, Fl_Paged_Device::page_formats[menu_to_size[i]].name) == 0) {
      rank = i;
      break;
    }
  }
  bool need_set_default_psize;
  int status = update_format_layout(rank, layout, need_set_default_psize);
  if (status == -1) return 1;
  Page_Format format = (Page_Format)(status & 0xFF);
  if (need_set_default_psize) print_prefs.set("PDF/page_size", Fl_Paged_Device::page_formats[format].name);

  Fl_Native_File_Chooser ch(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  ch.preset_file(defaultname);
  ch.filter("*.pdf");
  ch.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM);
  int retval = ch.show();
  if (retval) return (retval == -1 ? 2 : 1);

  layout = (Page_Layout)(status & 0x100);
  return begin_document(ch.filename(), format, layout, perr_message);
}


int Fl_PDF_Pango_File_Surface::begin_document(const char* outfname,
                                           enum Fl_Paged_Device::Page_Format format,
                                           enum Fl_Paged_Device::Page_Layout layout,
                                           char **perr_message) {
  int w = page_formats[format].width;
  int h = page_formats[format].height;
  if (layout == LANDSCAPE) {
    int tmp = w;
    w = h;
    h = tmp;
  }
  Fl_PostScript_Graphics_Driver *dr = driver();
  dr->output = fopen(outfname, "w");
  cairo_status_t status = CAIRO_STATUS_WRITE_ERROR;
  cairo_surface_t* cs = NULL;
  if (dr->output) {
    cs = cairo_pdf_surface_create_for_stream ( (cairo_write_func_t)write_to_cairo_stream,
                                              dr->output, w, h);
    status = cairo_surface_status(cs);
  }
  if (status != CAIRO_STATUS_SUCCESS) {
    if (perr_message) {
      const char *mess = cairo_status_to_string(status);
      size_t l = strlen(mess) + strlen(outfname) + 100;
      *perr_message = new char[l];
      snprintf(*perr_message, l, "Error '%s' while attempting to create %s.", mess, outfname);
    }
    if (cs) cairo_surface_destroy(cs);
    return 2;
  }
  cairo_pdf_surface_restrict_to_version(cs, CAIRO_PDF_VERSION_1_4);
  cairo_t *cr = cairo_create(cs);
  cairo_surface_destroy(cs);
  dr->set_cairo(cr);
  dr->pw_ = w;
  dr->ph_ = h;
  if (format == Fl_Paged_Device::A4) {
    dr->left_margin = 18;
    dr->top_margin = 18;
  }
  else {
    dr->left_margin = 12;
    dr->top_margin = 12;
  }
  doc_fname = strdup(outfname);
  return 0;
}


int Fl_PDF_Pango_File_Surface::begin_page(void)
{
  Fl_PostScript_Graphics_Driver *ps = driver();
  Fl_Surface_Device::push_current(this);
  cairo_save(ps->cr());
  cairo_translate(ps->cr(), ps->left_margin, ps->top_margin);
  cairo_set_line_width(ps->cr(), 1);
  cairo_set_source_rgb(ps->cr(), 1.0, 1.0, 1.0); // white background
  cairo_save(ps->cr());
  cairo_save(ps->cr());
  ps->check_status();
  x_offset = 0;
  y_offset = 0;
  ps->scale_x = ps->scale_y = 1.;
  ps->angle = 0;
  return 0;
}


void Fl_PDF_Pango_File_Surface::end_job() {
  Fl_PostScript_Graphics_Driver *ps = driver();
  int error = 0;
  cairo_surface_t *s = cairo_get_target(ps->cr());
  cairo_surface_finish(s);
  error = cairo_surface_status(s);
  int err2 = fclose(ps->output);
  ps->output = NULL;
  if (!error) error = err2;
  cairo_destroy(ps->cr());
  while (ps->clip_){
    Fl_PostScript_Graphics_Driver::Clip * c= ps->clip_;
    ps->clip_= ps->clip_->prev;
    delete c;
  }
  if (error) fl_alert ("Error during PostScript data output.");
}


Fl_Paged_Device *Fl_PDF_File_Surface::new_platform_pdf_surface_(const char ***pfname) {
  Fl_PDF_Pango_File_Surface *surf = new Fl_PDF_Pango_File_Surface();
  *pfname = (const char**)&surf->doc_fname;
  return surf;
}

int Fl_PDF_File_Surface::begin_job(const char* defaultfilename,
                                char **perr_message) {
  return ((Fl_PDF_Pango_File_Surface*)platform_surface_)->begin_job(defaultfilename, perr_message);
}


int Fl_PDF_File_Surface::begin_document(const char* defaultfilename,
                                     enum Fl_Paged_Device::Page_Format format,
                                     enum Fl_Paged_Device::Page_Layout layout,
                                     char **perr_message) {
  return ((Fl_PDF_Pango_File_Surface*)platform_surface_)->begin_document(defaultfilename, format, layout, perr_message);
}

#endif // USE_PANGO

/**
\}
\endcond
*/

void Fl_PostScript_File_Device::margins(int *left, int *top, int *right, int *bottom) // to implement
{
  Fl_PostScript_Graphics_Driver *ps = driver();
  if(left) *left = (int)(ps->left_margin / ps->scale_x + .5);
  if(right) *right = (int)(ps->left_margin / ps->scale_x + .5);
  if(top) *top = (int)(ps->top_margin / ps->scale_y + .5);
  if(bottom) *bottom = (int)(ps->top_margin / ps->scale_y + .5);
}

int Fl_PostScript_File_Device::printable_rect(int *w, int *h)
//returns 0 iff OK
{
  Fl_PostScript_Graphics_Driver *ps = driver();
  if(w) *w = (int)((ps->pw_ - 2 * ps->left_margin) / ps->scale_x + .5);
  if(h) *h = (int)((ps->ph_ - 2 * ps->top_margin) / ps->scale_y + .5);
  return 0;
}

void Fl_PostScript_File_Device::origin(int *x, int *y)
{
  Fl_Paged_Device::origin(x, y);
}

void Fl_PostScript_File_Device::origin(int x, int y)
{
  x_offset = x;
  y_offset = y;
  driver()->ps_origin(x, y);
}

void Fl_PostScript_File_Device::scale (float s_x, float s_y)
{
  if (s_y == 0.) s_y = s_x;
  Fl_PostScript_Graphics_Driver *ps = driver();
  ps->scale_x = s_x;
  ps->scale_y = s_y;
#if USE_PANGO
  cairo_restore(ps->cr());
  cairo_restore(ps->cr());
  cairo_save(ps->cr());
  cairo_scale(ps->cr(), s_x, s_y);
  cairo_rotate(ps->cr(), ps->angle * M_PI / 180);
  cairo_save(ps->cr());
#else
  ps->clocale_printf("GR GR GS %d %d TR  %f %f SC %f rotate GS\n",
          ps->left_margin, ps->top_margin, ps->scale_x, ps->scale_y, ps->angle);
#endif
}

void Fl_PostScript_File_Device::rotate (float rot_angle)
{
  Fl_PostScript_Graphics_Driver *ps = driver();
  ps->angle = - rot_angle;
#if USE_PANGO
  cairo_restore(ps->cr());
  cairo_restore(ps->cr());
  cairo_save(ps->cr());
  cairo_scale(ps->cr(), ps->scale_x, ps->scale_y);
  cairo_translate(ps->cr(), x_offset, y_offset);
  cairo_rotate(ps->cr(), ps->angle * M_PI / 180);
  cairo_save(ps->cr());
#else
  ps->clocale_printf("GR GR GS %d %d TR  %f %f SC %d %d TR %f rotate GS\n",
          ps->left_margin, ps->top_margin, ps->scale_x, ps->scale_y, x_offset, y_offset, ps->angle);
#endif
}

void Fl_PostScript_File_Device::translate(int x, int y)
{
  driver()->ps_translate(x, y);
}

void Fl_PostScript_File_Device::untranslate(void)
{
  driver()->ps_untranslate();
}

int Fl_PostScript_File_Device::begin_page (void)
{
  Fl_PostScript_Graphics_Driver *ps = driver();
  Fl_Surface_Device::push_current(this);
#if USE_PANGO
  cairo_ps_surface_dsc_begin_page_setup(cairo_get_target(ps->cr()));
  char feature[200];
  snprintf(feature, 200, "%%%%PageOrientation: %s", ps->pw_ > ps->ph_ ? "Landscape" : "Portrait");
  cairo_ps_surface_dsc_comment(cairo_get_target(ps->cr()), feature);
  cairo_save(ps->cr());
  if (ps->pw_ > ps->ph_) {
    cairo_translate(ps->cr(), 0, ps->pw_);
    cairo_rotate(ps->cr(), -M_PI/2);
  }
  cairo_translate(ps->cr(), ps->left_margin, ps->top_margin);
  cairo_set_line_width(ps->cr(), 1);
  cairo_set_source_rgb(ps->cr(), 1.0, 1.0, 1.0); // white background
  cairo_save(ps->cr());
  cairo_save(ps->cr());
  ps->check_status();
#else
  ps->page(ps->page_format_);
#endif
  x_offset = 0;
  y_offset = 0;
  ps->scale_x = ps->scale_y = 1.;
  ps->angle = 0;
#if ! USE_PANGO
  fprintf(ps->output, "GR GR GS %d %d translate GS\n", ps->left_margin, ps->top_margin);
#endif
  return 0;
}

int Fl_PostScript_File_Device::end_page (void)
{
#if USE_PANGO
  Fl_PostScript_Graphics_Driver *ps = (Fl_PostScript_Graphics_Driver*)driver();
  cairo_restore(ps->cr());
  cairo_restore(ps->cr());
  cairo_restore(ps->cr());
  cairo_show_page(ps->cr());
  ps->check_status();
#endif
  Fl_Surface_Device::pop_current();
  return 0;
}

void Fl_PostScript_File_Device::end_job (void)
// finishes PostScript & closes file
{
  Fl_PostScript_Graphics_Driver *ps = driver();
  int error = 0;
#if USE_PANGO
  cairo_surface_t *s = cairo_get_target(ps->cr());
  cairo_surface_finish(s);
  error = cairo_surface_status(s);
  if (error) {
    fclose(ps->output);
    fputs("\n", ps->output); // creates an stdio error
  }
  cairo_destroy(ps->cr());
  if (!error) error = fflush(ps->output);
#else
  if (ps->nPages) {  // for eps nPages is 0 so it is fine ....
    fprintf(ps->output, "CR\nGR\nGR\nGR\nSP\n restore\n");
    if (!ps->pages_){
      fprintf(ps->output, "%%%%Trailer\n");
      fprintf(ps->output, "%%%%Pages: %i\n" , ps->nPages);
    };
  } else
    fprintf(ps->output, "GR\n restore\n");
  fputs("%%EOF",ps->output);
  fflush(ps->output);
  error = ferror(ps->output);
  ps->reset();
#endif
  while (ps->clip_){
    Fl_PostScript_Graphics_Driver::Clip * c= ps->clip_;
    ps->clip_= ps->clip_->prev;
    delete c;
  }
  int err2 = (ps->close_cmd_ ? (ps->close_cmd_)(ps->output) : fclose(ps->output) );
  if (!error) error = err2;
  if (error && ps->close_cmd_ == NULL) {
    fl_alert ("Error during PostScript data output.");
    }
}

void Fl_PostScript_File_Device::close_command(Fl_PostScript_Close_Command cmd) {
  driver()->close_command(cmd);
}

Fl_EPS_File_Surface::Fl_EPS_File_Surface(int width, int height, FILE *eps, Fl_Color background, Fl_PostScript_Close_Command closef) :
        Fl_Widget_Surface(new Fl_PostScript_Graphics_Driver()) {
  Fl_PostScript_Graphics_Driver *ps = driver();
  ps->output = eps;
  ps->close_cmd_ = closef;
  if (ps->output) {
    float s = Fl::screen_scale(0);
    ps->start_eps(int(width*s), int(height*s));
#if USE_PANGO
      cairo_save(ps->cr());
      ps->left_margin = ps->top_margin = 0;
      cairo_scale(ps->cr(), s, s);
      cairo_set_line_width(ps->cr(), 1);
      cairo_set_source_rgb(ps->cr(), 1.0, 1.0, 1.0); // white background
      cairo_save(ps->cr());
      cairo_save(ps->cr());
      ps->check_status();
#else
    if (s != 1) {
      ps->clocale_printf("GR GR GS %f %f SC GS\n", s, s);
    }
    Fl::get_color(background, ps->bg_r, ps->bg_g, ps->bg_b);
#endif
    ps->scale_x = ps->scale_y = s;
  }
}

int Fl_EPS_File_Surface::close() {
  int error = 0;
  Fl_PostScript_Graphics_Driver *ps = driver();
#if USE_PANGO
  cairo_surface_t *s = cairo_get_target(ps->cr());
  cairo_surface_finish(s);
  cairo_status_t status = cairo_surface_status(s);
  cairo_destroy(ps->cr());
  fflush(ps->output);
  error = ferror(ps->output);
  if (status !=  CAIRO_STATUS_SUCCESS) error = status;
#else
  if(ps->output) {
    fputs("GR\nend %matches begin of FLTK dict\n", ps->output);
    fputs("restore\n", ps->output);
    fputs("%%EOF\n", ps->output);
    ps->reset();
    fflush(ps->output);
    error = ferror(ps->output);
  }
#endif
  int err2 = (ps->close_cmd_ ? (ps->close_cmd_)(ps->output) : fclose(ps->output));
  if (err2) error = err2;
  while (ps->clip_){
    Fl_PostScript_Graphics_Driver::Clip * c= ps->clip_;
    ps->clip_= ps->clip_->prev;
    delete c;
  }
  ps->output = NULL;
  return error;
}

Fl_EPS_File_Surface::~Fl_EPS_File_Surface() {
  if (driver()->output) {
    if ( close() ) {
      fl_open_display();
      fl_alert ("Error during encapsulated PostScript data output.");
    }
  }
  delete driver();
}

FILE *Fl_EPS_File_Surface::file() {
  Fl_PostScript_Graphics_Driver *ps = driver();
  return ps ? ps->output : NULL;
}

int Fl_EPS_File_Surface::printable_rect(int *w, int *h) {
  Fl_PostScript_Graphics_Driver *ps = driver();
  *w = int(ps->pw_);
  *h = int(ps->ph_);
  return 0;
}

void Fl_EPS_File_Surface::origin(int x, int y)
{
  x_offset = x;
  y_offset = y;
  driver()->ps_origin(x, y);
}

void Fl_EPS_File_Surface::origin(int *px, int *py) {
  Fl_Widget_Surface::origin(px, py);
}

void Fl_EPS_File_Surface::translate(int x, int y)
{
  driver()->ps_translate(x, y);
}

void Fl_EPS_File_Surface::untranslate()
{
  driver()->ps_untranslate();
}

#endif // !defined(FL_NO_PRINT_SUPPORT)
