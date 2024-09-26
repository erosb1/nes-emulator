# riscv-toolchain.cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)

# Specify the cross compiler
set(CMAKE_C_COMPILER riscv32-unknown-elf-gcc)
set(CMAKE_CXX_COMPILER riscv32-unknown-elf-g++)
set(CMAKE_ASM_COMPILER riscv32-unknown-elf-as)

# Specify the linker, objcopy, and objdump
set(CMAKE_LINKER riscv32-unknown-elf-ld)
set(CMAKE_OBJCOPY riscv32-unknown-elf-objcopy)
set(CMAKE_OBJDUMP riscv32-unknown-elf-objdump)

# Set a global variable for RISC_V
set(RISC_V TRUE CACHE INTERNAL "Building for RISC-V")
