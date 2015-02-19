# Compiler flags go here.
CFLAGS =  -Wall -I . -g

# Linker flags go here.
LDFLAGS = -lpthread -g

# list of sources
ELEVSRC = main.c

# program executable file name.
TARGET = main

# top-level rule, to compile everything.
all: $(TARGET)

# Define dependencies
DEPS = $(shell find -name "*.d")

# Define all object files.
ELEVOBJ = $(ELEVSRC:.c=.o)

# rule to link the program
$(TARGET): $(ELEVOBJ)
	clang  $^ -o $@ $(LDFLAGS)

# Compile: create object files from C source files.
%.o : %.c
	clang $(CFLAGS) -c  $< -o $@ 

# Include dependencies, if any.
-include $(DEPS) foo

# rule for cleaning re-compilable files.
clean:
	rm -f $(TARGET) $(ELEVOBJ) $(DEPS)

rebuild:	clean all

.PHONY: all rebuild clean
