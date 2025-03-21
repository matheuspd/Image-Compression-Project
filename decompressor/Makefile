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
LDFLAGS = -L$(LIBJPEG_LIBDIR) -ljpeg

# Source files for the decompressor project.
SRC = src/main.c src/decompressor.c

# Generate object file names from source files.
OBJ = $(SRC:.c=.o)

# Target executable name.
TARGET = decompressor

.PHONY: all clean

# Default target: build the decompressor executable.
all: $(TARGET)

# Link object files and the precompiled libjpeg library to create the final executable.
$(TARGET): $(OBJ) $(LIBJPEG_LIBDIR)/libjpeg.a
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Compile each .c file into an object file.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: remove all object files and the executable.
clean:
	rm -f $(OBJ) $(TARGET)
