# Introduction

This project is not intended for production use. The tool is primarily being developed for my personal needs — to generate **C** APIs for **C** libraries from **IDL** specifications, as well as to automatically wrap the resulting **C** libraries for other languages, enabling them to be used as native libraries.

Below is an online demo of the compiler:

@htmlonly
<div class="custom-container">
    <textarea id="editor" class="custom-editor" placeholder="Enter your IDL code here..."></textarea>
    <button id="compileC" class="custom-button">Compile C</button>
    <button id="compileJs" class="custom-button">Compile JS</button>
</div>

<script type="module">
    import idlc from './idlc.js'

    const module = await idlc();
</script>
@endhtmlonly

For example, the **IDLC** tool itself uses its own functionality to generate its API:
- here are the **IDL** specifications for the library;
- here’s the CMake target setup for generating **C** headers;
- and here are the resulting headers produced during the build.

`idlc_compile` function monitors changes in the specs (**.idl** files) and, if necessary, rebuilds the headers in the `./include/idlc` directory. These headers provide declarations for the **C** API, which is implemented by the **C++** library. Of course, these headers are committed and distributed alongside the library.

Even for this online demo, the tool uses itself to generate the **JavaScript** library, packaging the WASM module for native use in **JavaScript** code. **IDL** interfaces are exposed as classes with methods, properties, and other familiar constructs.

The compiler is distributed as a library with a **C** API for embedding, as well as a command-line tool for standalone compilation. In other words, it can be used both as an embedded compiler and as a separate tool.

As for the **IDL** language itself — it is an abstract language not tied to any specific programming language but designed to accommodate a wide range of languages. For example, it supports properties and case-sensitive symbols, but it does not allow two different symbols that differ only in case (due to case-insensitive languages), and so on.

The workflow of this tool can be roughly represented by the following diagram:

![Diagram](diagram.svg "Diagram")

While this project is not intended as an industrial-grade solution, it serves as a practical example of:
- building host tools as dependencies;
- integrating them into a build system.

For a step-by-step guide on embedding the compiler into your project (to enable API support and generate native wrappers for other languages), check out the Quick Start.

<div class="section_buttons">
 
|                            Next |
|--------------------------------:|
| [Quick Start](quick-start.html) |
 
</div>
