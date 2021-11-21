function (install_project
    PROJECT_NAME i_project_name
	TARGET_NAME i_target_name
	TARGET_OUTPUT_SUFFIX i_target_output_suffix
    EXPORT_HEADER i_export_header
    INCLUDE_PREFIX i_include_prefix
    HEADER_MATCHING_REGEX i_header_matching_regex
    VERSION_HEADER i_version_header
    NAMESPACE i_namespace)
  install (TARGETS ${i_target_name}
    EXPORT ${i_target_name}
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)

  install (DIRECTORY include/
    DESTINATION include
    FILES_MATCHING
    REGEX ${i_header_matching_regex}
    REGEX "CMakeLists\.txt" EXCLUDE)

  install (FILES ${i_version_header}
    DESTINATION include/${i_include_prefix}
    COMPONENT Devel)

  install (FILES include/${i_export_header}
    DESTINATION include/${i_include_prefix}
    COMPONENT Devel)

  install(
    EXPORT ${i_target_name}
    DESTINATION lib/cmake/${i_target_name}${i_target_output_suffix}
    FILE "${i_target_name}${i_target_output_suffix}.cmake"
    COMPONENT Devel)


  include(CMakePackageConfigHelpers)

  string (TOLOWER ${i_target_name} CMAKE_BASE_FILE_NAME)
  string (TOLOWER "${i_target_output_suffix}" CMAKE_FILE_OUTPUT_SUFFIX)

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${i_target_name}/${CMAKE_BASE_FILE_NAME}${CMAKE_FILE_OUTPUT_SUFFIX}-config-version.cmake"
    VERSION ${${i_project_name}_VERSION}
    COMPATIBILITY SameMajorVersion
    )

  export(EXPORT ${i_target_name}
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${i_target_name}/${CMAKE_BASE_FILE_NAME}${CMAKE_FILE_OUTPUT_SUFFIX}-targets.cmake"
    )

  configure_file("cmake/${i_project_name}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${i_target_name}/${CMAKE_BASE_FILE_NAME}${CMAKE_FILE_OUTPUT_SUFFIX}-config.cmake"
    @ONLY
    )

  set(ConfigPackageLocation "lib/cmake/${i_target_name}${i_target_output_suffix}")
  install(EXPORT ${i_target_name}
    FILE
    "${CMAKE_BASE_FILE_NAME}${CMAKE_FILE_OUTPUT_SUFFIX}-targets.cmake"
    NAMESPACE
    "${i_namespace}"
    DESTINATION
    ${ConfigPackageLocation}
    )
  install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${i_target_name}/${CMAKE_BASE_FILE_NAME}${CMAKE_FILE_OUTPUT_SUFFIX}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${i_target_name}/${CMAKE_BASE_FILE_NAME}${CMAKE_FILE_OUTPUT_SUFFIX}-config-version.cmake"
    DESTINATION
    ${ConfigPackageLocation}
    COMPONENT
    Devel
    )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${i_project_name}.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${i_target_name}${i_target_output_suffix}.pc
    @ONLY
    )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${i_target_name}${i_target_output_suffix}.pc
    DESTINATION
    lib/pkgconfig
    )
endfunction (install_project)
