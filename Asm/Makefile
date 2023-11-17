all:
	python3 src/asm.py test/prog1.mas test/prog1.bin
	cp test/prog1.bin ../VM/out/prog1.bin

lol:
	rm test/main
	clang src/main.c -o test/main
	./test/main test/prog1.bin