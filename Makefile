# the compiler
CC = gcc

# compiler flags
# -g     				adds debugging information
#  -Wall 				turns on most compiler warnings
# -Qunused-arguments	ignores unused compilings
CFLAGS = -Qunused-arguments -g -Wall -I/usr/local/opt/ncurses/include -L/usr/local/opt/ncurses/lib

# header dependencies
DEPS = itunes-cli.h

# object executables
OBJ = itunes-cli.o itunes-cli-func.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

itunes-cli: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lmenu -lncurses