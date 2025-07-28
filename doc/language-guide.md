@page language-guide Language Guide

@tableofcontents

# Interface Definition Language {#idl}

This guide describes the use of IDL for creating library specifications, its syntax, and capabilities.

## Declarations {#declarations}

All declarations, whether they're structure declarations or functions and their arguments, must begin with the appropriate keyword followed by the declaration name. Example:

```
@idl
@ API Sample
api Sample

@ Color values.
struct Color
    field Red @ Red channel clear value.
    field Green @ Green channel clear value.
    field Blue @ Blue channel clear value.
    field Alpha @ Alpha channel clear value.

@ Object type
interface Logger
    method TestName [static] @ Detail
```

In this example, `api`, `struct`, `interface`, and `method` are keywords that create declarations. They're followed by the declaration name.

The language requires all names to begin with a capital letter. The main question is how the compiler will represent these names for target languages during compilation? The answer is that naming conventions depend on the target language - for example, C uses snake_case, JS uses camelCase, and C# uses PascalCase.

Let's say we have a method name TestName. The compiler tokenizes it into two words `Test` and `Name` based on capital letters and letter-digit-letter transitions. So in C, the name would be:

```c
@c
sample_logger_test_name
```

and in JS:

```javascript
@javascript
testName
```

Note that C isn't an OOP language, but many C libraries (like cairo, etc.) manually "implement OOP": they add opaque types that are passed as the first parameter in functions (acting as `this`). The function prefix indicates the "class" name. These opaque types can be implemented in C or even C++, as in this library.

In JS, the interface translates to a JS class with methods and other features.

@note If default tokenization doesn't work for your case, you can use the `[tokenizer]` attribute as shown in this [[tokenizer]](#attr-tokenizer) section.

## Documentation {#documentation}

The language requires that all declarations have at least a brief description or detailed documentation.

Documentation begins with the `@` symbol followed by documentation until the end of the line. You can end the documentation with an attribute indicating what it refers to, for example:

```
@idl
@ Color values [brief]
@ Detailed description. [detail]
struct Color
    ...
```
The `[brief]` attribute is the default attribute when no attribute is specified.

You can also place documentation on the same line as the declaration. In this case, the default attribute for this documentation will be `[detail]`. The example below is equivalent to the previous one:

```
@idl
@ Color values
struct Color @ Detailed description.
    ...
```

This isn't very elegant. This feature is mainly intended for cleaner documentation of fields and function arguments:

```
@idl
@ Color values.
@ Detailed description. [detail]
struct Color
    field Red @ Red channel clear value.
    field Green @ Green channel clear value.
    field Blue @ Blue channel clear value.
    field Alpha @ Alpha channel clear value.
```

@note Note that the language doesn't care about line breaks and indentation. The language understands everything from context. Naturally, except for documentation context.

To create a multiline comment, use three consecutive backtick symbols `` ` ``:

````
@idl
@ Color values.
@ ```
    Detailed description.
    Other string.
       Save three spaces.
    There will be no spaces at the beginning of this line.``` [detail]
struct Color
    field Red @ Red channel clear value.
    field Green @ Green channel clear value.
    field Blue @ Blue channel clear value.
    field Alpha @ Alpha channel clear value.
````

Note that the first line of multiline documentation sets the initial indentation. Therefore, the line <code>&nbsp;&nbsp;&nbsp;Save three spaces.</code> will preserve the 3 leading spaces.

There are also special characters like `{`, `}`, `[`, `]`. Their purpose isn't important right now, but keep in mind they need to be escaped. For example, if you need to output `{`, the correct way is `\{`.

@note The `\` character doesn't need to be escaped. The language understands from context what it is.

## Specification Start {#start}

The first declaration the compiler must encounter should always be `api Decl`.

Everything else, including imports, must come after it.

## Enumerations {#enumerations}

A constant declaration can look like this:

```
@idl
@ Graphics features.
enum Feature
    const None @ No special features
    const Bindless @ Bindless resource access
    const GeometryShader @ Geometry shader support
    const MeshShader @ Mesh shader support
    const SamplerFilterMinmax @ Min/max sampler filtering
    const DrawIndirect @ Indirect drawing
```

The constant type must always be `Int32`, even if explicitly specified:

```
@idl
@ Graphics features.
enum Feature
    const None {Int32} @ No special features
```

By default, each constant's value increments by one from the previous constant's value. The first constant has a value of 0. However, we can explicitly specify constant values:

```
@idl
@ Graphics features.
enum Feature [flags]
    const None @ No special features
    const Bindless @ Bindless resource access
    const GeometryShader : 2 @ Geometry shader support
    const MeshShader : 4 @ Mesh shader support
    const SamplerFilterMinmax : 8 @ Min/max sampler filtering
    const DrawIndirect : 16 @ Indirect drawing
    const Combine : MeshShader, DrawIndirect @ Combine flags sample
```

Here None will be 0, and Bindless will be 1. For example, in C this would look like after compilation:

```
@c
/**
 * @brief Graphics features.
 */
typedef enum
{
    SAMPLE_FEATURE_NONE_BIT                  = 0, /**< No special features */
    SAMPLE_FEATURE_BINDLESS_BIT              = 1, /**< Bindless resource access */
    SAMPLE_FEATURE_GEOMETRY_SHADER_BIT       = 2, /**< Geometry shader support */
    SAMPLE_FEATURE_MESH_SHADER_BIT           = 4, /**< Mesh shader support */
    SAMPLE_FEATURE_SAMPLER_FILTER_MINMAX_BIT = 8, /**< Min/max sampler filtering */
    SAMPLE_FEATURE_DRAW_INDIRECT_BIT         = 16, /**< Indirect drawing */
    SAMPLE_FEATURE_COMBINE_BIT               = SAMPLE_FEATURE_MESH_SHADER_BIT | SAMPLE_FEATURE_DRAW_INDIRECT_BIT, /**< Combine flags sample */
    SAMPLE_FEATURE_MAX_ENUM                  = 0x7FFFFFFF /**< Max value of enum (not used) */
} sample_feature_flags_t;
SAMPLE_FLAGS(sample_feature_flags_t)
```

@note `SAMPLE_FEATURE_MAX_ENUM` ensures the constant is always 4 bytes across all platforms and compilers. `SAMPLE_FLAGS` enables bitwise operations in C++.

## Built-in Types {#builtin-types}

| Type        | Description                                                                 |
| -----------:|:--------------------------------------------------------------------------- |
|      `Void` | Absence of type. For example, if a function has no return type, it's `Void` |
|      `Char` | Character                                                                   |
|      `Bool` | Boolean value                                                               |
|      `Int8` | 8-bit signed integer                                                        |
|     `Uint8` | 8-bit unsigned integer                                                      |
|     `Int16` | 16-bit signed integer                                                       |
|    `Uint16` | 16-bit unsigned integer                                                     |
|     `Int32` | 32-bit signed integer                                                       |
|    `Uint32` | 32-bit unsigned integer                                                     |
|     `Int64` | 64-bit signed integer                                                       |
|    `Uint64` | 64-bit unsigned integer                                                     |
|   `Float32` | Floating-point number (32-bit)                                              |
|   `Float64` | Floating-point number (64-bit)                                              |
|       `Str` | String represented as UTF-8 encoding                                        |
|      `Data` | Pointer to data or buffer                                                   |
| `ConstData` | Pointer to immutable data or buffer                                         |

Generally these types look similar across languages, but there are exceptions. For example, in JS `Char` is represented as a string since JavaScript doesn't have single-character types. However, if you declare an array of `Char` in IDL, in C it will be a character array, while in JS it will simply be a string.

Similarly, in C `Data` and `ConstData` are represented as pointers, while in JS they might be sized `Uint8Array` or `ArrayBuffer`. IDL provides a unified way to properly format buffers in syntax that feels native to each language (pointers in C, buffers/strings/arrays in JS). More on this later in the [[datasize]](#attr-datasize) section.

The following special types are available for documentation purposes only:

| Type    | Description                         |
| -------:|:----------------------------------- |
| `Year`  | Gets current year                   |
| `Major` | Gets major component of api version |
| `Minor` | Gets minor component of api version |
| `Micro` | Gets micro component of api version |

## Structures {#structures}

Structure declarations are no different from other declarations:

```
@idl
@ Color values.
@ Detailed description. [detail]
struct Color
    field Red @ Red channel clear value.
    field Green @ Green channel clear value.
    field Blue @ Blue channel clear value.
    field Alpha @ Alpha channel clear value.
```

The default field type is `Int32`. To specify a type, use a type reference in braces `{DeclRef}`:

```
@idl
@ Color values.
struct Color
    field Red {Float32} @ Red channel clear value.
    field Green {Float32} @ Green channel clear value.
    field Blue {Float32} @ Blue channel clear value.
    field Alpha {Float32} @ Alpha channel clear value.
```

You can also set default values for fields. These will be assigned during initialization in languages that support it (e.g., C++):

```
@idl
@ Color values.
struct Color
    field Red {Float32} @ Red channel clear value.
    field Green {Float32} @ Green channel clear value.
    field Blue {Float32} @ Blue channel clear value.
    field Alpha {Float32} : 1 @ Alpha channel clear value.
```

@note Currently only integers and constants can be specified as defaults.

```
@idl
@ Brief.
struct Test
    field Flags {Feature} : Feature.Bindless, Feature.MeshShader @ Sample.
```

Note the use of the fully qualified name Feature.Bindless to indicate that Bindless belongs to the Feature declaration, not Test.

## Functions {#functions}

The simplest function can be declared as:

```
@idl
@ Sample
func Func
```

Let's add some arguments:

```
@idl
@ Sample
func Func
    arg Value @ First arg
    arg OtherValue @ Second arg
```

By default, argument types are `Int32`. Let's specify return type and argument types:


```
@idl
@ Sample
func Func {Float32}
    arg Value {Uint32} @ First arg
    arg OtherValue {Float32} @ Second arg
```

Arguments can also have default values similar to structure fields.

Arguments can have direction specified. This is useful in special cases to make function usage more native for target languages. More on this in [[result]](#attr-result) section.

## Interfaces {#interfaces}

In OOP languages, interfaces become classes, while in C they become opaque types.

Let's add an interface with two creation methods:

```
@idl
@ Brief.
interface ObjType
    @ Creates new object instance
    method CreateByValue {ObjType} [ctor]
        arg Name {Str} @ Name of object.

    @ Creates new object instance
    method Create {ObjType} [ctor]
```

Note both methods have `[ctor]` attributes, telling the compiler to make these methods constructors in OOP-capable languages. In JS, instantiation would look like ``const inst1 = new sample.ObjType('test');``. Constructor methods are implicitly static. Constructors must return the type either as return value or argument (here it's returned).

Let's add three more methods:

```
@idl
@ Brief.
interface ObjType
    @ Creates new object instance
    method CreateByValue {ObjType} [ctor]
        arg Name {Str} @ Name of object.

    @ Creates new object instance
    method Create {ObjType} [ctor]

    @ Destroy instance.
    method Destroy [destroy]
        arg Obj {ObjType} [this] @ object to destroy.

    @ Method of interface.
    method Method
        arg Obj {ObjType} [this] @ this object
        arg Val @ Value arg

    @ Method of interface.
    method ClassMethod [static]
        arg Val @ Value arg
```

The `Destroy` method will be used for object deallocation. The `[destroy]` attribute tells RAII or garbage-collected languages which method releases unmanaged handles.

`Method` is an instance method. It explicitly takes Obj argument with `[this]` attribute. In OOP languages this becomes implicit `this`/`self`. In non-OOP languages it remains as Obj argument. For example in C:

```
@c
/**
 * @brief     Method of interface.
 * @param[in] obj this object
 * @param[in] val Value arg
 */
sample_api void
sample_obj_type_method(sample_obj_type_t obj,
                       sample_sint32_t val);
```

`ClassMethod` is static and cannot have `[this]` argument.

Let's add two more methods:

```
@idl
@ Brief.
interface ObjType
    // ...

    @ Get method
    method GetValue {Float32} [const]
        arg Obj {ObjType} [this] @ this object

    @ Set method
    method SetValue
        arg Obj {ObjType} [this] @ this object
        arg Value {Float32} @ New value
```

Note `GetValue` is marked `[const]` to indicate it doesn't modify the instance.

## Properties {#properties}

Let's add a property to the interface from [Interfaces](#interfaces) section:

```
@idl
@ Brief.
interface ObjType
    // ...

    @ Get method
    method GetValue {Float32} [const]
        arg Obj {ObjType} [this] @ this object

    @ Set method
    method SetValue
        arg Obj {ObjType} [this] @ this object
        arg Value {Float32} @ New value

    prop Value [get(GetValue),set(SetValue)] @ Value property
```

In languages supporting properties (JS, C# etc.), this creates a `Value` property instead of separate `GetValue`/`SetValue` methods. Class properties can also be created by adding `[static]` to methods.

## Callbacks {#callbacks}

Callbacks are declared similarly to functions:

````
@idl
@ Callback to which the compilation result is passed.
@ ```
    If you need to save the compilation result to a location other than the file
    system, such as the network or console output, you can use this callback.``` [detail]
@ The compiler can output multiple sources. The exact number depends on the selected generator {Generator}. [note]
callback WriteCallback
    arg Source {Source} [const,ref] @ Source of compiler output.
    arg Data {Data} [userdata] @ User data specified when setting up a callback.
````

This example comes from the compiler's own specification.

We can now use this callback in functions, static methods, and instance methods. It can also be used as a field type in structures.

Let's examine the specification further:

````
@idl
@ Compilation options.
@ This object specifies various compilation options. [detail]
interface Options
    @ Get the current write callback.
    @ Returns a callback if one has been configured. [detail]
    @ Returns a callback. [return]
    @ {SetWriter} [see]
    method GetWriter {WriteCallback} [const]
        arg Options {Options} [this] @ Target options.
        arg Data {Data} [out,optional,userdata] @ Returning a callback user data pointer (may be null).

    @ Set write callback.
    @ ```
        Configures a callback to receive compiler output. If the callback is set, no output
        will be made to the file system ({SetOutputDir} will also not be used).``` [detail]
    @ Typical uses of a writer are writing to memory or outputting to the console and the like. [note]
    @ {GetWriter} [see]
    method SetWriter
        arg Options {Options} [this] @ Target options.
        arg Callback {WriteCallback} @ Callback function.
        arg Data {Data} [optional,userdata] @ Callback user data.
````

As we can see, the Options interface can set a callback and store user data (useful for unmanaged languages like C to pass context to callbacks or even propagate C++ exceptions through C functions using `std::current_exception`).

In C, these functions look like this:

```
@c
/**
 * @brief     Callback to which the compilation result is passed.
 * @details   If you need to save the compilation result to a location other than the file
 *            system, such as the network or console output, you can use this callback.
 * @param[in] source Source of compiler output.
 * @param[in] data User data specified when setting up a callback.
 * @note      The compiler can output multiple sources. The exact number depends on the selected generator ::idl_generator_t.
 * @ingroup   types
 */
typedef void
(*idl_write_callback_t)(const idl_source_t* source,
                        idl_data_t data);

/**
 * @brief      Get the current write callback.
 * @details    Returns a callback if one has been configured.
 * @param[in]  options Target options.
 * @param[out] data Returning a callback user data pointer (may be null).
 * @return     Returns a callback.
 * @sa         ::idl_options_set_writer
 * @ingroup    functions
 */
idl_api idl_write_callback_t
idl_options_get_writer(idl_options_t options,
                       idl_data_t* data);

/**
 * @brief     Set write callback.
 * @details   Configures a callback to receive compiler output. If the callback is set, no output
 *            will be made to the file system (::idl_options_set_output_dir will also not be used).
 * @param[in] options Target options.
 * @param[in] callback Callback function.
 * @param[in] data Callback user data.
 * @note      Typical uses of a writer are writing to memory or outputting to the console and the like.
 * @sa        ::idl_options_get_writer
 * @ingroup   functions
 */
idl_api void
idl_options_set_writer(idl_options_t options,
                       idl_write_callback_t callback,
                       idl_data_t data);
```

## Events {#events}

We can use callbacks as events. For example:

```
@idl
@ Compilation options.
@ This object specifies various compilation options. [detail]
interface Options
    event Writer [get(GetWriter),set(SetWriter)] @ Event that occurs when the compiler outputs.
```

This example comes from the compiler's own specification.

This can be useful in certain languages. Let's look at JavaScript code for an online editor using this event with the callback:

```javascript
@javascript
const options = new module.Options;

const results = {};
options.writer = function (source) {
    results[source.name] = source.data;
};

const compiler = new module.Compiler;
compiler.compile(generator, undefined, [source], options);
```

Here we assign a lambda to the writer property of the options instance, which takes compiled code and saves it to a captured dictionary variable as filename/code pairs.

## Import {#import}

Large specifications are rarely practical to describe in a single file. For this purpose, imports are available:

```
@idl
@ IDLC
api Idl [version(0,0,0)]

@ Warning/Error codes.
@ Here are the warning and error codes that may occur during compilation. [detail]
import Results

@ Compiler options.
@ This is where the structures and various compilation options are located. [detail]
import Options
```

Documentation for imports is also mandatory.

Repeated inclusion of an import has no effect - it's simply skipped.

@note Note that the C generator creates a separate file for each import.

## Declaration References {#decl-refs}

References to declarations use the `{DeclRef}` syntax. Forward references are allowed in documentation. Generated documentation will adapt reference names to match target language conventions.

Example Specification:

````
@idl
@ Compiler interface.
@ Interface for interacting with the compiler. [detail]
interface Compiler
    @ Compile IDL.
    @ Compilation result. [return]
    @ ```
        To read source code from memory instead of the file system, use {Sources} and/or configure 
        the importer with {Options.SetImporter} and pass the {File} argument as empty.``` [note]
    @ ```
        Priorities for resolving source code imports:
        - {Options.SetImporter} - import callback if specified;
        - {Sources} - then the source code array, if specified;
        - {Options.SetImportDirs} - then in the paths to the import directories, if specified;
        - then the current working directory.
        ``` [note]
    method Compile {Result}
        arg Compiler {Compiler} [this] @ Target compiler.
        arg Generator {Generator} @ Target of generator.
        arg File {Str} [optional] @ Path to .idl file for compile.
        arg SourceCount {Uint32} @ Number of sources.
        arg Sources {Source} [const,array(SourceCount)] @ Sources.
        arg Options {Options} [optional] @ Compile options, may be null.
        arg Result {CompilationResult} [optional,result] @ Compilation result.
````

Result of resolving declaration reference for C:

```
@c
/**
 * @brief      Compile IDL.
 * @param[in]  compiler Target compiler.
 * @param[in]  generator Target of generator.
 * @param[in]  file Path to .idl file for compile.
 * @param[in]  source_count Number of sources.
 * @param[in]  sources Sources.
 * @param[in]  options Compile options, may be null.
 * @param[out] result Compilation result.
 * @return     Compilation result.
 * @parblock
 * @note       To read source code from memory instead of the file system, use *sources* and/or configure
 *             the importer with ::idl_options_set_importer and pass the *file* argument as empty.
 * @endparblock
 * @parblock
 * @note       Priorities for resolving source code imports:
 *             - ::idl_options_set_importer - import callback if specified;
 *             - *sources* - then the source code array, if specified;
 *             - ::idl_options_set_import_dirs - then in the paths to the import directories, if specified;
 *             - then the current working directory.
 *             
 * @endparblock
 * @ingroup    functions
 */
idl_api idl_result_t
idl_compiler_compile(idl_compiler_t compiler,
                     idl_generator_t generator,
                     idl_utf8_t file,
                     idl_uint32_t source_count,
                     const idl_source_t* sources,
                     idl_options_t options,
                     idl_compilation_result_t* result);
```

## Comments

Comments start with two `//`, everything that comes after them will be considered a comment until the end of the line and will not be taken into account when compiling.

@note In the documentation, two consecutive characters `//` are just two characters `//`.

## Attributes {#attributes}

As you may have noticed, the language provides only declarations - everything else is achieved by attaching attributes to these declarations. Even argument and field types are actually attributes:

```
@idl
@ Color values.
struct Color
    field Red {Float32} @ Red channel clear value.
    field Green {Float32} @ Green channel clear value.
    field Blue {Float32} @ Blue channel clear value.
    field Alpha {Float32} @ Alpha channel clear value.
```

This is syntactic sugar for the `[type]` attribute:

```
@idl
@ Color values.
struct Color
    field Red [type(Float32)] @ Red channel clear value.
    field Green [type(Float32)] @ Green channel clear value.
    field Blue [type(Float32)] @ Blue channel clear value.
    field Alpha [type(Float32)] @ Alpha channel clear value.
```

The same applies to default values:

```
@idl
@ Graphics features.
enum Feature [flags]
    const None @ No special features
    const Bindless @ Bindless resource access
    const GeometryShader : 2 @ Geometry shader support
    const MeshShader : 4 @ Mesh shader support
    const SamplerFilterMinmax : 8 @ Min/max sampler filtering
    const DrawIndirect : 16 @ Indirect drawing
    const Combine : MeshShader, DrawIndirect @ Combine flags sample
```

Is actually:

```
@idl
@ Graphics features.
enum Feature [flags]
    const None [type(Int32),value(0)] @ No special features
    const Bindless [type(Int32),value(1)] @ Bindless resource access
    const GeometryShader [type(Int32),value(2)] @ Geometry shader support
    const MeshShader [type(Int32),value(4)] @ Mesh shader support
    const SamplerFilterMinmax [type(Int32),value(8)] @ Min/max sampler filtering
    const DrawIndirect [type(Int32),value(16)] @ Indirect drawing
    const Combine [type(Int32),value(MeshShader,DrawIndirect)] @ Combine flags sample
```

In short, everything except declarations and documentation context is implemented through attributes.

### [flags] {#attr-flags}

Marks an enum as a bitmask type where values can be combined using bitwise operations. Automatically adds bitwise operation support in target languages:

```
@idl
@ Graphics features.
enum Feature [flags]
    const None @ No special features
    const Bindless @ Bindless resource access
    const GeometryShader : 2 @ Geometry shader support
    const MeshShader : 4 @ Mesh shader support
    const SamplerFilterMinmax : 8 @ Min/max sampler filtering
    const DrawIndirect : 16 @ Indirect drawing
```

### [hex] {#attr-hex}

Specifies that enum values should be displayed in hexadecimal format in documentation and outputs:

```
@idl
@ Graphics features.
enum Feature [flags,hex]
    const None @ No special features
    const Bindless @ Bindless resource access
    const GeometryShader : 2 @ Geometry shader support
    const MeshShader : 4 @ Mesh shader support
    const SamplerFilterMinmax : 8 @ Min/max sampler filtering
    const DrawIndirect : 16 @ Indirect drawing
```

C output:

```
@c
/**
 * @brief Graphics features.
 */
typedef enum
{
    SAMPLE_FEATURE_NONE_BIT                  = 0x00, /**< No special features */
    SAMPLE_FEATURE_BINDLESS_BIT              = 0x01, /**< Bindless resource access */
    SAMPLE_FEATURE_GEOMETRY_SHADER_BIT       = 0x02, /**< Geometry shader support */
    SAMPLE_FEATURE_MESH_SHADER_BIT           = 0x04, /**< Mesh shader support */
    SAMPLE_FEATURE_SAMPLER_FILTER_MINMAX_BIT = 0x08, /**< Min/max sampler filtering */
    SAMPLE_FEATURE_DRAW_INDIRECT_BIT         = 0x10, /**< Indirect drawing */
    SAMPLE_FEATURE_MAX_ENUM                  = 0x7FFFFFFF /**< Max value of enum (not used) */
} sample_feature_flags_t;
SAMPLE_FLAGS(sample_feature_flags_t)
```

### [platform] {#attr-platform}

Indicates platform-specific declarations. Format: `[platform(windows,linux)]`. Generates conditional compilation in target languages.

@warning Generators currently do not implement this attribute.

### [value] {#attr-value}

Explicitly sets the numeric value for enum constants or default values for fields/arguments. Accepts expressions.

@warning Currently only supports integers and enum constants.

### [type] {#attr-type}

Specifies the underlying type for declarations. Used implicitly in shorthand syntax like `{Float32}`.

```
@idl
@ Color values.
struct Color
    field Red {Float32} @ Red channel clear value.
    field Green [type(Float32)] @ Green channel clear value.
```

### [static] {#attr-static}

Marks methods or properties as class-level rather than instance-level members.

### [ctor] {#attr-ctor}

Designates a method as a constructor. Implies `[static]`. In OOP languages, enables `new` syntax:

```
@idl
@ Brief.
interface ObjType
    @ Creates new object instance
    @ New instance [return]
    method CreateByName {ObjType} [ctor]
        arg Name {Str} @ Name of object.

    @ Creates new object instance
    method CreateByValue {Bool} [ctor]
        arg Name {Str} @ Name of object.
        arg Value {Float32} @ Name of object.
        arg Obj {ObjType} [result] @ New object instance.
```

In the second case, the new instance is returned as an argument:

```
@c
/**
 * @brief     Creates new object instance
 * @param[in] name Name of object.
 * @return    New instance
 */
sample_api sample_obj_type_t
sample_obj_type_create_by_name(sample_utf8_t name);

/**
 * @brief      Creates new object instance
 * @param[in]  name Name of object.
 * @param[in]  value Name of object.
 * @param[out] obj New object instance.
 */
sample_api sample_bool_t
sample_obj_type_create_by_value(sample_utf8_t name,
                                sample_float32_t value,
                                sample_obj_type_t* obj);
```

In OOP languages ​​these methods will become constructors:

```javascript
@javascript
const inst1 = new sample.ObjType('test');
const inst2 = new sample.ObjType('test', 3.4);
```

### [this] {#attr-this}

Identifies the implicit instance parameter in methods. Remapped to `this`/`self` in OOP languages.

### [get] {#attr-get}

Links a property to its getter method. Format: `[get(GetMethodName)]`.

### [set] {#attr-set}

Links a property to its setter method. Format: `[set(SetMethodName)]`.

### [handle] {#attr-handle}

Declares index descriptors. Useful for providing resources with an index or other structure. For example, distributing resources linearly from a pool in games:

```
@idl
@ Handle type.
@ This structure describes the handle template. [detail]
struct Handle [handle]
    field Index {Uint16} @ The type that determines the size of the handle.

handle Buffer        {Handle} @ Handle to GPU buffer resource.
handle Texture       {Handle} @ Handle to GPU texture resource.
handle Technique     {Handle} @ Handle to rendering technique.
handle DescriptorSet {Handle} @ Handle to resource binding set.
```

### [cname] {#attr-cname}

Overrides the default C identifier naming:

```
@idl
@ Keyboard scancodes.
enum Scancode
    const Unknown @ Unidentified key.
    const Digit0 [cname(0)] @ 0 key.
    const Digit1 [cname(1)] @ 1 key.
    const Digit2 [cname(2)] @ 2 key.
    ...
```

C output:

```
@c
/**
 * @brief Keyboard scancodes.
 */
typedef enum
{
    GERIUM_SCANCODE_UNKNOWN = 0, /**< Unidentified key. */
    GERIUM_SCANCODE_0       = 1, /**< 0 key. */
    GERIUM_SCANCODE_1       = 2, /**< 1 key. */
    ...
```

### [array] {#attr-array}

Indicates array:

```
@idl
@ Compilation options.
@ This object specifies various compilation options. [detail]
interface Options
    prop ImportDirs [get(GetImportDirs),set(SetImportDirs)] @ Directories to search for files when importing.

    @ Returns an array of directories to search for imports.
    @ These paths are used to search source code when an import is encountered during compilation. [detail]
    @ {SetImportDirs} [see]
    method GetImportDirs [const]
        arg Options {Options} [this] @ Target options.
        arg DirCount {Uint32} [in,out] @ Number of directories.
        arg Dirs {Str} [result,array(DirCount)] @ Import directories.

    @ Configures directories to search for source files.
    @ These paths are used to search source code when an import is encountered during compilation. [detail]
    @ {GetImportDirs} [see]
    method SetImportDirs
        arg Options {Options} [this] @ Target options.
        arg DirCount {Uint32} @ Number of directories.
        arg Dirs {Str} [const,array(DirCount)] @ Import directories.
```

C output:

```
@c
/**
 * @brief         Returns an array of directories to search for imports.
 * @details       These paths are used to search source code when an import is encountered during compilation.
 * @param[in]     options Target options.
 * @param[in,out] dir_count Number of directories.
 * @param[out]    dirs Import directories.
 * @sa            ::sample_options_set_import_dirs
 */
sample_api void
sample_options_get_import_dirs(sample_options_t options,
                               sample_uint32_t* dir_count,
                               sample_utf8_t* dirs);

/**
 * @brief     Configures directories to search for source files.
 * @details   These paths are used to search source code when an import is encountered during compilation.
 * @param[in] options Target options.
 * @param[in] dir_count Number of directories.
 * @param[in] dirs Import directories.
 * @sa        ::sample_options_get_import_dirs
 */
sample_api void
sample_options_set_import_dirs(sample_options_t options,
                               sample_uint32_t dir_count,
                               const sample_utf8_t* dirs);
```

JavaScript use:

```
@javascript
const options = new idlc.Options;
options.importDirs = [ "path1", "path2", "path3" ];
```

As you can see in higher-level languages, working with arrays becomes more native.

Arrays are also supported in structure fields. But fixed-length arrays are also supported in structure fields:

```
@idl
@ Brief.
struct Test
    field Values {Float32} [const,array(Size)] @ Values.
    field Size {Uint32} @ Count values.
    field Symbol {Char} [const,array(5)] @ Symbol.
```

C output:

```
@c
/**
 * @brief Brief.
 */
typedef struct
{
    const sample_float32_t* values; /**< Values. */
    sample_uint32_t         size; /**< Count values. */
    sample_char_t           symbol[5]; /**< Symbol. */
} sample_test_t;
```

JavaScript use:

```
@javascript
func({ values: [ 3.4, 2.3 ], symbol: 'a' });
```

### [datasize] {#attr-datasize}

Marks fields or arguments of types `Date` and `ConstData` as dimensional buffers:

@warning I'm not sure yet if this attribute is necessary

### [const] {#attr-const}

Refers to immutable fields, parameters, or methods that do not change the state of the instance.

### [ref] {#attr-ref}

Hints that a field or argument should be passed by reference rather than by value.

### [refinc] {#attr-refinc}

Marks a method as a specific method for reference counter incrementing.

```
@idl
@ Compilation options.
@ This object specifies various compilation options. [detail]
interface Options
    @ Increments reference count.
    @ Manages options instance lifetime. [detail]
    @ Reference to same options. [return]
    @ {Destroy} [see]
    method Reference {Options} [refinc]
        arg Options {Options} [this] @ Target options instance.

    @ Releases options instance.
    @ Destroys when reference count reaches zero. [detail]
    @ {Reference} [see]
    method Destroy [destroy]
        arg Options {Options} [this] @ Options to destroy.
```

High-level languages ​​will manage the lifetime of an object either through the garbage collector or automatically through this method (the method itself will be hidden).

### [userdata] {#attr-userdata}

Marks parameters for passing context data to callbacks.

In high-level languages ​​it will be removed, the generator will use it automatically for data closure.

### [errorcode] {#attr-errorcode}

Designates enums as error code containers. Enables special error handling.

You can also mark the function. Read about it in the [Error Handling](#error-handling) section

### [noerror] {#attr-noerror}

Marks an enum constant as not an error.

### [result] {#attr-result}

Marks the argument as the return value. Higher priority than the return value (unless it is an enumeration with error codes).

### [destroy] {#attr-destroy}

Marks resource cleanup methods.

### [in] {#attr-in}

Input-only parameters (default). Helps with code generation for `[out]` pairs.

### [out] {#attr-out}

Output parameters.

### [optional] {#attr-optional}

Marks nullable parameters. Generates different signatures in strict languages.

### [tokenizer] {#attr-tokenizer}

Overrides default name tokenization rules for specific declarations.

```
@idl
@ Texture formats.
enum Format
    const R4G4 [tokenizer(0)] @ R4G4.
    const BC1RgbSrgb [tokenizer(3-3)] @ block-compressed format.
    const PVRTC2v2BppSrgb [tokenizer(6-^1-4)] @ PVRTC compressed format.
```

In this example, the names will be tokenized as follows:
- `R4G4`;
- `BC1 Rgb Srgb`;
- `PVRTC2 2Bpp Srgb`.

The number in the tokenizer indicates how many characters to take as a token. Numbers are separated by hyphens. If there is `^` before the number, it indicates how many characters to skip.

### [version] {#attr-version}

Specifies API versioning. Format: [version(major,minor,micro)].

### [brief] {#attr-brief}

Short description attribute (default for standalone `@ docs`).

```
@idl
@ Logging severity levels.
enum LoggerLevel
    ...

// or

@ Logging severity levels. [brief]
enum LoggerLevel
    ...
```

### [detail] {#attr-detail}

Detailed description attribute (default for inline `@ docs`).

```
@idl
@ Logging severity levels. [detail]
enum LoggerLevel
    ...

// or

enum LoggerLevel @ Logging severity levels. [detail]
    ...
    
// or

enum LoggerLevel @ Logging severity levels.
    ...
```

### [author] {#attr-author}

Credits the original author:

```
@idl
@ IDLC
@ Author Name <authorname@email.com> [author]
api Idl
```

### [see] {#attr-see}

See also:

```
@idl
@ Callback to get sources.
@ If the callback allocates memory, then you can free it in the callback {ReleaseImportCallback}. [see]
callback ImportCallback {Source} [ref,optional]
    ...
```

### [note] {#attr-note}

Highlights important usage notes in documentation.

### [warning] {#attr-warning}

Marks warnings in documentation.

### [copyright] {#attr-copyright}

Copyright notice:

```
@idl
@ IDLC
@ MIT License [copyright]
api Idl
```

### [license] {#attr-license}

License text.

### [return] {#attr-return}

Documents the semantics of the return value from a method/function/callback.

## Error Handling {#error-handling}

The language provides special facilities for cross-language error handling. This is necessary because C lacks exceptions and typically uses error codes, while JS has exceptions. Therefore, IDL offers a built-in way to describe errors that works natively across all target languages.

```
@idl
@ Result codes.
@ Enumeration of result codes. [detail]
enum Result [errorcode]
    const Success [noerror] @ Indicates success (this is not an error).
    const ErrorUnknown @ Unknown error.
    const ErrorOutOfMemory @ Out of memory.
    const ErrorInvalidArg @ Invalid argument.
    const ErrorFileCreate @ Failed to create file.
    const ErrorCompilation @ Compilation failed.
    const ErrorNotSupported @ Not supporeted.

@ Converts error code to descriptive string.
@ Provides a text description for the result code. [detail]
@ Corresponding text description of the result code. [return]
func ResultToString {Str} [errorcode]
    arg Result {Result} @ Result code.

@ Compilation options.
@ This object specifies various compilation options. [detail]
interface Options
    @ Creates new options instance.
    @ Creates an object for setting compiler options. [detail]
    @ New options instance. [return]
    method Create {Result} [ctor]
        arg Options {Options} [result] @ New options instance.
```

Note that the `Create` method returns a new instance via an argument. This is quite normal, because the programmer will not be able to ignore the object allocation. Instead, the return value is an error code.

In C, this would look like this:

```
@c
/**
 * @brief      Creates new options instance.
 * @details    Creates an object for setting compiler options.
 * @param[out] options New options instance.
 * @return     New options instance.
 * @ingroup    functions
 */
idl_api idl_result_t
idl_options_create(idl_options_t* options);
```

Which can be used as:

```
@c
idl_options_t options;
idl_result_t code = idl_options_create(&options);
if (code != IDL_RESULT_SUCCESS) {
    printf("%s\n", idl_result_to_string(code));
    return;
}
idl_options_destroy(options);
```

In languages ​​with exceptions, when calling functions/methods if an error code is returned (in any way), in case of an error an exception will be thrown with the text of the error constant name. If there is a function marked `[errorcode]`:

```
@idl
func ResultToString {Str} [errorcode]
```

Then it will be used to convert the error code into a text message for the exception.

JavaScript:

```javascript
@javascript
const options = new module.Options;
```

If the object cannot be created, an exception will be thrown.

## Next Steps {#learn-how-to-install}

Learn how to install and use the compiler.

<div class="section_buttons">

| Previous                        |                                        Next |
|:--------------------------------|--------------------------------------------:|
| [Quick Start](quick-start.html) | [Embedded Compiler](embedded-compiler.html) |

</div>
