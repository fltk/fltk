
set(CPPFILES
	CodeEditor.cxx
	Fl_Function_Type.cxx
	Fl_Group_Type.cxx
	Fl_Menu_Type.cxx
	Fl_Type.cxx
	Fl_Widget_Type.cxx
	Fl_Window_Type.cxx
	Fluid_Image.cxx
	about_panel.cxx
	align_widget.cxx
	alignment_panel.cxx
	code.cxx
	factory.cxx
	file.cxx
	fluid.cxx
	function_panel.cxx
	template_panel.cxx
	undo.cxx
	widget_panel.cxx
)

add_executable(fluid ${CPPFILES})
target_link_libraries(fluid fltk fltk_images fltk_forms)

# link in optional libraries
if(FLTK_HAVE_CAIRO)
   target_link_libraries(fluid fltk_cairo)
endif(FLTK_HAVE_CAIRO)

if(FLTK_USE_GL)
   target_link_libraries(fluid fltk_gl)
   target_link_libraries(fluid ${OPENGL_LIBRARIES})
endif(FLTK_USE_GL)

if(USE_XFT)
   target_link_libraries(fluid ${X11_Xft_LIB})
endif(USE_XFT)

if(HAVE_XINERAMA)
   target_link_libraries(fluid ${X11_Xinerama_LIB})
endif(HAVE_XINERAMA)

install(TARGETS fluid
   EXPORT fltk-install
   DESTINATION ${PREFIX_BIN}
)
