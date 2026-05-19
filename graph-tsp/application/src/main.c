#include "settings.h"
#include "graph/tsp.h"
#include "graph/shortest_path.h"
#include "utils/list_int.h"

int main()
{
    char chgraph[256], chcoord[256];
    int n;

    if (fscanf(stdin, "%255s %255s %d", chgraph, chcoord, &n) != 3) return EXIT_FAILURE;

    int* points = malloc(n * sizeof(int));
    if (!points) return EXIT_FAILURE;

    for (int i = 0; i < n; i++)
        if (fscanf(stdin, "%d", &points[i]) != 1) return EXIT_FAILURE;

    Graph* graph = Graph_load(chgraph);
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
        }
    }

    Path* path = Graph_tspFromACO(reduced, 0, 200, 60, 2.0f, 3.0f, 0.1f, 2.0f);
    if (!path) { Graph_destroy(reduced); Graph_destroy(graph); free(points); return EXIT_FAILURE; }

    printf("%.1f %d\n", path->distance, n + 1);
    ListIntNode* sentinel = &(path->list->sentinel);
    ListIntNode* node = sentinel->next;
    while (node != sentinel)
    {
        printf("%d ", node->value);
        node = node->next;
    }
    printf("\n");

    Path_destroy(path);
    Graph_destroy(reduced);
    Graph_destroy(graph);
    free(points);
    return EXIT_SUCCESS;
}
