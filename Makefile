src=./src/

objects=main.o ui.o attaavv.o

vpath %.h $(src)
vpath %.c $(src)

CC      = gcc
CFLAGS  = -Wall -O3 -W -I$(src)
#CFLAGS  = -g -Wall -O0 -W -I$(src)  #For debugging and valgrind.
LDLIBS  = -lcfitsio -lwcs -pthread -lm

tifaa: $(objects) 
	$(CC) -o tifaa $(objects) $(LDLIBS) 
	@rm *.o
	./tifaa -g -ccat.txt -i1 -r2 -d3 -a0.03 -p7 -s/mnt/research/Images/SurveyImages/COSMOS/ACSF814W/ -b*sci.fits -oPS -f_s.fits 
