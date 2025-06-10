file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../vcpkg.json" IDLC_VCPKG_JSON)

string(REGEX MATCH "\"version\"[ \t\n\r]*:[ \t\n\r]*\"[^\"]*\"" IDLC_TMP_VERSION_PAIR ${IDLC_VCPKG_JSON})
string(REGEX REPLACE "\"version\"[ \t\n\r]*:[ \t\n\r]*\"([^\"]*)\"" "\\1" IDLC_VERSION ${IDLC_TMP_VERSION_PAIR})
string(REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\1" IDLC_VERSION_MAJOR ${IDLC_VERSION})
string(REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\2" IDLC_VERSION_MINOR ${IDLC_VERSION})
string(REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\3" IDLC_VERSION_MICRO ${IDLC_VERSION})

unset(IDLC_TMP_VERSION_PAIR)
unset(IDLC_VCPKG_JSON)
