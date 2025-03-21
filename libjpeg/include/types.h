#ifndef TYPES_H
#define TYPES_H

#define SUCCESS 0
#define FAILURE 1

#include <stdint.h>

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
} RGBPixel;

/*
typedef struct {
    unsigned int Y, Cb, Cr;
} YCbCrPixel;
*/

#endif /* TYPES_H */
