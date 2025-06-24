cmake_minimum_required(VERSION 3.16)

include(cmake/version.cmake)
include(cmake/vcpkg.cmake)

option(IDLC_BUILD_TOOLS "Build the idl compiler tool" ON)
if(IDLC_BUILD_TOOLS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tools")
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
if(IDLC_BUILD_TOOLS)
    find_package(argparse CONFIG REQUIRED)
endif()

bison_target(parser src/idl.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
flex_target(scanner src/idl.ll ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)
add_flex_bison_dependency(scanner parser)

add_library(idl
    src/compiler.cpp
    ${BISON_parser_OUTPUTS}
    ${FLEX_scanner_OUTPUTS})
add_library(idlc::idl ALIAS idl)
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

if(IDLC_BUILD_TOOLS)
    add_executable(${PROJECT_NAME}
        src/main.cpp
        src/generator_c.cpp
        ${BISON_parser_OUTPUTS}
        ${FLEX_scanner_OUTPUTS})
    add_executable(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse)
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)
    target_link_libraries(${PROJECT_NAME} PRIVATE xxHash::xxhash)
    target_link_libraries(${PROJECT_NAME} PRIVATE magic_enum::magic_enum)
    target_link_libraries(${PROJECT_NAME} PRIVATE idlc::idl)
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

if(IDLC_BUILD_PACKAGES)
    include(cmake/package.cmake)
endif()
