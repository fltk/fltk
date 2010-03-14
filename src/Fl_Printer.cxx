#include <FL/Fl_Printer.H>

#ifdef __APPLE__
#include <src/Fl_Quartz_Printer.mm>
#elif defined(WIN32)
#include <src/Fl_GDI_Printer.cxx>
#endif

#include <src/Fl_PS_Printer.cxx>
