#Makefile
# Created on: Feb 20, 2015
#     Author: yulongb

CC = clang
CFLAGS= -Wall -Wextra -g  -I . ../
LD = -lpthread

# list of sources
COMBSRC = server_client_pairs.c client_thread.c server_thread.c config_def.c
# top-level rule, to compile everything.
all : server client combine
# program executable file name.
combine : combine.out

# Define dependencies
DEPS = $(shell find -name "*.d")

# Define all object files.
COMBOBJ = $(COMBSRC:.c=.o)

# rule to link the program
combine.out : $(COMBOBJ)
	$(CC)  $^ -o $@ $(LD)

server: server.c
	$(CC) $(CFLAGS) -o $(@).out $< $(LD)

client: client.c
	$(CC) $(CFLAGS) -o $(@).out $< $(LD)
	
# Compile: create object files from C source files.
%.o : %.c %.h
	$(CC) $(CFLAGS) -c  $< -o $@
	
clean:
	rm -rf *.hex *.bin *.txt *.o *.out