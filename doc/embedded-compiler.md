@page embedded-compiler Embedded Compiler

@tableofcontents

# Using the Embedded Сompiler {#using-embedded-compiler}

This section covers how to integrate the built-in compiler and use the API.

## JavaScript API {#javascript-guide}

### Install the NPM Package {#install-npm-package}

@note I haven’t published the package to npmjs. Instead, it’s hosted in a public **GitHub** repository. You’ll need to:
* Add the **GitHub** repository to your `.npmrc` under the `NAMESPACE` `vladimirshaleev` ([GitHub repo setup](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-npm-registry#installing-a-package)).
* Authenticate with **GitHub** to access GitHub repositories ([GitHub auth](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-npm-registry#authenticating-to-github-packages)).

- Create an empty project (e.g., in a folder like `test-js`). Inside it, generate a package.json by running `npm init`.
- Then, add the **idlc** npm package as a dependency:
  ```
  @bash
  npm install @vladimirshaleev/idlc
  ```
  <details>
  <summary>After this, your `package.json` might look like this:</summary>
  
  ```json
  @json
  {
    "name": "test-js",
    "main": "index.js",
    "type": "module",
    "description": "",
    "dependencies": {
      "@vladimirshaleev/idlc": "^1.5.15"
    }
  }
  ```
</details>

### Using the Embedded Compiler {#using-compiler-in-js}

Now, in `index.js`, you can add the following code:

```javascript
@javascript
import idlcInit from '@vladimirshaleev/idlc';

const idlc = await idlcInit();

const compiledCodes = {};

const options = new idlc.Options;
options.warningsAsErrors = false;
options.writer = function (source) {
    compiledCodes[source.name] = source.data;
};

const source = {
    name: "myfile",
    data: `
        @ API Sample
        api Sample

        @ Sum of all values.
        @ Return sum. [return]
        func Sum {Float32}
            arg Values {Float32} [const,array(Count)] @ Array of values.
            arg Count {Uint32} @ Count of {Values}.
        `
};

const compiler = new idlc.Compiler;
const result = compiler.compile(idlc.Generator.C, undefined, [source], options);

result.messages.forEach(message => {
    const prefx = `${message.isError ? "error" : "warning"}`;
    const code = `${message.status.value < 2000 ? "W" : "E"}${message.status.value}`;
    const place = `${message.filename}:${message.line}:${message.column}`;
    console.log(`${prefx} [${code}]: ${message.message} at ${place}`)
});

result.delete();
compiler.delete();
options.delete();

console.log(`total files ${Object.keys(compiledCodes).length}`);
for (const [filename, code] of Object.entries(compiledCodes)) {
    console.log(`file: ${filename}`);
    console.log(`code:\n${code}`);
}
```

As we can see, the JavaScript API created by IDLC can work with lambda expressions, closures, properties, and JavaScript arrays.

## C API {#c-guide}

### Install the Vcpkg Port {#install-vcpkg-port}

The port is located in a custom registry. To add the custom registry, include the following registry with the **idlc** port in your `vcpkg-configuration.json` (in the same directory as your `vcpkg.json`):

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
      "baseline": "88b3d5b8d6466493c2215f10d5fc8618b578fe13",
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
@json
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "name": "test-c",
  "dependencies": [
    "idlc"
  ]
}
```

@note For information on using **vcpkg**, refer to the [official website](https://learn.microsoft.com/vcpkg/)

Add the following configuration to your `CMakeLists.txt`:

```cmake
@cmake
cmake_minimum_required(VERSION 3.16)

project(test-c CXX)

find_package(idlc CONFIG REQUIRED)

add_executable(test-c main.cpp)
target_link_libraries(test-c PRIVATE idlc::idl)
target_compile_features(test-c PRIVATE cxx_std_17)
```

### Using the Embedded Compiler {#using-compiler-in-cpp}

Now you can add the following code to `main.cpp`, which is functionally similar to the code above for JavaScript:

```cpp
@cpp
#include <idlc/idl.h>

#include <iostream>
#include <vector>
#include <map>

static void check(idl_result_t code)
{
    if (code != IDL_RESULT_SUCCESS)
    {
        throw std::runtime_error(std::string("error: ") + idl_result_to_string(code));
    }
};

int main()
{
    idl_options_t options{};
    idl_compiler_t compiler{};
    idl_compilation_result_t result{};
    int exitCode{};
    std::map<std::string, std::string> compiledCodes{};

    try
    {
        check(idl_options_create(&options));
        idl_options_set_warnings_as_errors(options, 0);
        idl_options_set_writer(options, [](const idl_source_t *source, idl_data_t data)
        {
            auto &compiledCodes = *static_cast<std::map<std::string, std::string>*>(data);
            compiledCodes[source->name] = std::string(source->data, source->size);
        }, &compiledCodes);

        constexpr char sourceCode[] = R"(
            @ API Sample
            api Sample

            @ Sum of all values.
            @ Return sum. [return]
            func Sum {Float32}
                arg Values {Float32} [const,array(Count)] @ Array of values.
                arg Count {Uint32} @ Count of {Values}.
        )";

        idl_source_t source{};
        source.name = "myfile";
        source.data = sourceCode;
        source.size = (idl_uint32_t)std::size(sourceCode) - 1;

        check(idl_compiler_create(&compiler));
        check(idl_compiler_compile(compiler, IDL_GENERATOR_C, nullptr, 1, &source, options, &result));

        idl_uint32_t messageCount{};
        idl_compilation_result_get_messages(result, &messageCount, nullptr);

        std::vector<idl_message_t> messages;
        messages.resize(size_t(messageCount));
        idl_compilation_result_get_messages(result, &messageCount, messages.data());

        for (const auto &message : messages)
        {
            auto& stream = message.is_error ? std::cerr : std::cout;
            stream << (message.is_error ? "error" : "warning") << " [";
            stream << ((int)message.status >= IDL_STATUS_E2001 ? 'E' : 'W') << (int)message.status << "]: ";
            stream << message.message << " at ";
            stream << message.filename << ':' << message.line << ':' << message.column << std::endl;
        }
        exitCode = idl_compilation_result_has_errors(result) ? 1 : 0;
    }
    catch (const std::exception &exc)
    {
        std::cerr << exc.what() << std::endl;
        exitCode = 1;
    }

    idl_compilation_result_destroy(result);
    idl_compiler_destroy(compiler);
    idl_options_destroy(options);

    std::cout << "total files " << compiledCodes.size() << std::endl;
    for (const auto &[filename, code] : compiledCodes)
    {
        std::cout << "file: " << filename << std::endl;
        std::cout << "code:" << std::endl << code << std::endl;
    }
    return exitCode;
}
```

## Next Steps {#learn-documentation}

To explore all the compiler's capabilities, check out the documentation at the link below.

<div class="section_buttons">

| Previous                              |                         Next |
|:--------------------------------------|-----------------------------:|
| [Language Guide](language-guide.html) | [Documentation](topics.html) |

</div>
