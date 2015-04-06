all:result

result:mmap.c
	gcc -std=c99 mmap.c -o result

mmap.c:
