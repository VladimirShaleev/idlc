set(M4_VERSION_REQUIRED "1.4.16")
find_program(M4_EXECUTABLE NAMES m4 gm4)

if(M4_EXECUTABLE)
    execute_process(COMMAND ${M4_EXECUTABLE} --version
        OUTPUT_VARIABLE M4_VERSION_OUTPUT
        ERROR_VARIABLE M4_VERSION_OUTPUT)
    string(REGEX MATCH "m4.*([0-9]+\\.[0-9]+\\.[0-9]+)" _ ${M4_VERSION_OUTPUT})
    set(M4_VERSION ${CMAKE_MATCH_1})
    if(M4_VERSION VERSION_LESS M4_VERSION_REQUIRED)
        message(STATUS "System M4 version ${M4_VERSION} is too old, need ${M4_VERSION_REQUIRED}")
        unset(M4_EXECUTABLE CACHE)
    endif()
endif()

if (NOT M4_EXECUTABLE)
    message(STATUS "m4 not found, building from source")
    
    set(M4_URL "https://ftp.gnu.org/gnu/m4/m4-${M4_VERSION_INSTALL}.tar.gz")
    set(M4_INSTALL_DIR "${CMAKE_BINARY_DIR}/m4_install")
    set(M4_EXECUTABLE "${M4_INSTALL_DIR}/bin/m4")

    ExternalProject_Add(
        m4_build
        URL ${M4_URL}
        URL_HASH MD5=f4a2b0284d80353b995f8ef2385ed73c
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        CONFIGURE_COMMAND <SOURCE_DIR>/configure
            --prefix=${M4_INSTALL_DIR}
            "CC=${CMAKE_C_COMPILER}"
            "CFLAGS=-O3"
        BUILD_COMMAND make -j${NPROC}
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 1
        BUILD_BYPRODUCTS ${M4_EXECUTABLE}
        STEP_TARGETS build install)

    set(M4_EXECUTABLE ${M4_EXECUTABLE} CACHE FILEPATH "Path to m4 executable" FORCE)
    set(M4_FOUND TRUE)
endif()
