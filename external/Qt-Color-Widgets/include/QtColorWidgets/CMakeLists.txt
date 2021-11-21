set (HEADERS
abstract_widget_list.hpp
bound_color_selector.hpp
color_2d_slider.hpp
color_delegate.hpp
color_dialog.hpp
color_line_edit.hpp
color_list_widget.hpp
color_names.hpp
color_palette.hpp
color_palette_model.hpp
color_palette_widget.hpp
color_preview.hpp
color_selector.hpp
color_wheel.hpp
colorwidgets_global.hpp
gradient_slider.hpp
hue_slider.hpp
swatch.hpp
gradient_editor.hpp
harmony_color_wheel.hpp
gradient_list_model.hpp
gradient_delegate.hpp
)

file(RELATIVE_PATH
  PREFIX
  ${PROJECT_SOURCE_DIR}
  ${CMAKE_CURRENT_LIST_DIR})


foreach (HEADER IN LISTS HEADERS)
  target_sources (${TARGET_NAME}
    PRIVATE
    $<INSTALL_INTERFACE:${PREFIX}/${HEADER}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${HEADER}>)
endforeach (HEADER IN HEADERS)
