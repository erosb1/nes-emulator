cmake_minimum_required(VERSION 3.16)

project(NESEmulator C)
set(CMAKE_C_STANDARD 11)


# If building for host system (development)
if(NOT RISC_V)
    message(STATUS "Building CMake for host system (development)")


    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -flto -march=native -mtune=native -ffast-math -fomit-frame-pointer -DNDEBUG")

    # Set compiler flags for Debug builds
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -g -O0")

    # Include directories
    include_directories(${CMAKE_SOURCE_DIR}/dev)
    include_directories(${CMAKE_SOURCE_DIR}/emulator)

    # Collect all source files from sdl and emulator
    file(GLOB_RECURSE DEV_SOURCES ${CMAKE_SOURCE_DIR}/dev/*.c)
    file(GLOB_RECURSE EMULATOR_SOURCES ${CMAKE_SOURCE_DIR}/emulator/*.c)

    # Add executable
    add_executable(main ${DEV_SOURCES} ${EMULATOR_SOURCES})

    # Find SDL2 (for development)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    target_link_libraries(main SDL2::SDL2)

    # Add a custom target for testing against nestest
    add_custom_target(nestest_cpu_only_diff
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/main ${CMAKE_SOURCE_DIR}/tests/nestest.nes --nestest > ${CMAKE_CURRENT_BINARY_DIR}/output.txt
        COMMAND echo "NESTEST Complete -- No Errors Detected" >> ${CMAKE_CURRENT_BINARY_DIR}/output.txt
        COMMAND diff ${CMAKE_CURRENT_BINARY_DIR}/output.txt ${CMAKE_SOURCE_DIR}/tests/nestest.txt | head -n 2 | tail -n 1
        DEPENDS main
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running NES Emulator tests and comparing logs..."
        VERBATIM
    )

# If cross-compiling to RISC-V
else()
    message(STATUS "Building CMake for RISC-V DTEKV-BOARD cross-compilation")
    add_compile_definitions(RISC_V)

    # Set object directory
    set(OBJ_DIR "${CMAKE_SOURCE_DIR}/build")

    # Set toolchain and flags
    set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/dtekv-build/dtekv-script.lds")
    set(SOFTFLOAT_LIB "${CMAKE_SOURCE_DIR}/dtekv-build/softfloat.a")

    # Set target names
    set(TARGET_ELF "main.elf")
    set(TARGET_BIN "main.bin")

    # Include directories
    include_directories(${CMAKE_SOURCE_DIR}/dtekv-build)
    include_directories(${CMAKE_SOURCE_DIR}/dtekv-drivers)
    include_directories(${CMAKE_SOURCE_DIR}/emulator)

    # Get all .c files needed for compilation
    file(GLOB_RECURSE DTEKV_BUILD_SOURCES ${CMAKE_SOURCE_DIR}/dtekv-build/*.c)
    file(GLOB_RECURSE DTEKV_DRIVERS_SOURCES ${CMAKE_SOURCE_DIR}/dtekv-drivers/*.c)
    file(GLOB_RECURSE EMULATOR_SOURCES ${CMAKE_SOURCE_DIR}/emulator/*.c)
    set(ALL_SOURCES ${DTEKV_DRIVERS_SOURCES} ${EMULATOR_SOURCES} ${DTEKV_BUILD_SOURCES})

    # Specify any necessary compile flags
    set(CMAKE_C_FLAGS "-Wall -Wno-unused-variable -Wno-unused-function -Wno-array-bounds -nostdlib -O3 -mabi=ilp32 -march=rv32imzicsr")

    # Specify output ELF target
    add_executable(${TARGET_ELF} ${ALL_SOURCES})

    # Use target_link_options to pass custom linker flags, including the linker script
    target_link_options(${TARGET_ELF} PRIVATE -T ${LINKER_SCRIPT})

    # Link any libraries (like softfloat or any other required libraries)
    target_link_libraries(${TARGET_ELF} PRIVATE ${SOFTFLOAT_LIB})

    # Ignore rwx warnings from the linker
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-warn-rwx-segments")

    # Generate .bin and disassemble RISC-V
    add_custom_command(
            TARGET ${TARGET_ELF}
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} --output-target=binary ${TARGET_ELF} ${TARGET_BIN}
            COMMAND ${CMAKE_OBJDUMP} -D ${TARGET_ELF} > ${TARGET_ELF}.txt
            COMMENT "Generating binary file ${TARGET_BIN}"
    )
endif()

