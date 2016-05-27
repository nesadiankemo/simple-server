CC = gcc
INCLUDE_DIR := ./include 
C_SOURCE_FILE :=\
	src/fm_server.c\
	src/wrapper.c\
	src/parse_result.c

BUILD_DIR = ./build
CFLAGS := $(addprefix -I, $(INCLUDE_DIR))
OBJECTS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SOURCE_FILE))
OBJ_DIRS := $(sort $(dir $(OBJECTS)))

.PHONY : clean all 

all : check_dirs serv

serv : $(OBJECTS)
	$(CC) -o serv $(OBJECTS)

$(OBJECTS) : $(BUILD_DIR)/%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

check_dirs: $(OBJ_DIRS)

$(OBJ_DIRS) :
	mkdir -p $@

clean:
	rm $(OBJECTS) serv