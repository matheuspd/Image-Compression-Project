#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

#include "img_functions.h"
#include "bmp.h"

/**
 * @brief Decompresses a binary-encoded image file and writes the decompressed result as a BMP image.
 *
 * This function opens the BIN file, reads its header and compressed data,
 * and then decompresses the data into BMP format.
 * 
 * @param input_bin Path to the input binary file that contains the compressed image.
 * @param output_bmp Path to the output BMP file where the decompressed image will be saved.
 * @return SUCCESS if decompression and writing are successful, or FAILURE on error.
 *
 * @note This function assumes that the input file contains valid compressed data including
 * the original BMP headers and the same compression pipeline.
 */
int decompress_bin(const char *input_bin, const char *output_bmp);

/**
 * @brief Reads all RLE-encoded blocks from the input binary file.
 *
 * This function initializes a bitreader and reads the Run-Length Encoded (RLE)
 * data for all blocks in the file. It separately reads the Y, Cb, and Cr
 * components for each block, storing the data and the number of coefficients in
 * dynamically allocated arrays.
 *
 * @param file Pointer to the binary input file opened for reading.
 * @param num_blocks Total number of blocks to read for each channel.
 * @return A pointer to an allocated RLE structure containing the RLE data
 *         for each component (Y, Cb, Cr), or NULL on failure.
 */
RLE *read_all_blocks(FILE *file, int num_blocks);

/**
 * @brief Converts RLE-encoded blocks into zigzag-ordered coefficient arrays.
 *
 * This function takes RLE data for Y, Cb, and Cr components and converts each block
 * into an array of 64 coefficients ordered according to the zigzag pattern.
 *
 * @param rle_blocks Pointer to the RLE structure containing RLE coefficients.
 * @param num_blocks Number of blocks to convert per channel.
 * @return A pointer to a Blocks_ZigZag structure containing arrays of 64 integer
 *         coefficients for each channel, or NULL on failure.
 */
Blocks_ZigZag *rle_to_blocks(RLE *rle_blocks, int num_blocks);

/**
 * @brief Converts DCT coefficient blocks to spatial-domain YCbCr pixels.
 *
 * This function processes all DCT blocks by:
 * 1. Undo the zigzag ordering of coefficients
 * 2. Dequantize the coefficients using the luminance and chrominance quantization matrices
 * 3. Apply the inverse DCT using precomputed matrices
 * 4. Convert each block back to Y, Cb, and Cr pixel values
 *
 * The result is a linear array of YCbCr_Pixel structures representing the entire image.
 *
 * @param blocks Pointer to the zigzag-ordered DCT coefficient blocks.
 * @param width Width of the image in pixels.
 * @param height Height of the image in pixels.
 * @param total_pixels Total number of pixels (width * height).
 * @param Ct Precomputed matrix for 8x8 DCT calculation.
 * @return A pointer to a dynamically allocated array of YCbCr_Pixel values,
 *         or NULL on memory allocation failure.
 */
YCbCr_Pixel *blocks_to_pixels(Blocks_ZigZag *blocks, int width, int height, int total_pixels, double Ct[BLOCK_SIZE][BLOCK_SIZE]);

#endif /* DECOMPRESSOR_H */
