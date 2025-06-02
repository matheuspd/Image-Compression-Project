#include "decompressor.h"

/**
 * Decompression Process:
 * 1) Read compressed file (header + [Huffman code + value])
 * 2) Redo the RLE encoding structure with the DC and AC coefficients
 * 3) Redo the zigzag blocks structure
 * 4) Undo the delta encoding on DC coefficients
 * 5) Undo the zigzag algorithm
 * 6) Dequantize
 * 7) IDCT using pre-calculated matrices
 * 8) YCbCr to RGB
 * 9) Write BMP decompressed file (with losses)
 */
int decompress_bin(const char *input_bin, const char *output_bmp) {
    FILE *file = fopen(input_bin, "rb");
    if (!file) {
        printf("Error opening BIN file.\n");
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

    int num_blocks = total_pixels / (BLOCK_SIZE * BLOCK_SIZE);

    // Redo the RLE structure
    RLE *rle_blocks = read_all_blocks(file, num_blocks);
    if(!rle_blocks) return FAILURE;

    // Redo the ZigZag blocks structure
    Blocks_ZigZag *blocks = rle_to_blocks(rle_blocks, num_blocks);

    // Undo delta encoding on DC values
    delta_decoding(blocks);

    double Ct[BLOCK_SIZE][BLOCK_SIZE];
    transpose((double (*)[BLOCK_SIZE])C, Ct); // Ct = C^T

    YCbCr_Pixel *pixels_YCrCb = blocks_to_pixels(blocks, width, height, total_pixels, Ct);
    if(!pixels_YCrCb) return FAILURE;
    
    RGB_Pixel *pixels = YCbCr_to_rgb(pixels_YCrCb, total_pixels);
    if(!pixels) return FAILURE;

    FILE *dst = fopen(output_bmp, "wb");
    if (!dst) {
        printf("Error creating output file.\n");
        free(pixels);
        return FAILURE;
    }
    
    if (write_bmp(dst, &fileHeader, &infoHeader, pixels) != SUCCESS) {
        printf("Error writing BMP file.\n");
        fclose(dst);
        free(pixels);
        return FAILURE;
    }

    printf("Decompression Successful.\n");

    return SUCCESS;
}

RLE *read_all_blocks(FILE *file, int num_blocks) {
    Bit_Read_Write br;
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

Blocks_ZigZag *rle_to_blocks(RLE *rle_blocks, int num_blocks) {
    Blocks_ZigZag *blocks = malloc(sizeof(Blocks_ZigZag));
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

YCbCr_Pixel *blocks_to_pixels(Blocks_ZigZag *blocks, int width, int height, int total_pixels, double Ct[BLOCK_SIZE][BLOCK_SIZE]) {

    YCbCr_Pixel *values = malloc(sizeof(YCbCr_Pixel) * total_pixels);
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

            zigzag(temp_Y , blocks->Y_blocks[idx], 1);
            zigzag(temp_Cb, blocks->Cb_blocks[idx], 1);
            zigzag(temp_Cr, blocks->Cr_blocks[idx], 1);

            idx++;


            dequantize(temp_Y, lumin_matrix, dct_Y);
            dequantize(temp_Cb, chrom_matrix, dct_Cb);
            dequantize(temp_Cr, chrom_matrix, dct_Cr);


            apply_matrix_idct(dct_Y, original_Y, Ct);
            apply_matrix_idct(dct_Cb, original_Cb, Ct);
            apply_matrix_idct(dct_Cr, original_Cr, Ct);

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

    return values;
}
