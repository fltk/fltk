//
// Fluid Image management for the Fast Light Tool Kit (FLTK).
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

#include "pixmaps.h"

#include "Fl_Type.h"

#include <FL/Fl_Pixmap.H>

#include "pixmaps/bind.xpm"
#include "pixmaps/lock.xpm"
#include "pixmaps/protected.xpm"
#include "pixmaps/invisible.xpm"
#include "pixmaps/compressed.xpm"

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
Fl_Pixmap *compressed_pixmap;

Fl_Pixmap *pixmap[Fl_Type::ID::MaxID] = { NULL };

void loadPixmaps()
{
  Fl_Pixmap *tmp;

  bind_pixmap = new Fl_Pixmap(bind_xpm); bind_pixmap->scale(16, 16);
  lock_pixmap = new Fl_Pixmap(lock_xpm); lock_pixmap->scale(16, 16);
  protected_pixmap = new Fl_Pixmap(protected_xpm); protected_pixmap->scale(16, 16);
  invisible_pixmap = new Fl_Pixmap(invisible_xpm); invisible_pixmap->scale(16, 16);
  compressed_pixmap = new Fl_Pixmap(compressed_xpm); compressed_pixmap->scale(16, 16);

  pixmap[Fl_Type::ID::Window] = tmp = new Fl_Pixmap(flWindow_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Button] = tmp = new Fl_Pixmap(flButton_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Check_Button] = tmp = new Fl_Pixmap(flCheckButton_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Round_Button] = tmp = new Fl_Pixmap(flRoundButton_xpm); tmp->scale(16, 16);

  pixmap[Fl_Type::ID::Box] = tmp = new Fl_Pixmap(flBox_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Group] = tmp = new Fl_Pixmap(flGroup_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Function] = tmp = new Fl_Pixmap(flFunction_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Code] = tmp = new Fl_Pixmap(flCode_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::CodeBlock] = tmp = new Fl_Pixmap(flCodeBlock_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Decl] = tmp = new Fl_Pixmap(flDeclaration_xpm); tmp->scale(16, 16);

  pixmap[Fl_Type::ID::DeclBlock] = tmp = new Fl_Pixmap(flDeclarationBlock_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Class] = tmp = new Fl_Pixmap(flClass_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Tabs] = tmp = new Fl_Pixmap(flTabs_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Input] = tmp = new Fl_Pixmap(flInput_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Choice] = tmp = new Fl_Pixmap(flChoice_xpm); tmp->scale(16, 16);

  pixmap[Fl_Type::ID::Menu_Item] = tmp = new Fl_Pixmap(flMenuitem_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Menu_Bar] = tmp = new Fl_Pixmap(flMenubar_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Submenu] = tmp = new Fl_Pixmap(flSubmenu_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Scroll] = tmp = new Fl_Pixmap(flScroll_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Tile] = tmp = new Fl_Pixmap(flTile_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Wizard] = tmp = new Fl_Pixmap(flWizard_xpm); tmp->scale(16, 16);

  pixmap[Fl_Type::ID::Pack] = tmp = new Fl_Pixmap(flPack_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Return_Button] = tmp = new Fl_Pixmap(flReturnButton_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Light_Button] = tmp = new Fl_Pixmap(flLightButton_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Repeat_Button] = tmp = new Fl_Pixmap(flRepeatButton_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Menu_Button] = tmp = new Fl_Pixmap(flMenuButton_xpm); tmp->scale(16, 16);

  pixmap[Fl_Type::ID::Output] = tmp = new Fl_Pixmap(flOutput_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Text_Display] = tmp = new Fl_Pixmap(flTextDisplay_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Text_Editor] = tmp = new Fl_Pixmap(flTextEdit_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::File_Input] = tmp = new Fl_Pixmap(flFileInput_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Browser] = tmp = new Fl_Pixmap(flBrowser_xpm); tmp->scale(16, 16);

  pixmap[Fl_Type::ID::Check_Browser] = tmp = new Fl_Pixmap(flCheckBrowser_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::File_Browser] = tmp = new Fl_Pixmap(flFileBrowser_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Clock] = tmp = new Fl_Pixmap(flClock_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Help_View] = tmp = new Fl_Pixmap(flHelp_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Progress] = tmp = new Fl_Pixmap(flProgress_xpm); tmp->scale(16, 16);

  pixmap[Fl_Type::ID::Slider] = tmp = new Fl_Pixmap(flSlider_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Scrollbar] = tmp = new Fl_Pixmap(flScrollBar_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Value_Slider] = tmp = new Fl_Pixmap(flValueSlider_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Adjuster] = tmp = new Fl_Pixmap(flAdjuster_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Counter] = tmp = new Fl_Pixmap(flCounter_xpm); tmp->scale(16, 16);

  pixmap[Fl_Type::ID::Dial] = tmp = new Fl_Pixmap(flDial_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Roller] = tmp = new Fl_Pixmap(flRoller_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Value_Input] = tmp = new Fl_Pixmap(flValueInput_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Value_Output] = tmp = new Fl_Pixmap(flValueOutput_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Comment] = tmp = new Fl_Pixmap(flComment_xpm); tmp->scale(16, 16);

  pixmap[Fl_Type::ID::Spinner] = tmp = new Fl_Pixmap(flSpinner_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Widget_Class] = tmp = new Fl_Pixmap(flWidgetClass_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Data] = tmp = new Fl_Pixmap(flData_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Tree] = tmp = new Fl_Pixmap(flTree_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Table] = tmp = new Fl_Pixmap(flTable_xpm); tmp->scale(16, 16);

  pixmap[Fl_Type::ID::Simple_Terminal] = tmp = new Fl_Pixmap(flSimpleTerminal_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Input_Choice] = tmp = new Fl_Pixmap(flInputChoice_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Checkbox_Menu_Item] = tmp = new Fl_Pixmap(flCheckMenuitem_xpm); tmp->scale(16, 16);
  pixmap[Fl_Type::ID::Radio_Menu_Item] = tmp = new Fl_Pixmap(flRadioMenuitem_xpm); tmp->scale(16, 16);

  pixmap[Fl_Type::ID::Flex] = tmp = new Fl_Pixmap(flFlex_xpm); tmp->scale(16, 16);
}

