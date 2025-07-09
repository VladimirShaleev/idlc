# Sample lib

This is an example library that uses **IDLC** to create a C library.

This project builds:
- A C library ready for distribution via **.deb** packages or package managers such as **vcpkg** or **Conan**;
- Packages this library into an **npm** package for native use in JavaScript applications.

## Compilation requirements

To build the library, you'll need:
- Build tools installed on your system;
- CMake;
- Preferably, the [vcpkg](https://learn.microsoft.com/vcpkg/get_started/get-started) package manager installed.

Alternatively, you can use the [Dev Container](https://code.visualstudio.com/docs/devcontainers/containers) provided with the library, which comes preconfigured with all the necessary tools for building the C library and creating the npm package.

> [!NOTE]
> It is recommended to use **Dev Container** to setup the environment.

## Building 

### Building and testing the C library

```bash
# Configure the project
cmake -B build -S .

# Build the project
cmake --build build

# Run tests
cd build
ctest --output-on-failure
```

### Building and testing the JS npm package

```bash
# Build js modules
npm run build

# Run tests
npm run tests
```

## How does this work

TODO:
