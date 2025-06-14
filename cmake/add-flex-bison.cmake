set(BISON_VERSION_INSTALL "3.8.2")
set(FLEX_VERSION_INSTALL "2.6.4")
set(M4_VERSION_INSTALL "1.4.19")
set(WINFLEXBISON_VERSION_INSTALL "2.5.24")

include(ProcessorCount)
ProcessorCount(NPROC)
if(NPROC EQUAL 0)
    set(NPROC 1)
endif()

find_package(FLEX 2.6.4)
find_package(BISON 3.7.4)

if(NOT FLEX_FOUND)
    include(cmake/flex-install.cmake)
endif()

if(NOT BISON_FOUND)
    include(cmake/bison-install.cmake)
endif()
