set (SOURCES
abstract_widget_list.cpp
bound_color_selector.cpp
color_2d_slider.cpp
color_delegate.cpp
color_dialog.cpp
color_dialog.ui
color_line_edit.cpp
color_list_widget.cpp
color_names.cpp
color_palette.cpp
color_palette_model.cpp
color_palette_widget.cpp
color_palette_widget.ui
color_preview.cpp
color_selector.cpp
color_utils.cpp
color_wheel.cpp
gradient_slider.cpp
hue_slider.cpp
swatch.cpp
gradient_editor.cpp
harmony_color_wheel.cpp
gradient_list_model.cpp
)

file(RELATIVE_PATH
  PREFIX
  ${PROJECT_SOURCE_DIR}
  ${CMAKE_CURRENT_LIST_DIR})


foreach (SOURCE IN LISTS SOURCES)
  target_sources (${TARGET_NAME}
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${SOURCE}>)
endforeach (SOURCE IN SOURCES)
