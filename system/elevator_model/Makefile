# Compiler flags go here.
CC = clang
CFLAGS = -c -g -Wall -O2 -I./ -I../ -MMD -std=gnu99

# Linker flags go here.
LDFLAGS = -lpthread -lcomedi -g -lm

# list of sources
ELEVSRC = ../drivers/elev.c ../drivers/io.c  elevator_controller_layer.c elevator_model_data_structure.c \
			../task_pool/taskpool_policies_wrapper.c ../task_pool/lift_task_queue.c\
			./server/network_module_tcp.c ./server/network_module_udp.c ./server/new.c

# list of sources
COMBSRC = ../process_pairs/server_client_pairs.c ../process_pairs/client_thread.c ../process_pairs/server_thread.c ../process_pairs/config_def.c
# program executable file name.
TARGET = combine.out
# top-level rule, to compile everything.
all: $(TARGET)

# Define dependencies
DEPS = $(shell find -name "*.d")

# Define all object files.
ELEVOBJ = $(ELEVSRC:.c=.o) $(COMBSRC:.c=.o)

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
