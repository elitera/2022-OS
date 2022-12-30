# all:
# 	nasm -f elf32 -o my_print.o my_print.asm
# 	g++ -g -m32 -std=c++11 main.cpp my_print.o -o res
# clean:
# 	rm my_print.o
# 	rm res
main:
	rm -f main
	nasm -f elf -o my_print.o my_print.asm
	g++ -m32 -o main main.cpp my_print.o
	rm -f *.o
	./main
.PHONY:clean
clean:
	-rm -f *.o
	-rm -f main 