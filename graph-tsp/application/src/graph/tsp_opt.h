/*
    2-opt and Lin-Kernighan local search for TSP
*/

#pragma once

#include "settings.h"
#include "graph.h"
#include "tsp.h"


void TSP_twoOpt(Graph* distances, Path* path);


void TSP_linKernighan(Graph* distances, Path* path);
