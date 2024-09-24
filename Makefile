all: bin asm test

bin: build/nes-emulator
asm: build/nes-emulator.s
test: bin
	./build/nes-emulator tests/nestest.nes

build/nes-emulator: build_dir
	gcc main.c -o build/nes-emulator

build/nes-emulator.s: build_dir
	gcc -S main.c -o build/nes-emulator.s

build_dir:
	mkdir -p build

clean:
	rm -rf build/*

.PHONY: bin asm test build_dir clean
