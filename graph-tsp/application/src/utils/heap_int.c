#include "heap_int.h"

HeapInt* heapCreate()
{
    HeapInt* heap = calloc(1, sizeof(HeapInt));

    heap->capacity = 8;
    heap->pairs = malloc(heap->capacity * sizeof(HeapIntPair));

    return heap;
}

void heapDestroy(HeapInt* heap)
{
    free(heap->pairs);
    free(heap);
}

void heapPush(HeapInt* heap, int value, float priority)
{
    HeapIntPair node = { priority, value };

    if (heap->size == heap->capacity)
    {
        heap->capacity *= 2;
        heap->pairs = realloc(heap->pairs, heap->capacity * sizeof(HeapIntPair));
    }
    int i = heap->size++;

    while (i > 0)
    {
        int parent = (i - 1) / 2;

        if (heap->pairs[parent].priority <= node.priority) break;

        heap->pairs[i] = heap->pairs[parent];
        i = parent;
    }
    heap->pairs[i] = node;
    return;
}

int heapPop(HeapInt* heap)
{
    if (heap->size == 0) return -1;

    HeapIntPair root = heap->pairs[0];
    HeapIntPair last = heap->pairs[heap->size - 1];

    heap->size--;
    int i = 0;

    while (i < heap->size)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left >= heap->size) break;

        int schild = left;

        if (right < heap->size && heap->pairs[left].priority > heap->pairs[right].priority) schild = right;

        if (heap->pairs[schild].priority >= last.priority) break;

        heap->pairs[i] = heap->pairs[schild];
        i = schild;
    }
    heap->pairs[i] = last;
    return root.value;
}
