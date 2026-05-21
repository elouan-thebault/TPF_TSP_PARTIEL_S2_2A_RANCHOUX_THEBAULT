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


