SET(CPPFILES
	Fl_Function_Type.cxx
	Fl_Menu_Type.cxx
	Fl_Group_Type.cxx
	Fl_Widget_Type.cxx
	Fl_Type.cxx
	Fl_Window_Type.cxx
	Fluid_Image.cxx
	code.cxx
	factory.cxx
	file.cxx
      fluid.cxx
	align_widget.cxx
	about_panel.cxx
	widget_panel.cxx
	alignment_panel.cxx
	function_panel.cxx
)


ADD_EXECUTABLE(fluid ${CPPFILES})
TARGET_LINK_LIBRARIES(fluid fltk fltk_images fltk_forms ${FLTK_PLATFORM_DEPENDENT_LIBS})
IF(OPENGL_FOUND)
	TARGET_LINK_LIBRARIES(fluid fltk_gl)
ENDIF(OPENGL_FOUND)

INSTALL_TARGETS(/bin fluid)
