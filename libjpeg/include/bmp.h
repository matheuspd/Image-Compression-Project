#ifndef BMP_H
#define BMP_H

#include "types.h"
#include <stdio.h>

/**
 * @brief Reads the BMP file header from the given file.
 *
 * This function reads the BMP file header and verifies the magic number.
 *
 * @param F Pointer to the BMP file opened in binary mode.
 * @param H Pointer to a BMPFILEHEADER structure where the header data will be stored.
 * @return SUCCESS if the header is read successfully, otherwise FAILURE.
 */
int readHeader(FILE *F, BMPFILEHEADER *H);

/**
 * @brief Reads the BMP file information header from the given file.
 *
 * This function reads the BMP info header and validates key properties such as image dimensions,
 * ensuring they are multiples of 8 and within the allowed range, and confirms that the pixel format is 24-bit uncompressed.
 *
 * @param F Pointer to the BMP file opened in binary mode.
 * @param H Pointer to a BMPINFOHEADER structure where the header info will be stored.
 * @return SUCCESS if the info header is read and valid, otherwise FAILURE.
 */
int readInfoHeader(FILE *F, BMPINFOHEADER *H);

/**
 * @brief Reads pixel data from the BMP file.
 *
 * This function calculates the necessary padding per row (since BMP rows are padded to a multiple of 4 bytes)
 * and reads the pixel data from the file in bottom-to-top order.
 *
 * @param file Pointer to the BMP file opened in binary mode.
 * @param H Pointer to the BMP file header (used for offset information).
 * @param width Image width.
 * @param height Image height.
 * @return Pointer to an array of RGBPixel structures containing the image pixels, or NULL on failure.
 */
RGBPixel *read_pixels(FILE *file, BMPFILEHEADER *H, int width, int height);

/**
 * @brief Frees memory allocated for pixel data.
 *
 * @param pixels Pointer to the pixel array to be freed.
 */
void free_pixels(RGBPixel *pixels);

#endif /* BMP_H */
