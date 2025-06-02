#include "bmp.h"
#include <stdlib.h>

int readHeader(FILE *F, BMPFILEHEADER *H) {
    fread(&H->bfType,sizeof (unsigned short int),1,F);


    // Magic number check: 0x4d42 corresponds to "BM" in little-endian format
    if(H->bfType != 0x4d42) {
        printf("The file is not a BMP file.\n");
        return FAILURE;
    }
    
    fread(&H->bfSize,sizeof (unsigned int),1,F);
    fread(&H->bfReserved1,sizeof (unsigned short int),1,F);
    fread(&H->bfReserved2,sizeof (unsigned short int),1,F);
    fread(&H->bfOffBits,sizeof (unsigned int),1,F);

    
    // Debug output
    
    // printf("\nBMP File Header:\n");
    // printf("Type: 0x%x\n", H->bfType);
    // printf("Size: %u\n", H->bfSize);
    // printf("Reserved1: %hu\n", H->bfReserved1);
    // printf("Reserved1: %hu\n", H->bfReserved2);
    // printf("OffBits: %u\n", H->bfOffBits);
    
    return SUCCESS;
}

int readInfoHeader(FILE *F, BMPINFOHEADER *H) {
    fread(&H->biSize,sizeof (unsigned int),1,F);
    fread(&H->biWidth,sizeof (int),1,F);
    fread(&H->biHeight,sizeof (int),1,F);

    // Validate that width and height are multiples of 8
    if((H->biWidth % 8 != 0) || (H->biHeight % 8 != 0)) {
        printf("Image width or height is not a multiple of 8.\n");
        return FAILURE;
    }

    
    // Validate allowed dimensions: from 8x8 to 1280x800
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

    // Debug output

    // printf("\nBMP File Header Info:\n");
    // printf("Size: %u\n", H->biSize);
    // printf("Width: %d\n", H->biWidth);
    // printf("Height: %d\n", H->biHeight);
    // printf("Planes: %hu\n", H->biPlanes);
    // printf("BitCount: %hu\n", H->biBitCount);
    // printf("Compression: %u\n", H->biCompression);
    // printf("SizeImage: %u\n", H->biSizeImage);
    // printf("XPelsPerMeter: %d\n", H->biXPelsPerMeter);
    // printf("YPelsPerMeter: %d\n", H->biYPelsPerMeter);
    // printf("ClrUsed: %u\n", H->biClrUsed);
    // printf("ClrImportant: %u\n", H->biClrImportant);
    
    return SUCCESS;
}

RGB_Pixel *read_pixels(FILE *file, BMPFILEHEADER *H, int width, int height) {
    
    // Calculate padding: each row must be a multiple of 4 bytes
    int padding = (4 - (width * 3) % 4) % 4;

    RGB_Pixel *pixels = malloc(width * height * sizeof(RGB_Pixel));

    if (!pixels) return NULL;


    // Move to the beginning of the image data
    fseek(file, H->bfOffBits, SEEK_SET);


    //BMP stores pixel rows from bottom to top
    for(int y = height - 1; y >= 0; y--) {
        for(int x = 0; x < width; x++) {
            fread(&pixels[y * width + x].b, sizeof(uint8_t), 1, file);
            fread(&pixels[y * width + x].g, sizeof(uint8_t), 1, file);
            fread(&pixels[y * width + x].r, sizeof(uint8_t), 1, file);
        }     

        // Skip the padding bytes at the end of each row
        fseek(file, padding, SEEK_CUR);
    }
    return pixels;
}

void free_pixels(RGB_Pixel *pixels) {
    free(pixels);
}

int write_bmp(FILE *dst, BMPFILEHEADER *fileHeader, BMPINFOHEADER *infoHeader, RGB_Pixel *pixels) {

    // Header BMP
    if (fwrite(&fileHeader->bfType, sizeof(unsigned short), 1, dst) != 1) return FAILURE;
    if (fwrite(&fileHeader->bfSize, sizeof(unsigned int), 1, dst) != 1) return FAILURE;
    if (fwrite(&fileHeader->bfReserved1, sizeof(unsigned short), 1, dst) != 1) return FAILURE;
    if (fwrite(&fileHeader->bfReserved2, sizeof(unsigned short), 1, dst) != 1) return FAILURE;
    if (fwrite(&fileHeader->bfOffBits, sizeof(unsigned int), 1, dst) != 1) return FAILURE;
    
    // Info Header BMP
    if (fwrite(&infoHeader->biSize, sizeof(unsigned int), 1, dst) != 1) return FAILURE;
    if (fwrite(&infoHeader->biWidth, sizeof(int), 1, dst) != 1) return FAILURE;
    if (fwrite(&infoHeader->biHeight, sizeof(int), 1, dst) != 1) return FAILURE;
    if (fwrite(&infoHeader->biPlanes, sizeof(unsigned short), 1, dst) != 1) return FAILURE;
    if (fwrite(&infoHeader->biBitCount, sizeof(unsigned short), 1, dst) != 1) return FAILURE;
    if (fwrite(&infoHeader->biCompression, sizeof(unsigned int), 1, dst) != 1) return FAILURE;
    if (fwrite(&infoHeader->biSizeImage, sizeof(unsigned int), 1, dst) != 1) return FAILURE;
    if (fwrite(&infoHeader->biXPelsPerMeter, sizeof(int), 1, dst) != 1) return FAILURE;
    if (fwrite(&infoHeader->biYPelsPerMeter, sizeof(int), 1, dst) != 1) return FAILURE;
    if (fwrite(&infoHeader->biClrUsed, sizeof(unsigned int), 1, dst) != 1) return FAILURE;
    if (fwrite(&infoHeader->biClrImportant, sizeof(unsigned int), 1, dst) != 1) return FAILURE;
    
    int width = infoHeader->biWidth;
    int height = infoHeader->biHeight;
    
    // Calculate the padding: each row must have a size that is a multiple of 4 bytes
    int padding = (4 - (width * 3) % 4) % 4;
    
    // Write the pixels to the destination file (remember that BMP stores the pixels from bottom to top)
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            RGB_Pixel *p = &pixels[y * width + x];
            if (fwrite(&p->b, sizeof(uint8_t), 1, dst) != 1) return FAILURE;
            if (fwrite(&p->g, sizeof(uint8_t), 1, dst) != 1) return FAILURE;
            if (fwrite(&p->r, sizeof(uint8_t), 1, dst) != 1) return FAILURE;
        }

        // Write the padding bytes (zero)
        uint8_t zero = 0;
        for (int p = 0; p < padding; p++) {
            if (fwrite(&zero, sizeof(uint8_t), 1, dst) != 1) return FAILURE;
        }
    }
    
    return SUCCESS;
}