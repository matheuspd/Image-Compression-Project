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

# Source files for the compressor project.
SRC = src/main.c src/compressor.c

# Generate object file names from source files.
OBJ = $(SRC:.c=.o)

# Final target executable.
TARGET = compressor

.PHONY: all clean

# Default target: build the compressor executable.
all: $(TARGET)

# Link object files and the precompiled libjpeg library to create the final executable.
$(TARGET): $(OBJ) $(LIBJPEG_LIBDIR)/libjpeg.a
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Compile each .c file to a .o file.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: remove all object files and the executable.
clean:
	rm -f $(OBJ) $(TARGET)
