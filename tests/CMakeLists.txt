enable_testing()

find_package(CMakeRC CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

file(GLOB IDLC_TEST_CASES "cases/*.idl")

cmrc_add_resource_library(
    ${PROJECT_NAME}-cases ALIAS ${PROJECT_NAME}::cases 
        NAMESPACE ${PROJECT_NAME}::cases
        TYPE STATIC
        ${IDLC_TEST_CASES}
)
target_compile_features(${PROJECT_NAME}-cases PRIVATE cxx_std_20)
set_target_properties(${PROJECT_NAME}-cases PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS OFF)
set_target_properties(${PROJECT_NAME}-cases PROPERTIES 
    VERSION ${PROJECT_VERSION} 
    SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-cases PROPERTIES
    CXX_VISIBILITY_PRESET hidden 
    VISIBILITY_INLINES_HIDDEN ON)
if(MSVC)
    if(IDLC_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(${PROJECT_NAME}-cases PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(${PROJECT_NAME}-cases PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

file(GLOB IDLC_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(${PROJECT_NAME}-tests ${IDLC_TEST_SOURCES} ${CMAKE_SOURCE_DIR}/src/string_pool.cpp)
target_include_directories(${PROJECT_NAME}-tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_features(${PROJECT_NAME}-tests PRIVATE cxx_std_20)
set_target_properties(${PROJECT_NAME}-tests PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS OFF)
set_target_properties(${PROJECT_NAME}-tests PROPERTIES 
    VERSION ${PROJECT_VERSION} 
    SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_NAME}-tests PROPERTIES
    CXX_VISIBILITY_PRESET hidden 
    VISIBILITY_INLINES_HIDDEN ON)
if(MSVC)
    if(IDLC_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(${PROJECT_NAME}-tests PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(${PROJECT_NAME}-tests PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()
target_link_libraries(${PROJECT_NAME}-tests PRIVATE idl ${PROJECT_NAME}-cases)
target_link_libraries(${PROJECT_NAME}-tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}-tests)
