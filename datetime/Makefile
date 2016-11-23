LIBPATH = ../src/libmco.a

APP = datetime

$(APP) : main.c
	$(MAKE) -C ../src
	gcc -o $@ -Wall -O2 main.c -I ../src -L ../src -lmco

clean :
	rm -f $(APP)
