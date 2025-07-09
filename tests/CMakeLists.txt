enable_testing()

find_package(doctest CONFIG REQUIRED)

add_executable(sample-tests tests.cpp)
target_link_libraries(sample-tests PRIVATE sample::sample)
target_link_libraries(sample-tests PRIVATE doctest::doctest)
target_compile_features(sample-tests PRIVATE cxx_std_20)
set_target_properties(sample-tests PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS OFF)
if(MSVC)
    if(SAMPLE_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(sample-tests PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(sample-tests PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

include(doctest)
doctest_discover_tests(sample-tests)
