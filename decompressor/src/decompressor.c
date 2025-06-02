#include "decompressor.h"

int decompress_bin(const char *input_bin, const char *output_bmp) {
    FILE *file = fopen(input_bin, "rb");
    if (!file) {
        printf("Erro ao abrir arquivo BMP.\n");
        return FAILURE;
    }

    if (output_bmp == NULL) return FAILURE;

    BMPFILEHEADER fileHeader;
    BMPINFOHEADER infoHeader;

    if (readHeader(file, &fileHeader) != SUCCESS) {
        printf("Erro lendo o header BMP.\n");
        fclose(file);
        return FAILURE;
    }

    if (readInfoHeader(file, &infoHeader) != SUCCESS) {
        printf("Erro lendo o info header BMP.\n");
        fclose(file);
        return FAILURE;
    }

    int width = infoHeader.biWidth;
    int height = infoHeader.biHeight;
    int total_pixels = width * height;

    int num_blocks = total_pixels / (BLOCK_SIZE * BLOCK_SIZE);

    RLE *rle_blocks = read_all_blocks(file, num_blocks);
    if(rle_blocks == NULL) return FAILURE;

    // for (int j = 0; j < num_blocks; j++) {
    //     for (int i = 0; i < rle_blocks->Y_sizes[j]; i++) {
    //         printf("%d/%d/%d, ", rle_blocks->Y_rle[j][i].skip,rle_blocks->Y_rle[j][i].category,rle_blocks->Y_rle[j][i].value);
    //     }
    //     printf("\n");
    // }

    BlocksZigZag *blocks = rle_to_blocks(rle_blocks, num_blocks);

    // for (int j = 0; j < num_blocks; j++) {
    //     for (int i = 0; i < 64; i++) {
    //         printf("%d ", blocks->Y_blocks[j][i]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    delta_decoding_DC(blocks); // desfaz o delta encoding nos valores DC

    // for (int j = 0; j < num_blocks; j++) {
    //     for (int i = 0; i < 64; i++) {
    //             printf("%d ", blocks->Y_blocks[j][i]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");
    

    double Ct[BLOCK_SIZE][BLOCK_SIZE];
    transpor((double (*)[BLOCK_SIZE])C, Ct); // Ct = C^T

    YCbCrPixel *pixels_YCrCb = blocks_to_pixels(blocks, width, height, total_pixels, Ct);
    if(pixels_YCrCb == NULL) return FAILURE;
    
    RGBPixel *pixels = YCbCr_to_rgb(pixels_YCrCb, total_pixels);
    if(pixels == NULL) return FAILURE;

    //Abre o arquivo de destino para escrita em modo binário
    FILE *dst = fopen(output_bmp, "wb");
    if (!dst) {
        printf("Erro ao criar o arquivo output.bmp.\n");
        free(pixels);
        return FAILURE;
    }
    
    // Chama a função que escreve o arquivo BMP, passando o ponteiro para o arquivo, os cabeçalhos e o vetor de pixels
    if (write_bmp(dst, &fileHeader, &infoHeader, pixels) != SUCCESS) {
        printf("Erro ao escrever o arquivo BMP.\n");
        fclose(dst);
        free(pixels);
        return FAILURE;
    }

    return SUCCESS;
}

void init_bitreader(BitReader *br, FILE *fp) {
    br->file = fp;
    br->buffer = 0;
    br->bit_count = 0;
}

int read_bit(BitReader *br) {
    if (br->bit_count == 0) {
        int c = fgetc(br->file);
        //printf("bits: %d\n", c);
        if (c == EOF) return -1; // EOF
        br->buffer = c;
        br->bit_count = 8;
    }
    int bit = (br->buffer >> 7) & 1;
    //printf("bit: %d\n", bit);
    br->buffer <<= 1;
    br->bit_count--;
    return bit;
}

int read_n_bits(BitReader *br, int n) {
    int value = 0;
    for (int i = 0; i < n; i++) {
        int bit = read_bit(br);
        if (bit == -1) return -1; // EOF
        value = (value << 1) | bit;
    }
    return value;
}

// Função que lê um valor inteiro em complemento de 1 com n bits
int read_bits_complement1(BitReader *br, int n_bits) {
    int raw = read_n_bits(br, n_bits);
    if (raw == -1) return 0; // erro

    int msb = raw >> (n_bits - 1);

    if (msb != 0) {
        //printf("positivo: %d\n", raw);
        return raw; // positivo
    } else {
        int inverted = (~raw) & ((1 << n_bits) - 1);
        //printf("negativo: %d\n", -inverted);
        return -inverted;
    }
}

int decode_dc(BitReader *br, int *category) {
    char buffer[9] = {0}; // prefixo máximo de 8 bits
    for (int i = 0; i < 8; i++) {
        int bit = read_bit(br);
        if (bit < 0) return 0;
        buffer[i] = '0' + bit;
        buffer[i + 1] = '\0';

        for (int j = 0; j < 11; j++) {
            if (dc_table[j].prefix && strcmp(buffer, dc_table[j].prefix) == 0) {
                *category = dc_table[j].category;
                //printf("DC: %s/%d\n", dc_table[j].prefix, dc_table[j].category);
                return 1;
            }
        }
    }
    return 0;
}

int decode_ac(BitReader *br, int *skip, int *category) {
    char buffer[27] = {0}; // prefixo máximo de 26 bits
    for (int i = 0; i < 26; i++) {
        int bit = read_bit(br);
        if (bit < 0) return 0;
        buffer[i] = '0' + bit;
        buffer[i + 1] = '\0';

        for (int j = 0; j < 162; j++) {
            if (ac_table[j].prefix && strcmp(buffer, ac_table[j].prefix) == 0) {
                *skip = ac_table[j].zeros;
                *category = ac_table[j].category;
                //printf("AC: %s/%d/%d\n", ac_table[j].prefix, ac_table[j].zeros, ac_table[j].category);
                return 1;
            }
        }
    }
    return 0;
}

RLE_coef *read_rle_block(BitReader *br, int *size) {
    RLE_coef *coefs = malloc(64 * sizeof(RLE_coef));
    int index = 0;

    // DC
    int category;
    if (!decode_dc(br, &category)) return NULL;

    int value = 0;
    if (category > 0) {
        value = read_bits_complement1(br, category);
    }
    //printf("DC value: %d\n", value);

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
        //printf("AC value: %d\n", val);
        coefs[index++] = (RLE_coef){skip, cat, val};
    }

    *size = index;
    return coefs;
}

RLE *read_all_blocks(FILE *file, int num_blocks) {
    BitReader br;
    init_bitreader(&br, file);

    RLE *rle = malloc(sizeof(RLE));
    rle->Y_rle = malloc(num_blocks * sizeof(RLE_coef *));
    rle->Y_sizes = malloc(num_blocks * sizeof(int));
    rle->Cb_rle = malloc(num_blocks * sizeof(RLE_coef *));
    rle->Cb_sizes = malloc(num_blocks * sizeof(int));
    rle->Cr_rle = malloc(num_blocks * sizeof(RLE_coef *));
    rle->Cr_sizes = malloc(num_blocks * sizeof(int));

    for (int i = 0; i < num_blocks; i++) {
        rle->Y_rle[i] = read_rle_block(&br, &rle->Y_sizes[i]);
    }
    for (int i = 0; i < num_blocks; i++) {
        rle->Cb_rle[i] = read_rle_block(&br, &rle->Cb_sizes[i]);
    }
    for (int i = 0; i < num_blocks; i++) {
        rle->Cr_rle[i] = read_rle_block(&br, &rle->Cr_sizes[i]);
    }

    return rle;
}

void delta_decoding_DC(BlocksZigZag *blocks) {
    if (blocks->num_blocks == 0) return;

    for (int i = 1; i < blocks->num_blocks; i++) {
        blocks->Y_blocks[i][0] += blocks->Y_blocks[i - 1][0];
        blocks->Cb_blocks[i][0] += blocks->Cb_blocks[i - 1][0];
        blocks->Cr_blocks[i][0] += blocks->Cr_blocks[i - 1][0];
    }
}

int *rle_to_block(RLE_coef *rle, int size) {
    int *block = calloc(64, sizeof(int)); // inicia com zeros
    int index = 0;

    for (int i = 0; i < size && index < 64; i++) {

        RLE_coef coef = rle[i];
        //printf("%d\n", coef.value);

        if (i != 0 && (coef.skip == 0 && coef.category == 0)) break; // EOB - o resto já está zerado

        index += coef.skip;
        if (index >= 64) break;

        block[index++] = coef.value;
    }

    return block;
}


BlocksZigZag *rle_to_blocks(RLE *rle_blocks, int num_blocks) {
    BlocksZigZag *blocks = malloc(sizeof(BlocksZigZag));
    blocks->num_blocks = num_blocks;

    blocks->Y_blocks = malloc(num_blocks * sizeof(int *));
    blocks->Cb_blocks = malloc(num_blocks * sizeof(int *));
    blocks->Cr_blocks = malloc(num_blocks * sizeof(int *));
    

    for (int i = 0; i < num_blocks; i++) {
        blocks->Y_blocks[i] = rle_to_block(rle_blocks->Y_rle[i], rle_blocks->Y_sizes[i]);
        blocks->Cb_blocks[i] = rle_to_block(rle_blocks->Cb_rle[i], rle_blocks->Cb_sizes[i]);
        blocks->Cr_blocks[i] = rle_to_block(rle_blocks->Cr_rle[i], rle_blocks->Cr_sizes[i]);
    }

    return blocks;
}

void desfazer_zigzag(int v[64], int block[8][8]) {
    int i = 0, j = 0;

    for (int k = 0; k < 64; k++) {
        block[i][j] = v[k];

        if ((i + j) % 2 == 0) {  // direção: para cima
            if (j == 7) {
                i++;
            } else if (i == 0) {
                j++;
            } else {
                i--;
                j++;
            }
        } else {  // direção: para baixo
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


YCbCrPixel *blocks_to_pixels(BlocksZigZag *blocks, int width, int height, int total_pixels, double Ct[BLOCK_SIZE][BLOCK_SIZE]) {

    YCbCrPixel *values = malloc(sizeof(YCbCrPixel) * total_pixels);
    if (!values) return NULL;

    int idx = 0;

    for (int j = 0; j < height; j += BLOCK_SIZE) {
        for (int i = 0; i < width; i += BLOCK_SIZE) {

            int temp_Y[BLOCK_SIZE][BLOCK_SIZE];
            int temp_Cb[BLOCK_SIZE][BLOCK_SIZE];
            int temp_Cr[BLOCK_SIZE][BLOCK_SIZE];

            double dct_Y[BLOCK_SIZE][BLOCK_SIZE];
            double dct_Cb[BLOCK_SIZE][BLOCK_SIZE];
            double dct_Cr[BLOCK_SIZE][BLOCK_SIZE];

            double original_Y[BLOCK_SIZE][BLOCK_SIZE];
            double original_Cb[BLOCK_SIZE][BLOCK_SIZE];
            double original_Cr[BLOCK_SIZE][BLOCK_SIZE];

            desfazer_zigzag(blocks->Y_blocks[idx], temp_Y);
            desfazer_zigzag(blocks->Cb_blocks[idx], temp_Cb);
            desfazer_zigzag(blocks->Cr_blocks[idx], temp_Cr);

            // for (int k = 0; k < 64; k++) {
            //     printf("%d ", blocks->Y_blocks[idx][k]);
            // }
            // printf("\n\n");

            idx++;


            desquantizar(temp_Y, lumin_matrix, dct_Y);
            desquantizar(temp_Cb, crom_matrix, dct_Cb);
            desquantizar(temp_Cr, crom_matrix, dct_Cr);


            aplicar_idct_matricial(dct_Y, original_Y, Ct);
            aplicar_idct_matricial(dct_Cb, original_Cb, Ct);
            aplicar_idct_matricial(dct_Cr, original_Cr, Ct);

            for (int y = 0; y < BLOCK_SIZE; y++) {
                for (int x = 0; x < BLOCK_SIZE; x++) {
                    int dy = j + y;
                    int dx = i + x;
                    values[dy * width + dx].Y = original_Y[x][y] + 128.0;
                    values[dy * width + dx].Cb = original_Cb[x][y] + 128.0;
                    values[dy * width + dx].Cr = original_Cr[x][y] + 128.0;
                }
            }

        }
    }

    // for(int j = 0; j < height; j++) {
    //     for(int i = 0; i < width; i++){
    //         printf("%f ", values[j * width + i].Y);
    //     }
    //     printf("\n");
    // }

    return values;
}

void desquantizar(int input[BLOCK_SIZE][BLOCK_SIZE], const uint8_t matrix[BLOCK_SIZE][BLOCK_SIZE],
                  double dct[BLOCK_SIZE][BLOCK_SIZE]) {
    
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            dct[i][j] = input[i][j] * matrix[i][j];
        }
    }
}

void aplicar_idct_matricial(double dct[BLOCK_SIZE][BLOCK_SIZE], double block[BLOCK_SIZE][BLOCK_SIZE],
                            double Ct[BLOCK_SIZE][BLOCK_SIZE]) {
    
    double temp[BLOCK_SIZE][BLOCK_SIZE];
    
    multiplicar_matrizes(Ct, dct, temp);    // temp = Ct * DCT
    multiplicar_matrizes(temp, (double (*)[BLOCK_SIZE])C, block);   // block = temp * C
}

void transpor(double A[BLOCK_SIZE][BLOCK_SIZE], double B[BLOCK_SIZE][BLOCK_SIZE]) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            B[i][j] = A[j][i];
        }
    }
}

// Multiplica duas matrizes 8x8: C = A × B
void multiplicar_matrizes(double A[BLOCK_SIZE][BLOCK_SIZE], double B[BLOCK_SIZE][BLOCK_SIZE], 
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

RGBPixel *YCbCr_to_rgb(YCbCrPixel *ycbcr, int total_pixels) {
    RGBPixel *rgb = malloc(sizeof(RGBPixel) * total_pixels);
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
