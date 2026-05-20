#include "settings.h"
#include "graph/tsp.h"
#include "graph/shortest_path.h"
#include "utils/list_int.h"

int tsp()
{
    char chgraph[256], chcoord[256];
    int n;

    if (fscanf(stdin, "%255s %255s %d", chgraph, chcoord, &n) != 3) return EXIT_FAILURE;

    int* points = malloc(n * sizeof(int));
    if (!points) return EXIT_FAILURE;

    for (int i = 0; i < n; i++)
    {
        if (fscanf(stdin, "%d", &points[i]) != 1)
        {
            free(points);
            return EXIT_FAILURE;
        }
    }

    Graph* graph = Graph_load(chgraph);
    if (!graph)
    {
        free(points);
        return EXIT_FAILURE;
    }
    int totalVertices = Graph_getVertexCount(graph);

    int* predecessors = (int*)calloc(totalVertices, sizeof(int));
    float* distances = (float*)calloc(totalVertices, sizeof(float));
    AssertNew(predecessors);
    AssertNew(distances);

    Graph* tsp_graph = Graph_create(n);

    for (int i = 0; i < n; i++)
    {
        Graph_dijkstra(graph, points[i], -1, predecessors, distances);

        for (int j = 0; j < n; j++)
        {
            if (i == j) continue;

            int targetRealVertex = points[j];
            if (distances[targetRealVertex] != INFINITY)
            {
                Graph_setArc(tsp_graph, i, j, distances[targetRealVertex]);
            }
        }
    }

    free(predecessors);
    free(distances);
    Graph_destroy(graph);

    Path* tsp_path = Graph_tspFromHeuristic(tsp_graph, 0);
    if (!tsp_path)
    {
        free(points);
        Graph_destroy(tsp_graph);
        return EXIT_FAILURE;
    }

    printf("%.1f %d\n", tsp_path->distance, n + 1);

    ListIntIter* iter = ListIntIter_create(tsp_path->list);
    while (ListIntIter_isValid(iter))
    {
        int index_tournee = ListIntIter_get(iter);
        printf("%d ", index_tournee);
        ListIntIter_next(iter);
    }
    printf("\n");

    ListIntIter_destroy(iter);
    Path_destroy(tsp_path);
    Graph_destroy(tsp_graph);
    free(points);

    return EXIT_SUCCESS;
}
int main()
{
    tsp();
    return EXIT_SUCCESS;
}
