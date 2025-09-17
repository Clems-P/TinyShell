# TinyShell

TinyShell is a minimal shell implementation written in C, designed specifically for microcontrollers and embedded systems with limited memory. All buffers are statically allocated to avoid dynamic memory allocation, making it suitable for resource-constrained environments.

## Features

- Execute built-in and external commands
- Minimal and easy-to-understand codebase
- All buffers are static (no dynamic memory allocation)
- Suitable for microcontrollers and embedded systems

## Build Instructions

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage

After building, run the shell:

```sh
./tinyshell
```

Type commands as you would in a regular shell. Use `exit` to quit.

## Example

```sh
tinyshell> help
tinyshell> status
```

## Requirements

- CMake 3.10+
- GCC or Clang
- Suitable for microcontroller toolchains

## License

MIT License