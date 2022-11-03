#pragma once

#include <cairo/cairo.h>

int
blur_surface(cairo_surface_t *surface, int margin);

void
render_shadow(cairo_t *cr, cairo_surface_t *surface,
	      int x, int y, int width, int height, int margin, int top_margin);
