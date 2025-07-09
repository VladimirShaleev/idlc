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

```c
logger_test_name
```

and in JS:

```javascript
testName
```

Note that C isn't an OOP language, but many C libraries (like cairo, etc.) manually "implement OOP": they add opaque types that are passed as the first parameter in functions (acting as `this`). The function prefix indicates the "class" name. These opaque types can be implemented in C or even C++, as in this library.

In JS, the interface translates to a JS class with methods and other features.

@note If default tokenization doesn't work for your case, you can use the `[tokenizer]` attribute as shown in this TODO section.

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

```c
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

Similarly, in C `Data` and `ConstData` are represented as pointers, while in JS they might be sized `Uint8Array` or `ArrayBuffer`. IDL provides a unified way to properly format buffers in syntax that feels native to each language (pointers in C, buffers/strings/arrays in JS). More on this later in the TODO section.

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

## Interfaces {#interfaces}

## Methods {#methods}

## Callbacks {#callbacks}

## Properties {#properties}

## Events {#events}

## Import {#import}

## Attributes {#attributes}

## Error Handling {#error-handling}

The language provides special facilities for cross-language error handling. This is necessary because C lacks exceptions and typically uses error codes, while JS has exceptions. Therefore, IDL offers a built-in way to describe errors that works natively across all target languages.

# Using the idlc Command Line Tool {#using-idlc}

<div class="section_buttons">
 
| Previous                        |                         Next |
|:--------------------------------|-----------------------------:|
| [Quick Start](quick-start.html) | [Documentation](topics.html) |
 
</div>
