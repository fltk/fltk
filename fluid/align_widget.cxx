//
// Alignment code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2023 by Bill Spitzak and others.
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

#include "align_widget.h"

#include "fluid.h"
#include "Fl_Group_Type.h"
#include "undo.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

/**
 the first behavior always uses the first selected widget as a reference
 the second behavior uses the largest widget (most extreme positions) as
 a reference.
 */
#define BREAK_ON_FIRST break
//#define BREAK_ON_FIRST

void align_widget_cb(Fl_Widget*, long how)
{
  const int max = 32768, min = -32768;
  int left, right, top, bot, wdt, hgt, n;
  Fl_Type *o;
  int changed = 0;
  switch ( how )
  {
  //---- align
  case 10: // align left
    left = max;
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget())
      {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        if (w->x()<left)
          left = w->x();
        BREAK_ON_FIRST;
      }
    if (left!=max)
      for (Fl_Type *o = Fl_Type::first; o; o = o->next)
        if (o->selected && o->is_widget())
        {
          if (!changed) {
            changed = 1;
            undo_checkpoint();
          }
          Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
          Fl_Type::allow_layout++;
          w->resize(left, w->y(), w->w(), w->h());
          Fl_Type::allow_layout--;
          w->redraw();
          if (w->window()) w->window()->redraw();
        }
    break;
  case 11: // align h.center
    left = max; right = min;
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget())
      {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        if (w->x()<left)
          left = w->x();
        if (w->x()+w->w()>right)
          right = w->x()+w->w();
        BREAK_ON_FIRST;
      }
    if (left!=max)
    {
      int center2 = left+right;
      for (Fl_Type *o = Fl_Type::first; o; o = o->next)
        if (o->selected && o->is_widget())
        {
          if (!changed) {
            changed = 1;
            undo_checkpoint();
          }
          Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
          Fl_Type::allow_layout++;
          w->resize((center2-w->w())/2, w->y(), w->w(), w->h());
          Fl_Type::allow_layout--;
          w->redraw();
          if (w->window()) w->window()->redraw();
        }
    }
    break;
  case 12: // align right
    right = min;
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget())
      {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        if (w->x()+w->w()>right)
          right = w->x()+w->w();
        BREAK_ON_FIRST;
      }
    if (right!=min)
      for (Fl_Type *o = Fl_Type::first; o; o = o->next)
        if (o->selected && o->is_widget())
        {
          if (!changed) {
            changed = 1;
            undo_checkpoint();
          }
          Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
          Fl_Type::allow_layout++;
          w->resize(right-w->w(), w->y(), w->w(), w->h());
          Fl_Type::allow_layout--;
          w->redraw();
          if (w->window()) w->window()->redraw();
        }
    break;
  case 13: // align top
    top = max;
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget())
      {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        if (w->y()<top)
          top = w->y();
        BREAK_ON_FIRST;
      }
    if (top!=max)
      for (Fl_Type *o = Fl_Type::first; o; o = o->next)
        if (o->selected && o->is_widget())
        {
          if (!changed) {
            changed = 1;
            undo_checkpoint();
          }
          Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
          Fl_Type::allow_layout++;
          w->resize(w->x(), top, w->w(), w->h());
          Fl_Type::allow_layout--;
          w->redraw();
          if (w->window()) w->window()->redraw();
        }
    break;
  case 14: // align v.center
    top = max; bot = min;
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget())
      {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        if (w->y()<top)
          top = w->y();
        if (w->y()+w->h()>bot)
          bot = w->y()+w->h();
        BREAK_ON_FIRST;
      }
    if (top!=max)
    {
      int center2 = top+bot;
      for (Fl_Type *o = Fl_Type::first; o; o = o->next)
        if (o->selected && o->is_widget())
        {
          if (!changed) {
            changed = 1;
            undo_checkpoint();
          }
          Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
          Fl_Type::allow_layout++;
          w->resize(w->x(), (center2-w->h())/2, w->w(), w->h());
          Fl_Type::allow_layout--;
          w->redraw();
          if (w->window()) w->window()->redraw();
        }
    }
    break;
  case 15: // align bottom
    bot = min;
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget())
      {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        if (w->y()+w->h()>bot)
          bot = w->y()+w->h();
        BREAK_ON_FIRST;
      }
    if (bot!=min)
      for (Fl_Type *o = Fl_Type::first; o; o = o->next)
        if (o->selected && o->is_widget())
        {
          if (!changed) {
            changed = 1;
            undo_checkpoint();
          }
          Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
          Fl_Type::allow_layout++;
          w->resize( w->x(), bot-w->h(), w->w(), w->h());
          Fl_Type::allow_layout--;
          w->redraw();
          if (w->window()) w->window()->redraw();
        }
    break;
  //---- space evenly
  case 20: // space evenly across
    left = max; right = min; wdt = 0; n = 0;
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget())
      {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        if (w->x()<left)
          left = w->x();
        if (w->x()+w->w()>right)
          right = w->x()+w->w();
        wdt += w->w();
        n++;
      }
    wdt = (right-left)-wdt;
    n--;
    if (n>0)
    {
      wdt = wdt/n*n; // make sure that all gaps are the same, possibly moving the rightmost widget
      int cnt = 0, wsum = 0;
      for (Fl_Type *o = Fl_Type::first; o; o = o->next)
        if (o->selected && o->is_widget())
        {
          if (!changed) {
            changed = 1;
            undo_checkpoint();
          }
          Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
          Fl_Type::allow_layout++;
          w->resize(left+wsum+wdt*cnt/n, w->y(), w->w(), w->h());
          Fl_Type::allow_layout--;
          w->redraw();
          if (w->window()) w->window()->redraw();
          cnt++;
          wsum += w->w();
        }
    }
    break;
  case 21: // space evenly down
    top = max; bot = min; hgt = 0, n = 0;
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget())
      {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        if (w->y()<top)
          top = w->y();
        if (w->y()+w->h()>bot)
          bot = w->y()+w->h();
        hgt += w->h();
        n++;
      }
    hgt = (bot-top)-hgt;
    n--;
    if (n>0)
    {
      hgt = hgt/n*n; // make sure that all gaps are the same, possibly moving the rightmost widget
      int cnt = 0, hsum = 0;
      for (Fl_Type *o = Fl_Type::first; o; o = o->next)
        if (o->selected && o->is_widget())
        {
          if (!changed) {
            changed = 1;
            undo_checkpoint();
          }
          Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
          Fl_Type::allow_layout++;
          w->resize(w->x(), top+hsum+hgt*cnt/n, w->w(), w->h());
          Fl_Type::allow_layout--;
          w->redraw();
          if (w->window()) w->window()->redraw();
          cnt++;
          hsum += w->h();
        }
    }
    break;
  //---- make same size
  case 30: // same width
    wdt = min;
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget())
      {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        if (w->w()>wdt)
          wdt = w->w();
        BREAK_ON_FIRST;
      }
    if (wdt!=min)
      for (Fl_Type *o = Fl_Type::first; o; o = o->next)
        if (o->selected && o->is_widget())
        {
          if (!changed) {
            changed = 1;
            undo_checkpoint();
          }
          Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
          Fl_Type::allow_layout++;
          w->resize(w->x(), w->y(), wdt, w->h());
          Fl_Type::allow_layout--;
          w->redraw();
          if (w->window()) w->window()->redraw();
        }
    break;
  case 31: // same height
    hgt = min;
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget())
      {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        if (w->h()>hgt)
          hgt = w->h();
        BREAK_ON_FIRST;
      }
    if (hgt!=min)
      for (Fl_Type *o = Fl_Type::first; o; o = o->next)
        if (o->selected && o->is_widget())
        {
          if (!changed) {
            changed = 1;
            undo_checkpoint();
          }
          Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
          Fl_Type::allow_layout++;
          w->resize( w->x(), w->y(), w->w(), hgt);
          Fl_Type::allow_layout--;
          w->redraw();
          if (w->window()) w->window()->redraw();
        }
    break;
  case 32: // same size
    hgt = min; wdt = min;
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget())
      {
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        if (w->w()>wdt)
          wdt = w->w();
        if (w->h()>hgt)
          hgt = w->h();
        BREAK_ON_FIRST;
      }
    if (hgt!=min)
      for (Fl_Type *o = Fl_Type::first; o; o = o->next)
        if (o->selected && o->is_widget())
        {
          if (!changed) {
            changed = 1;
            undo_checkpoint();
          }
          Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
          Fl_Type::allow_layout++;
          w->resize( w->x(), w->y(), wdt, hgt);
          Fl_Type::allow_layout--;
          w->redraw();
          if (w->window()) w->window()->redraw();
        }
    break;
  //---- center in group
  case 40: // center hor
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget() && o->parent)
      {
        if (!changed) {
          changed = 1;
          undo_checkpoint();
        }
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        Fl_Widget *p = ((Fl_Widget_Type *)o->parent)->o;
        int center2;

        if (w->window() == p) center2 = p->w();
        else center2 = 2*p->x()+p->w();

        Fl_Type::allow_layout++;
        w->resize((center2-w->w())/2, w->y(), w->w(), w->h());
        Fl_Type::allow_layout--;
        w->redraw();
        if (w->window()) w->window()->redraw();
      }
    break;
  case 41: // center vert
    for (o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget() && o->parent)
      {
        if (!changed) {
          changed = 1;
          undo_checkpoint();
        }
        Fl_Widget *w = ((Fl_Widget_Type *)o)->o;
        Fl_Widget *p = ((Fl_Widget_Type *)o->parent)->o;
        int center2;

        if (w->window() == p) center2 = p->h();
        else center2 = 2*p->y()+p->h();

        Fl_Type::allow_layout++;
        w->resize(w->x(), (center2-w->h())/2, w->w(), w->h());
        Fl_Type::allow_layout--;
        set_modflag(1);
        w->redraw();
        if (w->window()) w->window()->redraw();
      }
    break;
  }
  if (changed)
    set_modflag(1);
}
