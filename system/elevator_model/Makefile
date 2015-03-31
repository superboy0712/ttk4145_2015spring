# Compiler flags go here.
CC = clang
CFLAGS = -g -Wall -O2 -I./ -I../ -MMD -std=gnu99

# Linker flags go here.
LDFLAGS = -lpthread -lcomedi -g -lm

# list of sources
ELEVSRC = ../drivers/elev.c ../drivers/io.c  button_sensor_update_thread.c elevator_model_data_structure.c

# program executable file name.
TARGET = elevator_model

# top-level rule, to compile everything.
all: $(TARGET)

# Define dependencies
DEPS = $(shell find -name "*.d")

# Define all object files.
ELEVOBJ = $(ELEVSRC:.c=.o)

# rule to link the program
$(TARGET): $(ELEVOBJ)
	$(CC)  $^ -o $@ $(LDFLAGS)

# Compile: create object files from C source files.
%.o : %.c
	$(CC) $(CFLAGS) -c  $< -o $@ 

# Include dependencies, if any.
-include $(DEPS) foo

# rule for cleaning re-compilable files.
clean:
	rm -f $(TARGET) $(ELEVOBJ) $(DEPS) *~

rebuild:	clean all

.PHONY: all rebuild clean


#Martin Korsgaard, 2006
