/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#pragma once

#include "settings.h"

typedef struct HeapIntPair
{
    float priority;
    int value;
} HeapIntPair;

typedef struct HeapInt
{
    HeapIntPair* pairs;
    int size;
    int capacity;
} HeapInt;

