add_executable(idlc::idlc IMPORTED)

set_property(TARGET idlc::idlc APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(idlc::idlc PROPERTIES IMPORTED_LOCATION_RELEASE "${IDLC_EXECUTABLE}")
