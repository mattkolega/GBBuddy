# GBBuddy
A work-in-progress Game Boy emulator written using modern C++. I currently have a fully implemented CPU and am working on getting graphics functional. It cannot currently play games.

My aim for this project is to support both GB and GBC games with fairly accurate emulation.

## Building
This project requires:
- A C/C++ compiler with support for **C++23**. So far only tested with Clang
- A version of CMake as outlined in the root CMakeLists.txt
- Build system such as Make or Ninja

First clone this repo and cd into it. Then run the following:

```sh
cmake -S . -B build
cmake --build build
```

## Testing
GBBuddy comes with a test suite to validate functionality of the various emulator components.

To run it, do the following from the project root:

```sh
cd build
ctest
```

## References
- [Pan Docs](https://gbdev.io/pandocs/)
- [gbops](https://izik1.github.io/gbops/)
- [RGBDS CPU opcode reference](https://rgbds.gbdev.io/docs/v0.8.0/gbz80.7)
- [SM83 per-instruction tests](https://github.com/SingleStepTests/sm83)

## Licence
[MIT](LICENSE)