# Compiler flags go here.
CFLAGS = -g -Wall -O2 -I . -MMD -std=c99

# Linker flags go here.
LDFLAGS = -lpthread -lcomedi -g -lm

# list of sources
ELEVSRC = elev.c io.c  button_sensor_update_thread.c

# program executable file name.
TARGET = heis

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


#Martin Korsgaard, 2006
