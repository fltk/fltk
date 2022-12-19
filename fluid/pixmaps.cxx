//
// Fluid Image management for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#include "pixmaps.h"

#include <FL/Fl_Pixmap.H>

#include "pixmaps/bind.xpm"
#include "pixmaps/lock.xpm"
#include "pixmaps/protected.xpm"
#include "pixmaps/invisible.xpm"

#include "pixmaps/flWindow.xpm"
#include "pixmaps/flButton.xpm"
#include "pixmaps/flCheckButton.xpm"
#include "pixmaps/flRoundButton.xpm"
#include "pixmaps/flBox.xpm"
#include "pixmaps/flGroup.xpm"
#include "pixmaps/flFunction.xpm"
#include "pixmaps/flCode.xpm"
#include "pixmaps/flCodeBlock.xpm"
#include "pixmaps/flComment.xpm"
#include "pixmaps/flData.xpm"
#include "pixmaps/flDeclaration.xpm"
#include "pixmaps/flDeclarationBlock.xpm"
#include "pixmaps/flClass.xpm"
#include "pixmaps/flTabs.xpm"
#include "pixmaps/flInput.xpm"
#include "pixmaps/flChoice.xpm"
#include "pixmaps/flMenuitem.xpm"
#include "pixmaps/flMenubar.xpm"
#include "pixmaps/flSubmenu.xpm"
#include "pixmaps/flScroll.xpm"
#include "pixmaps/flTile.xpm"
#include "pixmaps/flWizard.xpm"
#include "pixmaps/flPack.xpm"
#include "pixmaps/flReturnButton.xpm"
#include "pixmaps/flLightButton.xpm"
#include "pixmaps/flRepeatButton.xpm"
#include "pixmaps/flMenuButton.xpm"
#include "pixmaps/flOutput.xpm"
#include "pixmaps/flTextDisplay.xpm"
#include "pixmaps/flTextEdit.xpm"
#include "pixmaps/flFileInput.xpm"
#include "pixmaps/flBrowser.xpm"
#include "pixmaps/flCheckBrowser.xpm"
#include "pixmaps/flFileBrowser.xpm"
#include "pixmaps/flClock.xpm"
#include "pixmaps/flHelp.xpm"
#include "pixmaps/flProgress.xpm"
#include "pixmaps/flSlider.xpm"
#include "pixmaps/flScrollBar.xpm"
#include "pixmaps/flValueSlider.xpm"
#include "pixmaps/flAdjuster.xpm"
#include "pixmaps/flCounter.xpm"
#include "pixmaps/flDial.xpm"
#include "pixmaps/flRoller.xpm"
#include "pixmaps/flValueInput.xpm"
#include "pixmaps/flValueOutput.xpm"
#include "pixmaps/flSpinner.xpm"
#include "pixmaps/flWidgetClass.xpm"
#include "pixmaps/flTree.xpm"
#include "pixmaps/flTable.xpm"
#include "pixmaps/flSimpleTerminal.xpm"
#include "pixmaps/flInputChoice.xpm"
#include "pixmaps/flCheckMenuitem.xpm"
#include "pixmaps/flRadioMenuitem.xpm"
#include "pixmaps/flFlex.xpm"

Fl_Pixmap *bind_pixmap;
Fl_Pixmap *lock_pixmap;
Fl_Pixmap *protected_pixmap;
Fl_Pixmap *invisible_pixmap;

Fl_Pixmap *window_pixmap;
Fl_Pixmap *button_pixmap;
Fl_Pixmap *checkbutton_pixmap;
Fl_Pixmap *roundbutton_pixmap;
Fl_Pixmap *box_pixmap;
Fl_Pixmap *group_pixmap;
Fl_Pixmap *function_pixmap;
Fl_Pixmap *code_pixmap;
Fl_Pixmap *codeblock_pixmap;
Fl_Pixmap *comment_pixmap;
Fl_Pixmap *declaration_pixmap;
Fl_Pixmap *declarationblock_pixmap;
Fl_Pixmap *class_pixmap;
Fl_Pixmap *tabs_pixmap;
Fl_Pixmap *input_pixmap;
Fl_Pixmap *choice_pixmap;
Fl_Pixmap *menuitem_pixmap;
Fl_Pixmap *menubar_pixmap;
Fl_Pixmap *submenu_pixmap;
Fl_Pixmap *scroll_pixmap;
Fl_Pixmap *tile_pixmap;
Fl_Pixmap *wizard_pixmap;
Fl_Pixmap *pack_pixmap;
Fl_Pixmap *returnbutton_pixmap;
Fl_Pixmap *lightbutton_pixmap;
Fl_Pixmap *repeatbutton_pixmap;
Fl_Pixmap *menubutton_pixmap;
Fl_Pixmap *output_pixmap;
Fl_Pixmap *textdisplay_pixmap;
Fl_Pixmap *textedit_pixmap;
Fl_Pixmap *fileinput_pixmap;
Fl_Pixmap *browser_pixmap;
Fl_Pixmap *checkbrowser_pixmap;
Fl_Pixmap *filebrowser_pixmap;
Fl_Pixmap *clock_pixmap;
Fl_Pixmap *help_pixmap;
Fl_Pixmap *progress_pixmap;
Fl_Pixmap *slider_pixmap;
Fl_Pixmap *scrollbar_pixmap;
Fl_Pixmap *valueslider_pixmap;
Fl_Pixmap *adjuster_pixmap;
Fl_Pixmap *counter_pixmap;
Fl_Pixmap *dial_pixmap;
Fl_Pixmap *roller_pixmap;
Fl_Pixmap *valueinput_pixmap;
Fl_Pixmap *valueoutput_pixmap;
Fl_Pixmap *spinner_pixmap;
Fl_Pixmap *widgetclass_pixmap;
Fl_Pixmap *data_pixmap;
Fl_Pixmap *tree_pixmap;
Fl_Pixmap *table_pixmap;
Fl_Pixmap *simple_terminal_pixmap;
Fl_Pixmap *input_choice_pixmap;
Fl_Pixmap *check_menuitem_pixmap;
Fl_Pixmap *radio_menuitem_pixmap;
Fl_Pixmap *flex_pixmap;

Fl_Pixmap *pixmap[57];

void loadPixmaps()
{
  bind_pixmap = new Fl_Pixmap(bind_xpm); bind_pixmap->scale(16, 16);
  lock_pixmap = new Fl_Pixmap(lock_xpm); lock_pixmap->scale(16, 16);
  protected_pixmap = new Fl_Pixmap(protected_xpm); protected_pixmap->scale(16, 16);
  invisible_pixmap = new Fl_Pixmap(invisible_xpm); invisible_pixmap->scale(16, 16);

  pixmap[1] = window_pixmap = new Fl_Pixmap(flWindow_xpm); window_pixmap->scale(16, 16);
  pixmap[2] = button_pixmap = new Fl_Pixmap(flButton_xpm); button_pixmap->scale(16, 16);
  pixmap[3] = checkbutton_pixmap = new Fl_Pixmap(flCheckButton_xpm); checkbutton_pixmap->scale(16, 16);
  pixmap[4] = roundbutton_pixmap = new Fl_Pixmap(flRoundButton_xpm); roundbutton_pixmap->scale(16, 16);

  pixmap[5] = box_pixmap = new Fl_Pixmap(flBox_xpm); box_pixmap->scale(16, 16);
  pixmap[6] = group_pixmap = new Fl_Pixmap(flGroup_xpm); group_pixmap->scale(16, 16);
  pixmap[7] = function_pixmap = new Fl_Pixmap(flFunction_xpm); function_pixmap->scale(16, 16);
  pixmap[8] = code_pixmap = new Fl_Pixmap(flCode_xpm); code_pixmap->scale(16, 16);
  pixmap[9] = codeblock_pixmap = new Fl_Pixmap(flCodeBlock_xpm); codeblock_pixmap->scale(16, 16);
  pixmap[10] = declaration_pixmap = new Fl_Pixmap(flDeclaration_xpm); declaration_pixmap->scale(16, 16);

  pixmap[11] = declarationblock_pixmap = new Fl_Pixmap(flDeclarationBlock_xpm); declarationblock_pixmap->scale(16, 16);
  pixmap[12] = class_pixmap = new Fl_Pixmap(flClass_xpm); class_pixmap->scale(16, 16);
  pixmap[13] = tabs_pixmap = new Fl_Pixmap(flTabs_xpm); tabs_pixmap->scale(16, 16);
  pixmap[14] = input_pixmap = new Fl_Pixmap(flInput_xpm); input_pixmap->scale(16, 16);
  pixmap[15] = choice_pixmap = new Fl_Pixmap(flChoice_xpm); choice_pixmap->scale(16, 16);

  pixmap[16] = menuitem_pixmap = new Fl_Pixmap(flMenuitem_xpm); menuitem_pixmap->scale(16, 16);
  pixmap[17] = menubar_pixmap = new Fl_Pixmap(flMenubar_xpm); menubar_pixmap->scale(16, 16);
  pixmap[18] = submenu_pixmap = new Fl_Pixmap(flSubmenu_xpm); submenu_pixmap->scale(16, 16);
  pixmap[19] = scroll_pixmap = new Fl_Pixmap(flScroll_xpm); scroll_pixmap->scale(16, 16);
  pixmap[20] = tile_pixmap = new Fl_Pixmap(flTile_xpm); tile_pixmap->scale(16, 16);
  pixmap[21] = wizard_pixmap = new Fl_Pixmap(flWizard_xpm); wizard_pixmap->scale(16, 16);

  pixmap[22] = pack_pixmap = new Fl_Pixmap(flPack_xpm); pack_pixmap->scale(16, 16);
  pixmap[23] = returnbutton_pixmap = new Fl_Pixmap(flReturnButton_xpm); returnbutton_pixmap->scale(16, 16);
  pixmap[24] = lightbutton_pixmap = new Fl_Pixmap(flLightButton_xpm); lightbutton_pixmap->scale(16, 16);
  pixmap[25] = repeatbutton_pixmap = new Fl_Pixmap(flRepeatButton_xpm); repeatbutton_pixmap->scale(16, 16);
  pixmap[26] = menubutton_pixmap = new Fl_Pixmap(flMenuButton_xpm); menubutton_pixmap->scale(16, 16);

  pixmap[27] = output_pixmap = new Fl_Pixmap(flOutput_xpm); output_pixmap->scale(16, 16);
  pixmap[28] = textdisplay_pixmap = new Fl_Pixmap(flTextDisplay_xpm); textdisplay_pixmap->scale(16, 16);
  pixmap[29] = textedit_pixmap = new Fl_Pixmap(flTextEdit_xpm); textedit_pixmap->scale(16, 16);
  pixmap[30] = fileinput_pixmap = new Fl_Pixmap(flFileInput_xpm); fileinput_pixmap->scale(16, 16);
  pixmap[31] = browser_pixmap = new Fl_Pixmap(flBrowser_xpm); browser_pixmap->scale(16, 16);

  pixmap[32] = checkbrowser_pixmap = new Fl_Pixmap(flCheckBrowser_xpm); checkbrowser_pixmap->scale(16, 16);
  pixmap[33] = filebrowser_pixmap = new Fl_Pixmap(flFileBrowser_xpm); filebrowser_pixmap->scale(16, 16);
  pixmap[34] = clock_pixmap = new Fl_Pixmap(flClock_xpm); clock_pixmap->scale(16, 16);
  pixmap[35] = help_pixmap = new Fl_Pixmap(flHelp_xpm); help_pixmap->scale(16, 16);
  pixmap[36] = progress_pixmap = new Fl_Pixmap(flProgress_xpm); progress_pixmap->scale(16, 16);

  pixmap[37] = slider_pixmap = new Fl_Pixmap(flSlider_xpm); slider_pixmap->scale(16, 16);
  pixmap[38] = scrollbar_pixmap = new Fl_Pixmap(flScrollBar_xpm); scrollbar_pixmap->scale(16, 16);
  pixmap[39] = valueslider_pixmap = new Fl_Pixmap(flValueSlider_xpm); valueslider_pixmap->scale(16, 16);
  pixmap[40] = adjuster_pixmap = new Fl_Pixmap(flAdjuster_xpm); adjuster_pixmap->scale(16, 16);
  pixmap[41] = counter_pixmap = new Fl_Pixmap(flCounter_xpm); counter_pixmap->scale(16, 16);

  pixmap[42] = dial_pixmap = new Fl_Pixmap(flDial_xpm); dial_pixmap->scale(16, 16);
  pixmap[43] = roller_pixmap = new Fl_Pixmap(flRoller_xpm); roller_pixmap->scale(16, 16);
  pixmap[44] = valueinput_pixmap = new Fl_Pixmap(flValueInput_xpm); valueinput_pixmap->scale(16, 16);
  pixmap[45] = valueoutput_pixmap = new Fl_Pixmap(flValueOutput_xpm); valueoutput_pixmap->scale(16, 16);
  pixmap[46] = comment_pixmap = new Fl_Pixmap(flComment_xpm); comment_pixmap->scale(16, 16);

  pixmap[47] = spinner_pixmap = new Fl_Pixmap(flSpinner_xpm); spinner_pixmap->scale(16, 16);
  pixmap[48] = widgetclass_pixmap = new Fl_Pixmap(flWidgetClass_xpm); widgetclass_pixmap->scale(16, 16);
  pixmap[49] = data_pixmap = new Fl_Pixmap(flData_xpm); data_pixmap->scale(16, 16);
  pixmap[50] = tree_pixmap = new Fl_Pixmap(flTree_xpm); tree_pixmap->scale(16, 16);
  pixmap[51] = table_pixmap = new Fl_Pixmap(flTable_xpm); table_pixmap->scale(16, 16);

  pixmap[52] = simple_terminal_pixmap = new Fl_Pixmap(flSimpleTerminal_xpm); simple_terminal_pixmap->scale(16, 16);
  pixmap[53] = input_choice_pixmap = new Fl_Pixmap(flInputChoice_xpm); input_choice_pixmap->scale(16, 16);
  pixmap[54] = check_menuitem_pixmap = new Fl_Pixmap(flCheckMenuitem_xpm); check_menuitem_pixmap->scale(16, 16);
  pixmap[55] = radio_menuitem_pixmap = new Fl_Pixmap(flRadioMenuitem_xpm); radio_menuitem_pixmap->scale(16, 16);

  pixmap[56] = flex_pixmap = new Fl_Pixmap(flFlex_xpm); flex_pixmap->scale(16, 16);
}

