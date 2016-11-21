LIBPATH = ../src/libmco.a


test : server.c $(LIBPATH)
	gcc -o $@ -Wall server.c -I ../src -L ../src -lmco

$(LIBPATH) :
	$(MAKE) -C ../src

clean :
	rm -f test
