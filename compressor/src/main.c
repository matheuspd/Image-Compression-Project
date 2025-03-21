#include "compressor.h"
#include <stdio.h>

/**
 * @brief Main entry point for the BMP-to-JPEG compressor.
 *
 * This function validates command-line arguments and initiates the compression process.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @return SUCCESS if the compression is successful, otherwise FAILURE.
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input.bmp> <output.jpg>\n", argv[0]);
        return FAILURE;
    }

    if (compress_jpeg(argv[1], argv[2]) != SUCCESS) {
        printf("Error compressing the BMP file.\n");
        return FAILURE;
    }
    
    return SUCCESS;
}
