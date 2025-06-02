#include "decompressor.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <entrada.bin> <saida.bmp>\n", argv[0]);
        exit(FAILURE);
    }

    if (decompress_bin(argv[1], argv[2]) != SUCCESS) {
        printf("Error decompressing the BIN file.\n");
        exit(FAILURE);
    }

    return SUCCESS;
}
