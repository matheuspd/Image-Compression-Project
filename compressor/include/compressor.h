#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "types.h"

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
int compress_jpeg(const char *input_bmp, const char *output_jpeg);

#endif /* COMPRESSOR_H */
