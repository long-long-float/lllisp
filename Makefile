CC = g++
CPPFLAGS = -W -Wall -std=c++11 `llvm-config --cxxflags` -fexceptions
LDLIBS = `llvm-config --ldflags --libs --cxxflags` -ldl -lpthread -ltinfo

all: lisp lib

lisp: lisp.o object.o gc.o token.o environment.o compiler.o
lib: lib.c
	clang -emit-llvm -S -O -o lib.ll lib.c

test:
	./test.sh

clean:
	@rm -f *.o *.ll lisp
