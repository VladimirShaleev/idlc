@page language-guide Language Guide

@tableofcontents

# Interface Definition Language {#idl}

This guide describes the use of IDL for creating library specifications, its syntax, and capabilities.

## Declarations {#declarations}

All declarations, whether they're structure declarations or functions and their arguments, must begin with the appropriate keyword followed by the declaration name. Example:

```
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

```
logger_test_name
```

and in JS:

```javascript
testName
```

Note that C isn't an OOP language, but many C libraries (like cairo, etc.) manually "implement OOP": they add opaque types that are passed as the first parameter in functions (acting as `this`). The function prefix indicates the "class" name. These opaque types can be implemented in C or even C++, as in this library.

In JS, the interface translates to a JS class with methods and other features.

@note If default tokenization doesn't work for your case, you can use the `[tokenizer]` attribute as shown in this [[tokenizer]](#attr-tokenizer) section.

## Documentation {#documentation}

The language requires that all declarations have at least a brief description or detailed documentation.

Documentation begins with the `@` symbol followed by documentation until the end of the line. You can end the documentation with an attribute indicating what it refers to, for example:

```
@ Color values [brief]
@ Detailed description. [detail]
struct Color
    ...
```
The `[brief]` attribute is the default attribute when no attribute is specified.

You can also place documentation on the same line as the declaration. In this case, the default attribute for this documentation will be `[detail]`. The example below is equivalent to the previous one:

```
@ Color values
struct Color @ Detailed description. [detail]
    ...
```

This isn't very elegant. This feature is mainly intended for cleaner documentation of fields and function arguments:

```
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
@ Graphics features.
enum Feature
    const None {Int32} @ No special features
```

By default, each constant's value increments by one from the previous constant's value. The first constant has a value of 0. However, we can explicitly specify constant values:

```
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
@ Color values.
struct Color
    field Red {Float32} @ Red channel clear value.
    field Green {Float32} @ Green channel clear value.
    field Blue {Float32} @ Blue channel clear value.
    field Alpha {Float32} @ Alpha channel clear value.
```

You can also set default values for fields. These will be assigned during initialization in languages that support it (e.g., C++):

```
@ Color values.
struct Color
    field Red {Float32} @ Red channel clear value.
    field Green {Float32} @ Green channel clear value.
    field Blue {Float32} @ Blue channel clear value.
    field Alpha {Float32} : 1 @ Alpha channel clear value.
```

@note Currently only integers and constants can be specified as defaults.

```
@ Brief.
struct Test
    field Flags {Feature} : Feature.Bindless, Feature.MeshShader @ Sample.
```

Note the use of the fully qualified name Feature.Bindless to indicate that Bindless belongs to the Feature declaration, not Test.

## Functions {#functions}

The simplest function can be declared as:

```
@ Sample
func Func
```

Let's add some arguments:

```
@ Sample
func Func
    arg Value @ First arg
    arg OtherValue @ Second arg
```

By default, argument types are `Int32`. Let's specify return type and argument types:


```
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
@ Compilation options.
@ This object specifies various compilation options. [detail]
interface Options
    event Writer [get(GetWriter),set(SetWriter)] @ Event that occurs when the compiler outputs.
```

This example comes from the compiler's own specification.

This can be useful in certain languages. Let's look at JavaScript code for an online editor using this event with the callback:

```javascript
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

TODO

## Attributes {#attributes}

As you may have noticed, the language provides only declarations - everything else is achieved by attaching attributes to these declarations. Even argument and field types are actually attributes:

```
@ Color values.
struct Color
    field Red {Float32} @ Red channel clear value.
    field Green {Float32} @ Green channel clear value.
    field Blue {Float32} @ Blue channel clear value.
    field Alpha {Float32} @ Alpha channel clear value.
```

This is syntactic sugar for the `[type]` attribute:

```
@ Color values.
struct Color
    field Red [type(Float32)] @ Red channel clear value.
    field Green [type(Float32)] @ Green channel clear value.
    field Blue [type(Float32)] @ Blue channel clear value.
    field Alpha [type(Float32)] @ Alpha channel clear value.
```

The same applies to default values:

```
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

TODO

### [hex] {#attr-hex}

TODO

### [platform] {#attr-platform}

TODO

### [value] {#attr-value}

TODO

### [type] {#attr-type}

TODO

### [static] {#attr-static}

TODO

### [ctor] {#attr-ctor}

TODO

### [this] {#attr-this}

TODO

### [get] {#attr-get}

TODO

### [set] {#attr-set}

TODO

### [handle] {#attr-handle}

TODO

### [cname] {#attr-cname}

TODO

### [array] {#attr-array}

TODO

### [datasize] {#attr-datasize}

TODO

### [const] {#attr-const}

TODO

### [ref] {#attr-ref}

TODO

### [refinc] {#attr-refinc}

TODO

### [userdata] {#attr-userdata}

TODO

### [errorcode] {#attr-errorcode}

TODO

### [noerror] {#attr-noerror}

TODO

### [result] {#attr-result}

TODO

### [destroy] {#attr-destroy}

TODO

### [in] {#attr-in}

TODO

### [out] {#attr-out}

TODO

### [optional] {#attr-optional}

TODO

### [tokenizer] {#attr-tokenizer}

TODO

### [version] {#attr-version}

TODO

### [brief] {#attr-brief}

TODO

### [detail] {#attr-detail}

TODO

### [author] {#attr-author}

TODO

### [see] {#attr-see}

TODO

### [note] {#attr-note}

TODO

### [warning] {#attr-warning}

TODO

### [copyright] {#attr-copyright}

TODO

### [license] {#attr-license}

TODO

### [return] {#attr-return}

TODO

## Error Handling {#error-handling}

The language provides special facilities for cross-language error handling. This is necessary because C lacks exceptions and typically uses error codes, while JS has exceptions. Therefore, IDL offers a built-in way to describe errors that works natively across all target languages.

TODO

# Using the idlc Command Line Tool {#using-idlc}

TODO

<div class="section_buttons">

| Previous                        |                         Next |
|:--------------------------------|-----------------------------:|
| [Quick Start](quick-start.html) | [Documentation](topics.html) |

</div>
