CC=g++
#CFLAGS = -ggdb -Wall
CFLAGS = 

all: clean commander processManager

#put all the file needed, like .h files as well
#note, you need a tab, not spaces.

commander.o: commander.cpp
	$(CC) $(CFLAGS) -c commander.cpp

commander: commander.o
	$(CC) $(CFLAGS) -o commander commander.o 

processManager.o: processManager.cpp
	$(CC) $(CFLAGS) -c processManager.cpp

processManager: processManager.o
	$(CC) $(CFLAGS) -o processManager processManager.o 

clean: 
	rm -f commander.o commander processManager.o processManager