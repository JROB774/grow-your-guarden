# nksdk

These are my personal libraries I use for my projects, they should currently be
considered WIP and lack proper documentation, tests, functionality, etc. Hopefully
in the future I will get around to adding these things to make them more accessible
and helpful for others, but feel free to use or contribute to them now if you wish!

I'm primarily open sourcing these in their current state just because the libs are
being used in a few of my projects that I wish to release the source to.

They are all single-header libraries similar to the [stb libs](https://github.com/nothings/stb)
so some of them will require `NK_xxx_IMPLEMENTATION` to be defined in a file. Each
library is mostly independent from one another so you can pick and choose what you
want from them. However, all of the libs require `nk_define.h` in order to work.

## Libraries

| Library     | Support | LoC  | Description                                          |
| ----------- | ------- | ---: | ---------------------------------------------------- |
| nk_define   | C/C++   | 197  | utility defines used by other nk libraries           |
| nk_endian   | C/C++   | 55   | simple byte-swapping functions                       |
| nk_filesys  | C/C++   | 691  | cross-platform file system functions                 |
| nk_npak     | C/C++   | 182  | file format for storing multiple files in a package  |
| nk_print    | C/C++   | 82   | extended text print functionality with formatting    |
| nk_test     | C/C++   | 81   | basic testing API for creating test suites           |
| nk_array    | C++     | 367  | simple generic dynamic array data structure          |
| nk_defer    | C++     | 20   | macro for deferring code execution to end of scope   |
| nk_hashmap  | C++     | 390  | simple generic hashmap data structure                |
| nk_hashset  | C++     | 358  | simple generic hashset data structure                |
| nk_math     | C++     | 761  | math functions and types (vector, matrix, etc.)      |
| nk_stack    | C++     | 51   | simple generic fixed-size stack data structure       |
| nk_string   | C++     | 348  | simple dynamic string data structure                 |
