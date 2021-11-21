include (ProjectVersioning)

function (generate_versioning_information
    TARGET_NAME i_target_name
    EXPORT_HEADER i_export_header
    EXPORT_MACRO i_export_macro
    VERSIONED_ENTITY i_versioned_entity
    INCLUDE_PREFIX i_include_prefix
    COMPANY_NAME i_company_name
    COMPANY_COPYRIGHT i_company_copyright
    FILE_DESCRIPTION i_file_description
)
    target_version_information (
        TARGET_NAME ${i_target_name}
        EXPORT_HEADER ${i_export_header}
        EXPORT_MACRO ${i_export_macro}
        VERSIONED_ENTITY ${i_versioned_entity}
    )

    if (WIN32)
        include (generate_product_version)

        get_target_property (gitDescribe
        ${i_target_name} GIT_DESCRIBE)

        get_target_property (gitUntracked
        ${i_target_name} GIT_UNTRACKED_FILES)

        generate_product_version (
            win32VersionInfoFiles
            NAME ${i_versioned_entity}
            VERSION_MAJOR ${${i_versioned_entity}_VERSION_MAJOR}
            VERSION_MINOR ${${i_versioned_entity}_VERSION_MINOR}
            VERSION_PATCH ${${i_versioned_entity}_VERSION_PATCH}
            COMPANY_NAME ${i_company_name}
            COMPANY_COPYRIGHT ${i_company_copyright}
            COMMENTS "${gitDescribe}${gitUntracked}"
            FILE_DESCRIPTION ${i_file_description}
        )

        target_sources (${i_target_name} PRIVATE ${win32VersionInfoFiles})
    endif (WIN32)
endfunction (generate_versioning_information)
