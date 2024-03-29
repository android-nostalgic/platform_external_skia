/*
 * Copyright 2007, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *     http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */

#include "SkScaledBitmapSampler.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkDither.h"

// 8888

static bool Sample_Gray_D8888(void* SK_RESTRICT dstRow,
                              const uint8_t* SK_RESTRICT src,
                              int width, int deltaSrc, int) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB32(0xFF, src[0], src[0], src[0]);
        src += deltaSrc;
    }
    return false;
}

static bool Sample_RGBx_D8888(void* SK_RESTRICT dstRow,
                              const uint8_t* SK_RESTRICT src,
                              int width, int deltaSrc, int) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB32(0xFF, src[0], src[1], src[2]);
        src += deltaSrc;
    }
    return false;
}

static bool Sample_RGBA_D8888(void* SK_RESTRICT dstRow,
                              const uint8_t* SK_RESTRICT src,
                              int width, int deltaSrc, int) {
    SkPMColor* SK_RESTRICT dst = (SkPMColor*)dstRow;
    unsigned alphaMask = 0xFF;
    for (int x = 0; x < width; x++) {
        unsigned alpha = src[3];
        dst[x] = SkPreMultiplyARGB(alpha, src[0], src[1], src[2]);
        src += deltaSrc;
        alphaMask &= alpha;
    }
    return alphaMask != 0xFF;
}

// 565

static bool Sample_Gray_D565(void* SK_RESTRICT dstRow,
                             const uint8_t* SK_RESTRICT src,
                             int width, int deltaSrc, int) {
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPack888ToRGB16(src[0], src[0], src[0]);
        src += deltaSrc;
    }
    return false;
}

static bool Sample_Gray_D565_D(void* SK_RESTRICT dstRow,
                               const uint8_t* SK_RESTRICT src,
                               int width, int deltaSrc, int y) {
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    DITHER_565_SCAN(y);
    for (int x = 0; x < width; x++) {
        dst[x] = SkDitherRGBTo565(src[0], src[0], src[0], DITHER_VALUE(x));
        src += deltaSrc;
    }
    return false;
}

static bool Sample_RGBx_D565(void* SK_RESTRICT dstRow,
                             const uint8_t* SK_RESTRICT src,
                             int width, int deltaSrc, int) {
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPack888ToRGB16(src[0], src[1], src[2]);
        src += deltaSrc;
    }
    return false;
}

static bool Sample_RGBx_D565_D(void* SK_RESTRICT dstRow,
                               const uint8_t* SK_RESTRICT src,
                               int width, int deltaSrc, int y) {
    uint16_t* SK_RESTRICT dst = (uint16_t*)dstRow;
    DITHER_565_SCAN(y);
    for (int x = 0; x < width; x++) {
        dst[x] = SkDitherRGBTo565(src[0], src[1], src[2], DITHER_VALUE(x));
        src += deltaSrc;
    }
    return false;
}

// 4444

static bool Sample_Gray_D4444(void* SK_RESTRICT dstRow,
                              const uint8_t* SK_RESTRICT src,
                              int width, int deltaSrc, int) {
    SkPMColor16* SK_RESTRICT dst = (SkPMColor16*)dstRow;
    for (int x = 0; x < width; x++) {
        unsigned gray = src[0] >> 4;
        dst[x] = SkPackARGB4444(0xF, gray, gray, gray);
        src += deltaSrc;
    }
    return false;
}

static bool Sample_Gray_D4444_D(void* SK_RESTRICT dstRow,
                                const uint8_t* SK_RESTRICT src,
                                int width, int deltaSrc, int y) {
    SkPMColor16* SK_RESTRICT dst = (SkPMColor16*)dstRow;
    DITHER_4444_SCAN(y);
    for (int x = 0; x < width; x++) {
        dst[x] = SkDitherARGB32To4444(0xFF, src[0], src[0], src[0],
                                      DITHER_VALUE(x));
        src += deltaSrc;
    }
    return false;
}

static bool Sample_RGBx_D4444(void* SK_RESTRICT dstRow,
                              const uint8_t* SK_RESTRICT src,
                              int width, int deltaSrc, int) {
    SkPMColor16* SK_RESTRICT dst = (SkPMColor16*)dstRow;
    for (int x = 0; x < width; x++) {
        dst[x] = SkPackARGB4444(0xF, src[0] >> 4, src[1] >> 4, src[2] >> 4);
        src += deltaSrc;
    }
    return false;
}

static bool Sample_RGBx_D4444_D(void* SK_RESTRICT dstRow,
                                const uint8_t* SK_RESTRICT src,
                                int width, int deltaSrc, int y) {
    SkPMColor16* dst = (SkPMColor16*)dstRow;
    DITHER_4444_SCAN(y);

    for (int x = 0; x < width; x++) {
        dst[x] = SkDitherARGB32To4444(0xFF, src[0], src[1], src[2],
                                      DITHER_VALUE(x));
        src += deltaSrc;
    }
    return false;
}

static bool Sample_RGBA_D4444(void* SK_RESTRICT dstRow,
                              const uint8_t* SK_RESTRICT src,
                              int width, int deltaSrc, int) {
    SkPMColor16* SK_RESTRICT dst = (SkPMColor16*)dstRow;
    unsigned alphaMask = 0xFF;

    for (int x = 0; x < width; x++) {
        unsigned alpha = src[3];
        SkPMColor c = SkPreMultiplyARGB(alpha, src[0], src[1], src[2]);
        dst[x] = SkPixel32ToPixel4444(c);
        src += deltaSrc;
        alphaMask &= alpha;
    }
    return alphaMask != 0xFF;
}

static bool Sample_RGBA_D4444_D(void* SK_RESTRICT dstRow,
                                const uint8_t* SK_RESTRICT src,
                                int width, int deltaSrc, int y) {
    SkPMColor16* SK_RESTRICT dst = (SkPMColor16*)dstRow;
    unsigned alphaMask = 0xFF;
    DITHER_4444_SCAN(y);

    for (int x = 0; x < width; x++) {
        unsigned alpha = src[3];
        SkPMColor c = SkPreMultiplyARGB(alpha, src[0], src[1], src[2]);
        dst[x] = SkDitherARGB32To4444(c, DITHER_VALUE(x));
        src += deltaSrc;
        alphaMask &= alpha;
    }
    return alphaMask != 0xFF;
}

// Index

static bool Sample_Index_DI(void* SK_RESTRICT dstRow,
                            const uint8_t* SK_RESTRICT src,
                            int width, int deltaSrc, int) {
    if (1 == deltaSrc) {
        memcpy(dstRow, src, width);
    } else {
        uint8_t* SK_RESTRICT dst = (uint8_t*)dstRow;
        for (int x = 0; x < width; x++) {
            dst[x] = src[0];
            src += deltaSrc;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkScaledBitmapSampler.h"

SkScaledBitmapSampler::SkScaledBitmapSampler(int width, int height,
                                             int sampleSize) {
    if (width <= 0 || height <= 0) {
        sk_throw();
    }
    
    if (sampleSize <= 1) {
        fScaledWidth = width;
        fScaledHeight = height;
        fX0 = fY0 = 0;
        fDX = fDY = 1;
        return;
    }
    
    int dx = SkMin32(sampleSize, width);
    int dy = SkMin32(sampleSize, height);
    
    fScaledWidth = width / dx;
    fScaledHeight = height / dy;
    
    SkASSERT(fScaledWidth > 0);
    SkASSERT(fScaledHeight > 0);
    
    fX0 = dx >> 1;
    fY0 = dy >> 1;
    
    SkASSERT(fX0 >= 0 && fX0 < width);
    SkASSERT(fY0 >= 0 && fY0 < height);
    
    fDX = dx;
    fDY = dy;
    
    SkASSERT(fDX > 0 && (fX0 + fDX * (fScaledWidth - 1)) < width);
    SkASSERT(fDY > 0 && (fY0 + fDY * (fScaledHeight - 1)) < height);
    
    fRowProc = NULL;
}

bool SkScaledBitmapSampler::begin(SkBitmap* dst, SrcConfig sc, bool dither) {
    static const RowProc gProcs[] = {
        // 8888 (no dither distinction)
        Sample_Gray_D8888,  Sample_Gray_D8888,
        Sample_RGBx_D8888,  Sample_RGBx_D8888,
        Sample_RGBA_D8888,  Sample_RGBA_D8888,
        NULL,               NULL,
        // 565 (no alpha distinction)
        Sample_Gray_D565,   Sample_Gray_D565_D,
        Sample_RGBx_D565,   Sample_RGBx_D565_D,
        Sample_RGBx_D565,   Sample_RGBx_D565_D,
        NULL,               NULL,
        // 4444
        Sample_Gray_D4444,  Sample_Gray_D4444_D,
        Sample_RGBx_D4444,  Sample_RGBx_D4444_D,
        Sample_RGBA_D4444,  Sample_RGBA_D4444_D,
        NULL,               NULL,
        // Index8
        NULL,               NULL,
        NULL,               NULL,
        NULL,               NULL,
        Sample_Index_DI,    Sample_Index_DI,
    };

    
    int index = 0;
    if (dither) {
        index += 1;
    }
    switch (sc) {
        case SkScaledBitmapSampler::kGray:
            fSrcPixelSize = 1;
            index += 0;
            break;
        case SkScaledBitmapSampler::kRGB:
            fSrcPixelSize = 3;
            index += 2;
            break;
        case SkScaledBitmapSampler::kRGBX:
            fSrcPixelSize = 4;
            index += 2;
            break;
        case SkScaledBitmapSampler::kRGBA:
            fSrcPixelSize = 4;
            index += 4;
            break;
        case SkScaledBitmapSampler::kIndex:
            fSrcPixelSize = 1;
            index += 6;
            break;
        default:
            return false;
    }

    switch (dst->config()) {
        case SkBitmap::kARGB_8888_Config:
            index += 0;
            break;
        case SkBitmap::kRGB_565_Config:
            index += 8;
            break;
        case SkBitmap::kARGB_4444_Config:
            index += 16;
            break;
        case SkBitmap::kIndex8_Config:
            index += 24;
            break;
        default:
            return false;
    }
    
    fRowProc = gProcs[index];
    fDstRow = (char*)dst->getPixels();
    fDstRowBytes = dst->rowBytes();
    fCurrY = 0;
    return fRowProc != NULL;
}

bool SkScaledBitmapSampler::next(const uint8_t* SK_RESTRICT src) {
    SkASSERT((unsigned)fCurrY < (unsigned)fScaledHeight);

    bool hadAlpha = fRowProc(fDstRow, src + fX0 * fSrcPixelSize, fScaledWidth,
                             fDX * fSrcPixelSize, fCurrY);
    fDstRow += fDstRowBytes;
    fCurrY += 1;
    return hadAlpha;
}
