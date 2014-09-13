src=./src/

objects=main.o tifaa.o ui.o surveyimginfo.o attaavv.o timing.o

vpath %.h $(src)
vpath %.c $(src)

CC      = gcc
CFLAGS  = -Wall -O3 -W -I$(src)
#CFLAGS  = -g3 -Wall -W -I$(src)  #For debugging and valgrind.
LDLIBS  = -lcfitsio -lwcs -pthread -lm

tifaa: $(objects) 
	$(CC) -o tifaa $(objects) $(LDLIBS) 
	@rm *.o

install:
	cp ./tifaa /usr/local/bin/
