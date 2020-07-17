CC = g++
CCFLAGS = -std=c++17 -g -I./src
all: mint

mint: ./obj/mel_classes.o ./obj/mel_bison.o ./obj/mel_flex.o ./obj/comp_test.o
	$(CC) $(CCFLAGS) ./obj/mel_classes.o ./obj/mel_bison.o ./obj/mel_flex.o ./obj/comp_test.o -o mint
	
./obj/comp_test.o: ./src/test_comp.cc ./obj
	$(CC) $(CCFLAGS) -c ./src/test_comp.cc -o ./obj/comp_test.o
	
./obj/mel_classes.o: ./src/mel_classes.h ./src/mel_classes.cc ./obj
	$(CC) $(CCFLAGS) -c ./src/mel_classes.cc -o ./obj/mel_classes.o
	
./obj/mel_bison.o: ./obj/mel_bison.cpp ./obj
	${CC} $(CCFLAGS) -c ./obj/mel_bison.cpp -o ./obj/mel_bison.o
	
./obj/mel_flex.o: ./obj/mel_flex.cpp ./obj/mel_bison.cpp ./obj
	${CC} $(CCFLAGS) -c ./obj/mel_flex.cpp -o ./obj/mel_flex.o

./obj/mel_bison.cpp: ./src/mel_bison.y ./obj/mel_classes.o ./obj
	bison -v -g -d -o ./obj/mel_bison.cpp ./src/mel_bison.y

./obj/mel_flex.cpp: ./src/mel_flex.l ./obj/mel_classes.o ./obj
	lex ${FLEXFLAG} -o ./obj/mel_flex.cpp ./src/mel_flex.l

debug: FLEXFLAG = -d
debug: all

clean:
	rm ./obj/* &> /dev/null
