#include "jpeg.h"
#include <math.h>

/*
void rgb_para_ycbcr(RGBPixel *rgb, YCbCrPixel *ycbcr, int largura, int altura) {
    for (int i = 0; i < largura * altura; i++) {
        ycbcr[i].Y  = (uint8_t)(0.299 * rgb[i].r + 0.587 * rgb[i].g + 0.114 * rgb[i].b);
        ycbcr[i].Cb = (uint8_t)(128 - 0.168736 * rgb[i].r - 0.331264 * rgb[i].g + 0.5 * rgb[i].b);
        ycbcr[i].Cr = (uint8_t)(128 + 0.5 * rgb[i].r - 0.418688 * rgb[i].g - 0.081312 * rgb[i].b);
    }
}
*/