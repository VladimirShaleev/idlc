include(ExternalProject)
if(WIN32)
    message(STATUS "flex not found, try download flex from winflexbison ${WINFLEXBISON_VERSION_INSTALL}")

    set(WINFLEX_URL "https://github.com/lexxmark/winflexbison/releases/download/v${WINFLEXBISON_VERSION_INSTALL}/win_flex_bison-${WINFLEXBISON_VERSION_INSTALL}.zip")
    set(WINFLEX_INSTALL_DIR "${CMAKE_BINARY_DIR}/winflex")
    set(FLEX_EXECUTABLE "${WINFLEX_INSTALL_DIR}/win_flex.exe")
    set(FLEX_INCLUDE_DIR "${WINFLEX_INSTALL_DIR}")
    
    ExternalProject_Add(
        winflex
        URL ${WINFLEX_URL}
        URL_HASH MD5=6b549d43e34ece0e8ed05af92daa31c4
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND 
            ${CMAKE_COMMAND} -E copy_directory
            <SOURCE_DIR>/
            ${WINFLEX_INSTALL_DIR}
        BUILD_BYPRODUCTS ${FLEX_EXECUTABLE})
    
    add_custom_target(project_flex DEPENDS winflex)
else()
    message(STATUS "flex not found, try download and build flex ${FLEX_VERSION_INSTALL}")
    include(cmake/m4-install.cmake)

    set(FLEX_URL "https://github.com/westes/flex/releases/download/v${FLEX_VERSION_INSTALL}/flex-${FLEX_VERSION_INSTALL}.tar.gz")
    set(FLEX_INSTALL_DIR "${CMAKE_BINARY_DIR}/flex_install")
    set(FLEX_EXECUTABLE "${FLEX_INSTALL_DIR}/bin/flex")
    set(FLEX_INCLUDE_DIR "${FLEX_INSTALL_DIR}/include")

    ExternalProject_Add(
        flex_build
        URL ${FLEX_URL}
        URL_HASH MD5=2882e3179748cc9f9c23ec593d6adc8d
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        DEPENDS ${FLEX_DEPENDS}
        CONFIGURE_COMMAND <SOURCE_DIR>/configure
            --prefix=${FLEX_INSTALL_DIR}
            "CC=${CMAKE_C_COMPILER}"
            "CFLAGS=-O3"
            "M4=${M4_EXECUTABLE}"
        BUILD_COMMAND make -j${NPROC}
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 1
        BUILD_BYPRODUCTS ${FLEX_EXECUTABLE}
        STEP_TARGETS build install)
        
    if(TARGET m4_build)
        add_dependencies(flex_build m4_build)
    endif()
    add_custom_target(project_flex DEPENDS bison_build)
endif()

macro(FLEX_TARGET Name Input Output)
    set(FLEX_TARGET_PARAM_OPTIONS)
    set(FLEX_TARGET_PARAM_ONE_VALUE_KEYWORDS COMPILE_FLAGS DEFINES_FILE)
    set(FLEX_TARGET_PARAM_MULTI_VALUE_KEYWORDS)
    cmake_parse_arguments(
        FLEX_TARGET_ARG
        "${FLEX_TARGET_PARAM_OPTIONS}"
        "${FLEX_TARGET_PARAM_ONE_VALUE_KEYWORDS}"
        "${FLEX_TARGET_MULTI_VALUE_KEYWORDS}"
        ${ARGN})
    set(FLEX_TARGET_usage "FLEX_TARGET(<Name> <Input> <Output> [COMPILE_FLAGS <string>] [DEFINES_FILE <string>]")
    if(NOT "${FLEX_TARGET_ARG_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(SEND_ERROR ${FLEX_TARGET_usage})
    else()
        set(_flex_INPUT "${Input}")
        set(_flex_WORKING_DIR "${CMAKE_CURRENT_BINARY_DIR}")
        if(NOT IS_ABSOLUTE "${_flex_INPUT}")
            set(_flex_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${_flex_INPUT}")
        endif()

        set(_flex_OUTPUT "${Output}")
        if(NOT IS_ABSOLUTE ${_flex_OUTPUT})
            set(_flex_OUTPUT "${_flex_WORKING_DIR}/${_flex_OUTPUT}")
        endif()
        set(_flex_TARGET_OUTPUTS "${_flex_OUTPUT}")

        set(_flex_EXE_OPTS "")
        if(NOT "${FLEX_TARGET_ARG_COMPILE_FLAGS}" STREQUAL "")
            set(_flex_EXE_OPTS "${FLEX_TARGET_ARG_COMPILE_FLAGS}")
            separate_arguments(_flex_EXE_OPTS)
        endif()

        set(_flex_OUTPUT_HEADER "")
        if(NOT "${FLEX_TARGET_ARG_DEFINES_FILE}" STREQUAL "")
            set(_flex_OUTPUT_HEADER "${FLEX_TARGET_ARG_DEFINES_FILE}")
            if(IS_ABSOLUTE "${_flex_OUTPUT_HEADER}")
            set(_flex_OUTPUT_HEADER_ABS "${_flex_OUTPUT_HEADER}")
            else()
            set(_flex_OUTPUT_HEADER_ABS "${_flex_WORKING_DIR}/${_flex_OUTPUT_HEADER}")
            endif()
            list(APPEND _flex_TARGET_OUTPUTS "${_flex_OUTPUT_HEADER_ABS}")
            list(APPEND _flex_EXE_OPTS --header-file=${_flex_OUTPUT_HEADER_ABS})
        endif()

        get_filename_component(_flex_EXE_NAME_WE "${FLEX_EXECUTABLE}" NAME_WE)
        add_custom_command(OUTPUT ${_flex_TARGET_OUTPUTS}
            COMMAND ${FLEX_EXECUTABLE} ${_flex_EXE_OPTS} -o${_flex_OUTPUT} ${_flex_INPUT}
            VERBATIM
            DEPENDS ${_flex_INPUT} project_flex
            COMMENT "[FLEX][${Name}] Building scanner with ${_flex_EXE_NAME_WE} ${FLEX_VERSION}"
            WORKING_DIRECTORY ${_flex_WORKING_DIR})

        set(FLEX_${Name}_DEFINED TRUE)
        set(FLEX_${Name}_OUTPUTS ${_flex_TARGET_OUTPUTS})
        set(FLEX_${Name}_INPUT ${_flex_INPUT})
        set(FLEX_${Name}_COMPILE_FLAGS ${_flex_EXE_OPTS})
        set(FLEX_${Name}_OUTPUT_HEADER ${_flex_OUTPUT_HEADER})
        unset(_flex_EXE_NAME_WE)
        unset(_flex_EXE_OPTS)
        unset(_flex_INPUT)
        unset(_flex_OUTPUT)
        unset(_flex_OUTPUT_HEADER)
        unset(_flex_OUTPUT_HEADER_ABS)
        unset(_flex_TARGET_OUTPUTS)
        unset(_flex_WORKING_DIR)
    endif()
endmacro()

macro(ADD_FLEX_BISON_DEPENDENCY FlexTarget BisonTarget)
    if(NOT FLEX_${FlexTarget}_OUTPUTS)
        message(SEND_ERROR "Flex target `${FlexTarget}' does not exist.")
    endif()
    if(NOT BISON_${BisonTarget}_OUTPUT_HEADER)
        message(SEND_ERROR "Bison target `${BisonTarget}' does not exist.")
    endif()
    set_source_files_properties(${FLEX_${FlexTarget}_OUTPUTS}
        PROPERTIES OBJECT_DEPENDS ${BISON_${BisonTarget}_OUTPUT_HEADER})
endmacro()

set(FLEX_EXECUTABLE ${FLEX_EXECUTABLE} CACHE FILEPATH "Path to Flex executable" FORCE)
set(FLEX_INCLUDE_DIR ${FLEX_INCLUDE_DIR} CACHE FILEPATH "Path to Flex include dir" FORCE)
set(FLEX_FOUND TRUE)
