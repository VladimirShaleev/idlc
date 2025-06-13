set(BISON_VERSION_INSTALL "3.8.2")
set(WINFLEXBISON_VERSION_INSTALL "2.5.24")

find_package(FLEX 2.6.4)
find_package(BISON 3.7.4)

if(NOT FLEX_FOUND)
    include(cmake/flex-install.cmake)
endif()

if(NOT BISON_FOUND)
    include(cmake/bison-install.cmake)
endif()
