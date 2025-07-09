cmake_minimum_required(VERSION 3.16)

include(cmake/vcpkg.cmake)

option(SAMPLE_BUILD_TESTS "Build tests" ON)
if(SAMPLE_BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(sample
    DESCRIPTION "Example of creating a library"
    VERSION 1.0.0
    LANGUAGES C CXX)

option(SAMPLE_MSVC_DYNAMIC_RUNTIME "Link dynamic runtime library instead of static" OFF)
option(SAMPLE_ENABLE_INSTALL "Enable installation" ON)
option(SAMPLE_BUILD_PACKAGES "Build packages" OFF)

find_package(idlc CONFIG REQUIRED)
idlc_compile(NAME api WARN_AS_ERRORS
    SOURCE "${PROJECT_SOURCE_DIR}/specs/api.idl"
    OUTPUT "${PROJECT_SOURCE_DIR}/include/sample/sample.h"
    VERSION ${PROJECT_VERSION}
    GENERATOR C)

add_library(sample src/sample.c ${IDLC_api_OUTPUTS})
add_library(sample::sample ALIAS sample)
target_include_directories(sample PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
set_target_properties(sample PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS OFF)
target_compile_definitions(sample PRIVATE _CRT_SECURE_NO_WARNINGS)
if(BUILD_SHARED_LIBS)
    set_target_properties(sample PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
    set_target_properties(sample PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
else()
    target_compile_definitions(sample PUBLIC SAMPLE_STATIC_BUILD)
endif()
if(MSVC)
    if(SAMPLE_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(sample PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(sample PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

if(EMSCRIPTEN)
    idlc_compile(NAME api_js WARN_AS_ERRORS
        SOURCE "${PROJECT_SOURCE_DIR}/specs/api.idl"
        OUTPUT "${PROJECT_BINARY_DIR}/sample.js.cpp"
        VERSION ${PROJECT_VERSION}
        GENERATOR JS)

    set(SAMPLE_JS_LINK_OPTIONS -sWASM=1 -sMODULARIZE=1 -sALLOW_MEMORY_GROWTH=1 -sEXPORT_NAME=sample --emit-tsd sample.d.ts)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND SAMPLE_JS_LINK_OPTIONS "-sSINGLE_FILE=0")
    else()
        list(APPEND SAMPLE_JS_LINK_OPTIONS "-sSINGLE_FILE=1")
    endif()

    add_executable(samplejs ${IDLC_api_js_OUTPUTS})
    target_link_libraries(samplejs PRIVATE embind sample::sample)
    target_include_directories(samplejs PRIVATE "${PROJECT_SOURCE_DIR}/include/sample/")
    target_compile_features(samplejs PRIVATE cxx_std_20)
    target_link_options(samplejs PRIVATE ${SAMPLE_JS_LINK_OPTIONS})
    set_target_properties(samplejs PROPERTIES OUTPUT_NAME "sample" SUFFIX ".js" RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/dist/")

    add_executable(samplejsesm ${IDLC_api_js_OUTPUTS})
    target_link_libraries(samplejsesm PRIVATE embind sample::sample)
    target_include_directories(samplejsesm PRIVATE "${PROJECT_SOURCE_DIR}/include/sample/")
    target_compile_features(samplejsesm PRIVATE cxx_std_20)
    target_link_options(samplejsesm PRIVATE ${SAMPLE_JS_LINK_OPTIONS} -sEXPORT_ES6=1)
    set_target_properties(samplejsesm PROPERTIES OUTPUT_NAME "sample.esm" SUFFIX ".js" RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/dist/")
endif()

if(SAMPLE_BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif()

if(SAMPLE_ENABLE_INSTALL)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in" 
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}"
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO)

    write_basic_package_version_file(${PROJECT_NAME}-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets)
    install(EXPORT ${PROJECT_NAME}-targets 
        DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}"
        NAMESPACE ${PROJECT_NAME}::)
    install(
        FILES
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME})
    install(DIRECTORY include/ DESTINATION include)
endif()

if(SAMPLE_BUILD_PACKAGES)
    include(cmake/package.cmake)
endif()
