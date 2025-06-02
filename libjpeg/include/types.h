#ifndef TYPES_H
#define TYPES_H

#define SUCCESS 0
#define FAILURE 1

#define BLOCK_SIZE 8
#define MAX_LEN_MATRIX_LINE_SIZE 65

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/**
 * @brief BMP file header structure.
 *
 * This structure represents the BMP file header.
 * 
 * @note The bfType field should be equal to 0x4d42, which corresponds to "BM" in little-endian format.
 */
typedef struct {                /**** BMP file header structure ****/
    unsigned short bfType;          /* Magic number for file */
    unsigned int bfSize;            /* Size of file */
    unsigned short bfReserved1;     /* Reserved */
    unsigned short bfReserved2;     /* ... */
    unsigned int bfOffBits;         /* Offset to bitmap data */
} __attribute__((packed)) BMPFILEHEADER;

/**
 * @brief BMP file information header structure.
 *
 * This structure contains information about the BMP image.
 */
typedef struct {                /**** BMP file info structure ****/
    unsigned int biSize;            /* Size of info header */
    int biWidth;                    /* Width of image */
    int biHeight;                   /* Height of image */
    unsigned short biPlanes;        /* Number of color planes */
    unsigned short biBitCount;      /* Number of bits per pixel */
    unsigned int biCompression;     /* Type of compression to use */
    unsigned int biSizeImage;       /* Size of image data */
    int biXPelsPerMeter;            /* X pixels per meter */
    int biYPelsPerMeter;            /* Y pixels per meter */
    unsigned int biClrUsed;         /* Number of colors used */
    unsigned int biClrImportant;    /* Number of important colors */
} __attribute__((packed)) BMPINFOHEADER;

/**
 * @brief Structure representing an RGB pixel.
 *
 * Contains blue, green, and red color components.
 */
typedef struct {
    uint8_t b;  /* Blue component */
    uint8_t g;  /* Green component */
    uint8_t r;  /* Red component */
} RGB_Pixel;

typedef struct {
    double Y;
    double Cb;
    double Cr;
} YCbCr_Pixel;

typedef struct {
    int **Y_blocks;
    int **Cb_blocks;
    int **Cr_blocks;
    int num_blocks;
} Blocks_ZigZag;

typedef struct {
    int skip;
    int category;
    int value;
} RLE_coef;

typedef struct {
    RLE_coef **Y_rle;
    RLE_coef **Cb_rle;
    RLE_coef **Cr_rle;
    int *Y_sizes;
    int *Cb_sizes;
    int *Cr_sizes;
} RLE;

typedef struct {
    int category;
    const char *prefix;
    int total_length;
    int mantissa_bits;
} DC_Huffman_Code;

typedef struct {
    int zeros;
    int category;
    const char *prefix;
    int total_length;
} AC_Huffman_Code;

typedef struct {
    FILE *file;
    unsigned char buffer;
    int bit_count;
} Bit_Read_Write;

extern const uint8_t lumin_matrix[BLOCK_SIZE][BLOCK_SIZE];

extern const uint8_t chrom_matrix[BLOCK_SIZE][BLOCK_SIZE];

// Matriz C fornecida (pré-calculada para DCT 8x8)
extern const double C[BLOCK_SIZE][BLOCK_SIZE];

extern const DC_Huffman_Code dc_table[11];

extern const AC_Huffman_Code ac_table[162];


#endif /* TYPES_H */
