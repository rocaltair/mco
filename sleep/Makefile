LIBPATH = ../src/libmco.a
APP = sleep

$(APP) : main.c $(LIBPATH)
	gcc -o $@ -Wall main.c -I ../src -L ../src -lmco

$(LIBPATH) :
	$(MAKE) -C ../src

clean :
	rm -f $(APP)
