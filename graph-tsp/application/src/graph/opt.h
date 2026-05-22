#pragma once

#include "settings.h"
#include "graph.h"
#include "shortest_path.h"
#include "tsp.h"

int** build_candidate_lists(Graph* distances, int cl_size);

void free_candidate_lists(int** cl, int n);

void two_opt(Path* path, Graph* graph, int** cl, int max_passes);

void or_opt(Path* path, Graph* graph, int** cl);


Path* aco_constructPathCL(
    Graph* distances, Graph* pheromones,
    int station, float alpha, float beta, int** cl);


Path* Graph_tspFromACO(
    Graph* distances, int station, int iterationCount, int antCount,
    float alpha, float beta, float rho, float q);
