#include "decompressor.h"

/**
 * @brief Main entry point for the BMP decompressor.
 *
 * This function validates command-line arguments and initiates the decompression process.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return SUCCESS if the compression is successful, otherwise FAILURE.
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <input.bin> <output.bmp>\n", argv[0]);
        exit(FAILURE);
    }

    if (decompress_bin(argv[1], argv[2]) != SUCCESS) {
        printf("Error decompressing the BIN file.\n");
        exit(FAILURE);
    }

    return SUCCESS;
}
