#include "compressor.h"


/**
 * @brief Compresses a BMP file into a JPEG file.
 *
 * This function opens the BMP file, reads its header and pixel data,
 * and then compresses the data into JPEG format.
 * 
 * @param input_bmp Path to the input BMP file.
 * @param output_jpeg Path to the output JPEG file.
 * @return SUCCESS if the compression is successful, otherwise FAILURE.
 */
// Função principal de compressão
int compress_jpeg(const char *input_bmp, const char *output_bin) {
    FILE *file = fopen(input_bmp, "rb");
    if (!file) {
        printf("Erro ao abrir arquivo BMP.\n");
        return FAILURE;
    }

    if (output_bin == NULL) return FAILURE;

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

    RGBPixel *pixels = read_pixels(file, &fileHeader, width, height);
    if (!pixels) {
        printf("Erro alocando pixels.\n");
        fclose(file);
        return FAILURE;
    }
    fclose(file);

    // DEBUGGING
    // Abre o arquivo de destino para escrita em modo binário
    // FILE *dst = fopen("output.bmp", "wb");
    // if (!dst) {
    //     printf("Erro ao criar o arquivo output.bmp.\n");
    //     free(pixels);
    //     return FAILURE;
    // }
    
    // Chama a função que escreve o arquivo BMP, passando o ponteiro para o arquivo, os cabeçalhos e o vetor de pixels
    // if (write_bmp(dst, &fileHeader, &infoHeader, pixels) != SUCCESS) {
    //     printf("Erro ao escrever o arquivo BMP.\n");
    //     fclose(dst);
    //     free(pixels);
    //     return FAILURE;
    // }    

    YCbCrPixel *pixels_YCrCb = rgb_to_YCrCb(pixels, total_pixels);
    if (!pixels_YCrCb) {
        free(pixels);
        return FAILURE;
    }

    // Considerando largura e altura múltiplos de 2
    downsample_4_2_0(pixels_YCrCb, width, height);

    double Ct[BLOCK_SIZE][BLOCK_SIZE];
    transpor((double (*)[BLOCK_SIZE])C, Ct); // Ct = C^T


    // Criar 3 matrizes (Y, Cb, Cr) que guardam os 'num_blocks' vetores de zigzag (64 elementos cada = 8x8)
    BlocksZigZag zigzag_vectors = process_channels(pixels_YCrCb, width, height, Ct);

    // for (int n = 0; n < (width * height) / (BLOCK_SIZE * BLOCK_SIZE); n++) {
    //     for (int i = 0; i < 64; i++) {
    //         printf("%d ", zigzag_vectors.Y_blocks[n][i]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    delta_encoding_DC(&zigzag_vectors);

    // for (int n = 0; n < (width * height) / (BLOCK_SIZE * BLOCK_SIZE); n++) {
    //     for (int i = 0; i < 64; i++) {
    //         printf("%d ", zigzag_vectors.Y_blocks[n][i]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    // APLICAR FUNÇÃO RLE PARA CADA VETOR DOS BLOCOS ZIGZAG Y, CB E CR

    RLE rle_result = process_zigzag_vectors(&zigzag_vectors);

    // for (int j = 0; j < zigzag_vectors.num_blocks; j++) {
    //     for (int i = 0; i < rle_result.Y_sizes[j]; i++) {
    //         printf("%d/%d/%d, ", rle_result.Y_rle[j][i].skip,rle_result.Y_rle[j][i].category,rle_result.Y_rle[j][i].value);
    //     }
    //     printf("\n");
    // }

    FILE *out = fopen(output_bin, "wb");

    BitWriter bw;
    init_bitwriter(&bw, out);

    // Cabeçalhos BMP
    fwrite(&fileHeader, sizeof(fileHeader), 1, out);
    fwrite(&infoHeader, sizeof(infoHeader), 1, out);

    write_channel_blocks(&bw, rle_result.Y_sizes, rle_result.Y_rle, zigzag_vectors.num_blocks);
    write_channel_blocks(&bw, rle_result.Cb_sizes, rle_result.Cb_rle, zigzag_vectors.num_blocks);
    write_channel_blocks(&bw, rle_result.Cr_sizes, rle_result.Cr_rle, zigzag_vectors.num_blocks);

    flush_bits(&bw);
    fclose(out);

    free_BlocosZigZag(zigzag_vectors);    
    free(pixels_YCrCb);
    free(pixels);

    return SUCCESS;
}

YCbCrPixel *rgb_to_YCrCb(RGBPixel *rgb, int total_pixels) {
    YCbCrPixel *values = malloc(sizeof(YCbCrPixel) * total_pixels);
    if (!values) return NULL;

    for (int i = 0; i < total_pixels; i++) {

        values[i].Y  = 0.299 * rgb[i].r + 0.587 * rgb[i].g + 0.114 * rgb[i].b;
        values[i].Cb = 0.564 * (rgb[i].b - values[i].Y);
        values[i].Cr = 0.713 * (rgb[i].r - values[i].Y);
    }

    return values;
}

void downsample_4_2_0(YCbCrPixel *pixels_YCrCb, int width, int height) {
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

// Transpõe matriz 8x8: B = A^T
void transpor(double A[BLOCK_SIZE][BLOCK_SIZE], double B[BLOCK_SIZE][BLOCK_SIZE]) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            B[i][j] = A[j][i];
        }
    }
}

// Aplica DCT usando a fórmula matricial DCT = C * B * C^T
void aplicar_dct_matricial(double block[BLOCK_SIZE][BLOCK_SIZE], double dct[BLOCK_SIZE][BLOCK_SIZE], 
                           double Ct[BLOCK_SIZE][BLOCK_SIZE]) {
    double temp[BLOCK_SIZE][BLOCK_SIZE];
    
    multiplicar_matrizes((double (*)[BLOCK_SIZE])C, block, temp);    // temp = C * B
    multiplicar_matrizes(temp, Ct, dct);                  // DCT = temp * Ct
}

void quantizar(double dct[BLOCK_SIZE][BLOCK_SIZE], const uint8_t matrix[BLOCK_SIZE][BLOCK_SIZE], 
               int output[BLOCK_SIZE][BLOCK_SIZE]) {

    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            output[i][j] = round(dct[i][j] / matrix [i][j]);
        }
    }
}

void aplicar_zigzag(int block[8][8], int v[64]) {
    int i = 0, j = 0;

    for (int k = 0; k < 64; k++) {
        v[k] = block[i][j];

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


BlocksZigZag process_channels(YCbCrPixel *pixels_YCrCb, int width, int height, double Ct[BLOCK_SIZE][BLOCK_SIZE]) {
    int blocos_x = width / BLOCK_SIZE;
    int blocos_y = height / BLOCK_SIZE;
    int num_blocks = blocos_x * blocos_y;

    // Aloca vetores para cada canal
    int **Y_blocks  = malloc(sizeof(int*) * num_blocks);
    int **Cb_blocks = malloc(sizeof(int*) * num_blocks);
    int **Cr_blocks = malloc(sizeof(int*) * num_blocks);

    int block_idx = 0;

    for (int j = 0; j < height; j += BLOCK_SIZE) {
        for (int i = 0; i < width; i += BLOCK_SIZE) {
            double block_Y[BLOCK_SIZE][BLOCK_SIZE];
            double block_Cb[BLOCK_SIZE][BLOCK_SIZE];
            double block_Cr[BLOCK_SIZE][BLOCK_SIZE];

            double dct_Y[BLOCK_SIZE][BLOCK_SIZE];
            double dct_Cb[BLOCK_SIZE][BLOCK_SIZE];
            double dct_Cr[BLOCK_SIZE][BLOCK_SIZE];

            int output_Y[BLOCK_SIZE][BLOCK_SIZE];
            int output_Cb[BLOCK_SIZE][BLOCK_SIZE];
            int output_Cr[BLOCK_SIZE][BLOCK_SIZE];

            for (int y = 0; y < BLOCK_SIZE; y++) {
                for (int x = 0; x < BLOCK_SIZE; x++) {
                    int dy = j + y;
                    int dx = i + x;
                    block_Y[x][y] = pixels_YCrCb[dy * width + dx].Y - 128.0;
                    block_Cb[x][y] = pixels_YCrCb[dy * width + dx].Cb - 128.0;
                    block_Cr[x][y] = pixels_YCrCb[dy * width + dx].Cr - 128.0;
                }
            }

            aplicar_dct_matricial(block_Y, dct_Y, Ct);
            aplicar_dct_matricial(block_Cb, dct_Cb, Ct);
            aplicar_dct_matricial(block_Cr, dct_Cr, Ct);

            quantizar(dct_Y, lumin_matrix, output_Y);
            quantizar(dct_Cb, crom_matrix, output_Cb);
            quantizar(dct_Cr, crom_matrix, output_Cr);

            // for (int k = 0; k < BLOCK_SIZE; k ++) {
            //     for (int l = 0; l < BLOCK_SIZE; l ++) {
            //         printf("%d ", output_Y[k][l]);
            //     }
            //     printf("\n");
            // }
            // printf("\n");

            // Aloca vetores zig-zag
            Y_blocks[block_idx]  = malloc(sizeof(int) * 64);
            Cb_blocks[block_idx] = malloc(sizeof(int) * 64);
            Cr_blocks[block_idx] = malloc(sizeof(int) * 64);

            aplicar_zigzag(output_Y, Y_blocks[block_idx]);
            aplicar_zigzag(output_Cb, Cb_blocks[block_idx]);
            aplicar_zigzag(output_Cr, Cr_blocks[block_idx]);

            // for (int k = 0; k < 64; k++) {
            //     printf("%d ", Y_blocks[block_idx][k]);
            // }
            // printf("\n\n");

            block_idx++;
        }
    }

    BlocksZigZag result = {Y_blocks, Cb_blocks, Cr_blocks, num_blocks};

    // for (int n = 0; n < num_blocks; n++) {
    //     for (int i = 0; i < 64; i++) {
    //         printf("%d ", result.Y_blocks[n][i]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    return result;
}

void free_BlocosZigZag(BlocksZigZag blocks) {
    for (int i = 0; i < blocks.num_blocks; i++) {
        free(blocks.Y_blocks[i]);
        free(blocks.Cb_blocks[i]);
        free(blocks.Cr_blocks[i]);
    }

    free(blocks.Y_blocks);
    free(blocks.Cb_blocks);
    free(blocks.Cr_blocks);
}

void delta_encoding_DC(BlocksZigZag *blocks) {
    if (blocks->num_blocks == 0) return;

    // for (int n = 0; n < blocks->num_blocks; n++) {
    //         printf("%d ", blocks->Y_blocks[n][0]);
    // }
    // printf("\n\n");

    for (int i = blocks->num_blocks - 1; i > 0; i--) {
        blocks->Y_blocks[i][0] -= blocks->Y_blocks[i - 1][0];
        blocks->Cb_blocks[i][0] -= blocks->Cb_blocks[i - 1][0];
        blocks->Cr_blocks[i][0] -= blocks->Cr_blocks[i - 1][0];
    }

    // for (int n = 0; n < blocks->num_blocks; n++) {
    //         printf("%d ", blocks->Y_blocks[n][0]);
    // }
    // printf("\n");
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

RLE_coef* RLE_encode_AC(int *coef, int *out_size) {
    RLE_coef *encoded = malloc(sizeof(RLE_coef) * 64);
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
                //printf("count 1 %d\n", count);
                count++;
                if (i != 63) skip = 0;
            }
        } else {
            cat = coef_category(val);
            encoded[count] = (RLE_coef){skip, cat, val};
            //printf("count 2 %d\n", count);
            count++;
            if (i != 63) skip = 0;
        }
    }
    //printf("skip %d\n", skip);
    // Se o final do vetor são apenas zeros, adiciona EOB
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
    //printf("out %d\n", *out_size);
    return encoded;
}

RLE_coef **process_AC_coef(int **blocks, int num_blocks, int *sizes) {
    RLE_coef **rle_results = malloc(num_blocks * sizeof(RLE_coef *));
    for (int i = 0; i < num_blocks; i++) {
        rle_results[i] = RLE_encode_AC(blocks[i], &sizes[i]);
    }
    return rle_results;
}

RLE process_zigzag_vectors(BlocksZigZag *blocks) {
    RLE result;
    result.Y_sizes = malloc(sizeof(int) * blocks->num_blocks);
    result.Cb_sizes = malloc(sizeof(int) * blocks->num_blocks);
    result.Cr_sizes = malloc(sizeof(int) * blocks->num_blocks);

    result.Y_rle = process_AC_coef(blocks->Y_blocks, blocks->num_blocks, result.Y_sizes);
    result.Cb_rle = process_AC_coef(blocks->Cb_blocks, blocks->num_blocks, result.Cb_sizes);
    result.Cr_rle = process_AC_coef(blocks->Cr_blocks, blocks->num_blocks, result.Cr_sizes);

    return result;
}

void init_bitwriter(BitWriter *bw, FILE *fp) {
    bw->file = fp;
    bw->buffer = 0;
    bw->bit_count = 0;
}

void write_bit(BitWriter *bw, int bit) {
    bw->buffer <<= 1;
    if (bit) bw->buffer |= 1;
    bw->bit_count++;

    //printf("buffer = %d, count = %d\n", bw->buffer, bw->bit_count);

    if (bw->bit_count == 8) {
        //printf("arquivo: %d\n", bw->buffer);
        fputc(bw->buffer, bw->file);
        bw->bit_count = 0;
        bw->buffer = 0;
    }
}

void write_bits(BitWriter *bw, const char *bits) {
    while (*bits) {
        //printf("%s - ", bits);
        write_bit(bw, *bits == '1');
        bits++;
    }
    //printf("\n");
}

void write_n_bits(BitWriter *bw, int value, int n) {
    for (int i = n - 1; i >= 0; i--) {
        //printf("val :%d - val >> i :%d - ", value, value >> i);
        write_bit(bw, (value >> i) & 1);
    }
}

void flush_bits(BitWriter *bw) {
    if (bw->bit_count > 0) {
        bw->buffer <<= (8 - bw->bit_count);
        fputc(bw->buffer, bw->file);
    }
}

void write_bits_complement1(BitWriter *bw, int value, int n_bits) {
    int mask = (1 << n_bits) - 1;
    int absval = abs(value);
    //printf("%d , %d = ", value, absval);
    if (value >= 0) {
        //printf("%d\n", absval);
        write_n_bits(bw, absval & mask, n_bits);
    } else {
        int inverted = (~absval) & mask;
        //printf("%d\n", inverted);
        write_n_bits(bw, inverted, n_bits);
    }
}

void write_channel_blocks(BitWriter *bw, int *sizes, RLE_coef **rle, int num_blocks) {
    for (int i = 0; i < num_blocks; i++) {
        int dc_value = rle[i][0].value;
        int dc_category = rle[i][0].category;
        const char *dc_prefix = dc_table[dc_category].prefix;
        //printf("%d/%s/%d\n", dc_category, dc_prefix, dc_value);

        // DC: prefix + valor em bits
        write_bits(bw, dc_prefix);
        if (dc_category > 0) {
            write_bits_complement1(bw, dc_value, dc_category);
        }

        // AC: processar todos os coeficientes RLE
        for (int j = 1; j < sizes[i]; j++) {
            RLE_coef coef = rle[i][j];
            //printf("%d/%d/%d\n", coef.skip, coef.category, coef.value);
            if (coef.skip == 0 && coef.category == 0) {
                write_bits(bw, "1010"); // EOB
                break;
            }
            
            const char *ac_prefix = NULL;

            // Skip (run of zeros) + categoria → prefix
            for (int k = 0; k < 162; k++) {
                if (ac_table[k].zeros == coef.skip && ac_table[k].category == coef.category) {
                    ac_prefix = ac_table[k].prefix;
                }
            }            

            if (ac_prefix == NULL) {
                printf("Prefixo Huffman não encontrado");
                return;
            }

            //printf("%d/%s/%d/%d\n", coef.skip, ac_prefix, coef.category, coef.value);
            write_bits(bw, ac_prefix);

            // Valor: mesma lógica do DC
            write_bits_complement1(bw, coef.value, coef.category);
        }
    }
}
