SET(CPPFILES
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


ADD_EXECUTABLE(fluid ${CPPFILES})
INSTALL_TARGETS(/bin fluid)
TARGET_LINK_LIBRARIES(fluid fltk fltk_images fltk_forms ${FLTK_PLATFORM_DEPENDENT_LIBS})
IF(OPENGL_FOUND)
	TARGET_LINK_LIBRARIES(fluid fltk_gl)
ENDIF(OPENGL_FOUND)

