# Install the Vcpkg Port

The port is located in a custom registry. To add the custom registry, include the following registry with the **idlc** port in your `vcpkg-configuration.json` (in the same directory as your `vcpkg.json`):

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

After that, you can add the dependency to your `vcpkg.json`:

```json
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "name": "sample",
  "dependencies": [
    "idlc"
  ]
}
```

> [!NOTE]
> For information on using **vcpkg**, refer to the [official website](https://learn.microsoft.com/vcpkg/)

Add the following configuration to your `CMakeLists.txt`:

```cmake
find_package(idlc CONFIG REQUIRED)
idlc_compile(NAME api WARN_AS_ERRORS
    SOURCE "${PROJECT_SOURCE_DIR}/specs/api.idl"
    OUTPUT "${PROJECT_SOURCE_DIR}/include/sample/sample.h"
    VERSION ${PROJECT_VERSION}
    GENERATOR C)

add_library(sample src/sample.c ${IDLC_api_OUTPUTS})
```
