/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "tsp.h"

Path* Graph_tspFromHeuristic(Graph* self, int station)
{
    return NULL;
}

float* Graph_acoGetProbabilities(
    Graph* distances, Graph* pheromones, int start, bool* explored,
    float alpha, float beta)
{
    return NULL;
}

Path* Graph_acoConstructPath(
    Graph* distances, Graph* pheromones, int station,
    float alpha, float beta)
{
    return NULL;
}

void Graph_acoPheromoneUpdatePath(Graph* pheromones, Path* path, float q)
{
}

void Graph_acoPheromoneGlobalUpdate(Graph* pheromones, float rho)
{
}

Path* Graph_tspFromACO(
    Graph* distances, int station, int iterationCount, int antCount,
    float alpha, float beta, float rho, float q)
{
    return NULL;
}
