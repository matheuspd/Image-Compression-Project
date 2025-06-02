#ifndef BIT_FUNCTIONS_H
#define BIT_FUNCTIONS_H

#include "types.h"

/**
 * @brief Initializes a Bit_Read_Write structure for writing bits to a file.
 *
 * Sets the internal buffer and bit counter to 0 and associates the bit writer with a file.
 *
 * @param bw Pointer to the Bit_Read_Write structure to initialize.
 * @param fp FILE pointer to the output binary file.
 */
void init_bitwriter(Bit_Read_Write *bw, FILE *fp);

/**
 * @brief Writes a single bit to the bitstream.
 *
 * Buffers bits until a full byte is accumulated, then writes the byte to the file.
 *
 * @param bw Pointer to the Bit_Read_Write structure.
 * @param bit The bit to write (0 or 1).
 */
void write_bit(Bit_Read_Write *bw, int bit);

/**
 * @brief Writes a string of bits to the bitstream.
 *
 * Each character in the string is interpreted as a binary digit and written as a bit.
 *
 * @param bw Pointer to the Bit_Read_Write structure.
 * @param bits Null-terminated string containing the bits to write.
 */
void write_bits(Bit_Read_Write *bw, const char *bits);

/**
 * @brief Writes the 'n' bits of a value to the bitstream, MSB first.
 *
 * Useful for writing binary representations of Huffman values.
 *
 * @param bw Pointer to the Bit_Read_Write structure.
 * @param value The value to write.
 * @param n Number of bits to write.
 */
void write_n_bits(Bit_Read_Write *bw, int value, int n);

/**
 * @brief Flushes the remaining bits in the buffer by padding with zeros and writing to file.
 *
 * This must be called at the end of writing to ensure all bits are flushed to the file.
 *
 * @param bw Pointer to the Bit_Read_Write structure.
 */
void flush_bits(Bit_Read_Write *bw);

/**
 * @brief Writes a signed value in "complement-1" binary format used in JPEG.
 *
 * Positive values are written directly. Negative values are written by inverting the bits
 * of their absolute value.
 *
 * @param bw Pointer to the Bit_Read_Write structure.
 * @param value The signed value to write.
 * @param n_bits The number of bits to write.
 */
void write_bits_complement1(Bit_Read_Write *bw, int value, int n_bits);

/**
 * @brief Initializes a Bit_Read_Write structure for reading bits from a file.
 *
 * Sets the internal buffer and bit counter to zero and associates the structure with the given file.
 *
 * @param br Pointer to the Bit_Read_Write structure to initialize.
 * @param fp FILE pointer to the input binary file.
 */
void init_bitreader(Bit_Read_Write *br, FILE *fp);

/**
 * @brief Reads a single bit from the bitstream.
 *
 * If the internal buffer is empty, reads the next byte from the file and returns its most significant bit.
 *
 * @param br Pointer to the Bit_Read_Write structure.
 * @return The bit read (0 or 1), or -1 on EOF or error.
 */
int read_bit(Bit_Read_Write *br);

/**
 * @brief Reads the next 'n' bits from the bitstream and returns them as an integer.
 *
 * Bits are read in MSB-first order. If the end of file is reached before reading all bits,
 * the function returns -1.
 *
 * @param br Pointer to the Bit_Read_Write structure.
 * @param n Number of bits to read.
 * @return The read integer value (non-negative), or -1 on EOF or error.
 */
int read_n_bits(Bit_Read_Write *br, int n);

/**
 * @brief Reads a signed integer encoded in "complement-1" format with 'n_bits'.
 *
 * Positive numbers are written directly and
 * negative numbers are represented by inverting the bits of their absolute value.
 *
 * @param br Pointer to the Bit_Read_Write structure.
 * @param n_bits Number of bits to read.
 * @return The decoded signed integer, or 0 on error.
 */
int read_bits_complement1(Bit_Read_Write *br, int n_bits);

#endif /* BIT_FUNCTIONS_H */