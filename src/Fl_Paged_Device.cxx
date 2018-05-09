//
// "$Id$"
//
// implementation of Fl_Paged_Device class for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010-2016 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems to:
//
//     http://www.fltk.org/str.php
//
/** \file Fl_Paged_Device.cxx
 \brief implementation of class Fl_Paged_Device.
 */

#include <FL/Fl_Paged_Device.H>
#include <FL/Fl.H>
#include <FL/fl_draw.H>



/**
 @brief Begins a print job.

 @param[in] pagecount the total number of pages of the job (or 0 if you don't know the number of pages)
 @param[out] frompage if non-null, *frompage is set to the first page the user wants printed
 @param[out] topage if non-null, *topage is set to the last page the user wants printed
 @return 0 if OK, non-zero if any error
 */
int Fl_Paged_Device::begin_job(int pagecount, int *frompage, int *topage) {return 1;}

/**
 @brief Begins a new printed page

 The page coordinates are initially in points, i.e., 1/72 inch, 
 and with origin at the top left of the printable page area.
 @return 0 if OK, non-zero if any error
 */
int Fl_Paged_Device::begin_page (void) {return 1;}

/**
 @brief Computes the dimensions of margins that lie between the printable page area and
 the full page.

 Values are in the same unit as that used by FLTK drawing functions. They are changed
 by scale() calls.
 @param[out] left If non-null, *left is set to the left margin size.
 @param[out] top If non-null, *top is set to the top margin size.
 @param[out] right If non-null, *right is set to the right margin size.
 @param[out] bottom If non-null, *bottom is set to the bottom margin size.
 */
void Fl_Paged_Device::margins(int *left, int *top, int *right, int *bottom) {}


/**
 @brief Changes the scaling of page coordinates.

 This function also resets the origin of graphics functions at top left of printable page area.
 After a scale() call, do a printable_rect() call to get the new dimensions of the printable page area.
 Successive scale() calls don't combine their effects.
 @param scale_x Horizontal dimensions of plot are multiplied by this quantity.
 @param scale_y Same as above, vertically. 
  The value 0. is equivalent to setting \p scale_y = \p scale_x. Thus, scale(factor);
  is equivalent to scale(factor, factor);
 */
void Fl_Paged_Device::scale (float scale_x, float scale_y) {}

/**
 @brief Rotates the graphics operations relatively to paper.

 The rotation is centered on the current graphics origin. 
 Successive rotate() calls don't combine their effects.
 @param angle Rotation angle in counter-clockwise degrees.
 */
void Fl_Paged_Device::rotate(float angle) {}

/**
 @brief To be called at the end of each page.

 @return 0 if OK, non-zero if any error.
 */
int Fl_Paged_Device::end_page (void) {return 1;}

/**
 @brief To be called at the end of a print job.
 */
void Fl_Paged_Device::end_job (void) {}


const Fl_Paged_Device::page_format Fl_Paged_Device::page_formats[NO_PAGE_FORMATS] = { 
  // order of enum Page_Format
  // comes from appendix B of 5003.PPD_Spec_v4.3.pdf
  
  // A* // index(Ai) = i
  {2384, 3370, "A0"},
  {1684, 2384, "A1"},
  {1191, 1684, "A2"},
  { 842, 1191, "A3"},
  { 595,  842, "A4"},
  { 420,  595, "A5"},
  { 297,  420, "A6"},
  { 210,  297, "A7"},
  { 148,  210, "A8"},
  { 105,  148, "A9"},
  
  // B* // index(Bi) = i+10
  {2920, 4127, "B0"},
  {2064, 2920, "B1"},
  {1460, 2064, "B2"},
  {1032, 1460, "B3"},
  { 729, 1032, "B4"},
  { 516,  729, "B5"},
  { 363,  516, "B6"},
  { 258,  363, "B7"},
  { 181,  258, "B8"},
  { 127,  181, "B9"},
  {  91,  127, "B10"},
  
  // others
  { 459,  649, "EnvC5"}, // envelope
  { 312,  624, "EnvDL"}, // envelope
  { 522,  756, "Executive"},
  { 595,  935, "Folio"},
  {1224,  792, "Ledger"}, // landscape
  { 612, 1008, "Legal"},
  { 612,  792, "Letter"},
  { 792, 1224, "Tabloid"},
  { 297,  684, "Env10"} // envelope
};

//
// End of "$Id$".
//
