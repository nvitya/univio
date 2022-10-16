# TARGET, SOURCE DIRECTORIES, INCLUDES

PROG_NAME   = uioconf

BUILD_DIR   = ./build

SRC_MAIN     = $(wildcard src/*.cpp)
SRC_UTILS_OS = $(wildcard ../utils_os/*.cpp)
SRC_UNIVIO   = $(wildcard ../univio/*.cpp)

OBJ_MAIN     = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC_MAIN:.cpp=.o)))
OBJ_UTILS_OS = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC_UTILS_OS:.cpp=.o)))
OBJ_UNIVIO   = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC_UNIVIO:.cpp=.o)))

All_DEPS = $(OBJ_MAIN:.o=.d) $(OBJ_UTILS_OS:.o=.d) $(OBJ_UNIVIO:.o=.d)

INCLUDES = -Isrc -I../utils_os -I../univio

# COMPILE PARAMETERS

CC          = g++
LD          = g++
CFLAGS      = $(INCLUDES) -g3
LDFLAGS = --static

CFLAGS += -std=gnu++11

# option to generate a .d (.h dependency list) files during compilation (they go into build dir too)
CFLAGS     += -MMD

# LINKING

$(PROG_NAME): $(BUILD_DIR) $(OBJ_MAIN) $(OBJ_UTILS_OS) $(OBJ_UNIVIO)
	$(LD) $(LDFLAGS) -o $(PROG_NAME) $(OBJ_MAIN) $(OBJ_UTILS_OS) $(OBJ_UNIVIO)

# COMPILE

# include all generated .d files in the makefile
-include $(All_DEPS)

$(BUILD_DIR)/%.o : ../univio/%.cpp
	$(CC) -c $< $(CFLAGS) -o $(BUILD_DIR)/$(notdir $@)

$(BUILD_DIR)/%.o : ../utils_os/%.cpp
	$(CC) -c $< $(CFLAGS) -o $(BUILD_DIR)/$(notdir $@)

$(BUILD_DIR)/%.o : src/%.cpp
	$(CC) -c $< $(CFLAGS) -o $(BUILD_DIR)/$(notdir $@)

# UTILITY

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

.PHONY: clean

clean:
	rm -f $(PROG_NAME) $(BUILD_DIR)/*
	rmdir $(BUILD_DIR)
