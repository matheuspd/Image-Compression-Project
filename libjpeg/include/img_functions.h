#ifndef IMG_FUNCTIONS_H
#define IMG_FUNCTIONS_H

#include "types.h"
#include "bit_functions.h"

#include <string.h>
#include <math.h>

/**
 * @brief Multiplies two 'BLOCK_SIZE' x 'BLOCK_SIZE' matrices (A * B = C).
 *
 * Performs standard matrix multiplication between two 'BLOCK_SIZE' x 'BLOCK_SIZE' matrices
 * A and B, storing the result in matrix C.
 *
 * @param A The first matrix.
 * @param B The second matrix.
 * @param C The output matrix to store the result.
 */
void multiply_matrix(double A[BLOCK_SIZE][BLOCK_SIZE], double B[BLOCK_SIZE][BLOCK_SIZE], 
                          double C[BLOCK_SIZE][BLOCK_SIZE]);

/**
 * @brief Transposes an 'BLOCK_SIZE' x 'BLOCK_SIZE' matrix.
 *
 * Computes the transpose of matrix A and stores it in matrix B.
 *
 * @param A Input matrix.
 * @param B Output transposed matrix.
 */
void transpose(double A[BLOCK_SIZE][BLOCK_SIZE], double B[BLOCK_SIZE][BLOCK_SIZE]);

/**
 * @brief Converts an array of RGB pixels to YCbCr color space.
 *
 * Applies standard color transformation from RGB to YCbCr for each pixel,
 * using the ITU-R BT-601 formula.
 *
 * @param rgb Pointer to input array of RGB_Pixel structures.
 * @param total_pixels Total number of pixels to convert.
 * @return Pointer to dynamically allocated array of YCbCr_Pixel structures,
 *         or NULL on allocation failure.
 */
YCbCr_Pixel *rgb_to_YCrCb(RGB_Pixel *pixels, int total_pixels);

/**
 * @brief Applies 4:2:0 chroma subsampling on YCbCr pixel data.
 *
 * Reduces the resolution of the Cb and Cr channels by averaging each 2x2 block
 * and assigning the average value to all four pixels in the block.
 *
 * @param pixels_YCrCb Array of YCbCr_Pixel structures.
 * @param width Width of the image in pixels (must be divisible by 2).
 * @param height Height of the image in pixels (must be divisible by 2).
 */
void subsample_4_2_0(YCbCr_Pixel *pixels_YCrCb, int width, int height);

/**
 * @brief Applies 2D Discrete Cosine Transform using matrix multiplication.
 *
 * Computes the DCT of an 8x8 block using the formula: DCT = C * B * C^T,
 * where C is the pre-calculated matrix for 8x8 DCT calculation and C^T is its transpose.
 *
 * @param block The input block in the spatial domain.
 * @param dct Output block in the frequency domain (DCT coefficients).
 * @param Ct The transposed DCT basis matrix (C^T).
 */
void apply_matrix_dct(double block[BLOCK_SIZE][BLOCK_SIZE], double output[BLOCK_SIZE][BLOCK_SIZE], 
                           double Ct[BLOCK_SIZE][BLOCK_SIZE]);

/**
 * @brief Quantizes DCT coefficients using the quantization matrix.
 *
 * Divides each DCT coefficient by the corresponding quantization matrix value,
 * rounding the result to the nearest integer.
 *
 * @param dct Input block of DCT coefficients.
 * @param matrix 8x8 quantization matrix (luminance or chrominance).
 * @param output Output matrix of quantized integer coefficients.
 */
void quantize(double dct[BLOCK_SIZE][BLOCK_SIZE], const uint8_t matrix[BLOCK_SIZE][BLOCK_SIZE], 
               int output[BLOCK_SIZE][BLOCK_SIZE]);

/**
 * @brief Converts between 2D 8x8 block and 1D 64-element array using zigzag order.
 *
 * Performs a zigzag scan when 'type == 0' (compression), converting an 8x8 matrix
 * to a 64-element vector. When 'type != 0' (decompression), it restores a 64-element
 * zigzag-ordered array back into an 8x8 block.
 *
 * @param block Input/output 8x8 block of coefficients.
 * @param v Input/output array of 64 integers.
 * @param type Conversion type: 0 = block => array, non-zero = array => block.
 */
void zigzag(int block[8][8], int v[64], int type);

/**
 * @brief Applies delta encoding to the DC coefficients of all blocks.
 *
 * Replaces the DC coefficient of each block with the difference from the previous
 * block’s DC value, reducing redundancy in preparation for entropy coding.
 *
 * @param blocks Pointer to 'Blocks_ZigZag' containing DCT coefficient blocks.
 */
void delta_encoding_DC(Blocks_ZigZag *blocks);

/**
 * @brief Determines the category (bit-length) needed to represent a coefficient value.
 *
 * This function returns the number of bits required to represent the absolute value of
 * a given integer. This corresponds to the "category" used in JPEG encoding.
 *
 * @param value Integer value whose category is to be calculated.
 * @return The category (number of bits) needed to encode the absolute value.
 */
int coef_category(int value);

/**
 * @brief Encodes a single 64-element AC coefficient block using JPEG-style run-length encoding (RLE).
 *
 * This function compresses the AC coefficients of a block (zigzag ordered) into a list of
 * (SKIP, CATEGORY, VALUE) tuples. It handles zero runs and appends
 * an End-Of-Block (EOB) marker if necessary.
 *
 * @param coef Pointer to a 64-element array of quantized coefficients.
 * @param out_size Pointer to an integer where the number of encoded symbols will be stored.
 * @return Pointer to an array of RLE_coef structures, or NULL on memory allocation failure.
 */
RLE_coef* RLE_encode_AC(int *coef, int *out_size);

/**
 * @brief Applies run-length encoding (RLE) to a set of quantized coefficient blocks.
 *
 * For each block, this function performs RLE using the JPEG AC coefficient encoding pattern.
 * It stores the resulting encoded symbols (skip, category, value) and their sizes.
 *
 * Memory is dynamically allocated for the output. If any encoding fails, previously allocated
 * memory is released and NULL is returned.
 *
 * @param blocks Array of pointers to 64-element coefficient arrays (zigzag ordered).
 * @param num_blocks Number of blocks to process.
 * @param sizes Output array to hold the size (number of RLE symbols) for each block.
 * @return A dynamically allocated array of RLE-encoded blocks, or NULL on failure.
 */
RLE_coef **process_AC_coef(int **blocks, int num_blocks, int *sizes);

/**
 * @brief Decodes a DC coefficient prefix and retrieves its category.
 *
 * @param br Pointer to the bitstream reader.
 * @param category Output pointer to store the decoded category.
 * @return 1 if decoding is successful, 0 otherwise.
 */
int decode_dc(Bit_Read_Write *br, int *category);

/**
 * @brief Decodes an AC coefficient prefix, retrieving the number of preceding zeros (skip)
 *        and the value category.
 *
 * @param br Pointer to the bitstream reader.
 * @param skip Output pointer to store the number of zero coefficients before this one.
 * @param category Output pointer to store the category of the AC coefficient.
 * @return 1 if decoding is successful, 0 otherwise.
 */
int decode_ac(Bit_Read_Write *br, int *skip, int *category);

/**
 * @brief Reads a block of RLE-encoded DCT coefficients from the bitstream.
 *
 * @param br Pointer to the bitstream reader.
 * @param size Output pointer to store the number of RLE entries read.
 * @return Pointer to an array of RLE_coef representing the block, or NULL on failure.
 */
RLE_coef *read_rle_block(Bit_Read_Write *br, int *size);

/**
 * @brief Reverses delta encoding on the DC coefficients of each block.
 *
 * @param blocks Pointer to the Blocks_ZigZag structure containing the Y, Cb, and Cr blocks.
 */
void delta_decoding(Blocks_ZigZag *blocks);

/**
 * @brief Converts a sequence of RLE coefficients back into a full 64-element block.
 *
 * @param rle Array of RLE_coef structures.
 * @param size Number of RLE entries in the array.
 * @return Pointer to a 64-element integer array representing the block (in zigzag order).
 */
int *rle_to_block(RLE_coef *rle, int size);

/**
 * @brief Dequantizes a DCT block by multiplying each coefficient by the corresponding quantization factor.
 *
 * @param input block of quantized integer values.
 * @param matrix 8x8 quantization matrix.
 * @param dct Output block of dequantized double values.
 */
void dequantize(int input[BLOCK_SIZE][BLOCK_SIZE], const uint8_t matrix[BLOCK_SIZE][BLOCK_SIZE],
                  double dct[BLOCK_SIZE][BLOCK_SIZE]);

/**
 * @brief Applies inverse DCT using matrix multiplication.
 *
 * @param dct Input 8x8 DCT-transformed matrix.
 * @param block Output 8x8 spatial-domain matrix.
 * @param Ct Transpose of the provided pre-calculated matrix for 8x8 DCT calculation (C^T).
 */
void apply_matrix_idct(double dct[BLOCK_SIZE][BLOCK_SIZE], double block[BLOCK_SIZE][BLOCK_SIZE],
                            double Ct[BLOCK_SIZE][BLOCK_SIZE]);

/**
 * @brief Converts an array of YCbCr pixels to RGB format.
 *
 * @param ycbcr Pointer to the input YCbCr pixel array.
 * @param total_pixels Number of pixels in the array.
 * @return Pointer to the newly allocated array of RGB_Pixel or NULL if allocation fails.
 */
RGB_Pixel *YCbCr_to_rgb(YCbCr_Pixel *ycbcr, int total_pixels);

/**
 * @brief Frees memory allocated for all Y, Cb, and Cr blocks in a Blocks_ZigZag structure.
 *
 * @param blocks Pointer to the structure to free.
 */
void free_BlocosZigZag(Blocks_ZigZag *blocks);

/**
 * @brief Frees memory allocated for RLE data in an RLE structure.
 *
 * @param rle Pointer to the structure to free.
 * @param num_blocks Number of RLE blocks in each channel.
 */
void free_rle(RLE *rle, int num_blocks);

#endif /* IMG_FUNCTIONS_H */
