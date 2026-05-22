#include "tsp.h"
#include "graph/graph.h"
#include "utils/list_int.h"

Path* Graph_tspFromHeuristic(Graph* self, int station)
{
    assert(self);
    const int n = Graph_getVertexCount(self);
    assert(station >= 0 && station < n);

    Path* tsp_path = Path_create(station);
    if (!tsp_path) return NULL;

    bool* explored = (bool*)calloc(n, sizeof(bool));
    if (!explored) return NULL;

    int current = station;
    explored[current] = true;
    int visited_count = 1;

    while (visited_count < n)
    {
        int next_vertex = -1;
        float min_dist = INFINITY;

        for (ArcList* arc = Graph_getArcList(self, current);
            arc;
            arc = arc->next)
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
    Graph* distances, Graph* pheromones,
    int start, bool* explored,
    float alpha, float beta)
{
    int n = distances->vertexCount;

    float* tab = calloc(n, sizeof(float));
    assert(tab);

    ArcList* arc = Graph_getArcList(distances, start);

    while (arc)
    {
        int v = arc->target;

        if (!explored[v])
        {
            float* tau = Graph_getArc(pheromones, start, v);
            if (tau)
            {
                float val = powf(*tau, alpha) * powf(arc->weight, -beta);
                tab[v] = val;
            }
        }
        arc = arc->next;
    }

    float sum = 0.0f;
    for (int i = 0; i < n; i++)
        sum += tab[i];

    if (sum > 0.0f)
    {
        for (int i = 0; i < n; i++)
            tab[i] /= sum;
    }

    return tab;
}

Path* Graph_acoConstructPath(
    Graph* distances, Graph* pheromones,
    int station, float alpha, float beta)
{
    int n = distances->vertexCount;

    bool* explored = calloc(n, sizeof(bool));
    assert(explored);
    int current = station;

    explored[current] = true;

    Path* path = Path_create(station);
    path->distance = 0.0f;

    for (int i = 0; i < n - 1; i++)
    {
        float* prob =
            Graph_acoGetProbabilities(
                distances, pheromones,
                current, explored,
                alpha, beta);

        float r = (float)rand() / RAND_MAX;

        float cum = 0.0f;
        int next = -1;

        for (int j = 0; j < n; j++)
        {
            cum += prob[j];
            if (r <= cum)
            {
                next = j;
                break;
            }
        }

        if (next == -1)
        {
            for (int j = 0; j < n; j++)
                if (!explored[j])
                {
                    next = j;
                    break;
                }
        }

        free(prob);

        if (next == -1)
        {
            free(explored);
            Path_destroy(path);
            return NULL;
        }

        float* w = Graph_getArc(distances, current, next);
        if (w)
            path->distance += *w;

        ListInt_insertLast(path->list, next);
        explored[next] = true;
        current = next;
    }

    float* back = Graph_getArc(distances, current, station);
    if (back)
        path->distance += *back;

    ListInt_insertLast(path->list, station);

    free(explored);
    return path;
}

void Graph_acoPheromoneUpdatePath(
    Graph* pheromones, Path* path, float q)
{
    if (!path) return;

    float deposit = q / path->distance;
    if (deposit <= 0) return;

    ListIntNode* s = &path->list->sentinel;
    ListIntNode* n = s->next;

    while (n != s && n->next != s)
    {
        int u = n->value;
        int v = n->next->value;

        ArcList* arc = Graph_getArcList(pheromones, u);

        while (arc && arc->target != v)
            arc = arc->next;

        if (arc)
            arc->weight += deposit;

        n = n->next;
    }
}

void Graph_acoPheromoneGlobalUpdate(Graph* pheromones, float rho)
{
    for (int i = 0; i < pheromones->vertexCount; i++)
    {
        for (ArcList* arc = Graph_getArcList(pheromones, i);
            arc;
            arc = arc->next)
        {
            arc->weight *= (1.0f - rho);
        }
    }
}
