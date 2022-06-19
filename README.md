# forest

**ANSI escape code text formatter**

## Introduction

`forest` is a loose acronym for **for**mat **esc**ape **t**erminal. It is a single-header lightweight library for transforming marked-up text into a stream of characters and [ANSI escape code](https://en.wikipedia.org/wiki/ANSI_escape_code)s. This text can then be piped out to terminals that support such escape sequences.

## Example

`forest` provides some flexibility in usage of its API:

1. `print`: The most straightforward function, prints to `stdout`
2. `print_to`: Prints to provided `std::FILE*` stream
3. `format`: Obtains formatted text as a `std::string`
4. `format_to`: fully `constexpr` code path that formats text to a provided output iterator
5. `literal`: compile-time alternative for literals

[example/forest-example.cpp](example/forest-example.cpp) contains some samples.

## Markup Language

Below is a description of the markup language used by all functions exposed by the API

### Style

A style consists of two things

- Scope

- Logic

### Logic

Logic depicts the operations the style will perform in order to apply the changes to the group of text

### Scope

Together with logic, scope refers to the group of text to which the style can be applied to and how. There are 2 types of scopes

#### Opened

Styles with an open scope can and will be used to apply styling or logic to

- anything after the tag

- anything leading up to the tag

They are of the form `<style>...`

#### Closed

Styles with a closed scope can and will be used to apply styling to anything within the scope of the of the **opening** and **closing** tags. They are of the form

`<style>...</style>`

## Style Types

There are 3 types of styles

- **Hint**

- **Decoration**

- **Property**

### Hint

Refers to a set of styles that are single-use and open scoped

### Decoration

Refers to all closed scoped styles

### Property

Refers to a set of styles that parameterized over a set of specified values which can be given via the `=` syntax

- `<style=...>...</style>`

- `<style=...>...`

**Note**

**Properties with either open/closed scope can be used interchangeibly within a single group of text**

#### Color Properties

Styles with color properties are denoted by an `R, G, B` in their properties column in the table below. The set of acceptable values for each color unit is then any number within the range `[0..5]`. For example red would be `<style=500>...</style>`

**Note**

**Specifying a value outside of the allowed range is not an error as all values are effectively clamped**

## Styles

| NAME           | TAG            | ALT. TAG | OPENING TAG        | CLOSING TAG        | PROPERTIES | TYPE   |
| -------------- | -------------- | -------- | ------------------ | ------------------ | ---------- | ------ |
| bold           | `<b>`          |          | :white_check_mark: | :white_check_mark: | :x:        | `D`    |
| italic         | `<i>`          |          | :white_check_mark: | :white_check_mark: | :x:        | `D`    |
| underline      | `<u>`          |          | :white_check_mark: | :white_check_mark: | :x:        | `D`    |
| dim            | `<dim>`        |          | :white_check_mark: | :white_check_mark: | :x:        | `D`    |
| blink          | `<blink>`      |          | :white_check_mark: | :white_check_mark: | :x:        | `D`    |
| invert         | `<invert>`     |          | :white_check_mark: | :white_check_mark: | :x:        | `D`    |
| strike through | `<strike>`     |          | :white_check_mark: | :white_check_mark: | :x:        | `D`    |
| RGB            | `<rgb>`        |          | :white_check_mark: | :white_check_mark: | `R, G, B`  | `D\|P` |
| background     | `<background>` | `<bg>`   | :white_check_mark: | :white_check_mark: | `R, G, B`  | `D\|P` |
| reset          | `<reset>`      |          | :white_check_mark: | :x:                | :x:        | `H`    |
| clear          | `<clear>`      |          | :white_check_mark: | :x:                | :x:        | `H`    |

### Table Legend

Due to there being styles that fall under 1 or more types and the type names being rather long for the table cells, a combination of the first letter of each type separated by a `|` character will be used instead where applicable

### Tag Names

For the most part all tags clearly describe their intent, except for the follwing which may cause some confusion:

#### RGB

This tag acts essentially as a way to style the **foreground color** of a string and together with `background` can be used to create a more controlled version of what `inverse` provides

#### Clear

Unlike `reset`, which removes all styling apllied to a string, is used to **clear the output** (whatever that may be)

## Usage

### Requirements

1. C++17
2. CMake 3.17+ [optional]

### Steps

1. Copy [include/forest/forest.hpp](include/forest/forest.hpp) into your build tree, or
2. Use CMake:
   1. Import `forest`
      1. Add `forest` to build tree via `add_subdirectory(path/to/forest)`, or
      2. Locate `forest` via `find_package(forest)` (must be installed to a path in `CMAKE_INSTALL_PREFIX`)
   2. Link to `forest` via `target_link_libraries(your_project PRIVATE forest::forest)` (use `PUBLIC` instead to propagate include paths to all dependencies)
