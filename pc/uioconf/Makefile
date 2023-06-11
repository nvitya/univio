# TARGET, SOURCE DIRECTORIES, INCLUDES

PROG_NAME   = uioconf

BUILD_DIR   = ./build

SRC_MAIN      = $(wildcard src/*.cpp)
SRC_UTILS_OS  = $(wildcard ../utils_os/*.cpp)
SRC_UDO       = $(wildcard ../../udo/cpp/udo/*.cpp)
SRC_UDOMASTER = $(wildcard ../../udo/cpp/udomaster/*.cpp)

OBJ_MAIN      = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC_MAIN:.cpp=.o)))
OBJ_UTILS_OS  = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC_UTILS_OS:.cpp=.o)))
OBJ_UDO       = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC_UDO:.cpp=.o)))
OBJ_UDOMASTER = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC_UDOMASTER:.cpp=.o)))

All_DEPS = $(OBJ_MAIN:.o=.d) $(OBJ_UTILS_OS:.o=.d) $(OBJ_UDO:.o=.d) $(OBJ_UDOMASTER:.o=.d)

INCLUDES = -Isrc -I../utils_os -I../../udo/cpp/udo -I../../udo/cpp/udomaster

# COMPILE PARAMETERS

CC          = g++
LD          = g++
CFLAGS      = $(INCLUDES) -g3
LDFLAGS = --static

CFLAGS += -std=gnu++11

# option to generate a .d (.h dependency list) files during compilation (they go into build dir too)
CFLAGS     += -MMD

# LINKING

$(PROG_NAME): $(BUILD_DIR) $(OBJ_MAIN) $(OBJ_UTILS_OS) $(OBJ_UDO) $(OBJ_UDOMASTER)
	$(LD) $(LDFLAGS) -o $(PROG_NAME) $(OBJ_MAIN) $(OBJ_UTILS_OS) $(OBJ_UDO) $(OBJ_UDOMASTER)

# COMPILE

# include all generated .d files in the makefile
-include $(All_DEPS)

$(BUILD_DIR)/%.o : ../../udo/cpp/udo/%.cpp
	$(CC) -c $< $(CFLAGS) -o $(BUILD_DIR)/$(notdir $@)

$(BUILD_DIR)/%.o : ../../udo/cpp/udomaster/%.cpp
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
