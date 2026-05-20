/*
    2-opt and Lin-Kernighan local search for TSP
*/

#pragma once

#include "settings.h"
#include "graph.h"
#include "tsp.h"

/// @brief Applique l'optimisation 2-opt sur un chemin TSP.
/// Complexité : O(n²) par passage, répété jusqu'à convergence.
/// @param distances le graphe des distances.
/// @param path le chemin à optimiser (modifié en place).
void TSP_twoOpt(Graph* distances, Path* path);

/// @brief Applique l'optimisation Lin-Kernighan sur un chemin TSP.
/// Complexité moyenne : O(n² à n^2.2).
/// Meilleure qualité que 2-opt, ~1-3% de l'optimal en pratique.
/// @param distances le graphe des distances.
/// @param path le chemin à optimiser (modifié en place).
void TSP_linKernighan(Graph* distances, Path* path);
