#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "img_functions.h"
#include "bmp.h"

/**
 * @brief Compresses a BMP file into a BIN file.
 *
 * This function opens the BMP file, reads its header and pixel data,
 * and then compresses the data into BIN format still with the BMP header.
 * 
 * @param input_bmp Path to the input BMP file.
 * @param output_bin Path to the output BIN file.
 * @return SUCCESS if the compression is successful, otherwise FAILURE.
 */
int compress_bmp(const char *input_bmp, const char *output_bin);

/**
 * @brief Applies DCT, quantization, and zig-zag ordering to Y, Cb, and Cr image channels.
 *
 * This function receives a linear array of pixels in the YCbCr color space,
 * splits them into blocks, applies the Discrete Cosine Transform (DCT)
 * to each block, quantizes the resulting coefficients, and finally
 * reorders them in zig-zag order for compression.
 *
 * @param pixels_YCrCb Pointer to the input pixel array in YCbCr color space.
 * @param width Width of the image in pixels (must be divisible by BLOCK_SIZE).
 * @param height Height of the image in pixels (must be divisible by BLOCK_SIZE).
 * @param Ct Precomputed matrix used in the DCT calculation.
 * @return Pointer to a dynamically allocated Blocks_ZigZag struct containing the processed data,
 *         or NULL if a memory allocation fails.
 */
Blocks_ZigZag *process_channels(YCbCr_Pixel *pixels_YCrCb, int width, int height, double Ct[BLOCK_SIZE][BLOCK_SIZE]);

/**
 * @brief Applies RLE (Run-Length Encoding) on ZigZag vectors.
 *
 * This function takes the zigzag blocks for each color channel (Y, Cb, Cr),
 * performs run-length encoding (RLE) on the AC coefficients of each block, and stores the result.
 * The size of each RLE-compressed block is also recorded.
 *
 * The encoding follows JPEG-style RLE, encoding (SKIP, CATEGORY, VALUE) for each non-zero AC coefficient.
 * It also handles special cases like long runs of zeros (ZRL) and End-Of-Block (EOB) markers.
 *
 * @param blocks Pointer to the structure containing zigzag vectors for Y, Cb, and Cr components.
 * @return Pointer to a RLE structure containing encoded data and sizes for each block,
 *         or NULL if a memory allocation or encoding step fails.
 */
RLE *process_zigzag_vectors(Blocks_ZigZag *blocks);

/**
 * @brief Writes RLE-compressed DCT coefficients of a channel to a binary stream using Huffman coding.
 *
 * For each block, the function encodes the DC coefficient using the DC Huffman table,
 * and the AC coefficients using the AC Huffman table (both specified in the code).
 * Encoded bits are written using the bit writer structure.
 *
 * @param bw Pointer to the Bit_Read_Write structure used to write bits to file.
 * @param sizes Array with the number of encoded RLE coefficients for each block.
 * @param rle 2D array of RLE-encoded coefficients for each block.
 * @param num_blocks Number of blocks in the channel.
 * @return SUCCESS (0) on success, or FAILURE (non-zero) on error.
 */
int write_channel_blocks(Bit_Read_Write *bw, int *sizes, RLE_coef **rle, int num_blocks);

#endif /* COMPRESSOR_H */
