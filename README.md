# Interface Definition Language Compiler

The IDL Compiler generates native APIs across programming languages while maintaining a single source of truth. Unlike traditional IDLs that force lowest-common-denominator solutions, my compiler understands language idioms - producing idiomatic C headers, JavaScript classes, and other language bindings from unified interface definitions.

## Key Features

- **Native API Generation**: IDLC produces:
  - C headers with optimal layout
  - JavaScript/TypeScript classes with proper typing
- **Automatic C Wrapping**: Creates zero-boilerplate bridges:
  - JavaScript → C via Node-API/WebAssembly
  - Managed ↔ Native memory conversion
- **Smart Parameter Handling**: Automatically adapts:
  - C-style arrays ↔ Managed language collections
  - Error codes ↔ Exceptions
  - Opaque handles ↔ Class instances
- **Zero-Overhead**: Generated C code matches hand-written performance

[Explore Full Documentation](https://vladimirshaleev.github.io/idlc/)
