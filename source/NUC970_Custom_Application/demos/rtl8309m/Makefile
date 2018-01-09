.SUFFIXES : .x .o .c .s

CC := arm-linux-gcc

TOP_DIR = $(shell pwd)
LIB_DIR = $(TOP_DIR)/lib
BIN_DIR = $(TOP_DIR)/bin
OBJECT_DIR = $(TOP_DIR)/obj
SRC_DIR = $(TOP_DIR)/src
INC_DIR = $(TOP_DIR)/include

$(shell mkdir -p ${LIB_DIR})
$(shell mkdir -p ${BIN_DIR})
$(shell mkdir -p ${OBJECT_DIR})

SRCS := $(wildcard ${SRC_DIR}/*.c)
OBJS = $(patsubst %.c,%.o,$(notdir ${SRCS}))

INCLUDE_PATH = -I$(INC_DIR)
DEP_LIB := -lpthread

RTL_SRC_OBJECT = $(TOP_DIR)/test.c
RTL_OBJECT = test.o

LIB_OBJECT = librtl8309m.so
BIN_OBJECT = test

.PHONY:all
all: $(LIB_OBJECT) $(BIN_OBJECT)
	
$(LIB_OBJECT): $(OBJS)
	$(CC) $(OBJS) -shared -o $@
	mv $(LIB_OBJECT) $(LIB_DIR)

$(OBJS): $(SRCS)
	$(CC) -fpic -c $(SRCS) $(INCLUDE_PATH)

$(BIN_OBJECT):$(RTL_OBJECT)
	$(CC) $(OBJECT_DIR)/$(RTL_OBJECT) -L$(LIB_DIR) -lrtl8309m -o $(BIN_OBJECT) 
	mv $(BIN_OBJECT) $(BIN_DIR)

 
$(RTL_OBJECT):$(RTL_SRC_OBJECT)
	$(CC) -c $(RTL_SRC_OBJECT) $(INCLUDE_PATH)
	mv $(OBJS) $(RTL_OBJECT) $(OBJECT_DIR)
	
clean:
	rm -rf $(LIB_DIR) $(BIN_DIR) $(OBJECT_DIR) 
