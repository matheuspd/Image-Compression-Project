# Compiler
CC = gcc

# Include directory for libjpeg headers
INCDIR = include

# Compiler flags:
# -I$(INCDIR): add the header include directory
# -std=c99: use the C99 standard
# -Wall, -Wextra, -pedantic: enable comprehensive warnings
# -O2: optimize the code
CFLAGS = -I$(INCDIR) -std=c99 -Wall -Wextra -pedantic -O2

# Source directory and object directory
SRC_DIR = src
OBJ_DIR = obj

# Find all .c files in SRC_DIR
SRC = $(wildcard $(SRC_DIR)/*.c)

# Generate corresponding .o paths in OBJ_DIR
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Target static library
TARGET = libjpeg.a

# Archive tool and flags
AR = ar
ARFLAGS = rcs

# Declare phony targets
.PHONY: all clean

# Default target: build the static library
all: $(TARGET)

# Create the static library from object files
$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

# Ensure obj directory exists and compile .c to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target: remove object files and the static library
clean:
	rm -f $(OBJ) $(TARGET)
