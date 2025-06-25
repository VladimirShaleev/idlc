find_program(IDLC_EXECUTABLE NAMES idlc PATHS "${CMAKE_CURRENT_LIST_DIR}/../../../@HOST_TRIPLET@/tools/idlc" NO_DEFAULT_PATH)

_find_package(${ARGS} CONFIG)
