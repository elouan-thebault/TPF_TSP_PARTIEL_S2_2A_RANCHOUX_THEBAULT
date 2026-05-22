#include "settings.h"
#include "graph/tsp.h"
#include "graph/shortest_path.h"
#include "utils/geojson.h"
#include "graph/tsp.h"
#include "float.h"
#include "utils/search_trough.h"
#include "utils/list_int.h"

bool Arc_isInPath(Path* path, int u, int v)
{
    ListIntIter* iter = ListIntIter_create(path->list);
    while (ListIntIter_isValid(iter))
    {
        int a = ListIntIter_get(iter);
        ListIntIter_next(iter);
        if (!ListIntIter_isValid(iter)) break;
        int b = ListIntIter_get(iter);

        if (a == u && b == v)
        {
            ListIntIter_destroy(iter);
            return true;
        }
    }
    ListIntIter_destroy(iter);
    return false;
}
//../../../data/reduit_opti.txt
//../../../data/france_graph.txt
//../../../data/france_inter.txt
int villvill_aco()
{
    char chgraph[255], chcoord[255];
    int start = 0, end = 0;

    FILE* f = fopen(CONFIG, "r");
    if (!f) return EXIT_FAILURE;
    if (fscanf(f, "%255s %255s", chgraph, chcoord) != 2) return EXIT_FAILURE;
    if (fscanf(f, "%d %d", &start, &end) != 2) return EXIT_FAILURE;
    fclose(f);

    CoordGraph* coord_graph = CoordGraph_load(chcoord);
    Graph* graph = Graph_load(chgraph);

    Localisation* communes = NULL;
    int nbCommunes = 0;
    if (loadCommunes(&communes, &nbCommunes) != 0)
    {
        printf("Erreur lors du chargement\n");
        return 1;
    }
    communes->size = nbCommunes;
    printf("%d communes chargees\n", nbCommunes);

    end = indice_from_id(end, communes);
    start = indice_from_id(start, communes);
    assert(start != -1 && "start invalide");
    assert(end != -1 && "end invalide");

    int n = 2;
    int* points = calloc(n, sizeof(int));
    if (!points) return EXIT_FAILURE;

    points[0] = point_le_plus_proche(coord_graph, communes[start].coordid->coords);
    points[1] = point_le_plus_proche(coord_graph, communes[end].coordid->coords);

    int totalVertices = Graph_getVertexCount(graph);
    int* predecessors = malloc(totalVertices * sizeof(int));
    float* distances = malloc(totalVertices * sizeof(float));
    assert(predecessors && distances);

    Graph* reduced = Graph_create(n);

    for (int i = 0; i < n; i++)
    {
        Graph_dijkstra(graph, points[i], -1, predecessors, distances);
        for (int j = 0; j < n; j++)
        {
            if (i == j) continue;
            if (distances[points[j]] == INFINITY)
            {
                printf("Pas de chemin entre les deux villes\n");
                free(predecessors); free(distances);
                free(points);
                Graph_destroy(reduced); Graph_destroy(graph);
                CoordGraph_destroy(coord_graph);
                return EXIT_FAILURE;
            }
            Graph_setArc(reduced, i, j, distances[points[j]]);
        }
    }
    free(predecessors);
    free(distances);

    Path* path = Graph_tspFromACO(reduced, 0, 200, 60, 2.0f, 3.0f, 0.1f, 2.0f);

    if (!path)
    {
        printf("Erreur ACO\n");
        free(points);
        Graph_destroy(reduced); Graph_destroy(graph);
        CoordGraph_destroy(coord_graph);
        return EXIT_FAILURE;
    }

    float distance = path->distance;

    create_geojson(n, points, coord_graph);
    add_path_geojson(path, n, points, coord_graph);

    printf("Distance totale: %.2f m\n", distance);
    printf("Distance totale: %.2f km\n", distance / 1000.0f);

    Path_destroy(path);
    free(points);
    Graph_destroy(reduced);
    Graph_destroy(graph);
    CoordGraph_destroy(coord_graph);

    return EXIT_SUCCESS;
}

int main()
{
    villvill();

}



int villvill()
{
    char chgraph[255], chcoord[255];

    int start = 0, end = 0;

    FILE* f = fopen(CONFIG, "r");

    if (fscanf(f, "%255s %255s", chgraph, chcoord) != 2) return EXIT_FAILURE;
    if (fscanf(f, "%d %d", &start, &end) != 2)
    {
        return NULL;
    }

    CoordGraph* coord_graph = CoordGraph_load(chcoord);

    Graph* distances = Graph_load(chgraph);

    Localisation* communes = NULL;
    int nbCommunes = 0;

    if (loadCommunes(&communes, &nbCommunes) != 0)
    {
        printf("Erreur lors du chargement\n");
        return 1;
    }

    communes->size = nbCommunes;

    printf("%d communes chargees\n", nbCommunes);


    end = indice_from_id(end, communes);
    start = indice_from_id(start, communes);


    assert(start != -1 && "start invalide");
    assert(end != -1 && "end invalide");



    int n = 2;
    int* points = calloc(n, sizeof(int));

    points[0] = point_le_plus_proche(coord_graph, communes[start].coordid->coords);
    points[1] = point_le_plus_proche(coord_graph, communes[end].coordid->coords);



    Path* path = Graph_shortestPath(distances, points[0], points[1]);
    float distance = path->distance;

    create_geojson(n, points, coord_graph);
    add_path_geojson(path, n, points, coord_graph);

    printf("Distance totale: %.2f m\n", distance);
    distance = distance / 1000.0;
    printf("Distance totale: %.2f km\n", distance);
    close_geojson();

    free(points);
    Graph_destroy(distances);
    CoordGraph_destroy(coord_graph);


    /* for (int i = 0; i < nbCommunes; i++)
     {
         communes[i].coordid->identifiant = point_le_plus_proche(coord_graph, communes[i].coordid->coords);
         printf("La commune : %255s est relié a l'identifiant %d\n", communes[i].name, communes[i].coordid->identifiant);
     }

     for (int i = 0; i < nbCommunes; i++)
     {
         printf(
             "%s (%d) (%d) : %.6f %.6f\n",
             communes[i].name,
             communes[i].coordid->identifiant,
             communes[i].id,
             communes[i].coordid->coords->lat,
             communes[i].coordid->coords->lon
         );
     }*/

     //return tsp_ACO();
    return EXIT_SUCCESS;
}

Path *tspAcoOpti()
{
    char chgraph[255];
    if (fscanf(stdin, "%255s", chgraph) != 1) return EXIT_FAILURE;

    Graph* distances = Graph_load(chgraph);
    int n = Graph_getVertexCount(distances);

    int station = 0;
    int iterationCount = 200;
    int antCount = 60;
    float alpha = 2.0f;
    float beta = 3.0f;
    float rho = 0.1f;
    float q = 2.0f;
    Path* papath;
    float bestdist = INFINITY;

    for (int i = 0; i < n; i++)
    {
        papath = Graph_tspFromHeuristic(distances, i);
        if (papath->distance < bestdist)
        {
            bestdist = papath->distance;
        }
    }

    printf("graph bien charger\n");
    Graph* pheromones = Graph_create(n);
    printf("graph phéromone chargé\n");

    for (int i = 0; i < distances->vertexCount; i++)
    {
        for (ArcList* arc = Graph_getArcList(distances, i); arc; arc = arc->next)
        {
            float value = Arc_isInPath(papath, i, arc->target) ? 2.0f : 1.0f;
            Graph_setArc(pheromones, i, arc->target, value);
        }
    }
    float bestCost = INFINITY;

    printf("graph phéromone arc set\n");

    Path* bestPath = NULL;

    for (int iter = 0; iter < iterationCount; iter++)
    {
        for (int ant = 0; ant < antCount; ant++)
        {
            Path* antPath = Graph_acoConstructPath(
                distances, pheromones, station, alpha, beta);

            Graph_acoPheromoneUpdatePath(pheromones, antPath, q);

            if (antPath->distance < bestCost)
            {
                if (bestPath) Path_destroy(bestPath);
                bestPath = antPath;
                bestCost = antPath->distance;
            }
            else
            {
                Path_destroy(antPath);
            }

        }

        Graph_acoPheromoneGlobalUpdate(pheromones, rho);
    }

    bestPath->distance = bestCost;
    Graph_destroy(pheromones);

    printf("%.1f %d\n", bestPath->distance, distances->vertexCount+1);
    ListIntIter* it = ListIntIter_create(bestPath->list);
    while (ListIntIter_isValid(it))
    {
        int currID = ListIntIter_get(it);
        ListIntIter_next(it);
        printf("%d ", currID);
    }
    return bestPath;
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

    char chgraph[255], chcoord[255];
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
    int* predecessors = malloc(Graph_getVertexCount(graph) * sizeof(int));
    float* distances = malloc(Graph_getVertexCount(graph) * sizeof(float));
    assert(predecessors && distances);
    printf("Commencement Reduction\n");
    for (int i = 0; i < n; i++)
    {
        Graph_dijkstra(graph, points[i], -1, predecessors, distances);

        for (int j = 0; j < n; j++)
        {
            if (i == j) continue;

            if (distances[points[j]] == FLT_MAX)
            {
                Graph_destroy(reduced);
                Graph_destroy(graph);
                free(predecessors);
                free(distances);
                free(points);
                return 1;
            }

            Graph_setArc(reduced, i, j, distances[points[j]]);
        }

        printf("Reduced en cours i = %d/%d\n", i, n);
    }


    free(predecessors);
    free(distances);

    Path *path = Graph_tspFromACO(reduced, station, iterationCount, antCount, alpha, beta, rho, q);

    add_path_geojson(path, n, points, coord_graph);
    
    printf("%.1f %i\n", path->distance, n + 1);
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

int tsp()
{
    char chgraph[255], chcoord[255];
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

int ACO()
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
    int* predecessors = malloc(Graph_getVertexCount(graph) * sizeof(int));
    float* distances = malloc(Graph_getVertexCount(graph) * sizeof(float));
    assert(predecessors && distances);

    for (int i = 0; i < n; i++)
    {
        Graph_dijkstra(graph, points[i], -1, predecessors, distances);
        for (int j = 0; j < n; j++)
        {
            if (i == j) continue;
            if (distances[points[j]] == INFINITY)
            {
                Graph_destroy(reduced);
                Graph_destroy(graph);
                free(predecessors);
                free(distances);
                free(points);
                return 1;
            }
            Graph_setArc(reduced, i, j, distances[points[j]]);
        }
    }
    free(predecessors);
    free(distances);

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

    int w = WriteGeoJson(chcoord, n, points, graph, path);
    if (!w) { printf("Fichier ecrit"); }
    else printf("Erreur ecriture geojson");

    Path_destroy(path);
    Graph_destroy(reduced);
    Graph_destroy(graph);
    free(points);
    return EXIT_SUCCESS;
}
