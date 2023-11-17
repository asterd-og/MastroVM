# -fsanitize=address -fsanitize=undefined

all:
	clang src/main.c src/cpu.c src/tvo.c -w -lpthread -lSDL2 -I/usr/include/SDL2/ -o out/vm
	./out/vm out/prog1.bin
