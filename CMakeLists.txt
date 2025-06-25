cmake_minimum_required(VERSION 3.16)

include(cmake/version.cmake)
include(cmake/vcpkg.cmake)

option(IDLC_BUILD_TOOL "Build the idl compiler tool" ON)
if(IDLC_BUILD_TOOL)
    list(APPEND VCPKG_MANIFEST_FEATURES "tool")
endif()

project(idlc
    HOMEPAGE_URL "https://github.com/VladimirShaleev/idlc"
    DESCRIPTION "Interface Definition Language Compiler"
    VERSION ${IDLC_VERSION}
    LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(CMakeDependentOption)
include(cmake/flex-install.cmake)
include(cmake/bison-install.cmake)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(IDLC_MSVC_DYNAMIC_RUNTIME "Link dynamic runtime library instead of static" OFF)
cmake_dependent_option(IDLC_BUILD_PACKAGES "Build packages" ON
    "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

find_package(fmt CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
if(IDLC_BUILD_TOOL)
    find_package(argparse CONFIG REQUIRED)
endif()
if(NOT IDLC_PACKAGE_NOT_DOUND)
    find_package(idlc CONFIG)
endif()

bison_target(parser src/idl.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
flex_target(scanner src/idl.ll ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)
add_flex_bison_dependency(scanner parser)

if(idlc_FOUND AND NOT IDLC_PACKAGE_NOT_DOUND)
    idlc_compile(NAME api WARN_AS_ERRORS
        SOURCE "${PROJECT_SOURCE_DIR}/specs/api.idl"
        OUTPUT "${PROJECT_SOURCE_DIR}/include/idlc/idl.h"
        VERSION ${PROJECT_VERSION}
        GENERATOR C
        DEPENDS
            "${PROJECT_SOURCE_DIR}/specs/api.idl"
            "${PROJECT_SOURCE_DIR}/specs/options.idl"
            "${PROJECT_SOURCE_DIR}/specs/results.idl")
else()
    option(IDLC_PACKAGE_NOT_DOUND "Cache flag indicating that the idlc was not found." ON)
endif()

add_library(idl
    src/compiler.cpp
    src/generator_c.cpp
    ${IDLC_api_OUTPUTS}
    ${BISON_parser_OUTPUTS}
    ${FLEX_scanner_OUTPUTS})
target_link_libraries(idl PRIVATE fmt::fmt-header-only)
target_link_libraries(idl PRIVATE xxHash::xxhash)
target_link_libraries(idl PRIVATE magic_enum::magic_enum)
target_include_directories(idl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(idl PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(idl PRIVATE "$<$<BOOL:${FLEX_INCLUDE_DIR}>:${FLEX_INCLUDE_DIR}>")
target_include_directories(idl PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_definitions(idl PRIVATE $<$<CONFIG:Debug>:YYDEBUG>)
target_compile_features(idl PRIVATE cxx_std_20)
set_target_properties(idl PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS OFF)
if(BUILD_SHARED_LIBS)
    set_target_properties(idl PROPERTIES 
        VERSION ${PROJECT_VERSION} 
        SOVERSION ${PROJECT_VERSION_MAJOR})
    set_target_properties(idl PROPERTIES
        CXX_VISIBILITY_PRESET hidden 
        VISIBILITY_INLINES_HIDDEN ON)
else()
    target_compile_definitions(idl PUBLIC IDL_STATIC_BUILD)
endif()
if(MSVC)
    if(IDLC_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(idl PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(idl PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()
set(IDLC_TARGETS idl)

if(IDLC_BUILD_TOOL)
    add_executable(${PROJECT_NAME} src/main.cpp)
    add_executable(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse)
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)
    target_link_libraries(${PROJECT_NAME} PRIVATE xxHash::xxhash)
    target_link_libraries(${PROJECT_NAME} PRIVATE magic_enum::magic_enum)
    target_link_libraries(${PROJECT_NAME} PRIVATE idl)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_include_directories(${PROJECT_NAME} PRIVATE "$<$<BOOL:${FLEX_INCLUDE_DIR}>:${FLEX_INCLUDE_DIR}>")
    target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:YYDEBUG>)
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
        WINDOWS_EXPORT_ALL_SYMBOLS OFF)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        VERSION ${PROJECT_VERSION} 
        SOVERSION ${PROJECT_VERSION_MAJOR})
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden 
        VISIBILITY_INLINES_HIDDEN ON)
    if(MSVC)
        if(IDLC_MSVC_DYNAMIC_RUNTIME)
            set_target_properties(${PROJECT_NAME} PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        else()
            set_target_properties(${PROJECT_NAME} PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
    endif()
    list(APPEND IDLC_TARGETS ${PROJECT_NAME})
endif()
add_library(idlc::idl ALIAS idl)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in" 
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}"
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(TARGETS ${IDLC_TARGETS} EXPORT ${PROJECT_NAME}-targets)
install(EXPORT ${PROJECT_NAME}-targets 
    DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}"
    NAMESPACE ${PROJECT_NAME}::)
install(
    FILES
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)

if(IDLC_BUILD_PACKAGES)
    include(cmake/package.cmake)
endif()

if(TARGET ${PROJECT_NAME})
    function(idlc_compile)
        set(options WARN_AS_ERRORS)
        set(oneValueArgs NAME GENERATOR SOURCE OUTPUT VERSION)
        set(multiValueArgs IMPORTS DEPENDS)
        cmake_parse_arguments(PARSE_ARGV 0 IDLC_ARG "${options}" "${oneValueArgs}" "${multiValueArgs}")
        set(IDLC_ARG_CMD "")
        if(IDLC_ARG_WARN_AS_ERRORS)
            list(APPEND IDLC_ARG_CMD "-w")
        endif()
        if(IDLC_ARG_VERSION)
            list(APPEND IDLC_ARG_CMD "--apiver" ${IDLC_ARG_VERSION})
        endif()
        foreach(IMPORT ${IDLC_ARG_IMPORTS})
            list(APPEND IDLC_ARG_CMD "-i" "\"${IMPORT}\"")
        endforeach()
        string(TOLOWER ${IDLC_ARG_GENERATOR} IDLC_ARG_GEN)
        set(IDLC_TARGET_OUTPUTS ${IDLC_ARG_OUTPUT})
        if(IDLC_ARG_GEN STREQUAL "c")
            get_filename_component(IDLC_ARG_OUTPUT "${IDLC_ARG_OUTPUT}/../" ABSOLUTE)
        endif()
        add_custom_command(OUTPUT ${IDLC_TARGET_OUTPUTS}
            COMMAND ${PROJECT_NAME}::${PROJECT_NAME} ${IDLC_ARG_CMD} -g ${IDLC_ARG_GEN} -o ${IDLC_ARG_OUTPUT} ${IDLC_ARG_SOURCE}
            VERBATIM
            DEPENDS ${IDLC_ARG_SOURCE} ${IDLC_ARG_DEPENDS}
            COMMENT "[idlc] Building IDL")
        set(IDLC_${IDLC_ARG_NAME}_OUTPUTS ${IDLC_TARGET_OUTPUTS} PARENT_SCOPE)  
    endfunction()
endif()
