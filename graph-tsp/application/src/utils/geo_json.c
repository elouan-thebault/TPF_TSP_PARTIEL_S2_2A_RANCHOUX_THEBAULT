#include "geo_json.h"

/* ── CoordGraph ─────────────────────────────────────────────────────────── */

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

/* ── Interne ────────────────────────────────────────────────────────────── */

static int validate_nodes(int n, int* points, CoordGraph* cg)
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

static void write_placeholder(FILE* f)
{
    fprintf(f, "    {\"type\":\"__PLACEHOLDER__\"}\n");
    fprintf(f, "  ]\n}\n");
}

static char* read_file(const char* filepath, long* out_size)
{
    FILE* f = fopen(filepath, "r");
    if (!f) { perror("read_file: fopen"); return NULL; }
    fseek(f, 0, SEEK_END);
    *out_size = ftell(f);
    rewind(f);
    char* buf = malloc(*out_size + 1);
    AssertNew(buf);
    fread(buf, 1, *out_size, f);
    buf[*out_size] = '\0';
    fclose(f);
    return buf;
}

/* ── create_geojson ─────────────────────────────────────────────────────── */

int create_geojson(int n, int* points, CoordGraph* coord_graph)
{
    if (!coord_graph || validate_nodes(n, points, coord_graph) < 0) return -1;

    FILE* f = fopen(GEOJSON_PATH, "w");
    if (!f) { perror("create_geojson: fopen"); return -1; }

    fprintf(f, "{\n  \"type\": \"FeatureCollection\",\n  \"features\": [\n");

    for (int i = 0; i < n; i++)
    {
        int id = points[i];
        fprintf(f,
            "    {\n"
            "      \"type\": \"Feature\",\n"
            "      \"geometry\": { \"type\": \"Point\", \"coordinates\": [%.8f, %.8f] },\n"
            "      \"properties\": { \"index\": %d, \"node_id\": %d, \"name\": \"Point %d\" }\n"
            "    },\n",
            coord_graph->coords[id].lon,
            coord_graph->coords[id].lat,
            i, id, i
        );
    }

    write_placeholder(f);
    fclose(f);
    printf("GeoJSON initialisé : %d points %s\n", n, GEOJSON_PATH);
    return 0;
}

/* ── add_path_geojson ───────────────────────────────────────────────────── */

int add_path_geojson(Path* path, int n, int* points, CoordGraph* coord_graph)
{
    if (!path || !path->list || ListInt_isEmpty(path->list))
    {
        fprintf(stderr, "add_path_geojson: chemin vide ou NULL\n");
        return -1;
    }
    if (!coord_graph) return -1;

    long size = 0;
    char* buf = read_file(GEOJSON_PATH, &size);
    if (!buf) return -1;

    char* cut = strstr(buf, "    {\"type\":\"__PLACEHOLDER__\"}");
    if (!cut)
    {
        fprintf(stderr, "add_path_geojson: marqueur introuvable\n");
        free(buf);
        return -1;
    }

    FILE* f = fopen(GEOJSON_PATH, "w");
    if (!f) { perror("add_path_geojson: fopen"); free(buf); return -1; }
    fwrite(buf, 1, cut - buf, f);
    free(buf);

    int from_node = ListInt_getFirst(path->list);
    int to_node = ListInt_getLast(path->list);

    int from_index = -1, to_index = -1;
    for (int k = 0; k < n; k++)
    {
        if (points[k] == from_node) from_index = k;
        if (points[k] == to_node)   to_index = k;
    }

    // ✅ CONDITION : Si pas trouvé, utiliser les indices par défaut
    if (from_index == -1) from_index = 0;
    if (to_index == -1) to_index = n - 1;

    fprintf(f,
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
            if (!first) fprintf(f, ",\n");
            fprintf(f, "          [%.8f, %.8f]",
                coord_graph->coords[id].lon,
                coord_graph->coords[id].lat);
            first = 0;
        }
        ListIntIter_next(it);
    }
    ListIntIter_destroy(it);

    fprintf(f,
        "\n        ]\n"
        "      },\n"
        "      \"properties\": {\n"
        "        \"from_index\": %d,\n"
        "        \"to_index\": %d,\n"
        "        \"label\": \"%d -> %d\",\n"
        "        \"distance\": %.4f\n"
        "      }\n"
        "    },\n",
        from_index, to_index,
        from_index, to_index,
        path->distance
    );

    write_placeholder(f);
    fclose(f);
    printf("  + Segment %d -> %d ajouté (%.2f)\n", from_index, to_index, path->distance);
    return 0;
}
