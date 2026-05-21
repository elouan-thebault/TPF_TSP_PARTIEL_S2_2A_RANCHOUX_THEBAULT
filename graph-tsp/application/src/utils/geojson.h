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
void CoordGraph_destroy(CoordGraph* self);

int create_geojson(int n, int* points, CoordGraph* coord_graph);
int add_path_geojson(Path* path, int n, int* points, CoordGraph* coord_graph);
int close_geojson(void);
int WriteGeoJson(const char* chcoord, int n, int* points, Graph* graph, Path* tour);
