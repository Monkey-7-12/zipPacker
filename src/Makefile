
#  Makefile
#
#  This file is part of zipPacker
#
#  Copyright © 1990−2019 by nýx.ch, CC-BY 4.0
#
#  Dieses Werk ist lizenziert unter der CC-BY
#  Creative Commons Namensnennung 4.0 International Lizenz.
#  https://creativecommons.org/licenses/by/4.0/deed.de

CC	  	= cc
CFLAGS  = -x c  -Wall -pthread
LDFLAGS = -lzip -lm -lpthread



OBJ = fbuffer.o dirwalk.o strsize.o main.o
BIN = zipPacker

ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG -ggdb
endif

ifeq ($(shell uname -s),Darwin)
	OBJ += 
endif


%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(BIN): all


all: $(OBJ)
	$(CC) -o zipPacker  $(OBJ) $(LDFLAGS)


clean:
	- rm -rf $(BIN) $(OBJ)
	- rm -rd *~


# vim: ts=4:sw=4:
