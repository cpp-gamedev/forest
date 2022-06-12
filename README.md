# forest

**ANSI escape code text formatter**

## Introduction

`forest` is a loose acronym for **for**mat **esc**ape **t**erminal. It is a single-header lightweight library for transforming marked-up text into a stream of characters and [ANSI escape code](https://en.wikipedia.org/wiki/ANSI_escape_code)s. This text can then be piped out to terminals that support such escape sequences.

## Example

`forest` provides some flexibility in usage of its API:

1. `print`: The most straightforward function, prints to `stdout`
1. `print_to`: Prints to provided `std::FILE*` stream
1. `format`: Obtains formatted text as a `std::string`
1. `format_to`: fully `constexpr` code path that formats text to a provided output iterator
1. `literal`: compile-time alternative (`consteval`) for literals

[example/forest-example.cpp](example/forest-example.cpp) contains some samples.

## Usage

1. Copy [include/forest/forest.hpp](include/forest/forest.hpp) into your build tree, or
1. Use CMake to:
  1. `add_subdirectory(path/to/forest)` or `find_package(forest)` (if installed and visible to CMake)
  1. `target_link_libraries(your_project PRIVATE forest::forest)`
