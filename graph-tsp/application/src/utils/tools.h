/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#pragma once

#include "settings.h"

void Memcpy(void *const dst, size_t dstSize, const void *src, size_t srcSize);
char *Strdup(const char *src);

INLINE int Int_min(int a, int b) { return a < b ? a : b; }
INLINE int Int_max(int a, int b) { return a > b ? a : b; }

typedef struct Vec2d
{
    double x;
    double y;
} Vec2d;
