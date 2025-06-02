/**
 * =============== Autor ===============
 * Matheus Pereira Dias - NUSP: 11207752
 * =====================================
*/

#include "compressor.h"

/**
 * Compression Process:
 * 1) RGB to YCbCr
 * 2) Subsample 4:2:0
 * 3) DCT using pre-calculated matrices
 * 4) Quantization
 * 5) Apply zigzag algorithm
 * 6) Delta enconding on DC coefficients
 * 7) RLE encoding on AC coefficients
 * 8) Write compressed file (header + [Huffman code + value])
 */
int compress_bmp(const char *input_bmp, const char *output_bin) {

    FILE *file = fopen(input_bmp, "rb");
    if (!file) {
        printf("Error opening BMP file.\n");
        return FAILURE;
    }

    BMPFILEHEADER fileHeader;
    BMPINFOHEADER infoHeader;

    if (readHeader(file, &fileHeader) != SUCCESS) {
        printf("Error reading BMP header.\n");
        fclose(file);
        return FAILURE;
    }

    if (readInfoHeader(file, &infoHeader) != SUCCESS) {
        printf("Error reading BMP info header.\n");
        fclose(file);
        return FAILURE;
    }

    int width = infoHeader.biWidth;
    int height = infoHeader.biHeight;
    int total_pixels = width * height;

    RGB_Pixel *pixels = read_pixels(file, &fileHeader, width, height);
    if (!pixels) {
        printf("Error allocating RGB pixels.\n");
        fclose(file);
        return FAILURE;
    }

    long file_lenght_in = ftell(file);
    fclose(file);

    YCbCr_Pixel *pixels_YCrCb = rgb_to_YCrCb(pixels, total_pixels);
    if (!pixels_YCrCb) {
        printf("Error allocating YCbCr pixels.\n");
        free(pixels);
        return FAILURE;
    }

    // Considering width and height multiples of 2
    subsample_4_2_0(pixels_YCrCb, width, height);

    double Ct[BLOCK_SIZE][BLOCK_SIZE];
    transpose((double (*)[BLOCK_SIZE])C, Ct); // Ct = C^T

    // Create 3 matrices (Y, Cb, Cr) that hold the 'num_blocks' zigzag vectors (64 elements each = 8x8)
    Blocks_ZigZag *zigzag_vectors = process_channels(pixels_YCrCb, width, height, Ct);
    if (!zigzag_vectors) {
        printf("Error allocating zigzag vectors.\n");
        free(pixels_YCrCb);
        free(pixels);
        return FAILURE;
    }

    int num_blocks = zigzag_vectors->num_blocks;

    delta_encoding_DC(zigzag_vectors);

    RLE *rle_result = process_zigzag_vectors(zigzag_vectors);
    if (!rle_result) {
        printf("Error allocating rle blocks.\n");
        free_BlocosZigZag(zigzag_vectors); 
        free(pixels_YCrCb);
        free(pixels); 
        return FAILURE;
    }

    FILE *out = fopen(output_bin, "wb");
    if (!file) {
        printf("Error creating output file.\n");
        free_rle(rle_result, num_blocks);
        free_BlocosZigZag(zigzag_vectors); 
        free(pixels_YCrCb);
        free(pixels);
        return FAILURE;
    }

    Bit_Read_Write bw;
    init_bitwriter(&bw, out);

    // Write BMP headers
    fwrite(&fileHeader, sizeof(fileHeader), 1, out);
    fwrite(&infoHeader, sizeof(infoHeader), 1, out);

    int temp;

    // Write each channel
    temp = write_channel_blocks(&bw, rle_result->Y_sizes, rle_result->Y_rle, num_blocks);
    if (temp != SUCCESS) {
        printf("Error writing channel Y.\n");
        free_rle(rle_result, num_blocks);
        free_BlocosZigZag(zigzag_vectors); 
        free(pixels_YCrCb);
        free(pixels);
        fclose(out);
        return FAILURE;
    }
    
    temp = write_channel_blocks(&bw, rle_result->Cb_sizes, rle_result->Cb_rle, num_blocks);
    if (temp != SUCCESS) {
        printf("Error writing channel Cb.\n");
        free_rle(rle_result, num_blocks);
        free_BlocosZigZag(zigzag_vectors); 
        free(pixels_YCrCb);
        free(pixels);
        fclose(out);
        return FAILURE;
    }

    temp = write_channel_blocks(&bw, rle_result->Cr_sizes, rle_result->Cr_rle, num_blocks);
    if (temp != SUCCESS) {
        printf("Error writing channel Cr.\n");
        free_rle(rle_result, num_blocks);
        free_BlocosZigZag(zigzag_vectors); 
        free(pixels_YCrCb);
        free(pixels);
        fclose(out);
        return FAILURE;
    }

    flush_bits(&bw);
    long file_lenght_out = ftell(out);
    fclose(out);

    printf("Compression Successful.\n");
    
    printf("Input File Lenght: %ld bytes\n", file_lenght_in);
    printf("Output File Lenght: %ld bytes\n", file_lenght_out);

    printf("Compression Ratio = %.2f%%\n", 100.0 * (1.0 - ((float)file_lenght_out / file_lenght_in)));

    free_rle(rle_result, num_blocks);
    free_BlocosZigZag(zigzag_vectors); 
    free(pixels_YCrCb);
    free(pixels);

    return SUCCESS;
}

Blocks_ZigZag *process_channels(YCbCr_Pixel *pixels_YCrCb, int width, int height, double Ct[BLOCK_SIZE][BLOCK_SIZE]) {
    int blocos_x = width / BLOCK_SIZE;
    int blocos_y = height / BLOCK_SIZE;
    int num_blocks = blocos_x * blocos_y;

    Blocks_ZigZag *result = malloc(sizeof(Blocks_ZigZag));
    if (!result) return NULL;

    result->Y_blocks  = malloc(sizeof(int*) * num_blocks);
    if (!result->Y_blocks) {
        free(result);
        return NULL;
    }
    result->Cb_blocks = malloc(sizeof(int*) * num_blocks);
    if (!result->Cb_blocks) {
        free(result->Y_blocks);
        free(result);
        return NULL;
    }
    result->Cr_blocks = malloc(sizeof(int*) * num_blocks);
    if (!result->Cr_blocks) {
        free(result->Y_blocks);
        free(result->Cb_blocks);
        free(result);
        return NULL;
    }
    result->num_blocks = num_blocks;

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

            apply_matrix_dct(block_Y, dct_Y, Ct);
            apply_matrix_dct(block_Cb, dct_Cb, Ct);
            apply_matrix_dct(block_Cr, dct_Cr, Ct);

            quantize(dct_Y, lumin_matrix, output_Y);
            quantize(dct_Cb, chrom_matrix, output_Cb);
            quantize(dct_Cr, chrom_matrix, output_Cr);

            result->Y_blocks[block_idx] = malloc(sizeof(int) * 64);
            if (!result->Y_blocks[block_idx]) {
                for (int k = 0; k <= block_idx; k++) {
                    free(result->Y_blocks[k]);
                }
                free(result->Y_blocks);
                free(result->Cb_blocks);
                free(result->Cr_blocks);
                free(result);
                return NULL;
            }

            result->Cb_blocks[block_idx] = malloc(sizeof(int) * 64);
            if (!result->Y_blocks[block_idx]) {
                for (int k = 0; k <= block_idx; k++) {
                    free(result->Y_blocks[k]);
                    free(result->Cb_blocks[k]);
                }
                free(result->Y_blocks);
                free(result->Cb_blocks);
                free(result->Cr_blocks);
                free(result);
                return NULL;
            }
            
            result->Cr_blocks[block_idx] = malloc(sizeof(int) * 64);
            if (!result->Y_blocks[block_idx]) {
                for (int k = 0; k <= block_idx; k++) {
                    free(result->Y_blocks[k]);
                    free(result->Cb_blocks[k]);
                    free(result->Cr_blocks[k]);
                }
                free(result->Y_blocks);
                free(result->Cb_blocks);
                free(result->Cr_blocks);
                free(result);
                return NULL;
            }

            zigzag(output_Y,  result->Y_blocks[block_idx],  0);
            zigzag(output_Cb, result->Cb_blocks[block_idx], 0);
            zigzag(output_Cr, result->Cr_blocks[block_idx], 0);

            block_idx++;
        }
    }

    return result;
}

RLE *process_zigzag_vectors(Blocks_ZigZag *blocks) {
    RLE *result = malloc(sizeof(RLE));
    if (!result) return NULL;

    result->Y_sizes = malloc(sizeof(int) * blocks->num_blocks);
    if (!result->Y_sizes) {
        free(result);
        return NULL;
    }

    result->Cb_sizes = malloc(sizeof(int) * blocks->num_blocks);
    if (!result->Cb_sizes) {
        free(result->Y_sizes);
        free(result);
        return NULL;
    }

    result->Cr_sizes = malloc(sizeof(int) * blocks->num_blocks);
    if (!result->Cr_sizes) {
        free(result->Y_sizes);
        free(result->Cb_sizes);
        free(result);
        return NULL;
    }

    result->Y_rle = process_AC_coef(blocks->Y_blocks, blocks->num_blocks, result->Y_sizes);
    if (!result->Y_rle) {
        free(result->Y_sizes);
        free(result->Cb_sizes);
        free(result->Cr_sizes);
        free(result);
        return NULL;
    }

    result->Cb_rle = process_AC_coef(blocks->Cb_blocks, blocks->num_blocks, result->Cb_sizes);
    if (!result->Cb_rle) {
        for (int i = 0; i < blocks->num_blocks; i++) free(result->Y_rle[i]);
        free(result->Y_rle);
        free(result->Y_sizes);
        free(result->Cb_sizes);
        free(result->Cr_sizes);
        free(result);
        return NULL;
    }

    result->Cr_rle = process_AC_coef(blocks->Cr_blocks, blocks->num_blocks, result->Cr_sizes);
    if (!result->Cr_rle) {
        for (int i = 0; i < blocks->num_blocks; i++) {
            free(result->Y_rle[i]);
            free(result->Cb_rle[i]);
        }
        free(result->Y_rle);
        free(result->Cb_rle);
        free(result->Y_sizes);
        free(result->Cb_sizes);
        free(result->Cr_sizes);
        free(result);
        return NULL;
    }

    return result;
}

int write_channel_blocks(Bit_Read_Write *bw, int *sizes, RLE_coef **rle, int num_blocks) {
    for (int i = 0; i < num_blocks; i++) {
        int dc_value = rle[i][0].value;
        int dc_category = rle[i][0].category;

        if (dc_category < 0 || dc_category > 10) {
            printf("DC Huffman Prefix not found");
            return FAILURE;
        }

        const char *dc_prefix = dc_table[dc_category].prefix;

        // DC: prefix + value in bits
        write_bits(bw, dc_prefix);
        if (dc_category > 0) {
            write_bits_complement1(bw, dc_value, dc_category);
        }

        // AC
        for (int j = 1; j < sizes[i]; j++) {
            RLE_coef coef = rle[i][j];
            if (coef.skip == 0 && coef.category == 0) {
                write_bits(bw, "1010"); // EOB
                break;
            }
            
            const char *ac_prefix = NULL;

            // Skip + category => prefix
            for (int k = 0; k < 162; k++) {
                if (ac_table[k].zeros == coef.skip && ac_table[k].category == coef.category) {
                    ac_prefix = ac_table[k].prefix;
                }
            }            

            if (ac_prefix == NULL) {
                printf("AC Huffman Prefix not found");
                return FAILURE;
            }

            write_bits(bw, ac_prefix);
            write_bits_complement1(bw, coef.value, coef.category);
        }
    }

    return SUCCESS;
}
