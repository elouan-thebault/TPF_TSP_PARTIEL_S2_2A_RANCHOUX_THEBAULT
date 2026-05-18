/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/
#ifndef NO_LIST_INT

#include "list_int.h"

ListInt* ListInt_create()
{
    ListInt* self = (ListInt*)calloc(1, sizeof(ListInt));
    AssertNew(self);

    ListIntNode* sentinel = &(self->sentinel);

    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    sentinel->value = 0;

    self->nodeCount = 0;
    return self;
}

void ListInt_destroy(ListInt* self)
{
    if (self == NULL) return;

    ListIntNode* sentinel = &(self->sentinel);
    ListIntNode* curr = sentinel->next;
    while (curr != sentinel)
    {
        ListIntNode* next = curr->next;
        free(curr);
        curr = next;
    }
    free(self);
}

int ListInt_size(ListInt* self)
{
    assert(self && "self must not be NULL");
    return self->nodeCount;
}

bool ListInt_isEmpty(ListInt* self)
{
    assert(self && "self must not be NULL");
    return (self->nodeCount <= 0);
}

void ListInt_print(ListInt* self)
{
    assert(self && "self must not be NULL");

    ListIntNode* sentinel = &(self->sentinel);
    ListIntNode* curr = sentinel->next;
    printf("[");
    while (curr != sentinel->prev)
    {
        printf("%d,", curr->value);
        curr = curr->next;
    }
    if (curr != sentinel)
    {
        printf("%d", curr->value);
    }
    printf("]\n");
}

void ListInt_popNode(ListInt* self, ListIntNode* node)
{
    assert(self && "self must not be NULL");
    assert(self->nodeCount > 0);
    assert(node != &(self->sentinel));

    ListIntNode* prev = node->prev;
    ListIntNode* next = node->next;
    prev->next = next;
    next->prev = prev;
    node->prev = node;
    node->next = node;

    self->nodeCount--;
}

void ListInt_insertNodeAfter(ListInt* self, ListIntNode* ref, ListIntNode* node)
{
    assert(self && "self must not be NULL");
    assert(ref && node);

    ListIntNode* next = ref->next;
    node->prev = ref;
    node->next = next;
    ref->next = node;
    next->prev = node;

    self->nodeCount++;
}

int ListInt_getFirst(ListInt* self)
{
    assert(self && "self must not be NULL");
    return self->sentinel.next->value;
}

int ListInt_getLast(ListInt* self)
{
    assert(self && "self must not be NULL");
    return self->sentinel.prev->value;
}

void ListInt_insertFirst(ListInt* self, int value)
{
    assert(self && "self must not be NULL");
    ListIntNode* node = (ListIntNode*)calloc(1, sizeof(ListIntNode));
    AssertNew(node);
    node->value = value;
    ListInt_insertNodeAfter(self, &(self->sentinel), node);
}

void ListInt_insertLast(ListInt* self, int value)
{
    assert(self && "self must not be NULL");
    ListIntNode* node = (ListIntNode*)calloc(1, sizeof(ListIntNode));
    AssertNew(node);
    node->value = value;
    ListInt_insertNodeAfter(self, self->sentinel.prev, node);
}

int ListInt_popFirst(ListInt* self)
{
    assert(self && "self must not be NULL");
    if (self->nodeCount <= 0)
    {
        assert(false);
        return 0;
    }
    ListIntNode* node = self->sentinel.next;
    int value = node->value;
    ListInt_popNode(self, node);
    free(node);
    return value;
}

int ListInt_popLast(ListInt* self)
{
    assert(self && "self must not be NULL");
    if (self->nodeCount <= 0)
    {
        assert(false);
        return 0;
    }
    ListIntNode* node = self->sentinel.prev;
    int value = node->value;
    ListInt_popNode(self, node);
    free(node);
    return value;
}

int ListInt_isIn(ListInt* self, int element)
{
    assert(self && "self must not be NULL");
    ListIntNode* sentinel = &(self->sentinel);
    ListIntNode* node = sentinel->next;
    while (node != sentinel)
    {
        if (node->value == element) return true;
        node = node->next;
    }
    return false;
}

ListInt* ListInt_copy(ListInt* self)
{
    assert(self && "self must not be NULL");
    ListInt* newList = ListInt_create();

    ListIntNode* sentinel = &(self->sentinel);
    ListIntNode* node = sentinel->next;
    while (node != sentinel)
    {
        ListInt_insertLast(newList, node->value);
        node = node->next;
    }

    return newList;
}

void ListInt_concatenate(ListInt* list1, ListInt* list2)
{
    assert(list1 && "list1 must not be NULL");
    assert(list2 && "list2 must not be NULL");

    if (list2->nodeCount <= 0)
    {
        return;
    }

    list1->nodeCount += list2->nodeCount;
    list2->nodeCount = 0;

    ListIntNode* last1 = list1->sentinel.prev;
    ListIntNode* last2 = list2->sentinel.prev;
    ListIntNode* first2 = list2->sentinel.next;
    ListIntNode* sentinel1 = &(list1->sentinel);
    ListIntNode* sentinel2 = &(list2->sentinel);

    last1->next = first2;
    first2->prev = last1;

    last2->next = sentinel1;
    sentinel1->prev = last2;

    sentinel2->next = sentinel2;
    sentinel2->prev = sentinel2;
}


ListIntIter* ListIntIter_create(ListInt* list)
{
    assert(list && "list must not be NULL");

    ListIntIter* self = (ListIntIter*)calloc(1, sizeof(ListIntIter));
    AssertNew(self);

    self->sentinel = &(list->sentinel);
    self->current = self->sentinel->next;

    return self;
}

void ListIntIter_destroy(ListIntIter* self)
{
    if (!self) return;
    free(self);
}

int ListIntIter_get(ListIntIter* self)
{
    assert(self && "self must not be NULL");
    assert(self->current != self->sentinel && "The ListIntIter is invalid");
    return self->current->value;
}

void ListIntIter_next(ListIntIter* self)
{
    assert(self && "self must not be NULL");
    assert(self->current != self->sentinel && "The ListIntIter is invalid");
    self->current = self->current->next;
}

bool ListIntIter_isValid(ListIntIter* self)
{
    assert(self && "self must not be NULL");
    return (self->current != self->sentinel);
}

#endif
