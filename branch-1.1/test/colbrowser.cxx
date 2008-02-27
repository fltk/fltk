//
// "$Id$"
//
// Forms test program for the Fast Light Tool Kit (FLTK).
//
// This is an XForms program from the 0.86 distribution of XForms.
// It has been modified as little as possible to work under fltk by
// using fltk's Forms emulation.  Search for "fltk" to find all the
// changes
//
// Copyright 1998-2005 by Bill Spitzak and others.
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
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

#include <FL/forms.H>	// changed for fltk
#include <stdlib.h>
#include <stdio.h>	// added for fltk
#include <string.h>	// added for fltk

#define MAX_RGB 3000

static FL_FORM *cl;
static Fl_Widget *rescol, *dbobj, *colbr, *rs, *gs, *bs;
char dbname[FL_PATH_MAX];
static void create_form_cl(void);
static int load_browser(char *);

typedef struct { int r, g, b; } RGBdb;

static RGBdb rgbdb[MAX_RGB];

int
main(int argc, char *argv[])
{

    fl_initialize(&argc, argv, "FormDemo", 0, 0);

    create_form_cl();
    strcpy(dbname, "rgb.txt");

    if (load_browser(dbname))
	fl_set_object_label(dbobj, dbname);
    else
	fl_set_object_label(dbobj, "None");

//    fl_set_form_minsize(cl, cl->w , cl->h); // removed for fltk
//    fl_set_form_maxsize(cl, 2*cl->w , 2*cl->h); // removed for fltk
    cl->size_range(cl->w(),cl->h(),2*cl->w(),2*cl->h()); // added for fltk
    // border changed from FL_TRANSIENT for fltk:
    // This is so Esc & the close box will close the window.
    // (on transient windows attempting to close it just calls the callback)
    fl_show_form(cl, FL_PLACE_FREE, 1/*FL_TRANSIENT*/, "RGB Browser");


    while (fl_do_forms())
	;
    return 0;
}

static void
set_entry(int i)
{
    RGBdb *db = rgbdb + i;

    fl_freeze_form(cl);
// unclear why demo is doing this.  This messes up FL:
//    fl_mapcolor(FL_FREE_COL4+i, db->r, db->g, db->b);
    fl_mapcolor(FL_FREE_COL4, db->r, db->g, db->b);
    fl_set_slider_value(rs, db->r);
    fl_set_slider_value(gs, db->g);
    fl_set_slider_value(bs, db->b);
    fl_redraw_object(rescol);
    fl_unfreeze_form(cl);
}

static void
br_cb(Fl_Widget * ob, long)
{
    int r = fl_get_browser(ob);

    if (r <= 0)
	return;
    set_entry(r - 1);
}

static int
read_entry(FILE * fp, int *r, int *g, int *b, char *name)
{
    int  n;
    char buf[512], *p;

    if (!fgets(buf, sizeof(buf) - 1, fp))
      return 0;

    if(buf[0] == '!')
      fgets(buf,sizeof(buf)-1,fp);

    if(sscanf(buf, " %d %d %d %n", r, g, b, &n) < 3)
	return 0;

    p = buf + n;

    /* squeeze out all spaces */
    while (*p)
    {
	if (*p != ' ' && *p != '\n')
	    *name++ = *p;
	p++;
    }
    *name = 0;

    return (feof(fp) || ferror(fp)) ? 0 : 1;
}


static int
load_browser(char *fname)
{
    FILE *fp;
    RGBdb *db = rgbdb, *dbs = db + MAX_RGB;
    int r, g, b,  lr  = -1 , lg = -1, lb = -1;
    char name[256], buf[256];
#ifdef __EMX__
    if (!(fp = fopen(__XOS2RedirRoot(fname), "r")))
#else
    if (!(fp = fopen(fname, "r")))
#endif
    {
	fl_show_alert("Load", fname, "Can't open", 0);
	return 0;
    }

    /* read the items */

    fl_freeze_form(cl);

    for (; db < dbs && read_entry(fp, &r, &g, &b, name);)
    {
	db->r = r;
	db->g = g;
	db->b = b;

	/* unique the entries on the fly */
	if (lr != r || lg != g || lb != b)
	{
	    db++;
	    lr = r;
	    lg = g;
	    lb = b;
	    sprintf(buf, "(%3d %3d %3d) %s", r, g, b, name);
	    fl_addto_browser(colbr, buf);
	}
    }
    fclose(fp);

    if (db < dbs)
	db->r = 1000;		/* sentinel */
    else
    {
	db--;
	db->r = 1000;
    }

    fl_set_browser_topline(colbr, 1);
    fl_select_browser_line(colbr, 1);
    set_entry(0);
    fl_unfreeze_form(cl);
    return 1;
}

static int
search_entry(int r, int g, int b)
{
    register RGBdb *db = rgbdb;
    int i, j, diffr, diffg, diffb;
    unsigned int diff, mindiff;

    mindiff = (unsigned int)~0;
    for (i = j = 0; db->r < 256; db++, i++)
    {
       diffr = r - db->r;
       diffg = g - db->g;
       diffb = b - db->b;

#ifdef FL_LINEAR
	diff = unsigned(3.0 * (FL_abs(r - db->r)) +
			(5.9 * FL_abs(g - db->g)) +
			(1.1 * (FL_abs(b - db->b)));
#else
        diff = unsigned(3.0 * (diffr *diffr) +
			5.9 * (diffg *diffg) +
			1.1 * (diffb *diffb));
#endif

	if (mindiff > diff)
	{
	    mindiff = diff;
	    j = i;
	}
    }

    return j;
}

static void
search_rgb(Fl_Widget *, long)
{
    int r, g, b, i;
    int top  = fl_get_browser_topline(colbr);

    r = int(fl_get_slider_value(rs));
    g = int(fl_get_slider_value(gs));
    b = int(fl_get_slider_value(bs));

    fl_freeze_form(cl);
    fl_mapcolor(FL_FREE_COL4, r, g, b);
    fl_redraw_object(rescol);
    i = search_entry(r, g, b);
    /* change topline only if necessary */
    if(i < top || i > (top+15))
       fl_set_browser_topline(colbr, i-8);
    fl_select_browser_line(colbr, i + 1);
    fl_unfreeze_form(cl);
}

/* change database */
static void
db_cb(Fl_Widget * ob, long)
{
    const char *p = fl_show_input("Enter New Database Name", dbname);
    char buf[512];

    if (!p || strcmp(p, dbname) == 0)
	return;

    strcpy(buf, p);
    if (load_browser(buf))
	strcpy(dbname, buf);
    else
	fl_set_object_label(ob, dbname);
}

static void
done_cb(Fl_Widget *, long)
{
    exit(0);
}

static void
create_form_cl(void)
{
    Fl_Widget *obj;

    if (cl)
	return;

    cl = fl_bgn_form(FL_NO_BOX, 330, 385);
    obj = fl_add_box(FL_UP_BOX, 0, 0, 330, 385, "");
    fl_set_object_color(obj, FL_INDIANRED, FL_COL1);

    obj = fl_add_box(FL_NO_BOX, 40, 10, 250, 30, "Color Browser");
    fl_set_object_lcol(obj, FL_RED);
    fl_set_object_lsize(obj, FL_HUGE_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE + FL_SHADOW_STYLE);

    dbobj = obj = fl_add_button(FL_NORMAL_BUTTON, 40, 50, 250, 25, "");
    fl_set_object_boxtype(obj, FL_BORDER_BOX);
    fl_set_object_color(obj, /*fl_get_visual_depth()==1 ? FL_WHITE:*/ FL_INDIANRED,
                        FL_INDIANRED);
    fl_set_object_callback(obj, db_cb, 0);
    rs = obj = fl_add_valslider(FL_VERT_FILL_SLIDER, 225, 130, 30, 200, "");
    fl_set_object_color(obj, FL_INDIANRED, FL_RED);
    fl_set_slider_bounds(obj, 0, 255);
    fl_set_slider_precision(obj, 0);
    fl_set_object_callback(obj, search_rgb, 0);
    fl_set_slider_return(obj, 0);

    gs = obj = fl_add_valslider(FL_VERT_FILL_SLIDER, 255, 130, 30, 200, "");
    fl_set_object_color(obj, FL_INDIANRED, FL_GREEN);
    fl_set_slider_bounds(obj, 0, 255);
    fl_set_slider_precision(obj, 0);
    fl_set_object_callback(obj, search_rgb, 1);
    fl_set_slider_return(obj, 0);

    bs = obj = fl_add_valslider(FL_VERT_FILL_SLIDER, 285, 130, 30, 200, "");
    fl_set_object_color(obj, FL_INDIANRED, FL_BLUE);
    fl_set_slider_bounds(obj, 0, 255);
    fl_set_slider_precision(obj, 0);
    fl_set_object_callback(obj, search_rgb, 2);
    fl_set_slider_return(obj, 0);


    colbr = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 90, 205, 240, "");
    fl_set_browser_fontstyle(obj, FL_FIXED_STYLE); 
    fl_set_object_callback(obj, br_cb, 0);


    obj = fl_add_button(FL_NORMAL_BUTTON, 135, 345, 80, 30, "Done");
    fl_set_object_callback(obj, done_cb, 0);

    rescol = obj = fl_add_box(FL_FLAT_BOX, 225, 90, 90, 35, "");
    fl_set_object_color(obj, FL_FREE_COL4, FL_FREE_COL4);
    fl_set_object_boxtype(obj, FL_BORDER_BOX);


    fl_end_form();
    fl_scale_form(cl, 1.1, 1.0);
}

//
// End of "$Id$".
//
