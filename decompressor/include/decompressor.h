#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

#include "types.h"
#include "bmp.h"
#include "jpeg.h"

#include <string.h>
#include <math.h>

int decompress_bin(const char *input_bin, const char *output_bmp);

void init_bitreader(BitReader *br, FILE *fp);

int read_bit(BitReader *br);

int read_n_bits(BitReader *br, int n);

int read_bits_complement1(BitReader *br, int n_bits);

int decode_dc(BitReader *br, int *category);

int decode_ac(BitReader *br, int *skip, int *category);

RLE_coef *read_rle_block(BitReader *br, int *size);

RLE *read_all_blocks(FILE *file, int num_blocks);

void delta_decoding_DC(BlocksZigZag *blocks);

int *rle_to_block(RLE_coef *rle, int size);

BlocksZigZag *rle_to_blocks(RLE *rle_blocks, int num_blocks);

void desfazer_zigzag(int v[64], int block[8][8]);

YCbCrPixel *blocks_to_pixels(BlocksZigZag *blocks, int width, int height, int total_pixels, double Ct[BLOCK_SIZE][BLOCK_SIZE]);

void desquantizar(int input[BLOCK_SIZE][BLOCK_SIZE], const uint8_t matrix[BLOCK_SIZE][BLOCK_SIZE],
                  double dct[BLOCK_SIZE][BLOCK_SIZE]);

void aplicar_idct_matricial(double dct[BLOCK_SIZE][BLOCK_SIZE], double block[BLOCK_SIZE][BLOCK_SIZE],
                            double Ct[BLOCK_SIZE][BLOCK_SIZE]);

void multiplicar_matrizes(double A[BLOCK_SIZE][BLOCK_SIZE], double B[BLOCK_SIZE][BLOCK_SIZE], 
                          double C[BLOCK_SIZE][BLOCK_SIZE]);

void transpor(double A[BLOCK_SIZE][BLOCK_SIZE], double B[BLOCK_SIZE][BLOCK_SIZE]);

RGBPixel *YCbCr_to_rgb(YCbCrPixel *ycbcr, int total_pixels);

#endif /* DECOMPRESSOR_H */
