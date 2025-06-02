#include "compressor.h"

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
        printf("Usage: %s <input.bmp> <output.bin>\n", argv[0]);
        exit(FAILURE);
    }

    if (compress_jpeg(argv[1], argv[2]) != SUCCESS) {
        printf("Error compressing the BMP file.\n");
        exit(FAILURE);
    }
    
    return SUCCESS;
}
