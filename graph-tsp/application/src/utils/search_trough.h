#pragma once


#pragma once

#include "settings.h"
#include "geojson.h"



typedef struct
{
    Coord* coords;
    int identifiant;
}CoordIdentifiant;

typedef struct
{
    CoordIdentifiant* coordid;
    char name[255];
    char Type[255];
    int id;
    int size;
}Localisation;


/// @brief Donne la distance en KM entre 2 point
/// @param PointA et PointB
double distance_km(CoordIdentifiant* pointa, CoordIdentifiant* pointb);

int point_le_plus_proche(CoordGraph* graph, Coord* cible);

int loadCommunes(Localisation** localisations, int* count);

int indice_from_id(int id, Localisation* local);
