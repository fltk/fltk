//
// "$Id:$"
//
// Flmm_Scalebar source file for the FLMM extension to FLTK.
//
// Copyright 2002-2004 by Matthias Melcher.
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
// Please report all bugs and problems to "flmm@matthiasm.com".
//

/** \class Flmm_Scalebar
 * A Scrollbar that can also scale the content window.
 *
 * This slider has two additional boxes which can be dragged to change
 * the amount of data shown from within a document. This widget is
 * great for vector data like time lines and curve editors.
 */

#include <FL/Fl.H>
#include <FL/Fl_Range_Slider.H>
#include <FL/fl_draw.H>
#include <math.h>

#define INITIALREPEAT .5
#define REPEAT .05

Fl_Range_Slider::Fl_Range_Slider( int x, int y, int w, int h, const char *l )
: Fl_Scrollbar( x, y, w, h, l ) {
  pushed_ = 0;
  slider_size_i_ = 1;
  min_sash_size_ = 0;
}

int Fl_Range_Slider::value() {
  return int(Fl_Slider::value());
}

int Fl_Range_Slider::value(int position, int size, int top, int total) {
  return scrollvalue(position, size, top, total);
}

int Fl_Range_Slider::slider_size_i() { 
  return slider_size_i_;
}

void Fl_Range_Slider::drawSliderBg(int x, int y, int w, int h) {
  if (!(damage()&FL_DAMAGE_ALL)) { // not a complete redraw
    draw_box();
  }
  Fl_Color black = active_r() ? FL_BLACK : FL_INACTIVE_COLOR;
  if (type() == FL_VERT_NICE_SLIDER) {
    draw_box(FL_THIN_DOWN_BOX, x+w/2-2, y, 4, h, black);
  } else if (type() == FL_HOR_NICE_SLIDER) {
    draw_box(FL_THIN_DOWN_BOX, x, y+h/2-2, w, 4, black);
  }
}

int Fl_Range_Slider::scrollvalue(int p, int n, int t, int l) {
  //	p = position, first line displayed
  //	n = window, number of lines displayed
  //	t = top, number of first line
  //	l = length, total number of lines
  step(1, 1);
  int minSash;
  if (horizontal()) {
    if ( w()<3*h() ) {
      minSash = 0;
    } else {
      minSash = 3 * h(); // minimum sash size in pixel
      int div = w()-2*h()-minSash;
      if (div) minSash = l*minSash/div; // convert to size in 'lines'
    }
  } else {
    if ( h()<3*w() ) {
      minSash = 0;
    } else {
      minSash = 3 * w(); // minimum sash size in pixel
      int div = h()-2*w()-minSash;
      if (div) minSash = l*minSash/div;
    }
  }
  min_sash_size_ = minSash;
  slider_size_i_ = n;
  n += minSash;
  l += minSash;
  if (p+n > t+l) l = p+n-t;
  slider_size(n >= l ? 1.0 : double(n)/double(l));
  bounds(t, l-n+t);
  return Fl_Valuator::value(p);
}

void Fl_Range_Slider::drawSlider(int x, int y, int w, int h) {  
  double val;
  if (minimum() == maximum())
    val = 0.5;
  else {
    val = (value()-minimum())/(maximum()-minimum());
    if (val > 1.0) val = 1.0;
    else if (val < 0.0) val = 0.0;
  }
  
  int W = (horizontal() ? w : h);
  int X, S;
  if (type()==FL_HOR_FILL_SLIDER || type() == FL_VERT_FILL_SLIDER) {
    S = int(val*W+.5);
    if (minimum()>maximum()) {S = W-S; X = W-S;}
    else X = 0;
  } else {
    S = int(slider_size()*W+.5);
    int T = (horizontal() ? h : w)/2+1;
    if (type()==FL_VERT_NICE_SLIDER || type()==FL_HOR_NICE_SLIDER) T += 4;
    if (S < T) S = T;
    X = int(val*(W-S)+.5);
  }
  int xsl, ysl, wsl, hsl;
  if (horizontal()) {
    xsl = x+X;
    wsl = S;
    ysl = y;
    hsl = h;
  } else {
    ysl = y+X;
    hsl = S;
    xsl = x;
    wsl = w;
  }
  
  if (damage()&FL_DAMAGE_ALL) { // complete redraw
    drawSliderBg(x, y, w, h);
  } else { // partial redraw, clip off new position of slider
    if (X > 0) {
      if (horizontal()) fl_clip(x, ysl, X, hsl);
      else fl_clip(xsl, y, wsl, X);
      drawSliderBg(x, y, w, h);
      fl_pop_clip();
    }
    if (X+S < W) {
      if (horizontal()) fl_clip(xsl+wsl, ysl, x+w-xsl-wsl, hsl);
      else fl_clip(xsl, ysl+hsl, wsl, y+h-ysl-hsl);
      drawSliderBg(x, y, w, h);
      fl_pop_clip();
    }
  }
  
  Fl_Boxtype box1 = slider();
  if (!box1) {box1 = (Fl_Boxtype)(box()&-2); if (!box1) box1 = FL_UP_BOX;}
  if (type() == FL_VERT_NICE_SLIDER) {
    draw_box(box1, xsl, ysl, wsl, hsl, FL_GRAY);
    int d = (hsl-4)/2;
    draw_box(FL_THIN_DOWN_BOX, xsl+2, ysl+d, wsl-4, hsl-2*d,selection_color());
  } else if (type() == FL_HOR_NICE_SLIDER) {
    draw_box(box1, xsl, ysl, wsl, hsl, FL_GRAY);
    int d = (wsl-4)/2;
    draw_box(FL_THIN_DOWN_BOX, xsl+d, ysl+2, wsl-2*d, hsl-4,selection_color());
  } else {
    if (wsl>0 && hsl>0) draw_box(box1, xsl, ysl, wsl, hsl, selection_color());
    if ( horizontal() ) {
      if ( hsl>6 && wsl>=hsl*2.5f ) {
        draw_box( pushed_==4 ? fl_down(box1) : box1, xsl+2, ysl+2, hsl-4, hsl-4, selection_color() );
        draw_box( pushed_==5 ? fl_down(box1) : box1, xsl+wsl-hsl+2, ysl+2, hsl-4, hsl-4, selection_color() );
      }
    } else {
      if ( wsl>6 && hsl>=wsl*2.5f ) {
        draw_box( pushed_==4 ? fl_down(box1) : box1, xsl+2, ysl+2, wsl-4, wsl-4, selection_color() );
        draw_box( pushed_==5 ? fl_down(box1) : box1, xsl+2, ysl+hsl-wsl+2, wsl-4, wsl-4, selection_color() );
      }
    }
  }
  draw_label(xsl, ysl, wsl, hsl);
}

void Fl_Range_Slider::draw(){
  if (damage()&FL_DAMAGE_ALL) draw_box();
  int X = x()+Fl::box_dx(box());
  int Y = y()+Fl::box_dy(box());
  int W = w()-Fl::box_dw(box());
  int H = h()-Fl::box_dh(box());
  if (horizontal()) {
    if (W < 3*H) {Fl_Slider::draw(X,Y,W,H); return;}
    drawSlider(X+H,Y,W-2*H,H);
    if (damage()&FL_DAMAGE_ALL) {
      // left arrow box
      draw_box((pushed_==1) ? fl_down(slider()) : slider(),
               X, Y, H, H, selection_color());
      // right arrow box
      draw_box((pushed_==2) ? fl_down(slider()) : slider(),
               X+W-H, Y, H, H, selection_color());
      // left and right arrows
      if (active_r())
        fl_color(labelcolor());
      else
        fl_color(fl_inactive(labelcolor()));
      int d1 = (H-4)/3; if (d1 < 1) d1 = 1;
      int x1 = X+(H-d1-1)/2;
      int y1 = Y+(H-2*d1-1)/2;
      fl_polygon(x1, y1+d1, x1+d1, y1+2*d1, x1+d1, y1);
      x1 += (W-H);
      fl_polygon(x1, y1, x1, y1+2*d1, x1+d1, y1+d1);
    }
  } else { // vertical
    if (H < 3*W) {Fl_Slider::draw(X,Y,W,H); return;}
    drawSlider(X,Y+W,W,H-2*W);
    if (damage()&FL_DAMAGE_ALL) {
      draw_box((pushed_==1) ? fl_down(slider()) : slider(),
               X, Y, W, W, selection_color());
      draw_box((pushed_==2) ? fl_down(slider()) : slider(),
               X, Y+H-W, W, W, selection_color());
      if (active_r())
        fl_color(labelcolor());
      else
        fl_color(fl_inactive(labelcolor()));
      int d1 = (W-4)/3; if (d1 < 1) d1 = 1;
      int x1 = X+(W-2*d1-1)/2;
      int y1 = Y+(W-d1-1)/2;
      fl_polygon(x1, y1+d1, x1+2*d1, y1+d1, x1+d1, y1);
      y1 += H-W;
      fl_polygon(x1, y1, x1+d1, y1+d1, x1+2*d1, y1);
    }
  }
}

int Fl_Range_Slider::handle(int event){
  static int evDown, evVal, evSize;
  // we have to do a lot of stuff that will be done again in the Scrollbar handle
  if ( event==FL_PUSH ) {
    int X=x(); int Y=y(); int W=w(); int H=h();
    int sashWdt, sashPos, mouseDown, scaleWdt, wdt;
    if ( horizontal() ) {
      // check if we are in the arrow areas and set our marker for drawing if so
      evDown = Fl::event_x();
      if (W >= 3*H) {
        X += H; W -= 2*H;
        if ( evDown<X )   { pushed_ = 1; goto handleEv; }
        if ( evDown>X+W ) { pushed_ = 2; goto handleEv; }
      }
      // check if the user clicked into the sash scaling area
      mouseDown = evDown-X;
      scaleWdt = H; wdt = W;
    } else {
      evDown = Fl::event_y();
      if (H >= 3*W) {
        Y += W; H -= 2*W;
        if ( evDown<Y )   { pushed_ = 1; goto handleEv; }
        if ( evDown>Y+H ) { pushed_ = 2; goto handleEv; }
      }
      mouseDown = evDown-Y;
      scaleWdt = W; wdt = H;
    }
    sashWdt = (int)( slider_size()*wdt+0.5f );
    double val = (maximum()-minimum()) ? (value()-minimum())/(maximum()-minimum()) : 0.5;
    if ( val > 1.0 ) sashPos = wdt-sashWdt;
    else if ( val < 0.0 ) sashPos = 0;
    else sashPos = int(val*(wdt-sashWdt)+.5);
    if ( sashWdt<2.5f*scaleWdt) { // scalers are not visible, so don't use them!
      pushed_ = 3;
    } else if ( mouseDown>=sashPos && mouseDown<sashPos+scaleWdt-2 ) {
      pushed_ = 4; evVal = value(); evSize = slider_size_i_;
    } else if ( mouseDown>=sashPos+sashWdt-scaleWdt+2 && mouseDown<sashPos+sashWdt ) {
      pushed_ = 5; evVal = value(); evSize = slider_size_i_;
    } else {
      pushed_ = 3;
    }
  }
  
handleEv:
  if ( pushed_>3 ) {
    switch ( event ) {
      case FL_PUSH:
      case FL_DRAG: {
        int evDelta;
        int max = (int)maximum(), min = (int)minimum();
        int v = value(), s = slider_size_i_;
        if ( horizontal() ) {
          evDelta = evDown-Fl::event_x();
          int nLines = (int)(maximum() - minimum() + slider_size_i_ + min_sash_size_);
          int wdt = w()-2*h();
          evDelta = evDelta*nLines/wdt;
        } else {
          evDelta = evDown-Fl::event_y();
          int nLines = (int)(maximum() - minimum() + slider_size_i_ + min_sash_size_);
          int wdt = h()-2*w();
          evDelta = evDelta*nLines/wdt;
        }
        if ( pushed_ == 4 ) { // left or upper sash box
          int maxx = max-min+slider_size_i_;
          if ( evVal-evDelta < min ) evDelta = evVal-min;
          if ( evDelta < -evSize ) evDelta = -evSize;
          scrollvalue( evVal-evDelta, evSize+evDelta, min, maxx );
        } else if ( pushed_ == 5 ) { // right or lower sash box
          int maxx = max-min+slider_size_i_;
          if ( evDelta > evSize ) evDelta = evSize;
          if ( evVal+evSize-evDelta-min > maxx ) evDelta = evVal+evSize-min-maxx;
          scrollvalue( evVal, evSize-evDelta, min, maxx );
        }
        damage(FL_DAMAGE_ALL);
        if ( v != value() || s != slider_size_i_ ) {
          do_callback();
        }
        return 1;
      }
      case FL_RELEASE:
        damage(FL_DAMAGE_ALL);
        pushed_ = 0;
        return 1;
    }
  } else {
    if ( event == FL_RELEASE ) {
      pushed_ = 0;
      damage(FL_DAMAGE_ALL);
    }
  }
  return Fl_Scrollbar::handle( event );
}

//
// End of "$Id:$".
//
