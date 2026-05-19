#include "settings.h"
#include "graph/graph.h"
#include "graph/shortest_path.h"
#include "utils/geo_json.h"
#include "graph/tsp.h"
#include "float.h"

int main()
{
    return tsp_ACO();
}

int tsp_ACO()
{
    int station = 0;
    int iterationCount = 200;
    int antCount = 60;
    float alpha = 2.0f;
    float beta = 3.0f;
    float rho = 0.1f;
    float q = 2.0f;

    char chgraph[256], chcoord[256];
    int n;

    if (fscanf(stdin, "%255s %255s %d", chgraph, chcoord, &n) != 3) return EXIT_FAILURE;

    CoordGraph* coord_graph = CoordGraph_load(&chcoord);
    printf("CoordGraph Load\n");
    int* points = malloc(n * sizeof(int));
    int* ordre = malloc(n * sizeof(int));

    float* bestCost = malloc(sizeof * bestCost);
    if (!bestCost) return EXIT_FAILURE;
    *bestCost = INFINITY;

    if (!points || !ordre) return EXIT_FAILURE;

    for (int i = 0; i < n; i++) if (fscanf(stdin, "%d", &points[i]) != 1) return EXIT_FAILURE;
    create_geojson(n, points, coord_graph);

    Graph* graph = Graph_load(chgraph);
    printf("Graph Load\n");
    if (!graph) return EXIT_FAILURE;

    Graph* reduced = Graph_create(n);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i == j) continue;
            Path* sp = Graph_shortestPath(graph, points[i], points[j]);
            if (!sp) { Graph_destroy(reduced); Graph_destroy(graph); free(points); return 1; }
            Graph_setArc(reduced, i, j, sp->distance);
            Path_destroy(sp);
            printf("Reduced en cours i = %d/%d, j = %d/%d\n",i, n, j, n);
        }
    }

    Path *path = Graph_tspFromACO(reduced, station, iterationCount, antCount, alpha, beta, rho, q, bestCost);

    printf("%.1f %i\n", *bestCost, n + 1);
    ListIntNode* sentinel = &(path->list->sentinel);
    ListIntNode* node = sentinel->next;
    while (node != sentinel)
    {
        printf("%d ", node->value);
        node = node->next;
    }
    printf("\n");

    Path_destroy(path);
    free(points);
    free(ordre);
    free(bestCost);
    Graph_destroy(graph);
    CoordGraph_destroy(coord_graph);

    return EXIT_SUCCESS;
}
int tsp_heurr()
{
    char chgraph[256], chcoord[256];
    int n;

    if (fscanf(stdin, "%255s %255s %d", chgraph, chcoord, &n) != 3) return EXIT_FAILURE;

    CoordGraph* coord_graph = CoordGraph_load(&chcoord);

    int* points = malloc(n * sizeof(int));
    int* ordre = malloc(n * sizeof(int));
    bool* explored = calloc(n, sizeof(bool));
    if (!points || !ordre || !explored) return EXIT_FAILURE;

    for (int i = 0; i < n; i++) if (fscanf(stdin, "%d", &points[i]) != 1) return EXIT_FAILURE;

    create_geojson(n, points, coord_graph);

    Graph* graph = Graph_load(chgraph);
    if (!graph) return EXIT_FAILURE;

    float total_distance = 0;
    int current_idx = 0;
    explored[0] = true;
    ordre[0] = 0;
    int visited_count = 1;

    while (visited_count < n)
    {
        int id_min = -1;
        float dist_min = INFINITY;

        for (int i = 0; i < n; i++)
        {
            if (explored[i]) continue;

            Path* path = Graph_shortestPath(graph, points[current_idx], points[i]);
            if (!path) return 1;

            if (path->distance < dist_min)
            {
                dist_min = path->distance;
                id_min = i;
            }
            Path_destroy(path);
        }

        if (id_min == -1) return 1;
        Path* path = Graph_shortestPath(graph, points[current_idx], points[id_min]);
        add_path_geojson(path, n, points, coord_graph);
        total_distance += dist_min;
        explored[id_min] = true;
        current_idx = id_min;
        ordre[visited_count] = current_idx;
        visited_count++;
    }

    Path* pstart = Graph_shortestPath(graph, points[current_idx], points[0]);
    if (!pstart) return 1;
    add_path_geojson(pstart, n, points, coord_graph);
    total_distance += pstart->distance;
    Path_destroy(pstart);

    printf("%.1f %i\n", total_distance, n + 1);
    for (int i = 0; i < n; i++) printf("%d ", ordre[i]);
    printf("%d\n", 0);

    free(points);
    free(ordre);
    free(explored);
    Graph_destroy(graph);
    CoordGraph_destroy(coord_graph);

    return EXIT_SUCCESS;
}
