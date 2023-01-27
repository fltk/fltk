/*
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * functions 'blur_surface' and 'render_shadow' from weston project:
 * https://gitlab.freedesktop.org/wayland/weston/raw/master/shared/cairo-util.c
 */

#include "libdecor-cairo-blur.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

/**
 * Compile-time computation of number of items in a hardcoded array.
 *
 * @param a the array being measured.
 * @return the number of items hardcoded into the array.
 */
#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])
#endif

int
blur_surface(cairo_surface_t *surface, int margin)
{
	int32_t width, height, stride, x, y, z, w;
	uint8_t *src, *dst;
	uint32_t *s, *d, a, p;
	int i, j, k, size, half;
	uint32_t kernel[71];
	double f;

	size = ARRAY_LENGTH(kernel);
	width = cairo_image_surface_get_width(surface);
	height = cairo_image_surface_get_height(surface);
	stride = cairo_image_surface_get_stride(surface);
	src = cairo_image_surface_get_data(surface);

	dst = malloc(height * stride);
	if (dst == NULL)
		return -1;

	half = size / 2;
	a = 0;
	for (i = 0; i < size; i++) {
		f = (i - half);
		kernel[i] = exp(- f * f / ARRAY_LENGTH(kernel)) * 10000;
		a += kernel[i];
	}

	for (i = 0; i < height; i++) {
		s = (uint32_t *) (src + i * stride);
		d = (uint32_t *) (dst + i * stride);
		for (j = 0; j < width; j++) {
			if (margin < j && j < width - margin) {
				d[j] = s[j];
				continue;
			}

			x = 0;
			y = 0;
			z = 0;
			w = 0;
			for (k = 0; k < size; k++) {
				if (j - half + k < 0 || j - half + k >= width)
					continue;
				p = s[j - half + k];

				x += (p >> 24) * kernel[k];
				y += ((p >> 16) & 0xff) * kernel[k];
				z += ((p >> 8) & 0xff) * kernel[k];
				w += (p & 0xff) * kernel[k];
			}
			d[j] = (x / a << 24) | (y / a << 16) | (z / a << 8) | w / a;
		}
	}

	for (i = 0; i < height; i++) {
		s = (uint32_t *) (dst + i * stride);
		d = (uint32_t *) (src + i * stride);
		for (j = 0; j < width; j++) {
			if (margin <= i && i < height - margin) {
				d[j] = s[j];
				continue;
			}

			x = 0;
			y = 0;
			z = 0;
			w = 0;
			for (k = 0; k < size; k++) {
				if (i - half + k < 0 || i - half + k >= height)
					continue;
				s = (uint32_t *) (dst + (i - half + k) * stride);
				p = s[j];

				x += (p >> 24) * kernel[k];
				y += ((p >> 16) & 0xff) * kernel[k];
				z += ((p >> 8) & 0xff) * kernel[k];
				w += (p & 0xff) * kernel[k];
			}
			d[j] = (x / a << 24) | (y / a << 16) | (z / a << 8) | w / a;
		}
	}

	free(dst);
	cairo_surface_mark_dirty(surface);

	return 0;
}

void
render_shadow(cairo_t *cr, cairo_surface_t *surface,
	      int x, int y, int width, int height, int margin, int top_margin)
{
	cairo_pattern_t *pattern;
	cairo_matrix_t matrix;
	int i, fx, fy, shadow_height, shadow_width;

	cairo_set_source_rgba(cr, 0, 0, 0, 0.45);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	pattern = cairo_pattern_create_for_surface (surface);
	cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);

	for (i = 0; i < 4; i++) {
		/* when fy is set, then we are working with lower corners,
		 * when fx is set, then we are working with right corners
		 *
		 *  00 ------- 01
		 *   |         |
		 *   |         |
		 *  10 ------- 11
		 */
		fx = i & 1;
		fy = i >> 1;

		cairo_matrix_init_translate(&matrix,
					    -x + fx * (128 - width),
					    -y + fy * (128 - height));
		cairo_pattern_set_matrix(pattern, &matrix);

		shadow_width = margin;
		shadow_height = fy ? margin : top_margin;

		/* if the shadows together are greater than the surface, we need
		 * to fix it - set the shadow size to the half of
		 * the size of surface. Also handle the case when the size is
		 * not divisible by 2. In that case we need one part of the
		 * shadow to be one pixel greater. !fy or !fx, respectively,
		 * will do the work.
		 */
		if (height < 2 * shadow_height)
			shadow_height = (height + !fy) / 2;

		if (width < 2 * shadow_width)
			shadow_width = (width + !fx) / 2;

		cairo_reset_clip(cr);
		cairo_rectangle(cr,
				x + fx * (width - shadow_width),
				y + fy * (height - shadow_height),
				shadow_width, shadow_height);
		cairo_clip (cr);
		cairo_mask(cr, pattern);
	}


	shadow_width = width - 2 * margin;
	shadow_height = top_margin;
	if (height < 2 * shadow_height)
		shadow_height = height / 2;

	if (shadow_width > 0 && shadow_height) {
		/* Top stretch */
		cairo_matrix_init_translate(&matrix, 60, 0);
		cairo_matrix_scale(&matrix, 8.0 / width, 1);
		cairo_matrix_translate(&matrix, -x - width / 2, -y);
		cairo_pattern_set_matrix(pattern, &matrix);
		cairo_rectangle(cr, x + margin, y, shadow_width, shadow_height);

		cairo_reset_clip(cr);
		cairo_rectangle(cr,
				x + margin, y,
				shadow_width, shadow_height);
		cairo_clip (cr);
		cairo_mask(cr, pattern);

		/* Bottom stretch */
		cairo_matrix_translate(&matrix, 0, -height + 128);
		cairo_pattern_set_matrix(pattern, &matrix);

		cairo_reset_clip(cr);
		cairo_rectangle(cr, x + margin, y + height - margin,
				shadow_width, margin);
		cairo_clip (cr);
		cairo_mask(cr, pattern);
	}

	shadow_width = margin;
	if (width < 2 * shadow_width)
		shadow_width = width / 2;

	shadow_height = height - margin - top_margin;

	/* if height is smaller than sum of margins,
	 * then the shadow is already done by the corners */
	if (shadow_height > 0 && shadow_width) {
		/* Left stretch */
		cairo_matrix_init_translate(&matrix, 0, 60);
		cairo_matrix_scale(&matrix, 1, 8.0 / height);
		cairo_matrix_translate(&matrix, -x, -y - height / 2);
		cairo_pattern_set_matrix(pattern, &matrix);
		cairo_reset_clip(cr);
		cairo_rectangle(cr, x, y + top_margin,
				shadow_width, shadow_height);
		cairo_clip (cr);
		cairo_mask(cr, pattern);

		/* Right stretch */
		cairo_matrix_translate(&matrix, -width + 128, 0);
		cairo_pattern_set_matrix(pattern, &matrix);
		cairo_rectangle(cr, x + width - shadow_width, y + top_margin,
				shadow_width, shadow_height);
		cairo_reset_clip(cr);
		cairo_clip (cr);
		cairo_mask(cr, pattern);
	}

	cairo_pattern_destroy(pattern);
	cairo_reset_clip(cr);
}
