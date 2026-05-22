

#include "tsp_opt.h"
#include <float.h>

static int path_to_array(Path* path, int* tour)
{
    int idx = 0;
    ListIntIter* it = ListIntIter_create(path->list);
    while (ListIntIter_isValid(it))
    {
        tour[idx++] = ListIntIter_get(it);
        ListIntIter_next(it);
    }
    ListIntIter_destroy(it);
    return idx - 1;
}


static void array_to_path(Graph* distances, Path* path, int* tour, int n)
{
    while (!ListInt_isEmpty(path->list))
        ListInt_popFirst(path->list);

    for (int i = 0; i < n; i++)
        ListInt_insertLast(path->list, tour[i]);
    ListInt_insertLast(path->list, tour[0]);

    float dist = 0.0f;
    for (int i = 0; i < n; i++)
    {
        int u = tour[i];
        int v = tour[(i + 1) % n];
        float* w = Graph_getArc(distances, u, v);
        dist += w ? *w : 0.0f;
    }
    path->distance = dist;
}

static inline float arc_w(Graph* distances, int u, int v)
{
    float* w = Graph_getArc(distances, u, v);
    return w ? *w : 0.0f;
}

static void reverse_segment(int* tour, int i, int j, int n)
{
    int left = (i + 1) % n;
    int right = j;
    int len = (right - left + n) % n + 1;
    for (int k = 0; k < len / 2; k++)
    {
        int a = (left + k) % n;
        int b = (right - k + n) % n;
        int tmp = tour[a];
        tour[a] = tour[b];
        tour[b] = tmp;
    }
}


