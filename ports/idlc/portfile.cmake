vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO VladimirShaleev/idlc
    REF 15f6e625cac7db3937ca532011cb67b0a5a1f8d6
    SHA512 b96faeb19055b639c49b35e39c9cdaf383f0196d463cf2030aad774e5036d0bfb73bbd43ee82631ddab7cb288fd4e920065e5a5ed8aba3758abfa164b95acfa0
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
