/* This demo shows the different routines on browsers */

#include "forms.h"

FL_FORM *form;
FL_OBJECT *browserobj, *inputobj, *exitobj;

void addit(FL_OBJECT *, long)
{
  /* append and show the last line. Don't use this if you just want
   * to add some lines. use fl_add_browser_line
   */
  fl_addto_browser(browserobj,fl_get_input(inputobj));
}

void insertit(FL_OBJECT *, long)
{
  int n;
  if (! ( n = fl_get_browser(browserobj))) return;
  fl_insert_browser_line(browserobj,n,fl_get_input(inputobj));
}

void replaceit(FL_OBJECT *, long)
{
  int n;
  if (! (n=fl_get_browser(browserobj))) return;
  fl_replace_browser_line(browserobj,n,fl_get_input(inputobj));
}

void deleteit(FL_OBJECT *, long)
{
  int n;
  if (! (n = fl_get_browser(browserobj))) return;
  fl_delete_browser_line(browserobj,n);
}

void clearit(FL_OBJECT *, long)
{
  fl_clear_browser(browserobj);
}

/*---------------------------------------*/

void create_form(void)
{
  FL_OBJECT *obj;

  form = fl_bgn_form(FL_UP_BOX,390,420);
  browserobj = fl_add_browser(FL_HOLD_BROWSER,20,20,210,330,"");
//  fl_set_object_dblbuffer(browserobj, 1);
  inputobj = obj = fl_add_input(FL_NORMAL_INPUT,20,370,210,30,"");
    fl_set_object_callback(obj,addit,0);
    obj->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
  obj = fl_add_button(FL_NORMAL_BUTTON,250,20,120,30,"Add");
    fl_set_object_callback(obj,addit,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,250,60,120,30,"Insert");
    fl_set_object_callback(obj,insertit,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,250,100,120,30,"Replace");
    fl_set_object_callback(obj,replaceit,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,250,160,120,30,"Delete");
    fl_set_object_callback(obj,deleteit,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,250,200,120,30,"Clear");
    fl_set_object_callback(obj,clearit,0);
  exitobj = fl_add_button(FL_NORMAL_BUTTON,250,370,120,30,"Exit");
  fl_end_form();
}

/*---------------------------------------*/

int
main(int argc, char *argv[])
{
  FL_OBJECT *obj;

  fl_initialize(&argc, argv, "FormDemo", 0, 0);
  create_form();
  fl_show_form(form,FL_PLACE_CENTER,FL_TRANSIENT,"Browser Op");
  do obj = fl_do_forms(); while (obj != exitobj);
  fl_hide_form(form);
  return 0;
}
