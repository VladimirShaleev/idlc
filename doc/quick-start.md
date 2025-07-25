@page quick-start Quick Start

@tableofcontents

# Creating a Simple Library {#create-lib}

Here we'll create a simple C library using **IDLC**:
- ready for distribution via packages like **.deb** or package managers like **vcpkg** or **Conan**;
- then we'll package this library as an **npm** package for native use in JavaScript applications.

You can clone and build the ready-made example from [this branch](https://github.com/VladimirShaleev/idlc/tree/sample).

```bash
@bash
git clone -b sample https://github.com/VladimirShaleev/idlc.git idlc-sample
```

and also follow its `README` on building and testing the library. Here we will also take a detailed step-by-step look at how to create this library.

## Adding Specifications for a C Library {#add-c-lib}

### IDL Specifications {#idl-spec}

Add **IDL** specifications anywhere in your library. For example, create a `specs` folder in your project's root directory and add an `api.idl` file (or use any other filename).

A typical project structure might look like this:

```
sample/
|-- include/
|   `-- sample/
|-- src/
|   `-- sample.c
|-- specs/
|   `-- api.idl
`-- CMakeLists.txt
```

Add the following content to your `api.idl` file:

```
@idl
@ API Sample
@ Author <author@email.org> [author]
@ MIT License [copyright]
api Sample

@ Function sample.
@ The result of multiplying {First} by {Second}. [return]
func Mul {Float32}
    arg First {Float32} @ First value
    arg Second {Float32} @ Second value

@ Vector 3.
struct Vector
    field X {Float32} @ X component
    field Y {Float32} @ Y component
    field Z {Float32} @ Z component

@ Sample object.
interface Vehicle
    prop Name [get(GetName)] @ Name of vehicle

    @ Create new vehicle instance.
    @ Vehicle instance. [return]
    method Create {Vehicle} [ctor]
        arg Name {Str} @ Name of vehicle.

    @ Destroy vehicle instance.
    method Destroy [destroy]
        arg Vehicle {Vehicle} [this] @ The 'this/self' object in OOP languages.

    @ Get name
    @ Get name of vehicle [detail]
    @ Return name of vehicle [return]
    method GetName {Str} [const]
        arg Vehicle {Vehicle} [this] @ The 'this/self' object in OOP languages.

    @ Set velocity of vehicle.
    method SetVelocity
        arg Vehicle {Vehicle} [this] @ The 'this/self' object in OOP languages.
        arg Value {Vector} [const,ref] @ Value of velocity.

    @ Example calculation
    @ Dot product. [return]
    method DotVelocity {Float32} [const]
        arg Vehicle {Vehicle} [this] @ The 'this/self' object in OOP languages.
        arg Value {Vector} [const,ref] @ Value of velocity.
```

While the complete IDL syntax isn't crucial at this stage, here are the key points to understand:
- **Declarations** always begin with keywords like: `api`, `struct`, `interface`, etc.
- **Naming conventions**:
  * Names must start with a capital letter.
  * Automatic tokenization occurs at capital letters.
  * See the [[tokenizer]](language-guide.html#attr-tokenizer) for customizing tokenization rules.
- **Documentation requirements**:
  * Every declaration must include at least `[brief]` or `[detail]` documentation.
  * Documentation starts with the `@` symbol - everything until the newline counts as documentation.
- **Attributes**:
  * Optional attributes may follow declarations or documentation, enclosed in square brackets `[]` and comma-separated (e.g., `[author]` or `[const,ref]`).
  * Note: `arg Value {Vector} [const,ref]` is syntactic sugar for `arg Value [type(Vector),const,ref]` (here `type` explicitly specifies the argument type).
- **Documentation placement**: documentation on the same line after a declaration defaults to `[detail]` documentation for that declaration.
- **Formatting rules**: indentation and line breaks are insignificant in **IDL**, except within documentation contexts.

### Adding IDLC Dependency {#add-idlc-dep}

@note This example will use **vcpkg** for dependency management. Install **vcpkg** by following the [official installation guide](https://learn.microsoft.com/vcpkg/get_started/get-started).

**IDLC** is registered in the **vcpkg** user registry. You need to add this registry to your `vcpkg-configuration.json` file:

```json
@json
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg-configuration.schema.json",
  "default-registry": {
    "kind": "git",
    "baseline": "0cf34c184ce990471435b5b9c92edcf7424930b1",
    "repository": "https://github.com/microsoft/vcpkg"
  },
  "registries": [
    {
      "kind": "git",
      "baseline": "124f27dfa5e457147722d976eeecba8332937f2f",
      "reference": "vcpkg-registry",
      "repository": "https://github.com/VladimirShaleev/idlc",
      "packages": [
        "idlc"
      ]
    }
  ]
}
```

Then add the dependency in your `vcpkg.json` file:

```json
@json
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "name": "sample",
  "dependencies": [
    "idlc"
  ]
}
```

The **idlc** dependency will add the **idlc** build host tool. This means that when building for a different target platform, such as Android NDK (arm64-android, arm-neon-android, etc.), this dependency will use the host tool for the build platform, which could be x64-windows or x64-linux, etc. This dependency also adds CMake configuration with the `idlc_compile` function.

The project structure now looks like this:

```
sample/
|-- include/
|   `-- sample/
|-- src/
|   `-- sample.c
|-- specs/
|   `-- api.idl
`-- CMakeLists.txt
|-- vcpkg-configuration.json
`-- vcpkg.json
```

### Adding CMake Configuration {#add-cmake-config}

```cmake
@cmake
find_package(idlc CONFIG REQUIRED)
idlc_compile(NAME api WARN_AS_ERRORS
    SOURCE "${PROJECT_SOURCE_DIR}/specs/api.idl"
    OUTPUT "${PROJECT_SOURCE_DIR}/include/sample/sample.h"
    VERSION ${PROJECT_VERSION}
    GENERATOR C)

add_library(sample src/sample.c ${IDLC_api_OUTPUTS})
```

<details>
<summary>A complete `CMakeLists.txt` file might look like this</summary>

```cmake
@cmake
cmake_minimum_required(VERSION 3.16)

option(SAMPLE_BUILD_TESTS "Build tests" ON)
if(SAMPLE_BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(sample
    DESCRIPTION "Example of creating a library"
    VERSION 1.0.0
    LANGUAGES C CXX)

option(SAMPLE_MSVC_DYNAMIC_RUNTIME "Link dynamic runtime library instead of static" OFF)

find_package(idlc CONFIG REQUIRED)
idlc_compile(NAME api WARN_AS_ERRORS
    SOURCE "${PROJECT_SOURCE_DIR}/specs/api.idl"
    OUTPUT "${PROJECT_SOURCE_DIR}/include/sample/sample.h"
    VERSION ${PROJECT_VERSION}
    GENERATOR C)

add_library(sample src/sample.c ${IDLC_api_OUTPUTS})
add_library(sample::sample ALIAS sample)
target_include_directories(sample PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
set_target_properties(sample PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS OFF)
target_compile_definitions(sample PRIVATE _CRT_SECURE_NO_WARNINGS)
if(BUILD_SHARED_LIBS)
    set_target_properties(sample PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
    set_target_properties(sample PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
else()
    target_compile_definitions(sample PUBLIC SAMPLE_STATIC_BUILD)
endif()
if(MSVC)
    if(SAMPLE_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(sample PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(sample PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()
```
</details>

The `api` `NAME` specified in `idlc_compile` will be used to form the `IDLC_<NAME>_OUTPUTS` variable containing the generated output files.

### Implementing C Declarations {#impl-c-decls}

The public headers are now automatically updated when `.idl` specifications change. You can now implement the generated function definitions.

<details>
<summary>Here's an example implementation (`sample.c`)</summary>

```c
@c
#include "sample/sample.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NAME_LENGTH 256

struct _sample_vehicle
{
    char name[NAME_LENGTH];
    sample_vector_t velocity;
};

sample_float32_t sample_mul(sample_float32_t first, sample_float32_t second)
{
    return first * second;
}

sample_vehicle_t sample_vehicle_create(sample_utf8_t name)
{
    sample_vehicle_t instance = (sample_vehicle_t)malloc(sizeof(struct _sample_vehicle));
    memset(instance, 0, sizeof(struct _sample_vehicle));
    strncpy(instance->name, name, NAME_LENGTH);
    return instance;
}

void sample_vehicle_destroy(sample_vehicle_t vehicle)
{
    if (vehicle)
    {
        free(vehicle);
    }
}

sample_utf8_t sample_vehicle_get_name(sample_vehicle_t vehicle)
{
    assert(vehicle);
    return vehicle->name;
}

void sample_vehicle_set_velocity(sample_vehicle_t vehicle, const sample_vector_t *value)
{
    assert(vehicle);
    assert(value);
    vehicle->velocity = *value;
}

sample_float32_t sample_vehicle_dot_velocity(sample_vehicle_t vehicle, const sample_vector_t *value)
{
    assert(vehicle);
    const sample_vector_t *vec = &vehicle->velocity;
    return vec->x * value->x + vec->y * value->y + vec->z * value->z;
}
```
</details>

### Testing the Library {#doctest-lib}

We'll add **doctest** to test the library:

```cpp
@cpp
#include <sample/sample.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("mul test")
{
    const auto expected = doctest::Approx(7.68f).epsilon(0.01f);

    const auto actual = sample_mul(3.2f, 2.4f);

    CHECK(expected == actual);
}

TEST_CASE("object test")
{
    const auto expected = doctest::Approx(10.0f).epsilon(0.01f);

    sample_vehicle_t vehicle = sample_vehicle_create("test");
    CHECK(vehicle != nullptr);

    sample_vector_t first{1.0f, 2.0f, 3.0f};
    sample_vehicle_set_velocity(vehicle, &first);

    sample_vector_t second{3.0f, 2.0f, 1.0f};
    const auto actual = sample_vehicle_dot_velocity(vehicle, &second);

    sample_vehicle_destroy(vehicle);

    CHECK(expected == actual);
}
```

<details>
<summary>View all changes</summary>

Current project structure:

```
sample/
|-- cmake/
|   |-- vcpkg.cmake
|   `-- sample-config.cmake.in
|-- include/
|   `-- sample/
|       |-- sample-version.h
|       |-- sample-platform.h
|       |-- sample-types.h
|       `-- sample.h
|-- src/
|   `-- sample.c
|-- tests/
|   |-- CMakeLists.txt
|   `-- tests.cpp
|-- specs/
|   `-- api.idl
|-- CMakeLists.txt
|-- vcpkg-configuration.json
`-- vcpkg.json
```

Contents of `./CMakeLists.txt` (including **install** target):

```cmake
@cmake
cmake_minimum_required(VERSION 3.16)

include(cmake/vcpkg.cmake)

option(SAMPLE_BUILD_TESTS "Build tests" ON)
if(SAMPLE_BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(sample
    DESCRIPTION "Example of creating a library"
    VERSION 1.0.0
    LANGUAGES C CXX)

option(SAMPLE_MSVC_DYNAMIC_RUNTIME "Link dynamic runtime library instead of static" OFF)
option(SAMPLE_ENABLE_INSTALL "Enable installation" ON)

find_package(idlc CONFIG REQUIRED)
idlc_compile(NAME api WARN_AS_ERRORS
    SOURCE "${PROJECT_SOURCE_DIR}/specs/api.idl"
    OUTPUT "${PROJECT_SOURCE_DIR}/include/sample/sample.h"
    VERSION ${PROJECT_VERSION}
    GENERATOR C)

add_library(sample src/sample.c ${IDLC_api_OUTPUTS})
add_library(sample::sample ALIAS sample)
target_include_directories(sample PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
set_target_properties(sample PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS OFF)
target_compile_definitions(sample PRIVATE _CRT_SECURE_NO_WARNINGS)
if(BUILD_SHARED_LIBS)
    set_target_properties(sample PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
    set_target_properties(sample PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
else()
    target_compile_definitions(sample PUBLIC SAMPLE_STATIC_BUILD)
endif()
if(MSVC)
    if(SAMPLE_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(sample PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(sample PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

if(SAMPLE_BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif()

if(SAMPLE_ENABLE_INSTALL)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in" 
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}"
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO)

    write_basic_package_version_file(${PROJECT_NAME}-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets)
    install(EXPORT ${PROJECT_NAME}-targets 
        DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}"
        NAMESPACE ${PROJECT_NAME}::)
    install(
        FILES
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME})
    install(DIRECTORY include/ DESTINATION include)
endif()
```

Contents of `./cmake/vcpkg.cmake`:

```cmake
@cmake
if(DEFINED Z_VCPKG_ROOT_DIR)
    return()
endif()

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    return()
endif()

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
else()
    message(WARNING "vcpkg not found, so will be installed in the project build directory; "
        "it is recommended to install vcpkg according to the instructions, which will "
        "allow correct caching of build dependencies (https://learn.microsoft.com/vcpkg/get_started/get-started)")
    include(FetchContent)
    FetchContent_Declare(
        vcpkg
        GIT_REPOSITORY https://github.com/microsoft/vcpkg/
        GIT_TAG 2025.06.13)
    FetchContent_MakeAvailable(vcpkg)
    set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()
```

Contents of `./cmake/sample-config.cmake.in`:


```cmake
@cmake
@PACKAGE_INIT@
include("${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-targets.cmake")
```

Contents of `./tests/CMakeLists.txt`:

```cmake
@cmake
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
```

Contents of `./vcpkg.json`:

```json
@json
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "name": "sample",
  "dependencies": [
    "idlc"
  ],
  "features": {
    "tests": {
      "description": "Build tests",
      "dependencies": [
        "doctest"
      ]
    }
  }
}
```
</details>

You can now build the project and run the tests:

```bash
@bash
# Configure the project
cmake -B build -S .

# Build the project
cmake --build build

# Run tests
cd build
ctest --output-on-failure
# or (if cmake >= 3.20)
ctest --test-dir build --output-on-failure
```

## Building a Native JavaScript Library {#js-lib}

### Adding WASM Support {#add-wasm}

If you're not working in the **Dev Container**, you'll need to:
- Install **Emscripten** following the [official guide](https://emscripten.org/docs/getting_started/downloads.html) for JS module compilation
- Install Node.js for **npm** package building

While you could build the JS library directly using **emcc**, we'll instead add **Emscripten** configuration to `CMakeLists.txt`:

```cmake
@cmake
if(EMSCRIPTEN)
    idlc_compile(NAME api_js WARN_AS_ERRORS
        SOURCE "${PROJECT_SOURCE_DIR}/specs/api.idl"
        OUTPUT "${PROJECT_BINARY_DIR}/sample.js.cpp"
        VERSION ${PROJECT_VERSION}
        GENERATOR JS)

    set(SAMPLE_JS_LINK_OPTIONS -sWASM=1 -sMODULARIZE=1 -sALLOW_MEMORY_GROWTH=1 -sEXPORT_NAME=sample)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND SAMPLE_JS_LINK_OPTIONS "-sSINGLE_FILE=0")
    else()
        list(APPEND SAMPLE_JS_LINK_OPTIONS "-sSINGLE_FILE=1")
    endif()

    add_executable(samplejs ${IDLC_api_js_OUTPUTS})
    target_link_libraries(samplejs PRIVATE embind sample::sample)
    target_include_directories(samplejs PRIVATE "${PROJECT_SOURCE_DIR}/include/sample/")
    target_compile_features(samplejs PRIVATE cxx_std_20)
    target_link_options(samplejs PRIVATE ${SAMPLE_JS_LINK_OPTIONS} --emit-tsd sample.d.ts)
    set_target_properties(samplejs PROPERTIES OUTPUT_NAME "sample" SUFFIX ".js" RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/dist/")

    add_executable(samplejsesm ${IDLC_api_js_OUTPUTS})
    target_link_libraries(samplejsesm PRIVATE embind sample::sample)
    target_include_directories(samplejsesm PRIVATE "${PROJECT_SOURCE_DIR}/include/sample/")
    target_compile_features(samplejsesm PRIVATE cxx_std_20)
    target_link_options(samplejsesm PRIVATE ${SAMPLE_JS_LINK_OPTIONS} --emit-tsd sample.esm.d.ts -sEXPORT_ES6=1)
    set_target_properties(samplejsesm PROPERTIES OUTPUT_NAME "sample.esm" SUFFIX ".js" RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/dist/")
endif()
```

When CMake targets **WASM**, we'll generate **Embind** bindings in the build output directory. This output file will be a dependency for two targets: `samplejs` and `samplejsesm` (for UMD and ESM respectively). These two targets link our C library `sample::sample` along with **Embind**. When built, these targets create JS modules with **WASM** in the **npm** package's root `./dist/` directory.

### Creating an NPM Package {#npm-package}

If you also want to publish the resulting module as an npm package, you'll need to add a `package.json` file at the root of your library.

Its contents might look like this:

```json
@json
{
  "name": "sample",
  "version": "1.0.0",
  "description": "Example of creating a library",
  "main": "dist/sample.js",
  "module": "dist/sample.esm.js",
  "browser": "dist/sample.js",
  "types": "dist/sample.d.ts",
  "files": ["dist"],
  "type": "module",
  "exports": {
    ".": {
      "require": "./dist/sample.js",
      "import": "./dist/sample.esm.js",
      "browser": "./dist/sample.js"
    },
    "./package.json": "./package.json"
  },
  "scripts": {
    "build": "..."
  }
}

```

Add the following line to the build script:

```bash
@bash
cmake -Bwasmbuild -S. -DVCPKG_TARGET_TRIPLET=wasm32-emscripten -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_CROSSCOMPILING_EMULATOR=${EMSDK_NODE} -DSAMPLE_BUILD_TESTS=OFF -DSAMPLE_ENABLE_INSTALL=OFF -G Ninja -DCMAKE_BUILD_TYPE=MinSizeRel && cmake --build wasmbuild
```

This script configures CMake for the **WASM** target platform and then builds the `.js` modules.

Now you can run:

```bash
@bash
npm run build
```

That's essentially all you need to create the JS module.

### Adding Tests to the NPM Package {#npm-tests}

We won't add dependencies for unit tests, but instead will simply include a Node.js script for working with our package at `./tests/node-test-esm.js`:

```javascript
@javascript
import sampleInit from 'sample';

const sample = await sampleInit();

const result = sample.mul(3.2, 2.5)
console.log(`3.2 * 2.5 = ${result}`);

const vehicle = new sample.Vehicle("Truck");
vehicle.setVelocity({ x: 1.0, y: 2.0, z: 3.0 });
const dot = vehicle.dotVelocity({ x: 3.0, y: 2.0, z: 1.0 });

console.log(`Vehicle '${vehicle.name}' dot: ${dot}`);

vehicle.delete();
```

In the `package.json` file, you can add the following script:

```json
@json
"tests": "node tests/node-test-esm.js"
```

To run the test, execute:

```bash
@bash
npm run tests
```

The expected output should be:

```
3.2 * 2.5 = 8
Vehicle 'Truck' dot: 10
```

## Conclusion {#conclusion}

In this guide, we've covered:
- Creating a C library with automatic API updates from IDL specification files using IDLC
- Packaging an npm module for native JavaScript usage - all powered by the same IDLC tool

The complete example library can be cloned as follows. You'll also find a README with detailed build instructions there.

```bash
@bash
git clone -b sample https://github.com/VladimirShaleev/idlc.git idlc-sample
```

## Next Steps {#next-steps}

Continue learning about IDL by following the link below.

<div class="section_buttons">
 
| Previous                   |                                  Next |
|:---------------------------|--------------------------------------:|
| [Introduction](index.html) | [Language Guide](language-guide.html) |
 
</div>
