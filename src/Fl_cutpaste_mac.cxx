//
// "$Id: Fl_cutpaste_mac.cxx,v 1.1.2.3 2002/01/01 15:11:31 easysw Exp $"
//
// MacOS cut/paste code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2002 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

// Implementation of cut and paste for MacOS.

#include <FL/Fl.H>
#include <FL/mac.H>
#include <FL/Fl_Window.H>
#include <string.h>

static char *selection_buffer = 0L;
static int selection_length = 0;
static int selection_buffer_length = 0;

static ScrapRef myScrap = 0;

extern Fl_Widget *fl_selection_requestor; // widget doing request_paste()

/**
 * call this to retrieve the current slection
 * receiver: widget receiving the selection
 * Fl::e_text: pointer to selection
 * Fl::e_length: size of election
 */
void Fl::paste( Fl_Widget &receiver ) 
{
  ScrapRef scrap = 0;
  OSStatus ret = GetCurrentScrap( &scrap );
  if ( ( scrap != myScrap ) && ( ret == noErr ) )
  {
    Size len;
    ret = GetScrapFlavorSize( scrap, kScrapFlavorTypeText, &len );
    if ( ret != noErr ) return;
    if ( len > selection_buffer_length )
    {
      selection_buffer_length = len + 32;
      delete[] selection_buffer;
      selection_buffer = new char[len];
      selection_buffer_length = len;
    }
    GetScrapFlavorData( scrap, kScrapFlavorTypeText, &len, selection_buffer );
    selection_length = len;
  }
  for ( char *s = selection_buffer+selection_length; s >= selection_buffer; s-- ) // this will fail on PC line endings (CR+LF)
    if ( *s == 0x0d ) *s = 0x0a;
  Fl::e_text = selection_buffer;
  Fl::e_length = selection_length;
  receiver.handle( FL_PASTE );
  return;
}


/**
 * create a selection
 * owner: widget that created the selection
 * stuff: pointer to selected data
 * size of selected data
 */
void Fl::selection( Fl_Widget &owner, const char *stuff, int len ) {
  if ( !stuff || len<0 )
    return;
  if ( len+1 > selection_buffer_length ) {
    delete[] selection_buffer;
    selection_buffer = new char[len+100];
    selection_buffer_length = len+100;
  }
  memcpy( selection_buffer, stuff, len );
  for ( char *s = selection_buffer+len; s >= selection_buffer; s-- ) // this will fail on PC line endings (CR+LF)
    if ( *s == 0x0a ) *s = 0x0d;
  selection_buffer[len] = 0;
  selection_length = len;
  selection_owner( &owner );

  ClearCurrentScrap();
  OSStatus ret = GetCurrentScrap( &myScrap );
  if ( ret != noErr ) 
  {
    myScrap = 0;
    return;
  }
  PutScrapFlavor( myScrap, kScrapFlavorTypeText, 0, selection_length, selection_buffer );
}


//
// End of "$Id: Fl_cutpaste_mac.cxx,v 1.1.2.3 2002/01/01 15:11:31 easysw Exp $".
//
