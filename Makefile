CC = g++
CPPFLAGS = -W -Wall -std=c++11 `llvm-config --cxxflags` -fexceptions
LDLIBS = `llvm-config --ldflags --libs --cxxflags` -ldl -lpthread -ltinfo

lisp: lisp.o object.o gc.o token.o environment.o compiler.o

clean:
	@rm -f *.o lisp
