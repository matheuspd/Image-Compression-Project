#include "bmp.h"
#include <stdlib.h>

/**
 * @brief Reads the BMP file header.
 *
 * This function reads the BMP file header from the provided file.
 * It verifies that the file is indeed a BMP by checking the magic number.
 *
 * @param F Pointer to the BMP file opened in binary mode.
 * @param H Pointer to a BMPFILEHEADER structure where the header data will be stored.
 * @return SUCCESS if the header is read successfully, otherwise FAILURE.
 */
int readHeader(FILE *F, BMPFILEHEADER *H) {
    fread(&H->bfType,sizeof (unsigned short int),1,F);

    /**
     * Magic number check: 0x4d42 corresponds to "BM" in little-endian format
     */
    if(H->bfType != 0x4d42) {
        printf("The file is not a BMP file.\n");
        return FAILURE;
    }
    
    fread(&H->bfSize,sizeof (unsigned int),1,F);
    fread(&H->bfReserved1,sizeof (unsigned short int),1,F);
    fread(&H->bfReserved2,sizeof (unsigned short int),1,F);
    fread(&H->bfOffBits,sizeof (unsigned int),1,F);

    /**
     * Debug output
     */
    printf("\nBMP File Header:\n");
    printf("Type: 0x%x\n", H->bfType);
    printf("Size: %u\n", H->bfSize);
    printf("Reserved1: %hu\n", H->bfReserved1);
    printf("Reserved1: %hu\n", H->bfReserved2);
    printf("OffBits: %u\n", H->bfOffBits);
    
    return SUCCESS;
}

/**
 * @brief Reads the BMP file information header.
 *
 * This function reads the BMP info header from the file and performs validations:
 * - Checks that the width and height are multiples of 8.
 * - Ensures the dimensions are within the allowed range (8x8 to 1280x800).
 * - Confirms that the image has 24 bits per pixel and uses no compression.
 *
 * @param F Pointer to the BMP file opened in binary mode.
 * @param H Pointer to a BMPINFOHEADER structure where the header info will be stored.
 * @return SUCCESS if the info header is read and valid, otherwise FAILURE.
 */
int readInfoHeader(FILE *F, BMPINFOHEADER *H) {
    fread(&H->biSize,sizeof (unsigned int),1,F);
    fread(&H->biWidth,sizeof (int),1,F);
    fread(&H->biHeight,sizeof (int),1,F);

    /**
     * Validate that width and height are multiples of 8
     */
    if((H->biWidth % 8 != 0) || (H->biHeight % 8 != 0)) {
        printf("Image width or height is not a multiple of 8.\n");
        return FAILURE;
    }

    /**
     * Validate allowed dimensions: from 8x8 to 1280x800
     */
    if (H->biWidth < 8 || H->biHeight < 8 ||
        H->biWidth > 1280 || H->biHeight > 800) {
        printf("Dimensions are outside the allowed range (8x8 to 1280x800).\n");
        return FAILURE;
    }

    fread(&H->biPlanes,sizeof (unsigned short int),1,F);
    fread(&H->biBitCount,sizeof (unsigned short int),1,F);

    if(H->biBitCount != 24) {
        printf("The BMP file does not have 24 bits per pixel.\n");
        return FAILURE;
    }

    fread(&H->biCompression,sizeof (unsigned int),1,F);

    if(H->biCompression != 0) {
        printf("The BMP file uses compression, which is not allowed.\n");
        return FAILURE;
    }

    fread(&H->biSizeImage,sizeof (unsigned int),1,F);
    fread(&H->biXPelsPerMeter,sizeof (int),1,F);
    fread(&H->biYPelsPerMeter,sizeof (int),1,F);
    fread(&H->biClrUsed,sizeof (unsigned int),1,F);
    fread(&H->biClrImportant,sizeof (unsigned int),1,F);

    /**
     * Debug output
     */
    printf("\nBMP File Header Info:\n");
    printf("Size: %u\n", H->biSize);
    printf("Width: %d\n", H->biWidth);
    printf("Height: %d\n", H->biHeight);
    printf("Planes: %hu\n", H->biPlanes);
    printf("BitCount: %hu\n", H->biBitCount);
    printf("Compression: %u\n", H->biCompression);
    printf("SizeImage: %u\n", H->biSizeImage);
    printf("XPelsPerMeter: %d\n", H->biXPelsPerMeter);
    printf("YPelsPerMeter: %d\n", H->biYPelsPerMeter);
    printf("ClrUsed: %u\n", H->biClrUsed);
    printf("ClrImportant: %u\n", H->biClrImportant);
    
    return SUCCESS;
}

/**
 * @brief Reads pixel data from a BMP file.
 *
 * This function calculates the row padding (each row must have a size that is a multiple of 4 bytes)
 * and reads the pixel data from the BMP file in bottom-to-top order, which is how BMP stores its data.
 *
 * @param file Pointer to the BMP file opened in binary mode.
 * @param H Pointer to the BMP file header containing the offset to the pixel data.
 * @param width Image width.
 * @param height Image height.
 * @return Pointer to an array of RGBPixel structures containing the pixel data, or NULL on failure.
 */
RGBPixel *read_pixels(FILE *file, BMPFILEHEADER *H, int width, int height) {
    /**
     * Calculate padding: each row must be a multiple of 4 bytes 
     */
    int padding = (4 - (width * 3) % 4) % 4;

    RGBPixel *pixels = malloc(width * height * sizeof(RGBPixel));

    if (!pixels) return NULL;

    /**
     * Move to the beginning of the image data
     */
    fseek(file, H->bfOffBits, SEEK_SET);

    /**
     * BMP stores pixel rows from bottom to top
     */
    for(int y = height - 1; y >= 0; y--) {
        fread(&pixels[y * width], sizeof(RGBPixel), width, file);
        
        /**
         * Skip the padding bytes at the end of each row
         */
        fseek(file, padding, SEEK_CUR);
    }
    return pixels;
}

/**
 * @brief Frees memory allocated for pixel data.
 *
 * @param pixels Pointer to the pixel array to be freed.
 */
void free_pixels(RGBPixel *pixels) {
    free(pixels);
}
