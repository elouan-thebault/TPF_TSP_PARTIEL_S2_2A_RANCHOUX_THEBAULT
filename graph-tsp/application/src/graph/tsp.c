#include "tsp.h"
#include "graph/graph.h"
#include "utils/list_int.h"

double power(double base, int exp)
{
    double result = 1.0;
    int neg = exp < 0;
    if (neg) exp = -exp;

    while (exp > 0)
    {
        if (exp % 2 == 1)
            result *= base;
        base *= base;
        exp /= 2;
    }
    return neg ? 1.0 / result : result;
}

Path* Graph_tspFromHeuristic(Graph* self, int station)
{
    return NULL;
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
