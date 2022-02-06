LDLIBS=`llvm-config --libs core --system-libs`
LDFLAGS=-L/usr/local/opt/llvm/lib
CPPFLAGS=-I/usr/local/opt/llvm/include
#CXXFLAG=-std=c++11
CXXFLAGS=`llvm-config --cxxflags`

fplus: gen/Absyn.o gen/Buffer.o gen/Lexer.o gen/Parser.o gen/Printer.o

gen/Absyn.o: gen

gen/Lexer.o: gen

gen/Parser.o: gen

gen/Printer.o: gen

gen: fplus.cf
	bnfc --cpp -l -m -o $@ $<
	cd $@ && make

clean:
	rm -rf fplus gen
