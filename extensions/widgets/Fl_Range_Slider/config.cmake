
list(APPEND FLTK_EXT_CPP_FILES
  ${EXT_WIDGET_DIR}/src/Fl_Range_Slider.cxx
)

list(APPEND FLTK_EXT_HEADER_FILES
  ${EXT_WIDGET_DIR}/FL/Fl_Range_Slider.H
)

list(APPEND FLTK_EXT_INCLUDE_DIRECTORIES
  ${EXT_WIDGET_DIR}
)

list(APPEND FLTK_EXT_TESTS
  FL_RANGE_SLIDER_TEST
)

list(APPEND FLTK_EXT_FLUID_FILES
  ${EXT_WIDGET_DIR}/fluid/Fl_Range_Slider_Type.cxx
)

set (FL_RANGE_SLIDER_TEST_EXE rangeslider)
set (FL_RANGE_SLIDER_TEST_DEMO "Fl_Range_Slider\nDemo\nProgram")
set (FL_RANGE_SLIDER_TEST_CPP_FILES ${EXT_WIDGET_DIR}/test/rangeslider.cxx)
set (FL_RANGE_SLIDER_TEST_LIBS "fltk_images;fltk")

message (STATUS "Adding Widget Extension: Fl_Range_Slider")
