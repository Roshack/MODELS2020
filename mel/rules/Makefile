CC = g++
CCFLAGS = -std=c++14 -g -I./src
all: rint

rint: ./obj/rules_classes.o ./obj/rules_bison.o ./obj/rules_flex.o ./obj/rules_main.o
	$(CC) $(CCFLAGS) ./obj/rules_classes.o ./obj/rules_bison.o ./obj/rules_flex.o ./obj/rules_main.o -o rint
	
./obj/rules_main.o: ./src/rules_main.cc ./obj
	$(CC) $(CCFLAGS) -c ./src/rules_main.cc -o ./obj/rules_main.o
	
./obj/rules_classes.o: ./src/rules_classes.h ./src/rules_classes.cc ./obj
	$(CC) $(CCFLAGS) -c ./src/rules_classes.cc -o ./obj/rules_classes.o
	
./obj/rules_bison.o: ./obj/rules_bison.cpp ./obj
	${CC} $(CCFLAGS) -c ./obj/rules_bison.cpp -o ./obj/rules_bison.o
	
./obj/rules_flex.o: ./obj/rules_flex.cpp ./obj/rules_bison.cpp ./obj
	${CC} $(CCFLAGS) -c ./obj/rules_flex.cpp -o ./obj/rules_flex.o

./obj/rules_bison.cpp: ./src/rules_bison.y ./obj/rules_classes.o ./obj
	bison -v -g -d -o ./obj/rules_bison.cpp ./src/rules_bison.y

./obj/rules_flex.cpp: ./src/rules_flex.y ./obj/rules_classes.o ./obj
	lex  -o ./obj/rules_flex.cpp ./src/rules_flex.y

	
clean:
	rm ./obj/* &> /dev/null
