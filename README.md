# dtek-project

## Resources

### Formats

[iNES](https://www.nesdev.org/wiki/INES)
[NES 2.0 (backwards compatible with iNES)](https://www.nesdev.org/wiki/NES_2.0)

### NEStest (test ROM)

[Download](http://nickmass.com/images/nestest.nes)
[Docs](https://www.qmtpro.com/~nes/misc/nestest.txt)
[Verified log](https://www.qmtpro.com/~nes/misc/nestest.log)

### 6502 reference sheets

[Masswerk](https://www.masswerk.at/6502/6502_instruction_set.html)
[Obelisk](https://www.nesdev.org/obelisk-6502-guide/reference.html)
[Official](http://www.6502.org/tutorials/6502opcodes.html)

### 6502 tutorials

[Easy 6502 (interactive)](http://skilldrick.github.io/easy6502)

### javidx9 youtube emulator series
https://www.youtube.com/watch?v=nViZg02IMQo&list=PLrOv9FMX8xJHqMvSGB_9G9nZZ_4IgteYf


## Building for the Host System

This option builds the project using the default C compiler on your system. 
Graphics are rendered in an SDL window, and the standard computer keyboard is used as the controller. 
This build includes all `.h` and `.c` files from the `emulator/` and `sdl/` directories.

### Instructions:

```sh
mkdir build
cd build
cmake ..
make
```
The resulting executable will be named `main`, `main.exe`, or `main.bin`, depending on your operating system.

## Building for RISC-V (DTEKV DE10-Lite)
This build targets the RISC-V architecture for the DTEKV DE10-Lite board. 
It includes `.h` and `.c` files from the `emulator/`, `dtekv-build/`, and `dtekv-drivers/` directories. 
Graphics will be output to a VGA screen, and input will come from an NES controller.

**Note**: You must perform this build on a Linux machine.

```sh
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../riscv-toolchain.cmake ..
make
```
The executable will be named `main.bin`. Additionally, a disassembled RISC-V assembly dump will be generated in `main.elf.txt`.

## Running nestest.nes
This is how you can test the CPU using the `tests/nestest.nes` rom:
```sh
cd build
cmake ..
make
make nestest_cpu_only_diff
```

This will run a bash script that:
1. Runs the program with the `test/nestest.nes` rom as input.
2. Outputs the cpu execution log to a file named `build/output.txt`.
3. Compares the difference between `build/output.txt` file and the `tests/nestest_cpu_only.txt` file (which has the correct logs).
4. Outputs to the console the first line it finds that differs between the two log files.
