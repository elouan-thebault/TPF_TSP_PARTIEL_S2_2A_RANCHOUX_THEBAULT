#include "tsp.h"
#include "graph/graph.h"
#include "utils/list_int.h"

Path* Graph_tspFromHeuristic(Graph* self, int station)
{
    assert(self && "self must not be NULL");
    const int n = Graph_getVertexCount(self);
    assert(station >= 0 && station < n);

    Path* tsp_path = Path_create(station);
    if (!tsp_path) return NULL;

    bool* explored = (bool*)calloc(n, sizeof(bool));
    AssertNew(explored);

    int current = station;
    explored[current] = true;
    int visited_count = 1;

    while (visited_count < n)
    {
        int next_vertex = -1;
        float min_dist = INFINITY;

        for (ArcList* arc = Graph_getArcList(self, current); arc != NULL; arc = arc->next)
        {
            if (!explored[arc->target] && arc->weight < min_dist)
            {
                min_dist = arc->weight;
                next_vertex = arc->target;
            }
        }

        if (next_vertex == -1)
        {
            free(explored);
            Path_destroy(tsp_path);
            return NULL;
        }

        ListInt_insertLast(tsp_path->list, next_vertex);
        tsp_path->distance += min_dist;

        explored[next_vertex] = true;
        current = next_vertex;
        visited_count++;
    }

    float* return_weight = Graph_getArc(self, current, station);
    if (!return_weight)
    {
        free(explored);
        Path_destroy(tsp_path);
        return NULL;
    }

    ListInt_insertLast(tsp_path->list, station);
    tsp_path->distance += *return_weight;

    free(explored);
    return tsp_path;
}

float* Graph_acoGetProbabilities(
    Graph* distances, Graph* pheromones, int start, bool* explored,
    float alpha, float beta)
{
    int nbr = distances->vertexCount;
    float* tab = calloc(nbr, sizeof(float));
    assert(tab);

    ArcList* arclist = Graph_getArcList(distances, start);
    ArcList* arctau = Graph_getArcList(pheromones, start);

    float stau = 0;
    while (arclist && arctau)
    {
        int v = arclist->target;
        if (!explored[v])
        {
            float val = power(arctau->weight, alpha) * power(arclist->weight, -beta);
            tab[v] = val;
            stau += val;
        }
        arclist = arclist->next;
        arctau = arctau->next;
    }
    if (stau > 0.0f)
    {
        float inv = 1.0f / stau;
        for (int i = 0; i < nbr; i++)
            tab[i] *= inv;
    }
    return tab;
}

Path* Graph_acoConstructPath(
    Graph* distances, Graph* pheromones, int station,
    float alpha, float beta)
{
    int nbr = distances->vertexCount;
    bool* explored = calloc(nbr, sizeof(bool));
    int current = station;
    explored[current] = true;
    Path* path = Path_create(station);
    path->distance = 0.0f;

    for (int i = 0; i < nbr - 1; i++)
    {
        float* tab = Graph_acoGetProbabilities(
            distances, pheromones, current, explored, alpha, beta);

        float aleatoire = (float)rand() / ((float)RAND_MAX + 1.0f);
        float ss = 0;
        int tirage = -1;
        for (int j = 0; j < nbr; j++)
        {
            ss += tab[j];
            if (tab[j] > 0.0f && aleatoire <= ss) { tirage = j; break; }
        }
        if (tirage == -1)
        {
            for (int j = nbr - 1; j >= 0; j--)
                if (!explored[j]) { tirage = j; break; }
        }
        free(tab);
        if (tirage == -1)
        {
            free(explored);
            Path_destroy(path);
            return NULL;
        }

        ArcList* arc = Graph_getArcList(distances, current);
        while (arc && arc->target != tirage) arc = arc->next;
        if (arc) path->distance += arc->weight;

        explored[tirage] = true;
        ListInt_insertLast(path->list, tirage);
        current = tirage;
    }

    ArcList* arc = Graph_getArcList(distances, current);
    while (arc && arc->target != station) arc = arc->next;
    if (arc) path->distance += arc->weight;
    ListInt_insertLast(path->list, station);

    free(explored);
    return path;
}

void Graph_acoPheromoneUpdatePath(Graph* pheromones, Path* path, float q)
{
    float dist = path->distance;
    if (dist <= 0.0f) return;
    float depot = q / dist;

    ListIntNode* sentinel = &(path->list->sentinel);
    ListIntNode* node = sentinel->next;
    while (node != sentinel && node->next != sentinel)
    {
        int u = node->value;
        int v = node->next->value;

        ArcList* arc = Graph_getArcList(pheromones, u);
        while (arc && arc->target != v)
            arc = arc->next;
        if (arc)
            arc->weight += depot;

        node = node->next;
    }
}

void Graph_acoPheromoneGlobalUpdate(Graph* pheromones, float rho)
{
    for (int i = 0; i < pheromones->vertexCount; i++)
    {
        for (ArcList* arc = Graph_getArcList(pheromones, i); arc; arc = arc->next)
        {
            arc->weight *= (1 - rho);
        }
    }
}

Path* Graph_tspFromACO(
    Graph* distances, int station, int iterationCount, int antCount,
    float alpha, float beta, float rho, float q)
{
    int n = Graph_getVertexCount(distances);
    Graph* pheromones = Graph_create(n);
    for (int u = 0; u < n; u++)
    {
        for (ArcList* arc = Graph_getArcList(distances, u); arc; arc = arc->next)
            Graph_setArc(pheromones, u, arc->target, 1.0f);
    }

    Path* bestPath = NULL;

    for (int iter = 0; iter < iterationCount; iter++)
    {
        Graph_acoPheromoneGlobalUpdate(pheromones, rho);
        for (int ant = 0; ant < antCount; ant++)
        {
            Path* antPath = Graph_acoConstructPath(distances, pheromones, station, alpha, beta);
            if (!antPath) continue;

            Graph_acoPheromoneUpdatePath(pheromones, antPath, q);

            if (!bestPath || antPath->distance < bestPath->distance)
            {
                if (bestPath) Path_destroy(bestPath);
                bestPath = antPath;
            }
            else
            {
                Path_destroy(antPath);
            }
        }
    }

    Graph_destroy(pheromones);
    return bestPath;
}
