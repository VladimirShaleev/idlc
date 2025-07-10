# Interface Definition Language Compiler

The IDL Compiler generates native APIs across programming languages while maintaining a single source of truth. Unlike traditional IDLs that force lowest-common-denominator solutions, my compiler understands language idioms - producing idiomatic C headers, JavaScript classes, and other language bindings from unified interface definitions.

## Key Features

### Generates Native-Looking APIs

Produces idiomatic code for each target language as if written by hand:
- C: clean headers with optimal struct layouts and type-safe signatures.
- JavaScript/TypeScript: classes with methods, properties, and full typing.

### Automatic Cross-Language Wrapping

- Seamless C ↔ Managed Language Interop:
  * Converts C arrays ↔ JS/TS typed arrays (float[] ↔ number[]).
  * Maps error codes to exceptions.
  * Wraps opaque handles as class instances.
- WASM + Embind Integration: efficient JS bindings with managed ↔ native memory handling.
- The compiler generates its own C API and JS bindings.
- Works as a CLI tool or embeddable C library (CMake-supported).

[Explore Full Documentation](https://vladimirshaleev.github.io/idlc/)
