/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#pragma once

#include "settings.h"

typedef struct Graph Graph;
typedef struct Vertex Vertex;
typedef struct ArcList ArcList;

/// @brief Structure représentant un graphe orienté.
struct Graph
{
    /// @brief Tableau contenant les noeuds du graphe.
    Vertex *vertices;

    /// @brief Nombre de noeuds du graphe.
    int vertexCount;
};

/// @brief Structure représentant un noeud d'un graphe.
struct Vertex
{
    /// @brief Degré sortant du noeud.
    int arcCount;

    /// @brief Liste simplement chaînée des arcs sortants du noeud.
    /// Cette liste est triée selon les identifiants des noeuds de destination
    /// (dans l'ordre croissant).
    /// Vaut NULL si arcCount = 0.
    ArcList *arcList;
};

/// @brief Structure représentant une liste simplement chaînée des arcs sortants d'un noeud.
struct ArcList
{
    /// @brief Pointeur vers l'élément suivant de la liste.
    /// Vaut NULL s'il s'agit du dernier élément.
    ArcList *next;

    /// @brief Identifiant du noeud de destination de l'arc.
    int target;

    /// @brief Poids de l'arc.
    float weight;
};

/// @brief Crée un nouveau graphe.
/// Le nombre de noeuds doit être défini au moment de la création et ne peut
/// plus être modifié par la suite.
/// @param vertexCount le nombre de noeuds du graphe.
/// @return Le graphe créé.
Graph *Graph_create(int vertexCount);

/// @brief Détruit un graphe créé avec Graph_create().
/// @param self le graphe.
void Graph_destroy(Graph *self);

/// @brief Affiche le graphe.
/// @param self le graphe.
void Graph_print(Graph *self);

/// @deprecated
/// @brief Renvoie le nombre de noeuds d'un graphe.
/// @param self le graphe.
/// @return Le nombre de noeuds du graphe.
INLINE int Graph_size(Graph *self)
{
    assert(self && "self must not be NULL");
    return self->vertexCount;
}

/// @brief Renvoie le nombre de noeuds d'un graphe.
/// @param self le graphe.
/// @return Le nombre de noeuds du graphe.
INLINE int Graph_getVertexCount(Graph *self)
{
    assert(self && "self must not be NULL");
    return self->vertexCount;
}

/// @brief Retourne le nombre de voisins du noeud u.
/// @param self le graphe.
/// @param u l'identifiant du noeud.
/// @return Le degré sortant de u.
INLINE int Graph_getArcCount(Graph *self, int u)
{
    assert(self && "self must not be NULL");
    assert(0 <= u && u < self->vertexCount && "The node ID is invalid");
    return self->vertices[u].arcCount;
}

/// @brief Retourne la liste d'arcs reliant u à ses voisins.
/// @param self le graphe.
/// @param u l'identifiant du noeud.
/// @return La liste d'arcs reliant u à ses voisins.
INLINE ArcList *Graph_getArcList(Graph *self, int u)
{
    assert(self && "self must not be NULL");
    assert(0 <= u && u < self->vertexCount && "The node ID is invalid");
    return self->vertices[u].arcList;
}

/// @brief Retourne un pointeur sur le poids de l'arc entre les sommets u et v
/// ou NULL s'il n'existe pas.
/// @param self le graphe.
/// @param u l'identifiant du noeud source.
/// @param v l'identifiant du noeud de destination.
/// @return Un pointeur sur la donnée de l'arc.
float *Graph_getArc(Graph *self, int u, int v);

/// @brief Modifie le poids de l'arc entre les sommets u et v
/// ou crée cet arc s'il n'existe pas.
/// @param self le graphe.
/// @param u l'identifiant du noeud source.
/// @param v l'identifiant du noeud de destination.
/// @param weight le poids de l'arc.
void Graph_setArc(Graph *self, int u, int v, float weight);

/// @brief Supprime l'arc entre les sommets u et v s'il existe.
/// @param self le graphe.
/// @param u l'identifiant du noeud source.
/// @param v l'identifiant du noeud de destination.
void Graph_removeArc(Graph *self, int u, int v);

/// @brief Crée un nouveau graphe à partir du graphe stocké dans le fichier filename.
/// @param filenamne chemin vers le fichier texte contenant le graphe.
/// @return Le graphe créé.
Graph* Graph_load(char* filename);
