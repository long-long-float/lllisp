CC = g++
CPPFLAGS = -W -Wall -std=c++11
LDLIBS = -ldl

lisp: lisp.o object.o gc.o token.o

clean:
	@rm -f *.o lisp
