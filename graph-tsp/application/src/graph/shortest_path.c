/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "shortest_path.h"
#include "utils/heap_int.h"

Path* Graph_shortestPath(Graph* self, int start, int end)
{
    assert(self && "self must not be NULL");
    assert(0 <= start && start < self->vertexCount && "The start is invalid");
    assert(end < self->vertexCount && "The end is invalid");

    const int vertexCount = Graph_getVertexCount(self);

    int* predecessors = (int*)calloc(vertexCount, sizeof(int));
    AssertNew(predecessors);

    float* distances = (float*)calloc(vertexCount, sizeof(float));
    AssertNew(distances);

    Graph_dijkstra(self, start, end, predecessors, distances);

    Path* path = Graph_dijkstraGetPath(predecessors, distances, end);

    free(predecessors);
    predecessors = NULL;
    free(distances);
    distances = NULL;

    return path;
}

void Graph_dijkstra(Graph* self, int start, int end, int* predecessors, float* distances)
{
    assert(self && "self must not be NULL");
    assert(0 <= start && start < self->vertexCount && "The start is invalid");
    assert(end < self->vertexCount && "The end is invalid");
    assert(predecessors && distances);

    const int vertexCount = Graph_getVertexCount(self);

    assert(0 <= start && start < vertexCount);
    assert(end < vertexCount);

    bool* explored = (bool*)calloc(vertexCount, sizeof(bool));
    AssertNew(explored);

    for (int i = 0; i < vertexCount; i++)
    {
        predecessors[i] = -1;
        distances[i] = INFINITY;
    }
    distances[start] = 0.0f;

    while (true)
    {
        // Recherche le noeud de distance minimale
        int currID = -1;
        float currDist = INFINITY;
        for (int i = 0; i < vertexCount; ++i)
        {
            if (explored[i] == false && distances[i] < currDist)
            {
                currDist = distances[i];
                currID = i;
            }
        }

        // Condition d'arret
        if (currID < 0 || currID == end)
            break;

        explored[currID] = true;

        // Met à jour les voisins
        for (ArcList* arc = Graph_getArcList(self, currID);
             arc != NULL; arc = arc->next)
        {
            int nextID = arc->target;
            float dist = distances[currID] + arc->weight;
            if (distances[nextID] > dist)
            {
                distances[nextID] = dist;
                predecessors[nextID] = currID;
            }
        }
    }
    free(explored);
}

Path* Graph_dijkstraGetPath(int* predecessors, float* distances, int end)
{
    assert(predecessors && distances);
    assert(end >= 0);

    if (predecessors[end] < 0)
    {
        return NULL;
    }

    Path* path = Path_create(end);
    int currID = end;

    while (predecessors[currID] >= 0)
    {
        currID = predecessors[currID];
        ListInt_insertFirst(path->list, currID);
    }

    path->distance = distances[end];

    return path;
}

Path* Path_create(int start)
{
    Path* path = (Path*)calloc(1, sizeof(Path));
    AssertNew(path);

    path->distance = 0.0f;
    path->list = ListInt_create();
    ListInt_insertLast(path->list, start);

    return path;
}

void Path_destroy(Path* path)
{
    if (path == NULL) return;

    ListInt_destroy(path->list);
    free(path);
}

void Path_print(Path* path)
{
    if (path == NULL)
    {
        printf("path : NULL\n");
        return;
    }

    printf("path (distance = %f) : ", path->distance);
    ListInt_print(path->list);
}
