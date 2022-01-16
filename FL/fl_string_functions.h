/*
 * Platform agnostic string portability functions for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 2020-2022 by Bill Spitzak and others.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     https://www.fltk.org/COPYING.php
 *
 * Please see the following page on how to report bugs and issues:
 *
 *     https://www.fltk.org/bugs.php
 */

/**
  \file fl_string_functions.h
  \brief Public header for FLTK's own platform agnostic string handling.
*/

#ifndef _FL_fl_string_functions_h_
#define _FL_fl_string_functions_h_

#include "Fl_Export.H"
#include "fl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup fl_string
    @{
*/

FL_EXPORT char* fl_strdup(const char *s);

/** @} */

/*****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _FL_fl_string_functions_h_ */
