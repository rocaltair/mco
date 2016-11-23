LIBPATH = ../src/libmco.a
APP = forward

$(APP) : main.c
	$(MAKE) -C ../src
	gcc -o $@ -Wall -O3 main.c -I ../src -L ../src -lmco

clean :
	rm -f $(APP)
