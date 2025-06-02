#include "bit_functions.h"

void init_bitwriter(Bit_Read_Write *bw, FILE *fp) {
    bw->file = fp;
    bw->buffer = 0;
    bw->bit_count = 0;
}

void write_bit(Bit_Read_Write *bw, int bit) {
    bw->buffer <<= 1;
    if (bit) bw->buffer |= 1;
    bw->bit_count++;

    if (bw->bit_count == 8) {
        fputc(bw->buffer, bw->file);
        bw->bit_count = 0;
        bw->buffer = 0;
    }
}

void write_bits(Bit_Read_Write *bw, const char *bits) {
    while (*bits) {
        write_bit(bw, *bits == '1');
        bits++;
    }
}

void write_n_bits(Bit_Read_Write *bw, int value, int n) {
    for (int i = n - 1; i >= 0; i--) {
        write_bit(bw, (value >> i) & 1);
    }
}

void flush_bits(Bit_Read_Write *bw) {
    if (bw->bit_count > 0) {
        bw->buffer <<= (8 - bw->bit_count);
        fputc(bw->buffer, bw->file);
    }
}

void write_bits_complement1(Bit_Read_Write *bw, int value, int n_bits) {
    int mask = (1 << n_bits) - 1;
    int absval = abs(value);

    if (value >= 0) {
        write_n_bits(bw, absval & mask, n_bits);
    } else {
        int inverted = (~absval) & mask;
        write_n_bits(bw, inverted, n_bits);
    }
}

void init_bitreader(Bit_Read_Write *br, FILE *fp) {
    br->file = fp;
    br->buffer = 0;
    br->bit_count = 0;
}

int read_bit(Bit_Read_Write *br) {
    if (br->bit_count == 0) {
        int c = fgetc(br->file);
        if (c == EOF) return -1; // EOF
        br->buffer = c;
        br->bit_count = 8;
    }
    int bit = (br->buffer >> 7) & 1;
    br->buffer <<= 1;
    br->bit_count--;
    return bit;
}

int read_n_bits(Bit_Read_Write *br, int n) {
    int value = 0;
    for (int i = 0; i < n; i++) {
        int bit = read_bit(br);
        if (bit == -1) return -1; // EOF
        value = (value << 1) | bit;
    }
    return value;
}

int read_bits_complement1(Bit_Read_Write *br, int n_bits) {
    int positive = read_n_bits(br, n_bits);
    if (positive == -1) return 0; // Error

    int msb = positive >> (n_bits - 1);

    if (msb != 0) {
        return positive; // Positive value
    } else {
        int inverted = (~positive) & ((1 << n_bits) - 1);
        return -inverted; // Negative value in complement-1 format
    }
}