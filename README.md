# Image Compression Project

This project aims to create an image compressor/decompressor in C based on the JPEG specification and following the lossy compression model for 24-bit BMP images.

## Author

#### Matheus Pereira Dias - NUSP: 11207752

## BMP Image Requirements

* The image width and height must be multiples of 8.
* The image dimensions must be within the allowed range (8x8 to 1280x800).
* The image must have 24 bits per pixel and no compression.

## Notes

* There are 2 separated programs, the `compressor` and the `decompressor`.
* Some data is lost during compression (lossly).
* Huffman tables, DCT and quantization matrices used are standard ones, provided in the code (`types.c`).

## Compression Process (compressor)

1. Color Space Conversion: Converts RGB to YCbCr color space.

2. Chroma Subsampling (4:2:0): Reduces resolution of the Cb and Cr channels by averaging 2×2 blocks.

3. Block Splitting: Each channel is split into 8×8 pixel blocks.

4. Discrete Cosine Transform (DCT): Transforms each block from spatial to frequency domain.

5. Quantization: Applies a quantization matrix to discard less important frequencies.

6. Zigzag Ordering: Rearranges coefficients from each block into a 1D array prioritizing low-frequency values.

7. Differential & Run-Length Encoding (RLE):
    - DC coefficients use differential encoding.
    - AC coefficients are compressed with RLE.

8. Huffman Encoding: Encodes the RLE symbols using provided Huffman tables.

9. Binary Writing: The compressed output is saved in a .bin file.

## Decompression Process (decompressor)

1. Binary Parsing and Huffman Decoding

2. RLE Expansion and Delta Decoding

3. Reverse Zigzag

4. Dequantization

5. Inverse DCT (IDCT)

6. Block Reconstruction

7. Color Space Conversion: Converts YCbCr back to RGB.

## Build

To compile both compressor and decompressor, run:

```
bash compile.sh
```

or use any equivalent command like `sh`.

To remove all compiled binaries and object files, run:

```
bash clean.sh
```

## Usage

### Compress BMP to binary:

```
./compressor <input.bmp> <output.bin>
```

After compression, the program will display the input and output file sizes and the compression ratio and create the `<output.bin>` file.

### Decompress binary to BMP:

```
./decompressor <input.bin> <output.bmp>
```

After decompression, the program will create the `<output.bmp>` file that you can compare with the original image.