#include "tsp.h"
#include "graph/graph.h"
#include "utils/list_int.h"

#define ACO_CL_SIZE 20

static int** build_candidate_lists(Graph* distances, int cl_size)
{
    int n = Graph_getVertexCount(distances);
    int** cl = malloc(n * sizeof(int*));
    assert(cl);

    int* idx = malloc(n * sizeof(int));
    float* dist = malloc(n * sizeof(float));
    assert(idx && dist);

    for (int u = 0; u < n; u++)
    {
        int count = 0;
        for (ArcList* arc = Graph_getArcList(distances, u);
            arc;
            arc = arc->next)
        {
            if (arc->target == u) continue;
            idx[count] = arc->target;
            dist[count] = arc->weight;
            count++;
        }

        for (int i = 1; i < count; i++)
        {
            int   ki = idx[i];
            float kd = dist[i];
            int j = i - 1;
            while (j >= 0 && dist[j] > kd)
            {
                idx[j + 1] = idx[j];
                dist[j + 1] = dist[j];
                j--;
            }
            idx[j + 1] = ki;
            dist[j + 1] = kd;
        }

        int k = (count < cl_size) ? count : cl_size;
        cl[u] = malloc((k + 1) * sizeof(int));
        assert(cl[u]);
        for (int i = 0; i < k; i++) cl[u][i] = idx[i];
        cl[u][k] = -1;
    }

    free(idx);
    free(dist);
    return cl;
}

static void free_candidate_lists(int** cl, int n)
{
    if (!cl) return;
    for (int i = 0; i < n; i++) free(cl[i]);
    free(cl);
}

static void two_opt_cl_n(Path* path, Graph* graph, int** cl, int max_passes)
{
    if (!path || !path->list) return;
    int n = Graph_getVertexCount(graph);

    int* tour = malloc((n + 1) * sizeof(int));
    int* pos = malloc(n * sizeof(int));
    assert(tour && pos);

    for (int i = 0; i < n; i++) pos[i] = -1;

    int count = 0;
    ListIntNode* s = &path->list->sentinel;
    for (ListIntNode* it = s->next; it != s; it = it->next)
        tour[count++] = it->value;

    if (count != n + 1) { free(tour); free(pos); return; }
    for (int i = 0; i < n; i++)
    {
        int v = tour[i];
        if (v < 0 || v >= n) { free(tour); free(pos); return; }
        pos[v] = i;
    }

    bool improved = true;
    int passes = 0;
    while (improved && passes < max_passes)
    {
        improved = false;
        passes++;

        for (int i = 0; i < n - 1; i++)
        {
            int a = tour[i];
            int b = tour[i + 1];
            float* ab_p = Graph_getArc(graph, a, b);
            if (!ab_p) continue;
            float ab = *ab_p;

            for (int* cp = cl[a]; *cp != -1; cp++)
            {
                int c = *cp;
                if (c < 0 || c >= n) continue;
                int j = pos[c];
                if (j < 0 || j >= n) continue;
                if (j <= i + 1) continue;

                int d = tour[j + 1];
                float* cd_p = Graph_getArc(graph, c, d);
                float* ac_p = Graph_getArc(graph, a, c);
                float* bd_p = Graph_getArc(graph, b, d);
                if (!cd_p || !ac_p || !bd_p) continue;

                if (*ac_p + *bd_p + 1e-6f < ab + *cd_p)
                {
                    int l = i + 1, r = j;
                    while (l < r)
                    {
                        int tmp = tour[l]; tour[l] = tour[r]; tour[r] = tmp;
                        pos[tour[l]] = l;
                        pos[tour[r]] = r;
                        l++; r--;
                    }
                    if (l == r) pos[tour[l]] = l;

                    improved = true;
                    b = tour[i + 1];
                    float* nab = Graph_getArc(graph, a, b);
                    if (!nab) break;
                    ab = *nab;
                }
            }
        }
    }

    int idx = 0;
    for (ListIntNode* it = s->next; it != s; it = it->next)
        it->value = tour[idx++];

    path->distance = 0.0f;
    for (int i = 0; i < n; i++)
    {
        float* w = Graph_getArc(graph, tour[i], tour[i + 1]);
        if (w) path->distance += *w;
    }

    free(tour);
    free(pos);
}

static void two_opt_cl(Path* path, Graph* graph, int** cl)
{
    two_opt_cl_n(path, graph, cl, 5);
}

static void or_opt_cl(Path* path, Graph* graph, int** cl)
{
    if (!path || !path->list) return;
    int n = Graph_getVertexCount(graph);

    int* tour = malloc((n + 1) * sizeof(int));
    int* pos = malloc(n * sizeof(int));
    assert(tour && pos);

    for (int i = 0; i < n; i++) pos[i] = -1;

    int count = 0;
    ListIntNode* s = &path->list->sentinel;
    for (ListIntNode* it = s->next; it != s; it = it->next)
        tour[count++] = it->value;

    if (count != n + 1) { free(tour); free(pos); return; }
    for (int i = 0; i < n; i++)
    {
        int v = tour[i];
        if (v < 0 || v >= n) { free(tour); free(pos); return; }
        pos[v] = i;
    }

    bool improved = true;
    int passes = 0;
    while (improved && passes < 2)
    {
        improved = false;
        passes++;

        for (int seg_len = 1; seg_len <= 2; seg_len++)
        {
            for (int i = 1; i + seg_len <= n; i++)
            {
                int first = tour[i];
                int last = tour[i + seg_len - 1];
                int prev_v = tour[i - 1];
                int next_v = tour[i + seg_len];

                float* d_pf = Graph_getArc(graph, prev_v, first);
                float* d_ln = Graph_getArc(graph, last, next_v);
                float* d_pn = Graph_getArc(graph, prev_v, next_v);
                if (!d_pf || !d_ln || !d_pn) continue;

                float remove_gain = *d_pf + *d_ln - *d_pn;
                float best_delta = 1e-6f;
                int   best_j = -1;

                for (int* cp = cl[first]; *cp != -1; cp++)
                {
                    int v = *cp;
                    if (v < 0 || v >= n) continue;
                    int j = pos[v];
                    if (j < 0 || j >= n) continue;
                    if (j >= i - 1 && j <= i + seg_len - 1) continue;

                    int j_next = tour[j + 1];
                    if (j_next == first) continue;

                    float* d_jn = Graph_getArc(graph, v, j_next);
                    float* d_jf = Graph_getArc(graph, v, first);
                    float* d_lj = Graph_getArc(graph, last, j_next);
                    if (!d_jn || !d_jf || !d_lj) continue;

                    float delta = remove_gain + *d_jn - *d_jf - *d_lj;
                    if (delta > best_delta) { best_delta = delta; best_j = j; }
                }

                for (int* cp = cl[last]; *cp != -1; cp++)
                {
                    int v = *cp;
                    if (v < 0 || v >= n) continue;
                    int jn_pos = pos[v];
                    if (jn_pos < 0 || jn_pos >= n) continue;
                    int j = jn_pos - 1;
                    if (j < 0) continue;
                    if (j >= i - 1 && j <= i + seg_len - 1) continue;
                    if (jn_pos >= i && jn_pos <= i + seg_len - 1) continue;

                    int jv = tour[j];
                    int j_next = v;

                    float* d_jn = Graph_getArc(graph, jv, j_next);
                    float* d_jf = Graph_getArc(graph, jv, first);
                    float* d_lj = Graph_getArc(graph, last, j_next);
                    if (!d_jn || !d_jf || !d_lj) continue;

                    float delta = remove_gain + *d_jn - *d_jf - *d_lj;
                    if (delta > best_delta) { best_delta = delta; best_j = j; }
                }

                if (best_j >= 0)
                {
                    int seg[3];
                    for (int k = 0; k < seg_len; k++) seg[k] = tour[i + k];

                    if (best_j < i)
                    {
                        for (int k = i - 1; k > best_j; k--)
                            tour[k + seg_len] = tour[k];
                        for (int k = 0; k < seg_len; k++)
                            tour[best_j + 1 + k] = seg[k];
                    }
                    else
                    {
                        for (int k = i + seg_len; k <= best_j; k++)
                            tour[k - seg_len] = tour[k];
                        for (int k = 0; k < seg_len; k++)
                            tour[best_j - seg_len + 1 + k] = seg[k];
                    }
                    for (int k = 0; k < n; k++) pos[tour[k]] = k;
                    improved = true;
                }
            }
        }
    }

    int idx = 0;
    for (ListIntNode* it = s->next; it != s; it = it->next)
        it->value = tour[idx++];

    path->distance = 0.0f;
    for (int i = 0; i < n; i++)
    {
        float* w = Graph_getArc(graph, tour[i], tour[i + 1]);
        if (w) path->distance += *w;
    }

    free(tour);
    free(pos);
}

static Path* aco_constructPathCL(
    Graph* distances, Graph* pheromones,
    int station, float alpha, float beta,
    int** cl)
{
    int n = distances->vertexCount;

    bool* explored = calloc(n, sizeof(bool));
    assert(explored);
    int current = station;
    explored[current] = true;

    Path* path = Path_create(station);
    path->distance = 0.0f;

    float* prob = malloc(n * sizeof(float));
    assert(prob);

    for (int step = 0; step < n - 1; step++)
    {
        int next = -1;

        float sum = 0.0f;
        for (int* p = cl[current]; *p != -1; p++)
        {
            int v = *p;
            if (explored[v]) { prob[v] = 0.0f; continue; }

            float* w = Graph_getArc(distances, current, v);
            float* tau = Graph_getArc(pheromones, current, v);
            if (!w || !tau) { prob[v] = 0.0f; continue; }

            float val = powf(*tau, alpha) * powf(*w, -beta);
            prob[v] = val;
            sum += val;
        }

        if (sum > 0.0f)
        {
            float r = (float)rand() / (float)RAND_MAX * sum;
            float cum = 0.0f;
            for (int* p = cl[current]; *p != -1; p++)
            {
                cum += prob[*p];
                if (r <= cum) { next = *p; break; }
            }
            if (next == -1)
            {
                for (int* p = cl[current]; *p != -1; p++)
                    if (!explored[*p]) { next = *p; break; }
            }
        }
        else
        {
            float best = INFINITY;
            for (ArcList* arc = Graph_getArcList(distances, current);
                arc;
                arc = arc->next)
            {
                if (!explored[arc->target] && arc->weight < best)
                {
                    best = arc->weight;
                    next = arc->target;
                }
            }
        }

        if (next == -1)
        {
            free(prob);
            free(explored);
            Path_destroy(path);
            return NULL;
        }

        float* w = Graph_getArc(distances, current, next);
        if (w) path->distance += *w;

        ListInt_insertLast(path->list, next);
        explored[next] = true;
        current = next;
    }

    float* back = Graph_getArc(distances, current, station);
    if (back) path->distance += *back;
    ListInt_insertLast(path->list, station);

    free(prob);
    free(explored);
    return path;
}

Path* Graph_tspFromHeuristic(Graph* self, int station)
{
    assert(self);
    const int n = Graph_getVertexCount(self);
    assert(station >= 0 && station < n);

    Path* tsp_path = Path_create(station);
    if (!tsp_path) return NULL;

    bool* explored = (bool*)calloc(n, sizeof(bool));
    if (!explored) return NULL;

    int current = station;
    explored[current] = true;
    int visited_count = 1;

    while (visited_count < n)
    {
        int next_vertex = -1;
        float min_dist = INFINITY;

        for (ArcList* arc = Graph_getArcList(self, current);
            arc;
            arc = arc->next)
        {
            if (!explored[arc->target] && arc->weight < min_dist)
            {
                min_dist = arc->weight;
                next_vertex = arc->target;
            }
        }

        if (next_vertex == -1)
        {
            free(explored);
            Path_destroy(tsp_path);
            return NULL;
        }

        ListInt_insertLast(tsp_path->list, next_vertex);
        tsp_path->distance += min_dist;

        explored[next_vertex] = true;
        current = next_vertex;
        visited_count++;
    }

    float* return_weight = Graph_getArc(self, current, station);
    if (!return_weight)
    {
        free(explored);
        Path_destroy(tsp_path);
        return NULL;
    }

    ListInt_insertLast(tsp_path->list, station);
    tsp_path->distance += *return_weight;

    free(explored);
    return tsp_path;
}

float* Graph_acoGetProbabilities(
    Graph* distances, Graph* pheromones,
    int start, bool* explored,
    float alpha, float beta)
{
    int n = distances->vertexCount;

    float* tab = calloc(n, sizeof(float));
    assert(tab);

    ArcList* arc = Graph_getArcList(distances, start);

    while (arc)
    {
        int v = arc->target;

        if (!explored[v])
        {
            float* tau = Graph_getArc(pheromones, start, v);
            if (tau)
            {
                float val =
                    powf(*tau, alpha) *
                    powf(arc->weight, -beta);

                tab[v] = val;
            }
        }

        arc = arc->next;
    }

    float sum = 0.0f;
    for (int i = 0; i < n; i++)
        sum += tab[i];

    if (sum > 0.0f)
    {
        for (int i = 0; i < n; i++)
            tab[i] /= sum;
    }

    return tab;
}

Path* Graph_acoConstructPath(
    Graph* distances, Graph* pheromones,
    int station, float alpha, float beta)
{
    int n = distances->vertexCount;

    bool* explored = calloc(n, sizeof(bool));
    assert(explored);
    int current = station;

    explored[current] = true;

    Path* path = Path_create(station);
    path->distance = 0.0f;

    for (int i = 0; i < n - 1; i++)
    {
        float* prob =
            Graph_acoGetProbabilities(
                distances, pheromones,
                current, explored,
                alpha, beta);

        float r = (float)rand() / RAND_MAX;

        float cum = 0.0f;
        int next = -1;

        for (int j = 0; j < n; j++)
        {
            cum += prob[j];
            if (r <= cum)
            {
                next = j;
                break;
            }
        }

        if (next == -1)
        {
            for (int j = 0; j < n; j++)
                if (!explored[j])
                {
                    next = j;
                    break;
                }
        }

        free(prob);

        if (next == -1)
        {
            free(explored);
            Path_destroy(path);
            return NULL;
        }

        float* w = Graph_getArc(distances, current, next);
        if (w)
            path->distance += *w;

        ListInt_insertLast(path->list, next);
        explored[next] = true;
        current = next;
    }

    float* back = Graph_getArc(distances, current, station);
    if (back)
        path->distance += *back;

    ListInt_insertLast(path->list, station);

    free(explored);
    return path;
}

void Graph_acoPheromoneUpdatePath(
    Graph* pheromones, Path* path, float q)
{
    if (!path) return;

    float deposit = q / path->distance;
    if (deposit <= 0) return;

    ListIntNode* s = &path->list->sentinel;
    ListIntNode* n = s->next;

    while (n != s && n->next != s)
    {
        int u = n->value;
        int v = n->next->value;

        ArcList* arc = Graph_getArcList(pheromones, u);

        while (arc && arc->target != v)
            arc = arc->next;

        if (arc)
            arc->weight += deposit;

        n = n->next;
    }
}

void Graph_acoPheromoneGlobalUpdate(Graph* pheromones, float rho)
{
    for (int i = 0; i < pheromones->vertexCount; i++)
    {
        for (ArcList* arc = Graph_getArcList(pheromones, i);
            arc;
            arc = arc->next)
        {
            arc->weight *= (1.0f - rho);
        }
    }
}

void two_opt(Path* path, Graph* graph)
{
    if (!path) return;

    ListIntNode* s = &path->list->sentinel;

    for (ListIntNode* i = s->next;
        i->next != s;
        i = i->next)
    {
        for (ListIntNode* j = i->next->next;
            j != s;
            j = j->next)
        {
            int a = i->value;
            int b = i->next->value;
            int c = j->value;
            int d = j->next->value;

            float* ab = Graph_getArc(graph, a, b);
            float* cd = Graph_getArc(graph, c, d);
            float* ac = Graph_getArc(graph, a, c);
            float* bd = Graph_getArc(graph, b, d);

            if (!ab || !cd || !ac || !bd) continue;

            if (*ac + *bd < *ab + *cd)
            {
                ListIntNode* left = i->next;
                ListIntNode* right = j;

                while (left != right && left->prev != right)
                {
                    int tmp = left->value;
                    left->value = right->value;
                    right->value = tmp;

                    left = left->next;
                    right = right->prev;
                }
            }
        }
    }

    path->distance = 0.0f;

    for (ListIntNode* n = s->next;
        n->next != s;
        n = n->next)
    {
        float* w = Graph_getArc(graph, n->value, n->next->value);
        if (w) path->distance += *w;
    }
}

Path* Graph_tspFromACO(
    Graph* distances,
    int station,
    int iterationCount,
    int antCount,
    float alpha,
    float beta,
    float rho,
    float q)
{
    int n = Graph_getVertexCount(distances);

    Graph* pheromones = Graph_create(n);

    for (int u = 0; u < n; u++)
    {
        for (ArcList* arc = Graph_getArcList(distances, u);
            arc;
            arc = arc->next)
        {
            Graph_setArc(pheromones, u, arc->target, 1.0f);
        }
    }

    int cl_size = (n - 1 < ACO_CL_SIZE) ? (n - 1) : ACO_CL_SIZE;
    int** candidates = build_candidate_lists(distances, cl_size);

    Path* bestPath = Graph_tspFromHeuristic(distances, station);
    if (bestPath)
    {
        two_opt_cl(bestPath, distances, candidates);
        or_opt_cl(bestPath, distances, candidates);
        two_opt_cl(bestPath, distances, candidates);
        Graph_acoPheromoneUpdatePath(pheromones, bestPath, q * (float)antCount);
    }

    int stagnation = 0;
    const int STAGNATION_LIMIT = 30;

    for (int iter = 0; iter < iterationCount; iter++)
    {
        Graph_acoPheromoneGlobalUpdate(pheromones, rho);

        Path* iterationBest = NULL;

        for (int ant = 0; ant < antCount; ant++)
        {
            Path* p =
                aco_constructPathCL(
                    distances, pheromones,
                    station, alpha, beta,
                    candidates);

            if (!p) continue;

            two_opt_cl_n(p, distances, candidates, 1);

            if (!iterationBest ||
                p->distance < iterationBest->distance)
            {
                if (iterationBest) Path_destroy(iterationBest);
                iterationBest = p;
            }
            else
            {
                Path_destroy(p);
            }
        }

        if (!iterationBest) continue;

        or_opt_cl(iterationBest, distances, candidates);
        two_opt_cl(iterationBest, distances, candidates);

        Graph_acoPheromoneUpdatePath(pheromones, iterationBest, q);

        if (bestPath)
            Graph_acoPheromoneUpdatePath(pheromones, bestPath, q * 5.0f);

        if (!bestPath ||
            iterationBest->distance < bestPath->distance)
        {
            if (bestPath) Path_destroy(bestPath);
            bestPath = iterationBest;
            stagnation = 0;
        }
        else
        {
            Path_destroy(iterationBest);
            stagnation++;
        }

        if (stagnation >= STAGNATION_LIMIT && bestPath)
        {
            for (int u = 0; u < n; u++)
            {
                for (ArcList* arc = Graph_getArcList(pheromones, u);
                    arc;
                    arc = arc->next)
                {
                    arc->weight = 1.0f;
                }
            }
            Graph_acoPheromoneUpdatePath(
                pheromones, bestPath, q * (float)antCount);
            stagnation = 0;
        }
    }

    free_candidate_lists(candidates, n);
    Graph_destroy(pheromones);
    return bestPath;
}
