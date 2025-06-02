#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "types.h"
#include "bmp.h"
#include "jpeg.h"

#include <string.h>
#include <math.h>

/**
 * @brief Compresses a BMP file into JPEG format.
 *
 * This function reads the BMP headers, validates the image, and reads its pixel data.
 * The actual JPEG compression is assumed to be handled elsewhere.
 *
 * @param input_bmp Path to the input BMP file.
 * @param output_jpeg Path to the output JPEG file.
 * @return SUCCESS if the compression is successful, otherwise FAILURE.
 */
int compress_jpeg(const char *input_bmp, const char *output_bin);

YCbCrPixel *rgb_to_YCrCb(RGBPixel *pixels, int total_pixels);

void multiplicar_matrizes(double A[BLOCK_SIZE][BLOCK_SIZE], double B[BLOCK_SIZE][BLOCK_SIZE], 
                          double C[BLOCK_SIZE][BLOCK_SIZE]);

// Transpõe matriz 8x8: B = A^T
void transpor(double A[BLOCK_SIZE][BLOCK_SIZE], double B[BLOCK_SIZE][BLOCK_SIZE]);

// Aplica DCT usando a fórmula matricial DCT = C * B * C^T
void aplicar_dct_matricial(double block[BLOCK_SIZE][BLOCK_SIZE], double output[BLOCK_SIZE][BLOCK_SIZE], 
                           double Ct[BLOCK_SIZE][BLOCK_SIZE]);


void downsample_4_2_0(YCbCrPixel *pixels_YCrCb, int width, int height);


void quantizar(double dct[BLOCK_SIZE][BLOCK_SIZE], const uint8_t matrix[BLOCK_SIZE][BLOCK_SIZE], 
               int output[BLOCK_SIZE][BLOCK_SIZE]);

BlocksZigZag process_channels(YCbCrPixel *pixels_YCrCb, int width, int height, double Ct[BLOCK_SIZE][BLOCK_SIZE]);

void free_BlocosZigZag(BlocksZigZag blocks);

void delta_encoding_DC(BlocksZigZag *blocks);

int coef_category(int value);

RLE_coef* RLE_encode_AC(int *coef, int *out_size);

RLE_coef **process_AC_coef(int **blocks, int num_blocks, int *sizes);

RLE process_zigzag_vectors(BlocksZigZag *blocks);

void init_bitwriter(BitWriter *bw, FILE *fp);

void write_bit(BitWriter *bw, int bit);

void write_bits(BitWriter *bw, const char *bits);

void write_n_bits(BitWriter *bw, int value, int n);

void flush_bits(BitWriter *bw);

void write_bits_complement1(BitWriter *bw, int value, int n_bits);

void write_channel_blocks(BitWriter *bw, int *sizes, RLE_coef **rle, int num_blocks);

#endif /* COMPRESSOR_H */
