// Fl_Chart.C

// Emulation of the Forms Chart widget.
// I did not try to improve this much, as I doubt it is used.

// display code Written by: Mark Overmars

#include <FL/math.h>
#include <FL/Fl.H>
#include <FL/Fl_Chart.H>
#include <FL/fl_draw.H>
#include <string.h>

#define ARCINC	(2.0*M_PI/360.0)

// this function is in fl_boxtype.C:
void fl_rectbound(int x,int y,int w,int h, Fl_Color color);

/* Widget specific information */

static void draw_barchart(int x,int y,int w,int h,
			  int numb, FL_CHART_ENTRY entries[],
			  double min, double max, int autosize, int maxnumb,
			  Fl_Color textcolor)
/* Draws a bar chart. x,y,w,h is the bounding box, entries the array of
   numb entries and min and max the boundaries. */
{
  double incr = h/(max-min);
  int zeroh;
  double lh = fl_height();
  if ( -min*incr < lh) {
      incr = (h - lh + min*incr)/(max-min);
      zeroh = int(y+h-lh);
  } else {
      zeroh = int(y+h+min * incr + .5);
  }
  int bwidth = int(w/double(autosize?numb:maxnumb)+.5);
  /* Draw base line */
  fl_color(textcolor);
  fl_line(x, zeroh, x+w, zeroh);
  if (min == 0.0 && max == 0.0) return; /* Nothing else to draw */
  int i;
  /* Draw the bars */
  for (i=0; i<numb; i++) {
      int h = int(entries[i].val*incr+.5);
      if (h < 0)
	fl_rectbound(x+i*bwidth,zeroh,bwidth+1,-h+1, (Fl_Color)entries[i].col);
      else if (h > 0)
	fl_rectbound(x+i*bwidth,zeroh-h,bwidth+1,h+1,(Fl_Color)entries[i].col);
  }
  /* Draw the labels */
  fl_color(textcolor);
  for (i=0; i<numb; i++)
      fl_draw(entries[i].str,
	      x+i*bwidth+bwidth/2,zeroh,0,0,
	      FL_ALIGN_TOP);
}

static void draw_horbarchart(int x,int y,int w,int h,
			     int numb, FL_CHART_ENTRY entries[],
			     double min, double max, int autosize, int maxnumb,
			     Fl_Color textcolor)
/* Draws a horizontal bar chart. x,y,w,h is the bounding box, entries the
   array of numb entries and min and max the boundaries. */
{
  int i;
  double lw = 0.0;		/* Maximal label width */
  /* Compute maximal label width */
  for (i=0; i<numb; i++) {
      double w1 = fl_width(entries[i].str);
      if (w1 > lw) lw = w1;
  }
  if (lw > 0.0) lw += 4.0;
  double incr = w/(max-min);
  int zeroh;
  if ( -min*incr < lw) {
      incr = (w - lw + min*incr)/(max-min);
      zeroh = x+int(lw+.5);
  } else {
      zeroh = int(x-min * incr + .5);
  }
  int bwidth = int(h/double(autosize?numb:maxnumb)+.5);
  /* Draw base line */
  fl_color(textcolor);
  fl_line(zeroh, y, zeroh, y+h);
  if (min == 0.0 && max == 0.0) return; /* Nothing else to draw */
  /* Draw the bars */
  for (i=0; i<numb; i++) {
      int w = int(entries[i].val*incr+.5);
      if (w > 0)
	fl_rectbound(zeroh,y+i*bwidth,w+1,bwidth+1, (Fl_Color)entries[i].col);
      else if (w < 0)
	fl_rectbound(zeroh+w,y+i*bwidth,-w+1,bwidth+1,(Fl_Color)entries[i].col);
  }
  /* Draw the labels */
  for (i=0; i<numb; i++)
      fl_draw(entries[i].str,
	      zeroh-2,y+i*bwidth+bwidth/2,0,0,
	      FL_ALIGN_RIGHT);
}

static void draw_linechart(int type, int x,int y,int w,int h,
			   int numb, FL_CHART_ENTRY entries[],
			   double min, double max, int autosize, int maxnumb,
			   Fl_Color textcolor)
/* Draws a line chart. x,y,w,h is the bounding box, entries the array of
   numb entries and min and max the boundaries. */
{
  int i;
  double lh = fl_height();
  double incr = (h-2.0*lh)/ (max-min);
  int zeroh = int(y+h-lh+min * incr + .5);
  double bwidth = w/double(autosize?numb:maxnumb);
  /* Draw the values */
  for (i=0; i<numb; i++) {
      int x0 = x + int((i-.5)*bwidth+.5);
      int x1 = x + int((i+.5)*bwidth+.5);
      int y0 = i ? zeroh - int(entries[i-1].val*incr+.5) : 0;
      int y1 = zeroh - int(entries[i].val*incr+.5);
      if (type == FL_SPIKE_CHART) {
	  fl_color((Fl_Color)entries[i].col);
	  fl_line(x1, zeroh, x1, y1);
      } else if (type == FL_LINE_CHART && i != 0) {
	  fl_color((Fl_Color)entries[i-1].col);
	  fl_line(x0,y0,x1,y1);
      } else if (type == FL_FILLED_CHART && i != 0) {
	  fl_color((Fl_Color)entries[i-1].col);
	  if ((entries[i-1].val>0.0)!=(entries[i].val>0.0)) {
	      double ttt = entries[i-1].val/(entries[i-1].val-entries[i].val);
	      int xt = x + int((i-.5+ttt)*bwidth+.5);
	      fl_polygon(x0,zeroh, x0,y0, xt,zeroh);
	      fl_polygon(xt,zeroh, x1,y1, x1,zeroh);
	  } else {
	      fl_polygon(x0,zeroh, x0,y0, x1,y1, x1,zeroh);
	  }
	  fl_color(textcolor);
	  fl_line(x0,y0,x1,y1);
      }
  }
  /* Draw base line */
  fl_color(textcolor);
  fl_line(x,zeroh,x+w,zeroh);
  /* Draw the labels */
  for (i=0; i<numb; i++)
      fl_draw(entries[i].str,
	      x+int((i+.5)*bwidth+.5), zeroh - int(entries[i].val*incr+.5),0,0,
	      entries[i].val>=0 ? FL_ALIGN_BOTTOM : FL_ALIGN_TOP);
}

static void draw_piechart(int x,int y,int w,int h,
			  int numb, FL_CHART_ENTRY entries[], int special,
			  Fl_Color textcolor)
/* Draws a pie chart. x,y,w,h is the bounding box, entries the array of
   numb entries */
{
  int i;
  double xc,yc,rad;	/* center and radius */
  double tot;		/* sum of values */
  double incr;		/* increment in angle */
  double curang;		/* current angle we are drawing */
  double txc,tyc;	/* temporary center */
  double lh = fl_height();
  /* compute center and radius */
  xc = x+w/2.0; yc = y+h/2.0;
  rad = h/2.0 - lh;
  if (special) { yc += 0.1*rad; rad = 0.9*rad;}
  /* compute sum of values */
  tot = 0.0;
  for (i=0; i<numb; i++)
    if (entries[i].val > 0.0) tot += entries[i].val;
  if (tot == 0.0) return;
  incr = 360.0/tot;
  /* Draw the pie */
  curang = 0.0;
  for (i=0; i<numb; i++)
    if (entries[i].val > 0.0)
    {
      txc = xc; tyc = yc;
      /* Correct for special pies */
      if (special && i==0)
      {
        txc += 0.3*rad*cos(ARCINC*(curang+0.5*incr*entries[i].val));
        tyc -= 0.3*rad*sin(ARCINC*(curang+0.5*incr*entries[i].val));
      }
      fl_color((Fl_Color)entries[i].col);
      fl_begin_polygon(); fl_vertex(txc,tyc);
      fl_arc(txc,tyc,rad,curang, curang+incr*entries[i].val);
      fl_end_polygon();
      fl_color(textcolor);
      fl_begin_loop(); fl_vertex(txc,tyc);
      fl_arc(txc,tyc,rad,curang, curang+incr*entries[i].val);
      fl_end_loop();
      curang += 0.5 * incr * entries[i].val;
      /* draw the label */
      double xl = txc + 1.1*rad*cos(ARCINC*curang);
      fl_draw(entries[i].str,
	      int(xl+.5),
	      int(tyc - 1.1*rad*sin(ARCINC*curang)+.5),
	      0, 0,
	      xl<txc ? FL_ALIGN_RIGHT : FL_ALIGN_LEFT);
      curang += 0.5 * incr * entries[i].val;
    }
}

void Fl_Chart::draw() {
    int xx,yy,ww,hh;
    int i;

    xx = x()+9;
    yy = y()+9;
    ww = w()-2*9;
    hh = h()-2*9;

    if (min >= max) {
	min = max = 0.0;
	for (i=0; i<numb; i++) {
	    if (entries[i].val < min) min = entries[i].val;
	    if (entries[i].val > max) max = entries[i].val;
	}
    }

    draw_box();
    fl_font(textfont(),textsize());

    switch (type()) {
    case FL_BAR_CHART:
	draw_barchart(xx,yy,ww,hh, numb, entries, min, max,
			autosize(), maxnumb, textcolor());
	break;
    case FL_HORBAR_CHART:
	draw_horbarchart(xx,yy,ww,hh, numb, entries, min, max,
			autosize(), maxnumb, textcolor());
	break;
    case FL_PIE_CHART:
	draw_piechart(xx,yy,ww,hh,numb,entries,0, textcolor());
	break;
    case FL_SPECIALPIE_CHART:
	draw_piechart(xx,yy,ww,hh,numb,entries,1,textcolor());
	break;
    default:
	draw_linechart(type(),xx,yy,ww,hh, numb, entries, min, max,
			autosize(), maxnumb, textcolor());
	break;
    }
    draw_label();
}

/*------------------------------*/

#define FL_CHART_BOXTYPE	FL_BORDER_BOX
#define FL_CHART_COL1		FL_COL1
#define FL_CHART_LCOL		FL_LCOL
#define FL_CHART_ALIGN		FL_ALIGN_BOTTOM

Fl_Chart::Fl_Chart(int x,int y,int w,int h,const char *l) :
Fl_Widget(x,y,w,h,l) {
    box(FL_BORDER_BOX);
    align(FL_ALIGN_BOTTOM);
    numb = 0;
    maxnumb = FL_CHART_MAX;
    autosize_ = 1;
    min = max = 0;
    textfont_ = FL_HELVETICA;
    textsize_ = 10;
    textcolor_ = FL_BLACK;
}

void Fl_Chart::clear() {
    numb = 0;
    redraw();
}

void Fl_Chart::add(double val, const char *str, uchar col) {
  int i;
  /* Shift entries if required */
  if (numb >= maxnumb) {
    for (i=0; i<numb-1; i++) entries[i] = entries[i+1];
    numb--;
  }
  entries[numb].val = float(val);
  entries[numb].col = col;
    if (str) {
	strncpy(entries[numb].str,str,FL_CHART_LABEL_MAX+1);
	entries[numb].str[FL_CHART_LABEL_MAX] = 0;
    } else {
	entries[numb].str[0] = 0;
    }
  numb++;
  redraw();
}

void Fl_Chart::insert(int index, double val, const char *str, uchar col) {
    int i;
    if (index < 1 || index > numb+1) return;
    /* Shift entries */
    for (i=numb; i >= index; i--) entries[i] = entries[i-1];
    if (numb < maxnumb) numb++;
    /* Fill in the new entry */
    entries[index-1].val = float(val);
    entries[index-1].col = col;
    if (str) {
	strncpy(entries[index-1].str,str,FL_CHART_LABEL_MAX+1);
	entries[index-1].str[FL_CHART_LABEL_MAX] = 0;
    } else {
	entries[index-1].str[0] = 0;
    }
    redraw();
}

void Fl_Chart::replace(int index,double val, const char *str, uchar col) {
    if (index < 1 || index > numb) return;
    entries[index-1].val = float(val);
    entries[index-1].col = col;
    if (str) {
	strncpy(entries[index-1].str,str,FL_CHART_LABEL_MAX+1);
	entries[index-1].str[FL_CHART_LABEL_MAX] = 0;
    } else {
	entries[index-1].str[0] = 0;
    }
    redraw();
}

void Fl_Chart::bounds(double min, double max) {
    this->min = min;
    this->max = max;
    redraw();
}

void Fl_Chart::maxsize(int m) {
    int i;
    /* Fill in the new number */
    if (m < 0) return;
    if (m > FL_CHART_MAX)
	maxnumb = FL_CHART_MAX;
    else
	maxnumb = m;
    /* Shift entries if required */
    if (numb > maxnumb) {
	for (i = 0; i<maxnumb; i++)
	    entries[i] = entries[i+numb-maxnumb];
	numb = maxnumb;
	redraw();
    }
}
