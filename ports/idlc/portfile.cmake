vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO VladimirShaleev/idlc
    REF f34bb0cd6d68ca0fb4e060d42a52698bb1433322
    SHA512 798c49f23b4b77ef1cc726f88fde469cd64dd2fb6b3b386e296748d6c5c2f128b7a0c3020adc383dae4bbf9dd57c261fe3e573b436e68b877cde711960b8b662
    HEAD_REF main
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
        list(GET VERSION_LIST 1 VERSION_MINOR)
        list(GET VERSION_LIST 2 VERSION_PATCH)
        vcpkg_copy_tools(TOOL_NAMES idlc idlc-${VERSION_MINOR}.${VERSION_PATCH}.0 AUTO_CLEAN)
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
