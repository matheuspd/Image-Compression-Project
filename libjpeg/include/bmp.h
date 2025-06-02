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
int readInfoHeader(FILE *F, BMPINFOHEADER *H);

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
 * @return Pointer to an array of RGB_Pixel structures containing the pixel data, or NULL on failure.
 */
RGB_Pixel *read_pixels(FILE *file, BMPFILEHEADER *H, int width, int height);

/**
 * @brief Frees memory allocated for pixel data.
 *
 * @param pixels Pointer to the pixel array to be freed.
 */
void free_pixels(RGB_Pixel *pixels);

/**
 * @brief Writes a BMP file based on the provided headers and pixel array.
 *
 * This function receives the BMPFILEHEADER and BMPINFOHEADER, the pixel array, and
 * the destination file already opened, writing the data formatted correctly (including padding).
 *
 * @param dst Pointer to the destination file already opened in "wb" mode.
 * @param fileHeader Pointer to the BMP file header.
 * @param infoHeader Pointer to the BMP information header.
 * @param pixels Pixel array (RGB_Pixel) previously loaded.
 * @return SUCCESS if the operation is successful, otherwise FAILURE.
 */
int write_bmp(FILE *dst, BMPFILEHEADER *fileHeader, BMPINFOHEADER *infoHeader, RGB_Pixel *pixels);

#endif /* BMP_H */
