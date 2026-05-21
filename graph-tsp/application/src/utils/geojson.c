#include "geojson.h"

/* ── Chargement des coordonnées ─────────────────────────────────────────────────── */

CoordGraph* CoordGraph_load(const char* filepath)
{
    FILE* f = fopen(filepath, "r");
    if (!f) { perror("CoordGraph_load: fopen"); return NULL; }

    int n = 0;
    if (fscanf(f, "%d", &n) != 1 || n <= 0)
    {
        fprintf(stderr, "CoordGraph_load: header invalide\n");
        fclose(f);
        return NULL;
    }

    CoordGraph* self = malloc(sizeof(CoordGraph));
    AssertNew(self);
    self->coords = malloc(n * sizeof(Coord));
    AssertNew(self->coords);
    self->count = n;

    for (int i = 0; i < n; i++)
    {
        if (fscanf(f, "%lf %lf", &self->coords[i].lon, &self->coords[i].lat) != 2)
        {
            fprintf(stderr, "CoordGraph_load: erreur lecture noeud %d\n", i);
            CoordGraph_destroy(self);
            fclose(f);
            return NULL;
        }
    }

    fclose(f);
    return self;
}

void CoordGraph_destroy(CoordGraph* self)
{
    if (!self) return;
    free(self->coords);
    free(self);
}

/* ── Fichier ───────────────────────────────────────────────── */

FILE* file = NULL;
int virgule = 0;

int validate_nodes(int n, int* points, CoordGraph* cg)
{
    for (int i = 0; i < n; i++)
    {
        if (points[i] < 0 || points[i] >= cg->count)
        {
            fprintf(stderr, "node_id %d hors bornes [0, %d]\n", points[i], cg->count - 1);
            return -1;
        }
    }
    return 0;
}

/* ── Initialisation ─────────────────────────────────────────────────────── */

int create_geojson(int n, int* points, CoordGraph* coord_graph)
{
    if (!coord_graph || validate_nodes(n, points, coord_graph) < 0) return -1;

    if (file) { fclose(file); file = NULL; }

    file = fopen(GEOJSON_PATH, "w");
    if (!file) { perror("create_geojson: fopen"); return -1; }

    fprintf(file, "{\n  \"type\": \"FeatureCollection\",\n  \"features\": [\n");
    virgule = 0;

    for (int i = 0; i < n; i++)
    {
        int id = points[i];
        if (virgule) fprintf(file, ",\n");
        fprintf(file,
            "    {\n"
            "      \"type\": \"Feature\",\n"
            "      \"geometry\": { \"type\": \"Point\", \"coordinates\": [%.8f, %.8f] },\n"
            "      \"properties\": { \"index\": %d, \"node_id\": %d, \"name\": \"Point %d\" }\n"
            "    }",
            coord_graph->coords[id].lon,
            coord_graph->coords[id].lat,
            i, id, i
        );
        virgule = 1;
    }
    return 0;
}

/* ── Trajet entre les points ───────────────────────────────────────────────────── */

int add_path_geojson(Path* path, int n, int* points, CoordGraph* coord_graph)
{
    if (!file) { fprintf(stderr, "add_path_geojson: appeler create_geojson d'abord\n"); return -1; }
    if (!path || !path->list || ListInt_isEmpty(path->list))
    {
        fprintf(stderr, "add_path_geojson: chemin vide ou NULL\n");
        return -1;
    }
    if (!coord_graph) return -1;

    if (virgule) fprintf(file, ",\n");

    fprintf(file,
        "    {\n"
        "      \"type\": \"Feature\",\n"
        "      \"geometry\": {\n"
        "        \"type\": \"LineString\",\n"
        "        \"coordinates\": [\n"
    );

    ListIntIter* it = ListIntIter_create(path->list);
    AssertNew(it);
    int first = 1;
    while (ListIntIter_isValid(it))
    {
        int id = ListIntIter_get(it);
        if (id >= 0 && id < coord_graph->count)
        {
            if (!first) fprintf(file, ",\n");
            fprintf(file, "          [%.8f, %.8f]",
                coord_graph->coords[id].lon,
                coord_graph->coords[id].lat);
            first = 0;
        }
        ListIntIter_next(it);
    }
    ListIntIter_destroy(it);

    fprintf(file,
        "\n        ]\n"
        "      },\n"
        "      \"properties\": {\n"
        "        \"distance\": %.4f\n"
        "      }\n"
        "    }",
        path->distance
    );

    virgule = 1;
    return 0;
}

/* ── close_geojson ──────────────────────────────────────────────────────── */

int close_geojson(void)
{
    if (!file) return -1;
    fprintf(file, "\n  ]\n}\n");
    fclose(file);
    file = NULL;
    virgule = 0;
    return 0;
}



/* ── Fonction global ────────────────────────────────────────────────────── */

int WriteGeoJson(const char* chcoord, int n, int* points, Graph* graph, Path* tour)
{
    if (!tour || !tour->list) return -1;

    CoordGraph* coord_graph = CoordGraph_load(chcoord);
    if (!coord_graph) return -1;

    create_geojson(n, points, coord_graph);

    ListIntNode* sentinel = &(tour->list->sentinel);
    for (ListIntNode* it = sentinel->next; it != sentinel && it->next != sentinel; it = it->next)
    {
        int u = points[it->value];
        int v = points[it->next->value];
        Path* seg = Graph_shortestPath(graph, u, v);
        if (!seg) continue;
        add_path_geojson(seg, n, points, coord_graph);
        Path_destroy(seg);
    }

    close_geojson();
    CoordGraph_destroy(coord_graph);
    return 0;
}
