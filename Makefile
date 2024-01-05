vpath %.h include
vpath %.h src/screens
vpath %.h src/data_structures
vpath %.c src/screens
vpath %.c src/data_structures
vpath %.c src

OS := $(shell uname -s)

ifeq ($(OS), Linux)
	MKDIR = mkdir -p
	SED = sed
	CP = cp
	CPR = cp -r
	RM = rm -r
	FixPath = $1
	EXE_NAME = snake
	EXTERNAL_LIB := -lncurses
	includes :=	-Iinclude -Isrc/screens
else ifeq ($(findstring MSYS_NT,$(OS)), MSYS_NT)
	MKDIR = mkdir -p
	SED = sed
	CP = cp
	CPR = copy -r
	RM = rm -r
	FixPath = $(subst /,\,$1)
	EXE_NAME = snake.exe
	EXTERNAL_LIB := -Lexternal/pdcurses/lib -lpdcurses
	includes :=	-Iinclude -Isrc/screens -Iexternal/pdcurses/include
endif

CC = gcc
CFLAGS := -ggdb -Wall -std=c99 -Wextra -Wswitch-enum
BUILD_PATH := build/debug

#build folders
BIN_PATH := $(BUILD_PATH)/bin
TEMP_PATH := $(BUILD_PATH)/temp
#assets
ASSETS_SRC :=  $(wildcard src/assets/*.txt)
ASSETS_DEST :=  $(ASSETS_SRC:src/assets/%=$(BIN_PATH)/assets/%)
#exe
SRC := $(wildcard src/*.c)
SRC_SCREENS := $(wildcard src/screens/*.c)
SRC_DATA_STRUCTURES := $(wildcard src/data_structures/*.c)
OBJ := $(SRC:src/%.c=$(TEMP_PATH)/%.o) \
	   $(SRC_SCREENS:src/screens/%.c=$(TEMP_PATH)/%.o) \
	   $(SRC_DATA_STRUCTURES:src/data_structures/%.c=$(TEMP_PATH)/%.o)
DEP := $(OBJ:.o=.d)
EXE := $(BIN_PATH)/$(EXE_NAME)


.PHONY: all dir assets clean build run

all: dir assets build

dir: | $(BUILD_PATH)

assets: $(ASSETS_DEST)

build: $(EXE)

run: $(EXE)
	$(EXE)

clean:
	$(RM) $(call FixPath,$(BUILD_PATH))
#@echo $(SRC)


$(BIN_PATH)/assets/%: src/assets/%
	$(CP) $< $@	

$(EXE): $(OBJ)
	$(CC) $^ -o $@ $(EXTERNAL_LIB)

$(BUILD_PATH):
	$(MKDIR) $(call FixPath,$(BIN_PATH))    
	$(MKDIR) $(call FixPath,$(BIN_PATH)/assets)
	$(MKDIR) $(call FixPath,$(TEMP_PATH))

# dependencies
df = $(TEMP_PATH)/$(*F)

$(TEMP_PATH)/%.o: %.c
	$(CC) -MM -MP -MT $(df).o -MT $(df).d $(CFLAGS) $(includes) $< > $(df).d
	$(CC) -c $< $(CFLAGS) $(includes) -o $(df).o
