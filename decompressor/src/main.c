#include "decompressor.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <entrada.jpg> <saida.bmp>\n", argv[0]);
        return 1;
    }

    descomprimir_jpeg(argv[1], argv[2]);
    return 0;
}
