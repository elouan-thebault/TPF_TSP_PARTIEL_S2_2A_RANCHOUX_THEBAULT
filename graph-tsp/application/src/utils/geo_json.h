#pragma once

#include "graph/graph.h"
#include "graph/shortest_path.h"
#include "settings.h"

typedef struct
{
    double lon;
    double lat;
} Coord;

typedef struct
{
    Coord* coords;
    int    count;
} CoordGraph;

CoordGraph* CoordGraph_load(const char* filepath);
void        CoordGraph_destroy(CoordGraph* self);

/// @brief Crée un fichier geo_json
/// @param n le nombre de point.
/// @param points les identifiants des n points.
/// @param coord_graph les coordonnées du graph
/// @return path le chemin du geojson en chaine de caractère
int create_geojson(int n, int* points, CoordGraph* coord_graph);

/// @brief Ajoute un chemin au geo_json
/// @param path le chemin entre les 2 points a ajouter
/// @param n le nombre de points
/// @param points les identifiants des n points
/// @param coord_graph les coordonnées des points du graph
/// @return path le chemin du geojson en chaine de caractère
int add_path_geojson(Path* path, int n, int* points, CoordGraph* coord_graph);
