tifaa: tifaa.o attaavv.o tifaa_code.o
	gcc -static -o tifaa tifaa.o attaavv.o tifaa_code.o -lcfitsio -lwcs -lm -pthread

tifaa.o: ./src/tifaa.c ./src/attaavv.h ./src/tifaa_code.h
	gcc -c -Wall -W -ansi -pedantic ./src/tifaa.c

tifaa_code.o: ./src/tifaa_code.c ./src/tifaa_code.h
	gcc -O2 -c -Wall -W -ansi -pedantic ./src/tifaa_code.c

attaavv.o: ./src/attaavv.c ./src/attaavv.h
	gcc -c -Wall -W -ansi -pedantic ./src/attaavv.c
