#include "settings.h"
#include "graph/graph.h"

int main()
{
    Graph* graph = Graph_load(DATA_PATH"/laval_graph.txt");
    const int vertexCount = Graph_getVertexCount(graph);
    printf("Vertex count : %d\n", vertexCount);
    Graph_destroy(graph);
    graph = NULL;

    return EXIT_SUCCESS;
}
