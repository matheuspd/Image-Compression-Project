# Compiler
CC = gcc

# Include directories:
# - "include" for decompressor project headers.
# - "../libjpeg/include" for libjpeg headers.
INCDIR = include
LIBJPEG_INCDIR = ../libjpeg/include

# Library directory for libjpeg
LIBJPEG_LIBDIR = ../libjpeg

# Compiler flags:
# -I flags add the include directories.
# -std=c99 enforces the C99 standard.
# -Wall, -Wextra, and -pedantic enable comprehensive warnings.
# -O2 optimizes the code.
CFLAGS = -I$(INCDIR) -I$(LIBJPEG_INCDIR) -std=c99 -Wall -Wextra -pedantic -O2

# Linker flags:
# -L points to the directory of the libjpeg library,
# -ljpeg links with the jpeg library.
LDFLAGS = -L$(LIBJPEG_LIBDIR) -ljpeg -lm

# Source and object directories
SRC_DIR = src
OBJ_DIR = obj

# Source files
SRC = $(wildcard $(SRC_DIR)/*.c)

# Object files in obj/
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Target executable name.
TARGET = decompressor

.PHONY: all clean

# Default target: build the decompressor executable.
all: $(TARGET)

# Link object files and the precompiled libjpeg library to create the final executable.
$(TARGET): $(OBJ) $(LIBJPEG_LIBDIR)/libjpeg.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Ensure obj dir exists and compile .c into .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: remove all object files and the executable.
clean:
	rm -f $(OBJ) $(TARGET)
