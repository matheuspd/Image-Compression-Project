# Compiler
CC = gcc

# Include directories:
# - "include" is for the compressor project headers.
# - "../libjpeg/include" is for the libjpeg headers.
INCDIR = include
LIBJPEG_INCDIR = ../libjpeg/include

# Library directory for libjpeg
LIBJPEG_LIBDIR = ../libjpeg

# Compiler flags:
# -I: Add include directories.
# -std=c99: Use C99 standard.
# -Wall, -Wextra, -pedantic: Enable comprehensive warnings.
# -O2: Optimize the code.
CFLAGS = -I$(INCDIR) -I$(LIBJPEG_INCDIR) -std=c99 -Wall -Wextra -pedantic -O2

# Linker flags:
# -L: Library directory for libjpeg.
# -ljpeg: Link with the libjpeg library.
LDFLAGS = -L$(LIBJPEG_LIBDIR) -ljpeg -lm

# Source and object directories
SRC_DIR = src
OBJ_DIR = obj

# Find all .c files in src/
SRC = $(wildcard $(SRC_DIR)/*.c)

# Generate corresponding .o files in obj/
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Final target executable.
TARGET = compressor

.PHONY: all clean

# Default target: build the compressor executable.
all: $(TARGET)

# Link object files and the precompiled libjpeg library to create the final executable.
$(TARGET): $(OBJ) $(LIBJPEG_LIBDIR)/libjpeg.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each .c to .o inside obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: remove all object files and the executable.
clean:
	rm -f $(OBJ) $(TARGET)
