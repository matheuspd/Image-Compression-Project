#include "img_functions.h"

void multiply_matrix(double A[BLOCK_SIZE][BLOCK_SIZE], double B[BLOCK_SIZE][BLOCK_SIZE], 
                          double C[BLOCK_SIZE][BLOCK_SIZE]) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < BLOCK_SIZE; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void transpose(double A[BLOCK_SIZE][BLOCK_SIZE], double B[BLOCK_SIZE][BLOCK_SIZE]) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            B[i][j] = A[j][i];
        }
    }
}

YCbCr_Pixel *rgb_to_YCrCb(RGB_Pixel *rgb, int total_pixels) {
    YCbCr_Pixel *values = malloc(sizeof(YCbCr_Pixel) * total_pixels);
    if (!values) return NULL;

    for (int i = 0; i < total_pixels; i++) {

        values[i].Y  = 0.299 * rgb[i].r + 0.587 * rgb[i].g + 0.114 * rgb[i].b;
        values[i].Cb = 0.564 * (rgb[i].b - values[i].Y);
        values[i].Cr = 0.713 * (rgb[i].r - values[i].Y);
    }

    return values;
}

void subsample_4_2_0(YCbCr_Pixel *pixels_YCrCb, int width, int height) {
    for (int y = 0; y < height; y += 2) {
        for (int x = 0; x < width; x += 2) {
            int i1 = (y) * width + (x);
            int i2 = (y) * width + (x + 1);
            int i3 = (y + 1) * width + (x);
            int i4 = (y + 1) * width + (x + 1);

            // Calcula média dos Cb e Cr dos 4 pixels
            double cb_avg = (pixels_YCrCb[i1].Cb + pixels_YCrCb[i2].Cb +
                             pixels_YCrCb[i3].Cb + pixels_YCrCb[i4].Cb) / 4.0;

            double cr_avg = (pixels_YCrCb[i1].Cr + pixels_YCrCb[i2].Cr +
                             pixels_YCrCb[i3].Cr + pixels_YCrCb[i4].Cr) / 4.0;

            // Atribui a média aos 4 pixels
            pixels_YCrCb[i1].Cb = cb_avg;
            pixels_YCrCb[i2].Cb = cb_avg;
            pixels_YCrCb[i3].Cb = cb_avg;
            pixels_YCrCb[i4].Cb = cb_avg;

            pixels_YCrCb[i1].Cr = cr_avg;
            pixels_YCrCb[i2].Cr = cr_avg;
            pixels_YCrCb[i3].Cr = cr_avg;
            pixels_YCrCb[i4].Cr = cr_avg;
        }
    }
}

void apply_matrix_dct(double block[BLOCK_SIZE][BLOCK_SIZE], double dct[BLOCK_SIZE][BLOCK_SIZE], 
                           double Ct[BLOCK_SIZE][BLOCK_SIZE]) {
    double temp[BLOCK_SIZE][BLOCK_SIZE];
    
    multiply_matrix((double (*)[BLOCK_SIZE])C, block, temp);    // temp = C * B
    multiply_matrix(temp, Ct, dct);                  // DCT = temp * Ct
}

void quantize(double dct[BLOCK_SIZE][BLOCK_SIZE], const uint8_t matrix[BLOCK_SIZE][BLOCK_SIZE], 
               int output[BLOCK_SIZE][BLOCK_SIZE]) {

    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            output[i][j] = round(dct[i][j] / matrix [i][j]);
        }
    }
}

// compress = 0, decompress != 0
void zigzag(int block[8][8], int v[64], int type) {
    int i = 0, j = 0;

    for (int k = 0; k < 64; k++) {
        if(type == 0) v[k] = block[i][j];
        else block[i][j] = v[k];

        if ((i + j) % 2 == 0) {  // Direction: UP
            if (j == 7) {
                i++;
            } else if (i == 0) {
                j++;
            } else {
                i--;
                j++;
            }
        } else {  // Direction: DOWN
            if (i == 7) {
                j++;
            } else if (j == 0) {
                i++;
            } else {
                i++;
                j--;
            }
        }
    }
}

void delta_encoding_DC(Blocks_ZigZag *blocks) {
    if (blocks->num_blocks == 0) return;

    for (int i = blocks->num_blocks - 1; i > 0; i--) {
        blocks->Y_blocks[i][0] -= blocks->Y_blocks[i - 1][0];
        blocks->Cb_blocks[i][0] -= blocks->Cb_blocks[i - 1][0];
        blocks->Cr_blocks[i][0] -= blocks->Cr_blocks[i - 1][0];
    }
}

RLE_coef* RLE_encode_AC(int *coef, int *out_size) {
    RLE_coef *encoded = malloc(sizeof(RLE_coef) * 64);
    if (!encoded) {
        *out_size = 0;
        return NULL;
    }
    int skip = 0;
    int val = coef[0];
    int cat = coef_category(coef[0]);

    encoded[0] = (RLE_coef){skip, cat, val};
    int count = 1;

    for (int i = 1; i < 64; i++) {
        val = coef[i];
        if (val == 0) {
            skip++;
            if (skip == 16) {
                encoded[count] = (RLE_coef){15, 0, 0};
                count++;
                if (i != 63) skip = 0;
            }
        } else {
            cat = coef_category(val);
            encoded[count] = (RLE_coef){skip, cat, val};
            count++;
            if (i != 63) skip = 0;
        }
    }
    // If the end of the vector is all zeros, add EOB
    if (skip > 0) {
        encoded[count] = (RLE_coef){0, 0, 0}; // EOB
        if (encoded[count-1].skip == 15 && encoded[count-1].value == 0) {
            count--;
            while (encoded[count].skip == 15 && encoded[count].value == 0 ) {
                count--;
            }
            count++;
            encoded[count] = (RLE_coef){0, 0, 0}; // EOB
        }
        count++;
    }

    *out_size = count;
    return encoded;
}

int coef_category(int value) {
    int abs_val = abs(value);
    if (abs_val == 0) return 0;

    int category = 0;
    while (abs_val) {
        abs_val >>= 1;
        category++;
    }
    return category;
}

RLE_coef **process_AC_coef(int **blocks, int num_blocks, int *sizes) {
    RLE_coef **rle_results = malloc(num_blocks * sizeof(RLE_coef *));
    if (!rle_results) return NULL;

    for (int i = 0; i < num_blocks; i++) {
        rle_results[i] = RLE_encode_AC(blocks[i], &sizes[i]);
        if (!rle_results[i]) {
            for (int j = 0; j < i; j++) {
                free(rle_results[j]);
            }
            free(rle_results);
            return NULL;
        }
    }
    return rle_results;
}

int decode_dc(Bit_Read_Write *br, int *category) {
    char buffer[9] = {0}; // max prefix lenght = 8 bits
    for (int i = 0; i < 8; i++) {
        int bit = read_bit(br);
        if (bit < 0) return 0;
        buffer[i] = '0' + bit;
        buffer[i + 1] = '\0';

        for (int j = 0; j < 11; j++) {
            if (dc_table[j].prefix && strcmp(buffer, dc_table[j].prefix) == 0) {
                *category = dc_table[j].category;
                return 1;
            }
        }
    }
    return 0;
}

int decode_ac(Bit_Read_Write *br, int *skip, int *category) {
    char buffer[27] = {0}; // max prefix lenght = 26 bits
    for (int i = 0; i < 26; i++) {
        int bit = read_bit(br);
        if (bit < 0) return 0;
        buffer[i] = '0' + bit;
        buffer[i + 1] = '\0';

        for (int j = 0; j < 162; j++) {
            if (ac_table[j].prefix && strcmp(buffer, ac_table[j].prefix) == 0) {
                *skip = ac_table[j].zeros;
                *category = ac_table[j].category;
                return 1;
            }
        }
    }
    return 0;
}

RLE_coef *read_rle_block(Bit_Read_Write *br, int *size) {
    RLE_coef *coefs = malloc(64 * sizeof(RLE_coef));
    int index = 0;

    // DC
    int category;
    if (!decode_dc(br, &category)) return NULL;

    int value = 0;
    if (category > 0) {
        value = read_bits_complement1(br, category);
    }

    coefs[index++] = (RLE_coef){0, category, value};

    // AC
    while (index < 64) {
        int skip, cat;
        if (!decode_ac(br, &skip, &cat)) return NULL;

        if (skip == 0 && cat == 0) { // EOB
            coefs[index++] = (RLE_coef){0, 0, 0};
            break;
        }

        int val = read_bits_complement1(br, cat);
        
        coefs[index++] = (RLE_coef){skip, cat, val};
    }

    *size = index;
    return coefs;
}

void delta_decoding(Blocks_ZigZag *blocks) {
    if (blocks->num_blocks == 0) return;

    for (int i = 1; i < blocks->num_blocks; i++) {
        blocks->Y_blocks[i][0] += blocks->Y_blocks[i - 1][0];
        blocks->Cb_blocks[i][0] += blocks->Cb_blocks[i - 1][0];
        blocks->Cr_blocks[i][0] += blocks->Cr_blocks[i - 1][0];
    }
}

int *rle_to_block(RLE_coef *rle, int size) {
    int *block = calloc(64, sizeof(int));
    int index = 0;

    for (int i = 0; i < size && index < 64; i++) {

        RLE_coef coef = rle[i];

        if (i != 0 && (coef.skip == 0 && coef.category == 0)) break; // EOB - the rest is already zero

        index += coef.skip;
        if (index >= 64) break;

        block[index++] = coef.value;
    }

    return block;
}

void dequantize(int input[BLOCK_SIZE][BLOCK_SIZE], const uint8_t matrix[BLOCK_SIZE][BLOCK_SIZE],
                  double dct[BLOCK_SIZE][BLOCK_SIZE]) {
    
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            dct[i][j] = input[i][j] * matrix[i][j];
        }
    }
}

void apply_matrix_idct(double dct[BLOCK_SIZE][BLOCK_SIZE], double block[BLOCK_SIZE][BLOCK_SIZE],
                            double Ct[BLOCK_SIZE][BLOCK_SIZE]) {
    
    double temp[BLOCK_SIZE][BLOCK_SIZE];
    
    multiply_matrix(Ct, dct, temp);    // temp = Ct * DCT
    multiply_matrix(temp, (double (*)[BLOCK_SIZE])C, block);   // block = temp * C
}

RGB_Pixel *YCbCr_to_rgb(YCbCr_Pixel *ycbcr, int total_pixels) {
    RGB_Pixel *rgb = malloc(sizeof(RGB_Pixel) * total_pixels);
    if (!rgb) return NULL;

    for (int i = 0; i < total_pixels; i++) {
        double Y  = ycbcr[i].Y;
        double Cb = ycbcr[i].Cb;
        double Cr = ycbcr[i].Cr;

        double R = Y + 1.402 * Cr;
        double G = Y - 0.344 * Cb - 0.714 * Cr;
        double B = Y + 1.772 * Cb;
        
        rgb[i].r = (uint8_t)( (R > 255) ? 255 : ( (R < 0) ? 0 : round(R)) );
        rgb[i].g = (uint8_t)( (G > 255) ? 255 : ( (G < 0) ? 0 : round(G)) );
        rgb[i].b = (uint8_t)( (B > 255) ? 255 : ( (B < 0) ? 0 : round(B)) );
    }

    return rgb;
}

void free_BlocosZigZag(Blocks_ZigZag *blocks) {
    for (int i = 0; i < blocks->num_blocks; i++) {
        free(blocks->Y_blocks[i]);
        free(blocks->Cb_blocks[i]);
        free(blocks->Cr_blocks[i]);
    }

    free(blocks->Y_blocks);
    free(blocks->Cb_blocks);
    free(blocks->Cr_blocks);
    free(blocks);
}

void free_rle(RLE *rle, int num_blocks) {
    for (int i = 0; i < num_blocks; i++) {
        free(rle->Y_rle[i]);
        free(rle->Cb_rle[i]);
        free(rle->Cr_rle[i]);
    }

    free(rle->Y_rle);
    free(rle->Cb_rle);
    free(rle->Cr_rle);
    free(rle->Y_sizes);
    free(rle->Cb_sizes);
    free(rle->Cr_sizes);
    free(rle);
}