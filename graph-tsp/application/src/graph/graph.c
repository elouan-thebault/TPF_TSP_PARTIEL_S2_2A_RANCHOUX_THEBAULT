/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "graph.h"

Graph* Graph_create(int vertexCount)
{
    Graph* self = (Graph*)calloc(1, sizeof(Graph));
    AssertNew(self);

    self->vertexCount = vertexCount;

    self->vertices = (Vertex*)calloc(vertexCount, sizeof(Vertex));
    AssertNew(self->vertices);

    for (int i = 0; i < vertexCount; i++)
    {
        Vertex* node = &(self->vertices[i]);

        node->arcList = NULL;
        node->arcCount = 0;
    }

    return self;
}

void Graph_destroy(Graph* self)
{
    if (!self) return;

    if (self->vertices)
    {
        for (int i = 0; i < self->vertexCount; i++)
        {
            Vertex* node = &(self->vertices[i]);
            ArcList* currArc = node->arcList;
            while (currArc != NULL)
            {
                ArcList* nextArc = currArc->next;
                free(currArc);
                currArc = nextArc;
            }
        }
        free(self->vertices);
    }

    free(self);
}

void Graph_setArc(Graph* self, int u, int v, float weight)
{
    assert(self && "self must not be NULL");
    assert(0 <= u && u < self->vertexCount && "The node ID is invalid");
    assert(0 <= v && v < self->vertexCount && "The node ID is invalid");

    // Recherche si l'arc existe
    ArcList* prevArc = NULL;
    ArcList* currArc = self->vertices[u].arcList;
    while (currArc != NULL && v > currArc->target)
    {
        prevArc = currArc;
        currArc = currArc->next;
    }

    if (currArc && currArc->target == v)
    {
        // L'arc est déjà présent, on modifie ses données
        currArc->weight = weight;
    }
    else
    {
        // L'arc n'existe pas dans le graphe
        // On l'ajoute en début de list
        ArcList* arc = (ArcList*)calloc(1, sizeof(ArcList));
        AssertNew(arc);

        arc->next = currArc;
        arc->target = v;
        arc->weight = weight;

        if (prevArc == NULL)
        {
            self->vertices[u].arcList = arc;
        }
        else
        {
            prevArc->next = arc;
        }

        self->vertices[u].arcCount++;
    }
}

void Graph_removeArc(Graph* self, int u, int v)
{
    assert(self && "self must not be NULL");
    assert(0 <= u && u < self->vertexCount && "The node ID is invalid");
    assert(0 <= v && v < self->vertexCount && "The node ID is invalid");

    // Recherche si l'arc existe
    ArcList* prevArc = NULL;
    ArcList* currArc = self->vertices[u].arcList;
    while (currArc != NULL && v > currArc->target)
    {
        prevArc = currArc;
        currArc = currArc->next;
    }

    if (currArc && currArc->target == v)
    {
        // On supprime l'arc
        if (prevArc)
        {
            prevArc->next = currArc->next;
        }
        else
        {
            self->vertices[u].arcList = currArc->next;
        }
        free(currArc);

        self->vertices[u].arcCount--;
    }
}

float* Graph_getArc(Graph* self, int u, int v)
{
    assert(self && "self must not be NULL");
    assert(0 <= u && u < self->vertexCount && "The node ID is invalid");
    assert(0 <= v && v < self->vertexCount && "The node ID is invalid");


    ArcList* currArc = self->vertices[u].arcList;
    while (currArc != NULL && v > currArc->target)
    {
        currArc = currArc->next;
    }

    if (currArc && currArc->target == v)
    {
        return &(currArc->weight);
    }

    return NULL;
}

Graph* Graph_load(char* filename)
{
    FILE* file = stdin;
    if (filename)
    {
        file = fopen(filename, "r");
        if (file == NULL)
        {
            assert(false);
            return NULL;
        }
    }

    int scanCount, vertexCount, arcCount;
    scanCount = fscanf(file, "%d %d", &vertexCount, &arcCount);
    assert(scanCount == 2);

    Graph* self = Graph_create(vertexCount);
    for (int i = 0; i < arcCount; i++)
    {
        int u, v;
        float weight;

        scanCount = fscanf(file, "%d %d %f", &u, &v, &weight);
        assert(scanCount == 3);

        Graph_setArc(self, u, v, weight);
    }

    if (filename) fclose(file);

    return self;
}

void Graph_print(Graph* self)
{
    assert(self && "self must not be NULL");

    const int vertexCount = Graph_getVertexCount(self);
    for (int currID = 0; currID < vertexCount; currID++)
    {
        printf("Vertex %d : ", currID);
        for (ArcList* arc = Graph_getArcList(self, currID);
             arc != NULL; arc = arc->next)
        {
            printf("(tgt %d, wgt %.2f), ", arc->target, arc->weight);
        }
        printf("\n");
    }
}
