LIBPATH = ../src/libmco.a


test : test.c $(LIBPATH)
	gcc -o test -Wall test.c -I ../src -L ../src -lmco

$(LIBPATH) :
	$(MAKE) -C ../src

clean :
	rm -f test
