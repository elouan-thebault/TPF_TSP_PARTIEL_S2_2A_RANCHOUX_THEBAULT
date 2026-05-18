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

HeapInt* heapCreate();
void heapDestroy(HeapInt* heap);
void heapPush(HeapInt* heap, int value, float priority);
int heapPop(HeapInt* heap);
