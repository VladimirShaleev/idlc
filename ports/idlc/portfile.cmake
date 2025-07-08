vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO VladimirShaleev/idlc
    REF 5c24b33a03871d7119d395336e8197131f10cd46
    SHA512 5627ad3853e22a9a47eff583c11995698ecd5cd42f1217a7e6abdc4533c5e1fb6bff1d8e7babebd6c5778a49f583a1191a5987035e6c7e4e1de790bf05ef7c35
    HEAD_REF main
)

vcpkg_check_features(
    OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        enable-js IDLC_SUPPORTED_JS
)

string(COMPARE EQUAL "${TARGET_TRIPLET}" "${HOST_TRIPLET}" IDLC_BUILD_BINARIES)
string(COMPARE EQUAL "${VCPKG_CRT_LINKAGE}" "dynamic" IDLC_DYNAMIC_CRT)
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" IDLC_SHARED_LIBS)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DIDLC_USE_IDLC=OFF
        -DIDLC_ENABLE_INSTALL=ON
        -DIDLC_BUILD_PACKAGES=OFF
        -DIDLC_BUILD_TOOL=${IDLC_BUILD_BINARIES}
        -DIDLC_MSVC_DYNAMIC_RUNTIME=${IDLC_DYNAMIC_CRT}
        -DBUILD_SHARED_LIBS=${IDLC_SHARED_LIBS}
        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()

if(IDLC_BUILD_BINARIES)
    if(VCPKG_TARGET_IS_WINDOWS)
        vcpkg_copy_tools(TOOL_NAMES idlc AUTO_CLEAN)
    else()
        string(REPLACE "." ";" VERSION_LIST ${VERSION})
        list(GET VERSION_LIST 0 VERSION_MAJOR)
        list(GET VERSION_LIST 1 VERSION_MINOR)
        list(GET VERSION_LIST 2 VERSION_PATCH)
        vcpkg_copy_tools(TOOL_NAMES idlc idlc-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH} AUTO_CLEAN)
    endif()
else()
    file(COPY "${CURRENT_HOST_INSTALLED_DIR}/tools/${PORT}" DESTINATION "${CURRENT_PACKAGES_DIR}/tools")
endif()

vcpkg_cmake_config_fixup(PACKAGE_NAME idlc CONFIG_PATH share/cmake/idlc)

if(NOT IDLC_BUILD_BINARIES)
    configure_file("${CMAKE_CURRENT_LIST_DIR}/idlc-targets-vcpkg.cmake" "${CURRENT_PACKAGES_DIR}/share/${PORT}/idlc-targets-vcpkg.cmake" COPYONLY)
endif()

configure_file("${CMAKE_CURRENT_LIST_DIR}/vcpkg-cmake-wrapper.cmake" "${CURRENT_PACKAGES_DIR}/share/${PORT}/vcpkg-cmake-wrapper.cmake" @ONLY)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
