cmake_minimum_required(VERSION 3.16)
project(RangeCloud)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(Config.cmake)

set(DEBUG_EXT "")
set(PROJECT_HUMAN_REL_NAME "${PROJECT_HUMAN_NAME}")
set(PROJECT_SHORT_REL_NAME "${PROJECT_SHORT_NAME}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_EXT "_debug")
    add_compile_definitions(DEBUG)
    set(PROJECT_HUMAN_REL_NAME "${PROJECT_HUMAN_REL_NAME} (${CMAKE_BUILD_TYPE})")
    set(PROJECT_SHORT_REL_NAME "${PROJECT_SHORT_REL_NAME} (${CMAKE_BUILD_TYPE})")
endif()

set(BUILD_SHARED_LIBS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent HttpServer Network LinguistTools)
qt_standard_project_setup()

add_library(common_defines INTERFACE)
target_compile_definitions(common_defines
    INTERFACE
        VERSION="${PROJECT_VERSION}"
        YEAR=${PROJECT_YEAR}
        PACKAGENAME="${PROJECT_PACKAGE_NAME}"
        NAME="${PROJECT_HUMAN_REL_NAME}"
        SHORTNAME="${PROJECT_SHORT_NAME}"
        FAMILYNAME="${PROJECT_FAMILY_NAME}"
        TITLE="${PROJECT_TITLE}"
        DESCRIPTION="${PROJECT_DESCRIPTION}"
        AUTHOR="${PROJECT_AUTHOR}"
        EMAIL="${PROJECT_EMAIL}"
        WWW="${PROJECT_WWW}"
        WWWDOMAIN="${PROJECT_WWW_DOMAIN}"
)

function(set_debug_suffix_if_needed target_name)
    if(CMAKE_CONFIGURATION_TYPES)  # Multi-config (e.g. Visual Studio, Xcode)
        set_target_properties(${target_name} PROPERTIES
            OUTPUT_NAME_DEBUG "${target_name}${DEBUG_EXT}"
            OUTPUT_NAME_RELEASE "${target_name}"
        )
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")  # Single-config (e.g. Make, Ninja)
        set_target_properties(${target_name} PROPERTIES
            OUTPUT_NAME "${target_name}${DEBUG_EXT}"
        )
    else()
        set_target_properties(${target_name} PROPERTIES
            OUTPUT_NAME "${target_name}"
        )
    endif()
endfunction()

set(all_targets range-base-lib range-cloud-lib cloud-tool cloud)

foreach(tgt IN LISTS all_targets)
    add_subdirectory(${tgt})
    set_debug_suffix_if_needed(${tgt})
endforeach()

# Deploy Qt -------------------------------------------------------------------

get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

set(_qt_deploy_dir "$<TARGET_FILE_DIR:${PROJECT_TARGET_NAME}>/qtlib")

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    set(DEPLOY_COMMAND
        ${WINDEPLOYQT_EXECUTABLE}
        $<TARGET_FILE:${PROJECT_TARGET_NAME}>
        --dir ${_qt_deploy_dir}
    )
elseif(APPLE)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")
    set(DEPLOY_COMMAND
        ${MACDEPLOYQT_EXECUTABLE}
        $<TARGET_BUNDLE_DIR:${PROJECT_TARGET_NAME}>
    )
    if(DEFINED APPLE_ID)
        set(DEPLOY_COMMAND ${DEPLOY_COMMAND} -codesign=${APPLE_ID})
    endif()
else()
    get_filename_component(_qt_dir "${_qt_bin_dir}" DIRECTORY)
    set(DEPLOY_COMMAND
        ${CMAKE_SOURCE_DIR}/range-build-tools/linux_deploy_qt.sh
        --executable=$<TARGET_FILE:${PROJECT_TARGET_NAME}>
        --install-to=${_qt_deploy_dir}
        --only-qt
        --only-qt-core
        --qt-path=${_qt_dir}
    )
endif()

add_custom_target(deploy-qt ALL
    COMMAND ${DEPLOY_COMMAND}
    DEPENDS cloud
    COMMENT "Deploying QT libraries"
    VERBATIM
)

# Install ---------------------------------------------------------------------

if(WIN32)
    install(TARGETS cloud DESTINATION bin COMPONENT Application)
    install(TARGETS cloud-tool DESTINATION bin COMPONENT Application)
    install(DIRECTORY range-ca/ DESTINATION range-ca COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/doc/ DESTINATION doc COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/processes/ DESTINATION processes COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/scripts/ DESTINATION scripts COMPONENT Application)
    install(DIRECTORY ${_qt_deploy_dir}/ DESTINATION bin COMPONENT Application)
    install(FILES ${CMAKE_BINARY_DIR}/LICENSE.txt DESTINATION doc COMPONENT Application)
elseif(APPLE)
    install(TARGETS cloud BUNDLE DESTINATION . COMPONENT Application)
#    install(TARGETS cloud-tool BUNDLE DESTINATION bin COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/help/ DESTINATION $<TARGET_BUNDLE_DIR_NAME:${PROJECT_TARGET_NAME}>/Contents/Resources/help)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/doc/ DESTINATION $<TARGET_BUNDLE_DIR_NAME:${PROJECT_TARGET_NAME}>/Contents/Resources/doc)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/etc/ DESTINATION $<TARGET_BUNDLE_DIR_NAME:${PROJECT_TARGET_NAME}>/Contents/Resources/etc)
    install(FILES ${CMAKE_BINARY_DIR}/LICENSE.txt DESTINATION doc COMPONENT Application)
else()
    set(CMAKE_INSTALL_PREFIX "/opt/${PROJECT_FAMILY_NAME}/${PROJECT_SHORT_REL_NAME}" CACHE PATH "Install path prefix" FORCE)

    install(TARGETS cloud DESTINATION bin COMPONENT Application)
    install(TARGETS cloud-tool DESTINATION bin COMPONENT Application)
    install(DIRECTORY range-ca/ DESTINATION range-ca USE_SOURCE_PERMISSIONS COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/doc/ DESTINATION doc COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/processes/ DESTINATION processes COMPONENT Application)
    install(DIRECTORY ${PROJECT_TARGET_NAME}/scripts/ DESTINATION scripts USE_SOURCE_PERMISSIONS COMPONENT Application)
    install(DIRECTORY ${_qt_deploy_dir}/ DESTINATION . COMPONENT Application)
    install(FILES ${CMAKE_BINARY_DIR}/LICENSE.txt DESTINATION doc COMPONENT Application)
endif()

# CPack -----------------------------------------------------------------------

set(CPACK_COMPONENTS_ALL
    Application
)

set(CPACK_PACKAGE_NAME "${PROJECT_PACKAGE_NAME}${DEBUG_EXT}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_RELEASE "1")
set(CPACK_PACKAGE_CONTACT "${PROJECT_EMAIL}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_WWW}")
set(CPACK_SYSTEM_NAME "${CMAKE_HOST_SYSTEM_NAME}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

if(APPLE)
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE.rtf")
else()
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE.txt")
endif()
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/../README.md")

set(CPACK_GENERATOR "ZIP;TGZ;STGZ;DEB;RPM")

# DEB
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")

# RPM
set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")

# Convert files ---------------------------------------------------------------

file(READ "${CMAKE_SOURCE_DIR}/../LICENSE" LICENSE_TEXT)

configure_file(
    ${CMAKE_SOURCE_DIR}/range-build-tools/LICENSE.txt.in
    ${CMAKE_BINARY_DIR}/LICENSE.txt
    @ONLY
)

# -----------------------------------------------------------------------------

include(InstallRequiredSystemLibraries)
include(CPack)
