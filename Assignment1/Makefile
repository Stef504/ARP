CC = gcc

CFLAGS = -Wall

LIBS = -lncurses 
MATH_ONLY = -lm

all: main process_Drone BlackBoard process_In process_Ob process_Ta

main: main.c
	$(CC) $(CFLAGS) main.c -o main

process_Drone: process_Drone.c 
	$(CC) $(CFLAGS) process_Drone.c -o process_Drone $(MATH_ONLY)

BlackBoard: BlackBoard.c 
	$(CC) $(CFLAGS) BlackBoard.c -o BlackBoard $(LIBS) $(MATH_ONLY)

process_In: process_In.c 
	$(CC) $(CFLAGS) process_In.c -o process_In

process_Ob: process_Ob.c 
	$(CC) $(CFLAGS) process_Ob.c -o process_Ob

process_Ta: process_Ta.c 
	$(CC) $(CFLAGS) process_Ta.c -o process_Ta

clean:
	rm main process_Drone BlackBoard process_In process_Ob process_Ta					
