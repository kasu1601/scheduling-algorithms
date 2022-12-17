CC = gcc
CFLAGS = -Wall -ansi -pedantic
OBJ = simulator.o
EXEC = simulator

$(EXEC) : $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) -lpthread

simulator.o : simulator.c 
	$(CC) $(CFLAGS) simulator.c -c 

clean :
	rm -f $(EXEC) $(OBJ) 
