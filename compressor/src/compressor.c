#include "compressor.h"
#include "bmp.h"
#include "jpeg.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Compresses a BMP file into a JPEG file.
 *
 * This function opens the BMP file, reads its header and pixel data,
 * and then compresses the data into JPEG format.
 * 
 * @param input_bmp Path to the input BMP file.
 * @param output_jpeg Path to the output JPEG file.
 * @return SUCCESS if the compression is successful, otherwise FAILURE.
 */
int compress_jpeg(const char *input_bmp, const char *output_jpeg) {
    FILE *file = fopen(input_bmp, "rb");
    if (!file) {
        printf("Error opening BMP file.\n");
        return FAILURE;
    }

    //if (output_jpeg == NULL) return FAILURE;

    BMPFILEHEADER fileHeader;
    BMPINFOHEADER infoHeader;
    
    if (readHeader(file, &fileHeader) != SUCCESS) {
        printf("Error reading the BMP header.\n");
        fclose(file);
        return FAILURE;
    }

    if (readInfoHeader(file, &infoHeader) != SUCCESS) {
        printf("Error reading the BMP info header.\n");
        fclose(file);
        return FAILURE;
    }
    
    RGBPixel *pixels = read_pixels(file, &fileHeader, infoHeader.biWidth, infoHeader.biHeight);
    if (!pixels) {
        printf("Error allocating memory for pixels.\n");
        fclose(file);
        return FAILURE;
    }
    
    fclose(file); 

    free(pixels);

    return SUCCESS;
}
