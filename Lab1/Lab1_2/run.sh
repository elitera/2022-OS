#!/bin/bash
nasm -f elf32 main.asm 
ld -m elf_i386 main.o -o main
./main


nasm -f elf32 main20.10.asm 
ld -m elf_i386 main20.10.o -o main20.10
./main20.10

nasm -f elf32 main21.3.asm 
ld -m elf_i386 main21.3.o -o main21.3
./main21.3