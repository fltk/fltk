#include <FL/Fl_Dockable_Group.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Round_Clock.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tile.H>

void delete_win(Fl_Window *win) {
  delete win;
}


int main(int argc, char **argv) {
  Fl_Window *source = new Fl_Window(150, 150, 340, 180, "source (tab)");
  source->callback((Fl_Callback0*)delete_win);
  Fl_Tabs *tabs = new Fl_Tabs(20, 40, 310, 115);
  Fl_Dockable_Group *dock = new Fl_Dockable_Group(25, 60, 305, 95, "Fl_Dockable_Group-1");
  dock->color(FL_YELLOW);
  dock->box(FL_THIN_UP_BOX);
  new Fl_Round_Clock(160, 90, 60, 60);
  new Fl_Clock(245, 90, 60, 60);
  Fl_Box *r = new Fl_Box(FL_NO_BOX, 0, 151, 155, 10, NULL);
  dock->end();
  dock->resizable(r);
  new Fl_Box(FL_FLAT_BOX, 25, 60, 305, 95, "Tab 2");
  tabs->end();
  source->end();
  source->resizable(dock);
  dock->command_box(27, 67, 300, 13);
  dock->command_box()->labelsize(10);
  source->show();
  
  source = new Fl_Window(source->x() + source->w() + 50, source->y(), 340, 180, "source (group)");
  source->callback((Fl_Callback0*)delete_win);
  dock = new Fl_Dockable_Group(25, 60, 305, 95, "Fl_Dockable_Group-2");
  dock->color(FL_GREEN);
  dock->box(FL_THIN_UP_BOX);
  new Fl_Clock(160, 65, 60, 60);
  new Fl_Round_Clock(245, 65, 60, 60);
  r = new Fl_Box(FL_NO_BOX, 0, 126, 155, 10, NULL);
  dock->end();
  dock->resizable(r);
  source->end();
  source->resizable(dock);
  dock->command_box(27, 140, 60, 13);
  dock->command_box()->labelsize(10);
  source->show();

  Fl_Window *destination = new Fl_Window(150, source->y() + source->h() + 50,
                                         340, 190, "destination (tab)");
  destination->callback((Fl_Callback0*)delete_win);
  new Fl_Input(5,5,100,30, NULL);
  tabs = new Fl_Tabs(5, 40, 310, 140);
  new Fl_Box(FL_FLAT_BOX, 10, 60, 300, 120, "Tab 1");
  Fl_Dockable_Group::target_box(10, 60, 300, 120, tabs);
  tabs->end();
  tabs->value(dock);
  destination->end();
  destination->show(argc, argv);

  destination = new Fl_Window(destination->x() + destination->w() + 50, source->y() + source->h() + 50,
                              340, 190, "destination (group)");
  destination->callback((Fl_Callback0*)delete_win);
  destination->end();
  Fl_Dockable_Group::target_box(10, 30, 320, 150, destination);
  destination->show();
  
  source = new Fl_Window(source->x()+source->w()+30, source->y(), 340, 180, "source (Fl_Tile)");
  Fl_Tile *tile = new Fl_Tile(5, 5, 330, 160, "");
  Fl_Box *yellow = new Fl_Box(FL_DOWN_BOX, 5, 5, 250, 160, "");
  yellow->color(FL_YELLOW);
  dock = new Fl_Dockable_Group(255, 5, 80, 160, "");
  dock->box(FL_DOWN_BOX);
  dock->color(FL_BLUE);
  dock->command_box(260, 10, 70, 20);
  dock->begin();
  new Fl_Round_Clock(300, 130, 30, 30);
  dock->resizable(new Fl_Box(260, 40, 30, 50, ""));
  dock->end();
  tile->end();
  source->end();
  source->resizable(tile);
  
  source->show();
  return Fl::run();
}
