find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
  message(STATUS "No Doxygen found. Documentation won't be built")
  return()
endif()

include(FetchContent)
FetchContent_Declare(
  doxygen-awesome-css
  GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
  GIT_TAG 568f56cde6ac78b6dfcc14acd380b2e745c301ea # v2.3.4
)
FetchContent_MakeAvailable(doxygen-awesome-css)

file(GLOB IDLC_DOC_SOURCES "${CMAKE_CURRENT_LIST_DIR}/../include/idlc/*")
file(GLOB IDLC_DOC_MARKDOWN "${CMAKE_CURRENT_LIST_DIR}/*.md")

set(IDLC_DOC_CSS "${doxygen-awesome-css_SOURCE_DIR}" CACHE STRING "doxygen-awesome")
set(IDLC_DOC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}" CACHE PATH "Path to the documentation")
configure_file(Doxyfile.in Doxyfile @ONLY)

add_custom_command(OUTPUT html
  COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
  COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_BINARY_DIR}/html"
  DEPENDS 
    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    "${IDLC_DOC_SOURCES}"
    "${IDLC_DOC_MARKDOWN}"
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  VERBATIM)

# Hide sync button
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/replace.cmake"
  [=[
  file(READ "${SOURCE}" TEXT)
  string(REPLACE "n.html('<img src=\"'+relpath+'sync_off.png\" title=\"'+SYNCOFFMSG+'\"/>');" "" TEXT "${TEXT}")
  string(REPLACE "n.html('<img src=\"'+relpath+'sync_on.png\" title=\"'+SYNCONMSG+'\"/>');" "" TEXT "${TEXT}")
  file(WRITE "${TARGET}" "${TEXT}")
  ]=])
add_custom_command(
    OUTPUT navtree
    COMMAND ${CMAKE_COMMAND}
      -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/html/navtree.js
      -DTARGET=${CMAKE_CURRENT_BINARY_DIR}/html/navtree.js
      -P ${CMAKE_CURRENT_BINARY_DIR}/replace.cmake
    DEPENDS html "${CMAKE_CURRENT_BINARY_DIR}/replace.cmake"
    VERBATIM)

# Remove unused sections in ToC
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fixtoc.cmake"
  [=[
  file(READ "${SOURCE}" TEXT)
  string(REPLACE ",\n    [ \"Data Structures\", \"annotated.html\", [\n      [ \"Data Structures\", \"annotated.html\", \"annotated_dup\" ]\n    ] ],\n    [ \"Files\", \"files.html\", [\n      [ \"File List\", \"files.html\", \"files_dup\" ]\n    ] ]" "" TEXT "${TEXT}")
  file(WRITE "${TARGET}" "${TEXT}")
  ]=])
add_custom_command(
    OUTPUT navtreedata
    COMMAND ${CMAKE_COMMAND}
      -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/html/navtreedata.js
      -DTARGET=${CMAKE_CURRENT_BINARY_DIR}/html/navtreedata.js
      -P ${CMAKE_CURRENT_BINARY_DIR}/fixtoc.cmake
    DEPENDS html "${CMAKE_CURRENT_BINARY_DIR}/fixtoc.cmake"
    VERBATIM)

# Remove unused sections in menu
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fixmenu.cmake"
  [=[
  file(READ "${SOURCE}" TEXT)
  string(REPLACE ",\n{text:\"Data Structures\",url:\"annotated.html\",children:[\n{text:\"Data Structures\",url:\"annotated.html\"}]},\n{text:\"Files\",url:\"files.html\",children:[\n{text:\"File List\",url:\"files.html\"}]}" "" TEXT "${TEXT}")
  file(WRITE "${TARGET}" "${TEXT}")
  ]=])
add_custom_command(
    OUTPUT menudata
    COMMAND ${CMAKE_COMMAND}
      -DSOURCE=${CMAKE_CURRENT_BINARY_DIR}/html/menudata.js
      -DTARGET=${CMAKE_CURRENT_BINARY_DIR}/html/menudata.js
      -P ${CMAKE_CURRENT_BINARY_DIR}/fixmenu.cmake
    DEPENDS html "${CMAKE_CURRENT_BINARY_DIR}/fixmenu.cmake"
    VERBATIM)

add_custom_target(doc ALL DEPENDS html navtree navtreedata menudata)

if(IDLC_ENABLE_INSTALL)
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html" DESTINATION doc COMPONENT doc)
endif()
