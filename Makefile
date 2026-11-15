clean: a.out
	rm a.out
	rm -fr a.out.dSYM
	rm *.pch
	rm *.gch



debug_mem: arena.c arena.h
	clang -g arena.c arena.h


test: main.c base_test.h
	clang -g -Wall main.c
