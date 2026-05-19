#include "tsp.h"
#include <float.h>
#include <stdlib.h>
#include <math.h>

Path* Graph_tspFromHeuristic(Graph* self, int station)
{
    int n = Graph_getVertexCount(self);

    bool* visited = calloc(n, sizeof(bool));
    assert(visited);

    Path* path = Path_create(station);
    visited[station] = true;
    int current = station;

    for (int step = 1; step < n; step++)
    {
        float bestDist = FLT_MAX;
        int   bestNext = -1;

        ArcList* arc = Graph_getArcList(self, current);
        while (arc != NULL)
        {
            int v = arc->target;
            if (!visited[v] && arc->weight < bestDist)
            {
                bestDist = arc->weight;
                bestNext = v;
            }
            arc = arc->next;
        }

        assert(bestNext != -1);

        visited[bestNext] = true;
        ListInt_insertLast(path->list, bestNext);
        path->distance += bestDist;
        current = bestNext;
    }

    float* retWeight = Graph_getArc(self, current, station);
    assert(retWeight);
    path->distance += *retWeight;
    ListInt_insertLast(path->list, station);

    free(visited);
    return path;
}

float* Graph_acoGetProbabilities(
    Graph* distances, Graph* pheromones, int station,
    bool* explored, float alpha, float beta)
{
    int n = Graph_getVertexCount(distances);
    float* probs = calloc(n, sizeof(float));
    assert(probs);

    float sum = 0.f;

    ArcList* arc = Graph_getArcList(distances, station);
    while (arc != NULL)
    {
        int v = arc->target;
        if (!explored[v])
        {
            float eta = (arc->weight > 0.f) ? (1.f / arc->weight) : 0.f;

            float tau = 0.f;
            float* pheromoneArc = Graph_getArc(pheromones, station, v);
            if (pheromoneArc) tau = *pheromoneArc;

            probs[v] = powf(tau, alpha) * powf(eta, beta);
            sum += probs[v];
        }
        arc = arc->next;
    }

    if (sum > 0.f)
    {
        for (int v = 0; v < n; v++)
            probs[v] /= sum;
    }
    else
    {
        int remaining = 0;
        for (int v = 0; v < n; v++)
            if (!explored[v]) remaining++;

        if (remaining > 0)
        {
            float uniform = 1.f / (float)remaining;
            for (int v = 0; v < n; v++)
                probs[v] = explored[v] ? 0.f : uniform;
        }
    }

    return probs;
}

Path* Graph_acoConstructPath(
    Graph* distances, Graph* pheromones, int station,
    float alpha, float beta)
{
    int n = Graph_getVertexCount(distances);

    bool* explored = calloc(n, sizeof(bool));
    assert(explored);

    Path* path = Path_create(station);
    explored[station] = true;
    int current = station;

    for (int step = 1; step < n; step++)
    {
        float* probs = Graph_acoGetProbabilities(
            distances, pheromones, current, explored, alpha, beta);

        float r = (float)rand() / (float)RAND_MAX;
        float cumul = 0.f;
        int next = -1;

        for (int v = 0; v < n; v++)
        {
            if (explored[v]) continue;
            cumul += probs[v];
            if (r <= cumul) { next = v; break; }
        }

        if (next == -1)
            for (int v = n - 1; v >= 0; v--)
                if (!explored[v]) { next = v; break; }

        free(probs);
        assert(next != -1);

        float* w = Graph_getArc(distances, current, next);
        assert(w);

        path->distance += *w;
        ListInt_insertLast(path->list, next);
        explored[next] = true;
        current = next;
    }

    float* w = Graph_getArc(distances, current, station);
    assert(w);
    path->distance += *w;
    ListInt_insertLast(path->list, station);

    free(explored);
    return path;
}

void Graph_acoPheromoneUpdatePath(Graph* pheromones, Path* path, float q)
{
    if (path->distance <= 0.f) return;

    float deposit = q / path->distance;

    ListIntIter* iter = ListIntIter_create(path->list);
    while (ListIntIter_isValid(iter))
    {
        int u = ListIntIter_get(iter);
        ListIntIter_next(iter);
        if (!ListIntIter_isValid(iter)) break;
        int v = ListIntIter_get(iter);

        float* w = Graph_getArc(pheromones, u, v);
        Graph_setArc(pheromones, u, v, (w ? *w : 0.f) + deposit);

        float* w2 = Graph_getArc(pheromones, v, u);
        Graph_setArc(pheromones, v, u, (w2 ? *w2 : 0.f) + deposit);
    }
    ListIntIter_destroy(iter);
}

void Graph_acoPheromoneGlobalUpdate(Graph* pheromones, float rho)
{
    int n = Graph_getVertexCount(pheromones);

    for (int u = 0; u < n; u++)
    {
        ArcList* arc = Graph_getArcList(pheromones, u);
        while (arc != NULL)
        {
            arc->weight *= (1.f - rho);
            arc = arc->next;
        }
    }
}

Path* Graph_tspFromACO(
    Graph* distances, int station, int iterationCount, int antCount,
    float alpha, float beta, float rho, float q)
{
    int n = Graph_getVertexCount(distances);
    printf("graph bien charger\n");
    Graph* pheromones = Graph_create(n);
    printf("graph phéromone chargé\n");

    for (int i = 0; i < distances->vertexCount; i++)
    {
        for (ArcList* arc = Graph_getArcList(distances, i); arc; arc = arc->next)
        {
            Graph_setArc(pheromones, i, arc->target, 1.0f);
        }
    }
    float bestCost = INFINITY;

    printf("graph phéromone arc set\n");

    Path* bestPath = NULL;

    for (int iter = 0; iter < iterationCount; iter++)
    {
        for (int ant = 0; ant < antCount; ant++)
        {
            Path* antPath = Graph_acoConstructPath(
                distances, pheromones, station, alpha, beta);

            Graph_acoPheromoneUpdatePath(pheromones, antPath, q);

            if (antPath->distance < bestCost)
            {
                if (bestPath) Path_destroy(bestPath);
                bestPath = antPath;
                bestCost = antPath->distance;
            }
            else
            {
                Path_destroy(antPath);
            }
            printf("Itération Numéro ! %d, ant Numéro : %d\n", iter, ant);

        }
        printf("Itération Numéro ! %d\n", iter);

        Graph_acoPheromoneGlobalUpdate(pheromones, rho);
    }
    bestPath->distance = bestCost;
    Graph_destroy(pheromones);
    return bestPath;
}
