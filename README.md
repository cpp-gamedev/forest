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

### Open & Closed tags

Both `open` and `closed` tags behave the same the former is just a shorthand for the latter:

- Closed tags are of the form `<style>...</style>`

- Open tags are of the form `<style>...`

**note** **Both can be used interchangeibly within a single string**

### Properties

Properties can be used in both open & closed tags that support them and are

of the form:

- `<style=...>...`

- `<style=...>...</style>`

#### Color Properties

`R, G, B` can be any number ranging from :zero::arrow_right::five:

## Style Tags

| NAME           | TAG            | ALT. TAG | CLOSED             | OPEN               | PARAMETERS |
| -------------- | -------------- | -------- | ------------------ | ------------------ | ---------- |
| bold           | `<b>`          |          | :white_check_mark: | :white_check_mark: | :x:        |
| italic         | `<i>`          |          | :white_check_mark: | :white_check_mark: | :x:        |
| underline      | `<u>`          |          | :white_check_mark: | :white_check_mark: | :x:        |
| dim            | `<dim>`        |          | :white_check_mark: | :white_check_mark: | :x:        |
| blink          | `<blink>`      |          | :white_check_mark: | :white_check_mark: | :x:        |
| invert         | `<invert>`     |          | :white_check_mark: | :white_check_mark: | :x:        |
| strike through | `<strike>`     |          | :white_check_mark: | :white_check_mark: | :x:        |
| RGB            | `<rgb>`        |          | :white_check_mark: | :white_check_mark: | `R, G, B`  |
| background     | `<background>` | `<bg>`   | :white_check_mark: | :white_check_mark: | `R, G, B`  |
| reset          | `<reset>`      |          |                    | :white_check_mark: | :x:        |
| clear          | `<clear>`      |          |                    | :white_check_mark: | :x:        |

### Tag Names

For the most part all tags clearly describe their intent, except for the follwing which may cause some confusion:

#### RGB

This tag acts essentially as a way to style the **foreground color** of a string and together with `background` can be used to create a more controlled version of what `inverse` provides

#### Clear

Unlike `reset`, which removes all styling apllied to a string, is used to **clear the output** leading up to where it was last used

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
