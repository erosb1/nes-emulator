all: bin asm

bin: build/nes-emulator
asm: build/nes-emulator.s

build/nes-emulator: build_dir
	gcc main.c -o build/nes-emulator

build/nes-emulator.s: build_dir
	gcc -S main.c -o build/nes-emulator.s

build_dir:
	mkdir -p build

clean:
	rm -rf build/*

.PHONY: bin asm build_dir clean
