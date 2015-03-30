# Compiler flags go here.
CFLAGS = -g -Wall -O0 -I . -MMD -std=std99 -g
CC = clang
# Linker flags go here.
LDFLAGS = -lpthread -lm

# list of sources
ELEVSRC = ./lift_task_queue.c ./test_main.c ./taskpool_policies_wrapper.c

# program executable file name.
TARGET = a.out

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
	rm -f $(TARGET) $(ELEVOBJ) $(DEPS)

rebuild:	clean all

.PHONY: all rebuild clean


#Martin Korsgaard, 2006
