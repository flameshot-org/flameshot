define_property (TARGET
  PROPERTY GIT_DESCRIBE
  BRIEF_DOCS "Advanced version info for developers"
  FULL_DOCS "String with information that is important for developers during
  development process. This information includes git commit hash, durty status
  of repo, distance from the last tag.")

define_property (TARGET
  PROPERTY GIT_UNTRACKED_FILES
  BRIEF_DOCS "Information about presence of untracked files"
  FULL_DOCS "Used in helper functions generation to add .with-untracked suffix
  to version string. Suffix is only added if there are some untracked not
  ignored files in repository.")

set(HERE_DIR ${CMAKE_CURRENT_LIST_DIR})


function (target_version_information
	TARGET_NAME i_target_name
    EXPORT_HEADER i_export_header
    EXPORT_MACRO i_export_macro
    VERSIONED_ENTITY i_versioned_entity)

  find_file (
	headerFileTemplate
	"ProjectVersioning/version.h.in"
	PATHS ${CMAKE_MODULE_PATH})

    if ( NOT ${headerFileTemplate} )
        set(headerFileTemplate "${HERE_DIR}/ProjectVersioning/version.h.in")
    endif()

  find_file (
	sourceFileTemplate
	"ProjectVersioning/version.c.in"
	PATHS ${CMAKE_MODULE_PATH})

    if ( NOT ${sourceFileTemplate} )
        set(sourceFileTemplate "${HERE_DIR}/ProjectVersioning/version.c.in")
    endif()

  exec_program (
	"git"
	${CMAKE_SOURCE_DIR}
	ARGS "describe --always --dirty --long --tags"
	OUTPUT_VARIABLE gitDescribe)

  exec_program (
	"git"
	${CMAKE_SOURCE_DIR}
	ARGS "ls-files --others --exclude-standard"
	OUTPUT_VARIABLE gitUntracked)

  if (gitUntracked)
	set (gitUntracked ".with-untracked")
  endif (gitUntracked)

  configure_file (
	"${headerFileTemplate}"
	"${CMAKE_CURRENT_BINARY_DIR}/${i_versioned_entity}_version.h")

  configure_file(
	"${sourceFileTemplate}"
	"${CMAKE_BINARY_DIR}/${i_versioned_entity}_version.c")

  target_sources ("${i_target_name}"
    PRIVATE
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/${i_versioned_entity}_version.h>
    $<INSTALL_INTERFACE:include/${i_include_prefix}/${i_versioned_entity}_version.h>
	PRIVATE
	"${CMAKE_BINARY_DIR}/${i_versioned_entity}_version.c")

  set_target_properties (${i_target_name}
	PROPERTIES
	GIT_DESCRIBE "${gitDescribe}"
	GIT_UNTRACKED_FILES "${gitUntracked}")

  unset (headerFileTemplate PARENT_SCOPE)
  unset (sourceFileTemplate PARENT_SCOPE)
endfunction (target_version_information)
