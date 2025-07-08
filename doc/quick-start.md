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
    @ Create new Vehicle instance.
    @ Vehicle instance. [return]
    method Create {Vehicle} [ctor]
        arg Name {Str} @ Name of vehicle.

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
  * See the **SECTION** for customizing tokenization rules.
- **Documentation requirements**:
  * Every declaration must include at least `[brief]` or `[detail]` documentation.
  * Documentation starts with the `@` symbol - everything until the newline counts as documentation.
- **Attributes**:
  * Optional attributes may follow declarations or documentation, enclosed in square brackets `[]` and comma-separated (e.g., `[author]` or `[const,ref]`).
  * Note: `arg Value {Vector} [const,ref]` is syntactic sugar for `arg Value [type(Vector),const,ref]` (here `type` explicitly specifies the argument type).
- **Documentation placement**: documentation on the same line after a declaration defaults to [detail] documentation for that declaration.
- **Formatting rules**: indentation and line breaks are insignificant in IDL, except within documentation contexts.

<div class="section_buttons">
 
| Previous                        |                      Next |
|:--------------------------------|--------------------------:|
| [Introduction](introduction.md) | [Tutorial](tutorial.html) |
 
</div>
