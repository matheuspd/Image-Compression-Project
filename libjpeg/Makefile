# Compiler
CC = gcc

# Include directory for libjpeg headers
INCDIR = include

# Compiler flags:
# -I$(INCDIR): add the header include directory.
# -std=c99: use the C99 standard.
# -Wall, -Wextra, -pedantic: enable comprehensive warnings.
# -O2: optimize the code.
CFLAGS = -I$(INCDIR) -std=c99 -Wall -Wextra -pedantic -O2

# Source files in the libjpeg project.
SRC = src/jpeg.c src/bmp.c src/types.c

# Generate object files from source files.
OBJ = $(SRC:.c=.o)

# Target static library.
TARGET = libjpeg.a

# Archive tool and flags.
AR = ar
ARFLAGS = rcs

# Declare phony targets.
.PHONY: all clean

# Default target: build the static library.
all: $(TARGET)

# Create the static library from object files.
$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)

# Compile each .c file into a .o file.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: remove object files and the static library.
clean:
	rm -f $(OBJ) $(TARGET)
