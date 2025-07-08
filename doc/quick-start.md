@page quick-start Quick Start

@tableofcontents

## Adding Specifications for a C Library

### IDL Specifications

Add **IDL** specifications anywhere in your library. For example, create a `specs` folder in your project's root directory and add an `api.idl` file (or use any other filename).

A typical project structure might look like this:

```
lib/
|-- include/
|   `-- lib/
|-- src/
|   `-- lib.c
|-- specs/
|   `-- api.idl
`-- CMakeLists.txt
```

Add the following content to your `api.idl` file:

```
@ API Sample
@ Author <author@email.org> [author]
@ MIT License [copyright]
api Lib

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
    @ Create new vehicle instance.
    @ Vehicle instance. [return]
    method Create {Vehicle} [ctor]
        arg Name {Str} @ Name of vehicle.

    @ Destroy vehicle instance.
    method Destroy [destroy]
        arg Vehicle {Vehicle} [this] @ The 'this/self' object in OOP languages.

    @ Set velocity of vehicle.
    method SetVelocity
        arg Vehicle {Vehicle} [this] @ The 'this/self' object in OOP languages.
        arg Value {Vector} [const,ref] @ Value of velocity.

    @ Example calculation
    @ Dot product. [return]
    method DotVelocity {Float32} [const]
        arg Vehicle {Vehicle} [this] @ The 'this/self' object in OOP languages.
```

While the complete IDL syntax isn't crucial at this stage, here are the key points to understand:
- **Declarations** always begin with keywords like: `api`, `struct`, `interface`, etc.
- **Naming conventions**:
  * Names must start with a capital letter.
  * Automatic tokenization occurs at capital letters.
  * See the **TODO** for customizing tokenization rules.
- **Documentation requirements**:
  * Every declaration must include at least `[brief]` or `[detail]` documentation.
  * Documentation starts with the `@` symbol - everything until the newline counts as documentation.
- **Attributes**:
  * Optional attributes may follow declarations or documentation, enclosed in square brackets `[]` and comma-separated (e.g., `[author]` or `[const,ref]`).
  * Note: `arg Value {Vector} [const,ref]` is syntactic sugar for `arg Value [type(Vector),const,ref]` (here `type` explicitly specifies the argument type).
- **Documentation placement**: documentation on the same line after a declaration defaults to `[detail]` documentation for that declaration.
- **Formatting rules**: indentation and line breaks are insignificant in **IDL**, except within documentation contexts.

### Добавить зависимость от IDLC

IDLC находится в пользовательском реестре vcpkg. Поэтому необходимо добавить репозиторий в vcpkg-configuration.json:

### Adding IDLC Dependency

**IDLC** is registered in the **vcpkg** user registry. You need to add this registry to your `vcpkg-configuration.json` file:

```json
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
      "baseline": "4928d7f5d89d48ed446e15b1ab97d6dff9364cd2",
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
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "name": "lib",
  "dependencies": [
    "idlc"
  ]
}
```

The **idlc** dependency will add the **idlc** build host tool. This means that when building for a different target platform, such as Android NDK (arm64-android, arm-neon-android, etc.), this dependency will use the host tool for the build platform, which could be x64-windows or x64-linux, etc. This dependency also adds CMake configuration with the `idlc_compile` function.

The project structure now looks like this:

```
lib/
|-- include/
|   `-- lib/
|-- src/
|   `-- lib.c
|-- specs/
|   `-- api.idl
|-- CMakeLists.txt
|-- vcpkg-configuration.json
`-- vcpkg.json
```

### Adding CMake Configuration

```cmake
find_package(idlc CONFIG REQUIRED)
idlc_compile(NAME api WARN_AS_ERRORS
    SOURCE "${PROJECT_SOURCE_DIR}/specs/api.idl"
    OUTPUT "${PROJECT_SOURCE_DIR}/include/lib/lib.h"
    VERSION ${PROJECT_VERSION}
    GENERATOR C)

add_library(lib src/lib.c ${IDLC_api_OUTPUTS})
```

<details>
<summary>A complete `CMakeLists.txt` file might look like this</summary>

```cmake
cmake_minimum_required(VERSION 3.16)

# Pass CMAKE_TOOLCHAIN_FILE as a parameter -DCMAKE_TOOLCHAIN_FILE
# when configuring or add CMAKE_TOOLCHAIN_FILE to CMakePresets.json
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

project(lib VERSION 1.0.0 LANGUAGES C)

option(LIB_MSVC_DYNAMIC_RUNTIME "Link dynamic runtime library instead of static" OFF)

find_package(idlc CONFIG REQUIRED)
idlc_compile(NAME api WARN_AS_ERRORS
    SOURCE "${PROJECT_SOURCE_DIR}/specs/api.idl"
    OUTPUT "${PROJECT_SOURCE_DIR}/include/lib/lib.h"
    VERSION ${PROJECT_VERSION}
    GENERATOR C)

add_library(lib src/lib.c ${IDLC_api_OUTPUTS})
target_include_directories(lib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
set_target_properties(lib PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS OFF)
if(BUILD_SHARED_LIBS)
    set_target_properties(lib PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
    set_target_properties(lib PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
else()
    target_compile_definitions(lib PUBLIC LIB_STATIC_BUILD)
endif()
if(MSVC)
    if(LIB_MSVC_DYNAMIC_RUNTIME)
        set_target_properties(lib PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set_target_properties(lib PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()
```
</details>

The `api` `NAME` specified in `idlc_compile` will be used to form the `IDLC_<NAME>_OUTPUTS` variable containing the generated output files.

### Implementing C Declarations

The public headers are now automatically updated when `.idl` specifications change. You can now implement the generated function definitions.

<details>
<summary>Here's an example implementation (`lib.c`)</summary>

```c
#include "lib/lib.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NAME_LENGHT 256

struct _lib_vehicle
{
    char name[NAME_LENGHT];
    lib_vector_t velocity;
};

lib_float32_t lib_mul(lib_float32_t first, lib_float32_t second)
{
    return first * second;
}

lib_vehicle_t lib_vehicle_create(lib_utf8_t name)
{
    lib_vehicle_t instance = (lib_vehicle_t)malloc(sizeof(struct _lib_vehicle));
    memset(instance, 0, sizeof(struct _lib_vehicle));
    strncpy_s(instance->name, NAME_LENGHT, name, strlen(name));
    return instance;
}

void lib_vehicle_destroy(lib_vehicle_t vehicle)
{
    if (vehicle)
    {
        free(vehicle);
    }
}

void lib_vehicle_set_velocity(lib_vehicle_t vehicle, const lib_vector_t *value)
{
    assert(vehicle);
    assert(value);
    vehicle->velocity = *value;
}

lib_float32_t lib_vehicle_dot_velocity(lib_vehicle_t vehicle)
{
    assert(vehicle);
    const lib_vector_t* vec = &vehicle->velocity;
    return vec->x * vec->x + vec->y * vec->y + vec->z * vec->z;
}
```
</details>

<div class="section_buttons">
 
| Previous                        |                      Next |
|:--------------------------------|--------------------------:|
| [Introduction](introduction.md) | [Tutorial](tutorial.html) |
 
</div>
