/*
    2-opt and Lin-Kernighan local search for TSP
    Complexité 2-opt      : O(n²) par passage
    Complexité LK         : O(n² ~ n^2.2) en moyenne
*/

#include "tsp_opt.h"
#include <float.h>

// ─────────────────────────────────────────────────────────────────────────────
// Utilitaires internes
// ─────────────────────────────────────────────────────────────────────────────

/// Convertit la liste chaînée du Path en tableau d'entiers (sans le dernier
/// nœud qui est le retour à la station, identique au premier).
/// Retourne n (nombre de villes, sans la répétition finale).
static int path_to_array(Path* path, int* tour)
{
    int idx = 0;
    ListIntIter* it = ListIntIter_create(path->list);
    while (ListIntIter_isValid(it))
    {
        tour[idx++] = ListIntIter_get(it);
        ListIntIter_next(it);
    }
    ListIntIter_destroy(it);
    // Le dernier élément est la répétition du premier (retour station) : on l'ignore
    return idx - 1;
}

/// Reconstruit la liste chaînée du Path depuis le tableau tour[0..n-1].
/// Remet aussi à jour path->distance.
static void array_to_path(Graph* distances, Path* path, int* tour, int n)
{
    // Vide la liste existante
    while (!ListInt_isEmpty(path->list))
        ListInt_popFirst(path->list);

    // Remplit avec le tour + retour au départ
    for (int i = 0; i < n; i++)
        ListInt_insertLast(path->list, tour[i]);
    ListInt_insertLast(path->list, tour[0]);

    // Recalcule la distance totale
    float dist = 0.0f;
    for (int i = 0; i < n; i++)
    {
        int u = tour[i];
        int v = tour[(i + 1) % n];
        float* w = Graph_getArc(distances, u, v);
        dist += w ? *w : 0.0f;
    }
    path->distance = dist;
}

/// Retourne le poids de l'arc (u,v), 0 si inexistant.
static inline float arc_w(Graph* distances, int u, int v)
{
    float* w = Graph_getArc(distances, u, v);
    return w ? *w : 0.0f;
}

/// Inverse le segment tour[i+1 .. j] en place.
static void reverse_segment(int* tour, int i, int j, int n)
{
    int left = (i + 1) % n;
    int right = j;
    int len = (right - left + n) % n + 1;
    for (int k = 0; k < len / 2; k++)
    {
        int a = (left + k) % n;
        int b = (right - k + n) % n;
        int tmp = tour[a];
        tour[a] = tour[b];
        tour[b] = tmp;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 2-opt
// ─────────────────────────────────────────────────────────────────────────────

void TSP_twoOpt(Graph* distances, Path* path)
{
    int n = Graph_getVertexCount(distances);
    int* tour = (int*)malloc(n * sizeof(int));
    AssertNew(tour);

    int size = path_to_array(path, tour);  // size == n

    bool improved = true;
    while (improved)
    {
        improved = false;
        for (int i = 0; i < size - 1; i++)
        {
            for (int j = i + 2; j < size; j++)
            {
                // Évite de couper l'arête qui boucle (dernière → première)
                if (i == 0 && j == size - 1) continue;

                int a = tour[i];
                int b = tour[(i + 1) % size];
                int c = tour[j];
                int d = tour[(j + 1) % size];

                float old_cost = arc_w(distances, a, b) + arc_w(distances, c, d);
                float new_cost = arc_w(distances, a, c) + arc_w(distances, b, d);

                if (new_cost < old_cost - 1e-6f)
                {
                    reverse_segment(tour, i, j, size);
                    improved = true;
                }
            }
        }
    }

    array_to_path(distances, path, tour, size);
    free(tour);
}

// ─────────────────────────────────────────────────────────────────────────────
// Lin-Kernighan (version simplifiée à profondeur bornée)
//
// Principe :
//   Pour chaque sommet t1, on cherche une chaîne d'échanges d'arêtes
//   (t1-t2 supprimée, t2-t3 ajoutée, t3-t4 supprimée …) qui améliore
//   le coût total. On borne la profondeur à MAX_DEPTH pour garder O(n²).
// ─────────────────────────────────────────────────────────────────────────────

#define LK_MAX_DEPTH 5
#define LK_CANDIDATES 5   // voisins candidats considérés par niveau

/// Recherche récursive LK.
/// tour      : tour courant
/// n         : taille du tour
/// pos       : pos[v] = indice de v dans tour
/// t1_idx    : indice de t1 (point de départ de la chaîne)
/// t2_idx    : indice de t2 (extrémité de l'arête supprimée)
/// gain_so_far : gain accumulé jusqu'ici
/// depth     : profondeur actuelle
/// distances : graphe
/// best_gain : meilleur gain trouvé (sortie)
/// best_j    : indice j du meilleur échange de clôture (sortie)
static void lk_search(
    int* tour, int n, int* pos,
    int t1_idx, int t2_idx,
    float gain_so_far, int depth,
    Graph* distances,
    float* best_gain, int* best_j)
{
    if (depth > LK_MAX_DEPTH) return;

    int t2 = tour[t2_idx];

    // Essaie de clore la chaîne : réintroduire t2 → t1
    float close_gain = gain_so_far - arc_w(distances, t2, tour[t1_idx]);
    if (close_gain > *best_gain + 1e-6f)
    {
        *best_gain = close_gain;
        *best_j = t2_idx;
    }

    // Explore les candidats pour t3 (voisins de t2 triés par distance croissante)
    // On itère sur la liste d'arcs qui est triée par target, pas par poids,
    // donc on ramasse les LK_CANDIDATES premiers dont l'ajout est potentiellement bon.
    int cand_count = 0;
    for (ArcList* arc = Graph_getArcList(distances, t2);
        arc && cand_count < LK_CANDIDATES * 4;
        arc = arc->next)
    {
        int t3 = arc->target;
        if (t3 == tour[t1_idx]) continue;          // évite t1
        if (pos[t3] < 0) continue;                 // sommet invalide

        float g1 = gain_so_far - arc_w(distances, t2, t3); // gain après ajout t2-t3

        // t4 est le suivant ou le précédent de t3 dans le tour
        for (int sign = -1; sign <= 1; sign += 2)
        {
            int t4_idx = (pos[t3] + sign + n) % n;
            int t4 = tour[t4_idx];
            if (t4 == tour[t1_idx]) continue;

            float removed = arc_w(distances, t3, t4);
            float g2 = g1 + removed;

            if (g2 > 1e-6f)  // gain positif potentiel : vaut la peine d'explorer
            {
                lk_search(tour, n, pos,
                    t1_idx, t4_idx,
                    g2, depth + 1,
                    distances,
                    best_gain, best_j);
                cand_count++;
                if (cand_count >= LK_CANDIDATES) goto next_arc;
            }
        }
    next_arc:;
    }
}

void TSP_linKernighan(Graph* distances, Path* path)
{
    int n = Graph_getVertexCount(distances);
    int* tour = (int*)malloc(n * sizeof(int));
    int* pos = (int*)malloc(n * sizeof(int));  // pos[sommet] = indice dans tour
    AssertNew(tour);
    AssertNew(pos);

    int size = path_to_array(path, tour);

    // Construit le tableau de positions
    for (int i = 0; i < size; i++)
        pos[tour[i]] = i;

    bool improved = true;
    while (improved)
    {
        improved = false;

        for (int i = 0; i < size; i++)
        {
            int t1 = tour[i];

            // t2 = successeur de t1
            int t2_idx = (i + 1) % size;

            float best_gain = 0.0f;
            int   best_j = -1;

            float removed_t1t2 = arc_w(distances, t1, tour[t2_idx]);

            lk_search(tour, size, pos,
                i, t2_idx,
                removed_t1t2, 1,
                distances,
                &best_gain, &best_j);

            if (best_j >= 0 && best_gain > 1e-6f)
            {
                // Applique l'échange : inverse le segment entre t2 et best_j
                int lo = t2_idx < best_j ? t2_idx : best_j;
                int hi = t2_idx < best_j ? best_j : t2_idx;
                reverse_segment(tour, lo - 1, hi, size);

                // Met à jour le tableau pos
                for (int k = 0; k < size; k++)
                    pos[tour[k]] = k;

                improved = true;
            }
        }
    }

    array_to_path(distances, path, tour, size);
    free(tour);
    free(pos);
}
