//
// "$Id: Fl_File_Icon2.cxx,v 1.1.2.2 2001/11/17 15:59:53 easysw Exp $"
//
// Fl_File_Icon system icon routines.
//
// KDE icon code donated by Maarten De Boer.
//
// Copyright 1999-2001 by Michael Sweet.
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
// Contents:
//
//   Fl_File_Icon::load_fti()          - Load an SGI-format FTI file...
//   Fl_File_Icon::load_png()          - Load a PNG icon file...
//   Fl_File_Icon::load_xpm()          - Load an XPM icon file...
//   Fl_File_Icon::load_system_icons() - Load the standard system icons/filetypes.
//

//
// Include necessary header files...
//

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif // HAVE_STRINGS_H
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#if (defined(WIN32) && ! defined(__CYGWIN__)) || defined(__EMX__)
#  include <io.h>
#  define F_OK	0
#  define strcasecmp stricmp
#  define strncasecmp strnicmp
#else
#  include <unistd.h>
#endif /* WIN32 || __EMX__ */

#include <FL/Fl_File_Icon.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/filename.H>

extern "C"
{
#ifdef HAVE_LIBPNG
#  include <zlib.h>
#  include <png.h>
#endif // HAVE_LIBPNG
}


//
// Define missing POSIX/XPG4 macros as needed...
//

#ifndef S_ISDIR
#  define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#  define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#  define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#  define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif /* !S_ISDIR */


//
// Local functions...
//

static void	load_kde_icons(const char *directory);
static void	load_kde_mimelnk(const char *filename);
static char	*kde_to_fltk_pattern(const char *kdepattern);
static char	*get_kde_val(char *str, const char *key);


//
// 'Fl_File_Icon::load()' - Load an icon file...
//

void
Fl_File_Icon::load(const char *f)	// I - File to read from
{
  const char	*ext;			// File extension


  if ((ext = filename_ext(f)) == NULL)
  {
    fprintf(stderr, "Fl_File_Icon::load(): Unknown file type for \"%s\".\n", f);
    return;
  }

  if (strcmp(ext, ".fti") == 0)
    load_fti(f);
  else if (strcmp(ext, ".xpm") == 0)
    load_xpm(f);
  else if (strcmp(ext, ".png") == 0)
    load_png(f);
  else
  {
    fprintf(stderr, "Fl_File_Icon::load(): Unknown file type for \"%s\".\n", f);
    return;
  }
}


//
// 'Fl_File_Icon::load_fti()' - Load an SGI-format FTI file...
//

void
Fl_File_Icon::load_fti(const char *fti)	// I - File to read from
{
  FILE	*fp;			// File pointer
  int	ch;			// Current character
  char	command[255],		// Command string ("vertex", etc.)
	params[255],		// Parameter string ("10.0,20.0", etc.)
	*ptr;			// Pointer into strings
  int	outline;		// Outline polygon


  // Try to open the file...
  if ((fp = fopen(fti, "rb")) == NULL)
  {
    fprintf(stderr, "Fl_File_Icon::load_fti(): Unable to open \"%s\" - %s\n",
            fti, strerror(errno));
    return;
  }

  // Read the entire file, adding data as needed...
  outline = 0;

  while ((ch = getc(fp)) != EOF)
  {
    // Skip whitespace
    if (isspace(ch))
      continue;

    // Skip comments starting with "#"...
    if (ch == '#')
    {
      while ((ch = getc(fp)) != EOF)
        if (ch == '\n')
	  break;

      if (ch == EOF)
        break;
      else
        continue;
    }

    // OK, this character better be a letter...
    if (!isalpha(ch))
    {
      fprintf(stderr, "Fl_File_Icon::load_fti(): Expected a letter at file position %ld (saw '%c')\n",
              ftell(fp) - 1, ch);
      break;
    }

    // Scan the command name...
    ptr    = command;
    *ptr++ = ch;

    while ((ch = getc(fp)) != EOF)
    {
      if (ch == '(')
        break;
      else if (ptr < (command + sizeof(command) - 1))
        *ptr++ = ch;
    }

    *ptr++ = '\0';

    // Make sure we stopped on a parenthesis...
    if (ch != '(')
    {
      fprintf(stderr, "Fl_File_Icon::load_fti(): Expected a ( at file position %ld (saw '%c')\n",
              ftell(fp) - 1, ch);
      break;
    }

    // Scan the parameters...
    ptr = params;

    while ((ch = getc(fp)) != EOF)
    {
      if (ch == ')')
        break;
      else if (ptr < (params + sizeof(params) - 1))
        *ptr++ = ch;
    }

    *ptr++ = '\0';

    // Make sure we stopped on a parenthesis...
    if (ch != ')')
    {
      fprintf(stderr, "Fl_File_Icon::load_fti(): Expected a ) at file position %ld (saw '%c')\n",
              ftell(fp) - 1, ch);
      break;
    }

    // Make sure the next character is a semicolon...
    if ((ch = getc(fp)) != ';')
    {
      fprintf(stderr, "Fl_File_Icon::load_fti(): Expected a ; at file position %ld (saw '%c')\n",
              ftell(fp) - 1, ch);
      break;
    }

    // Now process the command...
    if (strcmp(command, "color") == 0)
    {
      // Set the color; for negative colors blend the two primaries to
      // produce a composite color.  Also, the following symbolic color
      // names are understood:
      //
      //     name           FLTK color
      //     -------------  ----------
      //     iconcolor      FL_ICON_COLOR; mapped to the icon color in
      //                    Fl_File_Icon::draw()
      //     shadowcolor    FL_DARK3
      //     outlinecolor   FL_BLACK
      if (strcmp(params, "iconcolor") == 0)
        add_color(FL_ICON_COLOR);
      else if (strcmp(params, "shadowcolor") == 0)
        add_color(FL_DARK3);
      else if (strcmp(params, "outlinecolor") == 0)
        add_color(FL_BLACK);
      else
      {
        int c = atoi(params);	// Color value


        if (c < 0)
	{
	  // Composite color; compute average...
	  c = -c;
	  add_color(fl_color_average((Fl_Color)(c >> 4),
	                             (Fl_Color)(c & 15), 0.5));
	}
	else
	  add_color((Fl_Color)c);
      }
    }
    else if (strcmp(command, "bgnline") == 0)
      add(LINE);
    else if (strcmp(command, "bgnclosedline") == 0)
      add(CLOSEDLINE);
    else if (strcmp(command, "bgnpolygon") == 0)
      add(POLYGON);
    else if (strcmp(command, "bgnoutlinepolygon") == 0)
    {
      add(OUTLINEPOLYGON);
      outline = add(0) - data_;
      add(0);
    }
    else if (strcmp(command, "endoutlinepolygon") == 0 && outline)
    {
      unsigned cval; // Color value

      // Set the outline color; see above for valid values...
      if (strcmp(params, "iconcolor") == 0)
        cval = FL_ICON_COLOR;
      else if (strcmp(params, "shadowcolor") == 0)
        cval = FL_DARK3;
      else if (strcmp(params, "outlinecolor") == 0)
        cval = FL_BLACK;
      else
      {
        int c = atoi(params);	// Color value


        if (c < 0)
	{
	  // Composite color; compute average...
	  c = -c;
	  cval = fl_color_average((Fl_Color)(c >> 4), (Fl_Color)(c & 15), 0.5);
	}
	else
	  cval = c;
      }

      // Store outline color...
      data_[outline]     = cval >> 16;
      data_[outline + 1] = cval;

      outline = 0;
      add(END);
    }
    else if (strncmp(command, "end", 3) == 0)
      add(END);
    else if (strcmp(command, "vertex") == 0)
    {
      float x, y;		// Coordinates of vertex


      if (sscanf(params, "%f,%f", &x, &y) != 2)
        break;

      add_vertex((short)(x * 100.0 + 0.5), (short)(y * 100.0 + 0.5));
    }
    else
    {
      fprintf(stderr, "Fl_File_Icon::load_fti(): Unknown command \"%s\" at file position %ld.\n",
              command, ftell(fp) - 1);
      break;
    }
  }

  // Close the file and return...
  fclose(fp);

#ifdef DEBUG
  printf("Icon File \"%s\":\n", fti);
  for (int i = 0; i < num_data_; i ++)
    printf("    %d,\n", data_[i]);
#endif /* DEBUG */
}


//
// 'Fl_File_Icon::load_png()' - Load a PNG icon file...
//

void
Fl_File_Icon::load_png(const char *png)	// I - File to read from
{
#ifdef HAVE_LIBPNG
  FILE		*fp;			// File pointer
  int		i;			// Looping vars
  int		x, y;			// X & Y in image
  int		startx;			// Starting X coord
  int		width, height;		// Width and height of image
  int		depth;			// Depth of image
  png_structp	pp;			// PNG read pointer
  png_infop	info;			// PNG info pointers
  png_bytep	pixels,			// Pixel buffer
		row,			// Current row
		*rows;			// PNG row pointers
  Fl_Color	c,			// Current color
		temp;			// Temporary color


  // Try to open the file...
  if ((fp = fopen(png, "rb")) == NULL)
    return;

  // Setup the PNG data structures...
  pp   = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info = png_create_info_struct(pp);

  // Initialize the PNG read "engine"...
  png_init_io(pp, fp);

  // Get the image dimensions and convert to grayscale or RGB...
  png_read_info(pp, info);

  if (info->color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_expand(pp);

  if (info->bit_depth < 8)
  {
    png_set_packing(pp);
    png_set_expand(pp);
  }
  else if (info->bit_depth == 16)
    png_set_strip_16(pp);

  if (info->color_type & PNG_COLOR_MASK_COLOR)
    depth = 3;
  else
    depth = 1;

  if ((info->color_type & PNG_COLOR_MASK_ALPHA) || info->num_trans)
    depth ++;

#if defined(HAVE_PNG_GET_VALID) && defined(HAVE_SET_TRNS_TO_ALPHA)
  // Handle transparency...
  if (png_get_valid(pp, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(pp);
#endif // HAVE_PNG_GET_VALID && HAVE_SET_TRNS_TO_ALPHA

  width  = (int)info->width;
  height = (int)info->height;
  pixels = (unsigned char *)malloc(width * height * depth);

  // Allocate pointers...
  rows = (png_bytep *)calloc(height, sizeof(png_bytep));

  for (i = 0; i < height; i ++)
    rows[i] = pixels + i * width * depth;

  // Read the image, handling interlacing as needed...
  for (i = png_set_interlace_handling(pp); i > 0; i --)
    png_read_rows(pp, rows, NULL, height);

  // Now loop through the image, adding strips as needed...
  for (y = height - 1; y >= 0; y --)
  {
    for (x = 0, startx = 0, row = rows[height - 1 - y], c = (Fl_Color)-1;
         x < width;
	 x ++, row += depth)
    {
      switch (depth)
      {
        case 1 :
            temp = fl_rgb_color(row[0], row[0], row[0]);
	    break;
        case 2 :
	    if (row[1] > 127)
              temp = fl_rgb_color(row[0], row[0], row[0]);
	    else
	      temp = (Fl_Color)-1;
	    break;
	case 3 :
            temp = fl_rgb_color(row[0], row[1], row[2]);
	    break;
	case 4 :
	    if (row[3] > 127)
              temp = fl_rgb_color(row[0], row[1], row[2]);
	    else
	      temp = (Fl_Color)-1;
	    break;
      }

      if (temp != c)
      {
	if (x > startx && c != (Fl_Color)-1)
	{
	  add_color(c);
	  add(POLYGON);
	  add_vertex(startx * 9000 / width + 1000, y * 9000 / height + 500);
	  add_vertex(x * 9000 / width + 1000,      y * 9000 / height + 500);
	  add_vertex(x * 9000 / width + 1000,      (y + 1) * 9000 / height + 500);
	  add_vertex(startx * 9000 / width + 1000, (y + 1) * 9000 / height + 500);
	  add(END);
	}

        c      = temp;
	startx = x;
      }
    }

    if (x > startx && c != (Fl_Color)-1)
    {
      add_color(c);
      add(POLYGON);
      add_vertex(startx * 9000 / width + 1000, y * 9000 / height + 500);
      add_vertex(x * 9000 / width + 1000,      y * 9000 / height + 500);
      add_vertex(x * 9000 / width + 1000,      (y + 1) * 9000 / height + 500);
      add_vertex(startx * 9000 / width + 1000, (y + 1) * 9000 / height + 500);
      add(END);
    }
  }

  // Free memory and return...
  free(rows);
  free(pixels);

  png_read_end(pp, info);
#  ifdef HAVE_PNG_READ_DESTROY
  png_read_destroy(pp, info, NULL);
#  else
  png_destroy_read_struct(&pp, &info, NULL);
#  endif // HAVE_PNG_READ_DESTROY

  // Close the file and return...
  fclose(fp);

#  ifdef DEBUG
  printf("Icon File \"%s\":\n", xpm);
  for (i = 0; i < num_data_; i ++)
    printf("    %d,\n", data_[i]);
#  endif // DEBUG
#endif // HAVE_LIBPNG
}


//
// 'Fl_File_Icon::load_xpm()' - Load an XPM icon file...
//

void
Fl_File_Icon::load_xpm(const char *xpm)	// I - File to read from
{
  FILE		*fp;			// File pointer
  int		i, j;			// Looping vars
  int		ch;			// Current character
  int		bg;			// Background color
  char		line[1024],		// Line from file
		val[16],		// Color value
		*ptr;			// Pointer into line
  int		x, y;			// X & Y in image
  int		startx;			// Starting X coord
  int		width, height;		// Width and height of image
  int		ncolors;		// Number of colors
  Fl_Color	colors[256];		// Colors
  int		red, green, blue;	// Red, green, and blue values


  // Try to open the file...
  if ((fp = fopen(xpm, "rb")) == NULL)
    return;

  // Read the file header until we find the first string...
  ptr = NULL;
  while (fgets(line, sizeof(line), fp) != NULL)
    if ((ptr = strchr(line, '\"')) != NULL)
      break;

  if (ptr == NULL)
  {
    // Nothing to load...
    fclose(fp);
    return;
  }

  // Get the size of the image...
  sscanf(ptr + 1, "%d%d%d", &width, &height, &ncolors);

  // Now read the colormap...
  memset(colors, 0, sizeof(colors));
  bg = ' ';

  for (i = 0; i < ncolors; i ++)
  {
    while (fgets(line, sizeof(line), fp) != NULL)
      if ((ptr = strchr(line, '\"')) != NULL)
	break;

    if (ptr == NULL)
    {
      // Nothing to load...
      fclose(fp);
      return;
    }

    // Get the color's character
    ptr ++;
    ch = *ptr++;

    // Get the color value...
    if ((ptr = strstr(ptr, "c ")) == NULL)
    {
      // No color; make this black...
      colors[ch] = FL_BLACK;
    }
    else if (ptr[2] == '#')
    {
      // Read the RGB triplet...
      ptr += 3;
      for (j = 0; j < 12; j ++)
        if (!isxdigit(ptr[j]))
	  break;

      switch (j)
      {
        case 0 :
	    bg = ch;
	default :
	    red = green = blue = 0;
	    break;

        case 3 :
	    val[0] = ptr[0];
	    val[1] = '\0';
	    red = 255 * strtol(val, NULL, 16) / 15;

	    val[0] = ptr[1];
	    val[1] = '\0';
	    green = 255 * strtol(val, NULL, 16) / 15;

	    val[0] = ptr[2];
	    val[1] = '\0';
	    blue = 255 * strtol(val, NULL, 16) / 15;
	    break;

        case 6 :
        case 9 :
        case 12 :
	    j /= 3;

	    val[0] = ptr[0];
	    val[1] = ptr[1];
	    val[2] = '\0';
	    red = strtol(val, NULL, 16);

	    val[0] = ptr[j + 0];
	    val[1] = ptr[j + 1];
	    val[2] = '\0';
	    green = strtol(val, NULL, 16);

	    val[0] = ptr[2 * j + 0];
	    val[1] = ptr[2 * j + 1];
	    val[2] = '\0';
	    blue = strtol(val, NULL, 16);
	    break;
      }

      colors[ch] = fl_rgb_color(red, green, blue);
    }
    else
    {
      // Read a color name...
      if (strncasecmp(ptr + 2, "white", 5) == 0)
        colors[ch] = FL_WHITE;
      else if (strncasecmp(ptr + 2, "black", 5) == 0)
        colors[ch] = FL_BLACK;
      else if (strncasecmp(ptr + 2, "none", 4) == 0)
      {
        colors[ch] = FL_BLACK;
	bg = ch;
      }
      else
        colors[ch] = FL_GRAY;
    }
  }

  // Read the image data...
  for (y = height - 1; y >= 0; y --)
  {
    while (fgets(line, sizeof(line), fp) != NULL)
      if ((ptr = strchr(line, '\"')) != NULL)
	break;

    if (ptr == NULL)
    {
      // Nothing to load...
      fclose(fp);
      return;
    }

    startx = 0;
    ch     = bg;
    ptr ++;

    for (x = 0; x < width; x ++, ptr ++)
      if (*ptr != ch)
      {
	if (ch != bg)
	{
          add_color(colors[ch]);
	  add(POLYGON);
	  add_vertex(startx * 9000 / width + 1000, y * 9000 / height + 500);
	  add_vertex(x * 9000 / width + 1000,      y * 9000 / height + 500);
	  add_vertex(x * 9000 / width + 1000,      (y + 1) * 9000 / height + 500);
	  add_vertex(startx * 9000 / width + 1000, (y + 1) * 9000 / height + 500);
	  add(END);
        }

	ch     = *ptr;
	startx = x;
      }

    if (ch != bg)
    {
      add_color(colors[ch]);
      add(POLYGON);
      add_vertex(startx * 9000 / width + 1000, y * 9000 / height + 500);
      add_vertex(x * 9000 / width + 1000,      y * 9000 / height + 500);
      add_vertex(x * 9000 / width + 1000,      (y + 1) * 9000 / height + 500);
      add_vertex(startx * 9000 / width + 1000, (y + 1) * 9000 / height + 500);
      add(END);
    }
  }

  // Close the file and return...
  fclose(fp);

#ifdef DEBUG
  printf("Icon File \"%s\":\n", xpm);
  for (i = 0; i < num_data_; i ++)
    printf("    %d,\n", data_[i]);
#endif /* DEBUG */
}


//
// 'Fl_File_Icon::load_system_icons()' - Load the standard system icons/filetypes.

void
Fl_File_Icon::load_system_icons(void)
{
  Fl_File_Icon	*icon;		// New icons
  static int	init = 0;	// Have the icons been initialized?
  static short	plain[] =	// Plain file icon
		{
		  COLOR, -1, -1, OUTLINEPOLYGON, 0, FL_GRAY,
		  VERTEX, 2000, 1000, VERTEX, 2000, 9000,
		  VERTEX, 6000, 9000, VERTEX, 8000, 7000,
		  VERTEX, 8000, 1000, END, OUTLINEPOLYGON, 0, FL_GRAY,
		  VERTEX, 6000, 9000, VERTEX, 6000, 7000,
		  VERTEX, 8000, 7000, END,
		  COLOR, 0, FL_BLACK, LINE, VERTEX, 6000, 7000,
		  VERTEX, 8000, 7000, VERTEX, 8000, 1000,
		  VERTEX, 2000, 1000, END, LINE, VERTEX, 3000, 7000,
		  VERTEX, 5000, 7000, END, LINE, VERTEX, 3000, 6000,
		  VERTEX, 5000, 6000, END, LINE, VERTEX, 3000, 5000,
		  VERTEX, 7000, 5000, END, LINE, VERTEX, 3000, 4000,
		  VERTEX, 7000, 4000, END, LINE, VERTEX, 3000, 3000,
		  VERTEX, 7000, 3000, END, LINE, VERTEX, 3000, 2000,
		  VERTEX, 7000, 2000, END, 
		  END
		};
  static short	image[] =	// Image file icon
		{
		  COLOR, -1, -1, OUTLINEPOLYGON, 0, FL_GRAY,
		  VERTEX, 2000, 1000, VERTEX, 2000, 9000,
		  VERTEX, 6000, 9000, VERTEX, 8000, 7000,
		  VERTEX, 8000, 1000, END, OUTLINEPOLYGON, 0, FL_GRAY,
		  VERTEX, 6000, 9000, VERTEX, 6000, 7000,
		  VERTEX, 8000, 7000, END,
		  COLOR, 0, FL_BLACK, LINE, VERTEX, 6000, 7000,
		  VERTEX, 8000, 7000, VERTEX, 8000, 1000,
		  VERTEX, 2000, 1000, END,
		  COLOR, 0, FL_RED, POLYGON, VERTEX, 3500, 2500,
		  VERTEX, 3000, 3000, VERTEX, 3000, 4000,
		  VERTEX, 3500, 4500, VERTEX, 4500, 4500,
		  VERTEX, 5000, 4000, VERTEX, 5000, 3000,
		  VERTEX, 4500, 2500, END,
		  COLOR, 0, FL_GREEN, POLYGON, VERTEX, 5500, 2500,
		  VERTEX, 5000, 3000, VERTEX, 5000, 4000,
		  VERTEX, 5500, 4500, VERTEX, 6500, 4500,
		  VERTEX, 7000, 4000, VERTEX, 7000, 3000,
		  VERTEX, 6500, 2500, END,
		  COLOR, 0, FL_BLUE, POLYGON, VERTEX, 4500, 3500,
		  VERTEX, 4000, 4000, VERTEX, 4000, 5000,
		  VERTEX, 4500, 5500, VERTEX, 5500, 5500,
		  VERTEX, 6000, 5000, VERTEX, 6000, 4000,
		  VERTEX, 5500, 3500, END,
		  END
		};
  static short	dir[] =		// Directory icon
		{
		  COLOR, -1, -1, POLYGON, VERTEX, 1000, 1000,
		  VERTEX, 1000, 7500,  VERTEX, 9000, 7500,
		  VERTEX, 9000, 1000, END,
		  POLYGON, VERTEX, 1000, 7500, VERTEX, 2500, 9000,
		  VERTEX, 5000, 9000, VERTEX, 6500, 7500, END,
		  COLOR, 0, FL_WHITE, LINE, VERTEX, 1500, 1500,
		  VERTEX, 1500, 7000, VERTEX, 9000, 7000, END,
		  COLOR, 0, FL_BLACK, LINE, VERTEX, 9000, 7500,
		  VERTEX, 9000, 1000, VERTEX, 1000, 1000, END,
		  COLOR, 0, FL_GRAY, LINE, VERTEX, 1000, 1000,
		  VERTEX, 1000, 7500, VERTEX, 2500, 9000,
		  VERTEX, 5000, 9000, VERTEX, 6500, 7500,
		  VERTEX, 9000, 7500, END,
		  END
		};


  // Add symbols if they haven't been added already...
  if (!init)
  {
    if (!access("/usr/share/mimelnk", F_OK))
    {
      // Load KDE icons...
      icon = new Fl_File_Icon("*", Fl_File_Icon::PLAIN);
      if (!access("/usr/share/icons/hicolor/32x32/mimetypes/unknown.png", F_OK))
        icon->load_png("/usr/share/icons/hicolor/32x32/mimetypes/unknown.png");
      else
        icon->load_xpm("/usr/share/icons/unknown.xpm");

      load_kde_icons("/usr/share/mimelnk");
    }
    else if (!access("/usr/share/icons/folder.xpm", F_OK))
    {
      // Load GNOME icons...
      icon = new Fl_File_Icon("*", Fl_File_Icon::PLAIN);
      icon->load_xpm("/usr/share/icons/page.xpm");

      icon = new Fl_File_Icon("*", Fl_File_Icon::DIRECTORY);
      icon->load_xpm("/usr/share/icons/folder.xpm");
    }
    else if (!access("/usr/dt/appconfig/icons", F_OK))
    {
      // Load CDE icons...
      icon = new Fl_File_Icon("*", Fl_File_Icon::PLAIN);
      icon->load_xpm("/usr/dt/appconfig/icons/C/Dtdata.m.pm");

      icon = new Fl_File_Icon("*", Fl_File_Icon::DIRECTORY);
      icon->load_xpm("/usr/dt/appconfig/icons/C/DtdirB.m.pm");

      icon = new Fl_File_Icon("core", Fl_File_Icon::PLAIN);
      icon->load_xpm("/usr/dt/appconfig/icons/C/Dtcore.m.pm");

      icon = new Fl_File_Icon("*.{bmp|bw|gif|jpg|pbm|pcd|pgm|ppm|png|ras|rgb|tif|xbm|xpm}", Fl_File_Icon::PLAIN);
      icon->load_xpm("/usr/dt/appconfig/icons/C/Dtimage.m.pm");

      icon = new Fl_File_Icon("*.{eps|pdf|ps}", Fl_File_Icon::PLAIN);
      icon->load_xpm("/usr/dt/appconfig/icons/C/Dtps.m.pm");

      icon = new Fl_File_Icon("*.ppd", Fl_File_Icon::PLAIN);
      icon->load_xpm("/usr/dt/appconfig/icons/C/DtPrtpr.m.pm");
    }
    else if (!access("/usr/lib/filetype", F_OK))
    {
      // Load SGI icons...
      icon = new Fl_File_Icon("*", Fl_File_Icon::PLAIN);
      icon->load_fti("/usr/lib/filetype/iconlib/generic.doc.fti");

      icon = new Fl_File_Icon("*", Fl_File_Icon::DIRECTORY);
      icon->load_fti("/usr/lib/filetype/iconlib/generic.folder.closed.fti");

      icon = new Fl_File_Icon("core", Fl_File_Icon::PLAIN);
      icon->load_fti("/usr/lib/filetype/default/iconlib/CoreFile.fti");

      icon = new Fl_File_Icon("*.{bmp|bw|gif|jpg|pbm|pcd|pgm|ppm|png|ras|rgb|tif|xbm|xpm}", Fl_File_Icon::PLAIN);
      icon->load_fti("/usr/lib/filetype/system/iconlib/ImageFile.fti");

      if (!access("/usr/lib/filetype/install/iconlib/acroread.doc.fti", F_OK))
      {
	icon = new Fl_File_Icon("*.{eps|ps}", Fl_File_Icon::PLAIN);
	icon->load_fti("/usr/lib/filetype/system/iconlib/PostScriptFile.closed.fti");

	icon = new Fl_File_Icon("*.pdf", Fl_File_Icon::PLAIN);
	icon->load_fti("/usr/lib/filetype/install/iconlib/acroread.doc.fti");
      }
      else
      {
	icon = new Fl_File_Icon("*.{eps|pdf|ps}", Fl_File_Icon::PLAIN);
	icon->load_fti("/usr/lib/filetype/system/iconlib/PostScriptFile.closed.fti");
      }

      if (!access("/usr/lib/filetype/install/iconlib/html.fti", F_OK))
      {
	icon = new Fl_File_Icon("*.{htm|html|shtml}", Fl_File_Icon::PLAIN);
        icon->load_fti("/usr/lib/filetype/iconlib/generic.doc.fti");
	icon->load_fti("/usr/lib/filetype/install/iconlib/html.fti");
      }

      if (!access("/usr/lib/filetype/install/iconlib/color.ps.idle.fti", F_OK))
      {
	icon = new Fl_File_Icon("*.ppd", Fl_File_Icon::PLAIN);
	icon->load_fti("/usr/lib/filetype/install/iconlib/color.ps.idle.fti");
      }
    }
    else
    {
      // Create the default icons...
      new Fl_File_Icon("*", Fl_File_Icon::PLAIN, sizeof(plain) / sizeof(plain[0]), plain);
      new Fl_File_Icon("*.{bmp|bw|gif|jpg|pbm|pcd|pgm|ppm|png|ras|rgb|tif|xbm|xpm}", Fl_File_Icon::PLAIN,
                   sizeof(image) / sizeof(image[0]), image);
      new Fl_File_Icon("*", Fl_File_Icon::DIRECTORY, sizeof(dir) / sizeof(dir[0]), dir);
    }

    // Mark things as initialized...
    init = 1;
  }
}


//
// 'load_kde_icons()' - Load KDE icon files.
//

static void
load_kde_icons(const char *directory)	// I - Directory to load
{
  int		i;			// Looping var
  int		n;			// Number of entries in directory
  dirent	**entries;		// Entries in directory
  char		full[1024];		// Full name of file


  entries = (dirent **)0;
  n       = filename_list(directory, &entries);

  for (i = 0; i < n; i ++)
  {
    if (entries[i]->d_name[0] != '.')
    {
      strcpy(full, directory);
      strcat(full,"/");
      strcat(full, entries[i]->d_name);

      if (filename_isdir(full))
	load_kde_icons(full);
      else
	load_kde_mimelnk(full);				
    }

    free((void *)entries[i]);
  }

  free((void*)entries);
}


//
// 'load_kde_mimelnk()' - Load a KDE "mimelnk" file.
//

static void
load_kde_mimelnk(const char *filename)
{
  FILE		*fp;
  char		tmp[256];
  char		iconfilename[1024];
  char		pattern[1024];
  char		mimetype[1024];
  char		*val;
  char		full_iconfilename[1024];
  Fl_File_Icon	*icon;


  mimetype[0]     = '\0';
  pattern[0]      = '\0';
  iconfilename[0] = '\0';

  if ((fp = fopen(filename, "rb")) != NULL)
  {
    while (fgets(tmp, sizeof(tmp), fp))
    {
      if ((val = get_kde_val(tmp, "Icon")) != NULL)
	strcpy(iconfilename, val);
      else if ((val = get_kde_val(tmp, "MimeType")) != NULL)
	strcpy(mimetype, val);
      else if ((val = get_kde_val(tmp, "Patterns")) != NULL)
	strcpy(pattern, val);
    }

    if (iconfilename && pattern)
    {
      if (!access("/usr/share/icons/locolor", F_OK))
      {
        if (strncmp(mimetype, "inode/", 6) != 0)
          sprintf(full_iconfilename, "/usr/share/icons/hicolor/32x32/mimetypes/%s.png", iconfilename);
        else
          sprintf(full_iconfilename, "/usr/share/icons/hicolor/32x32/filesystems/%s.png", iconfilename);
      }
      else
        sprintf(full_iconfilename, "/usr/share/icons/%s", iconfilename);

      if (strcmp(mimetype, "inode/directory") == 0)
	icon = new Fl_File_Icon("*", Fl_File_Icon::DIRECTORY);
      else
        icon = new Fl_File_Icon(kde_to_fltk_pattern(pattern), Fl_File_Icon::PLAIN);

      icon->load(full_iconfilename);
    }

    fclose(fp);
  }
}


//
// 'kde_to_fltk_pattern()' - Convert a KDE pattern to a FLTK pattern.
//

static char *
kde_to_fltk_pattern(const char *kdepattern)
{
  char	*pattern,
	*patptr;


  pattern = (char *)malloc(strlen(kdepattern) + 3);
  strcpy(pattern, "{");
  strcat(pattern, kdepattern);

  if (pattern[strlen(pattern) - 1] == ';')
    pattern[strlen(pattern) - 1] = '\0';

  strcat(pattern, "}");

  for (patptr = pattern; *patptr; patptr ++)
    if (*patptr == ';')
      *patptr = '|';

  return (pattern);
}


//
// 'get_kde_val()' - Get a KDE value.
//

static char *
get_kde_val(char       *str,
            const char *key)
{
  while (*str == *key)
  {
    str ++;
    key ++;
  }

  if (*key == '\0' && *str == '=')
  {
    if (str[strlen(str) - 1] == '\n')
      str[strlen(str) - 1] = '\0';

    return (str + 1);
  }

  return ((char *)0);
}


//
// End of "$Id: Fl_File_Icon2.cxx,v 1.1.2.2 2001/11/17 15:59:53 easysw Exp $".
//
