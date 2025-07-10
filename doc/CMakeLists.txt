find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
    message(STATUS "No Doxygen found. Documentation won't be built")
    return()
endif()

include(FetchContent)
FetchContent_Declare(
    doxygen-awesome-css
    GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
    GIT_TAG 568f56cde6ac78b6dfcc14acd380b2e745c301ea) # v2.3.4
FetchContent_MakeAvailable(doxygen-awesome-css)
set(IDLC_DOC_CSS "${doxygen-awesome-css_SOURCE_DIR}" CACHE STRING "doxygen-awesome")

file(GLOB IDLC_DOC_SOURCES "${PROJECT_SOURCE_DIR}/include/idlc/*")
set(IDLC_DOC_MARKDOWN
    "${CMAKE_CURRENT_LIST_DIR}/introduction.md"
    "${CMAKE_CURRENT_LIST_DIR}/quick-start.md"
    "${CMAKE_CURRENT_LIST_DIR}/language-guide.md")

set(DOXYGEN_PROJECT_LOGO "${CMAKE_CURRENT_LIST_DIR}/logo.svg")
set(DOXYGEN_PROJECT_ICON "${CMAKE_CURRENT_LIST_DIR}/icon.svg")
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(DOXYGEN_INLINE_INHERITED_MEMB YES)
set(DOXYGEN_STRIP_FROM_PATH "${PROJECT_SOURCE_DIR}/include/idlc/")
set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
set(DOXYGEN_QT_AUTOBRIEF YES)
set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
set(DOXYGEN_HIDE_UNDOC_MEMBERS YES)
set(DOXYGEN_HIDE_UNDOC_CLASSES YES)
set(DOXYGEN_HIDE_FRIEND_COMPOUNDS YES)
set(DOXYGEN_CASE_SENSE_NAMES YES)
set(DOXYGEN_HIDE_SCOPE_NAMES YES)
set(DOXYGEN_SORT_MEMBER_DOCS YES)
set(DOXYGEN_LAYOUT_FILE "${CMAKE_CURRENT_LIST_DIR}/layout.xml")
set(DOXYGEN_WARN_NO_PARAMDOC YES)
set(DOXYGEN_WARN_IF_UNDOC_ENUM_VAL YES)
set(DOXYGEN_WARN_LOGFILE "${CMAKE_CURRENT_BINARY_DIR}/doxygen-log.txt")
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_LIST_DIR}/introduction.md")
set(DOXYGEN_CLANG_ASSISTED_PARSING YES)
set(DOXYGEN_HTML_HEADER "${CMAKE_CURRENT_LIST_DIR}/header.html")
set(DOXYGEN_HTML_FOOTER "${CMAKE_CURRENT_LIST_DIR}/footer.html")
set(DOXYGEN_HTML_EXTRA_STYLESHEET
    "${IDLC_DOC_CSS}/doxygen-awesome.css"
    "${IDLC_DOC_CSS}/doxygen-custom/custom.css"
    "${IDLC_DOC_CSS}/doxygen-awesome-sidebar-only.css"
    "${IDLC_DOC_CSS}/doxygen-awesome-sidebar-only-darkmode-toggle.css"
    "${IDLC_DOC_CSS}/doxygen-custom/custom-alternative.css"
    "${CMAKE_CURRENT_LIST_DIR}/custom.css")
set(DOXYGEN_HTML_EXTRA_FILES
    "${IDLC_DOC_CSS}/doxygen-awesome-darkmode-toggle.js"
    "${IDLC_DOC_CSS}/doxygen-awesome-fragment-copy-button.js"
    "${IDLC_DOC_CSS}/doxygen-awesome-paragraph-link.js"
    "${IDLC_DOC_CSS}/doxygen-custom/toggle-alternative-theme.js"
    "${IDLC_DOC_CSS}/doxygen-awesome-interactive-toc.js"
    "${IDLC_DOC_CSS}/doxygen-awesome-tabs.js"
    "${CMAKE_CURRENT_LIST_DIR}/idlc.esm.js"
    "${CMAKE_CURRENT_LIST_DIR}/diagram.svg")
set(DOXYGEN_HTML_COLORSTYLE LIGHT)
set(DOXYGEN_HTML_COLORSTYLE_HUE 209)
set(DOXYGEN_HTML_COLORSTYLE_SAT 255)
set(DOXYGEN_HTML_COLORSTYLE_GAMMA 113)
set(DOXYGEN_SITEMAP_URL "https://vladimirshaleev.github.io/idlc/")
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_ENUM_VALUES_PER_LINE 1)
set(DOXYGEN_TREEVIEW_WIDTH 335)
set(DOXYGEN_GENERATE_LATEX NO)
set(DOXYGEN_MACRO_EXPANSION YES)
set(DOXYGEN_HAVE_DOT YES)
set(DOXYGEN_COLLABORATION_GRAPH NO)
set(DOXYGEN_TEMPLATE_RELATIONS YES)
set(DOXYGEN_INCLUDE_GRAPH NO)
set(DOXYGEN_INCLUDED_BY_GRAPH NO)
set(DOXYGEN_DIRECTORY_GRAPH NO)
set(DOXYGEN_DOT_IMAGE_FORMAT svg)
set(DOXYGEN_GENERATE_LEGEND YES)
doxygen_add_docs(html
    "${CMAKE_CURRENT_LIST_DIR}/groups.h"
    ${IDLC_DOC_SOURCES}
    ${IDLC_DOC_MARKDOWN}
    USE_STAMP_FILE)

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
