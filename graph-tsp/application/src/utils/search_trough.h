#pragma once


#include "settings.h"
#include "geo_json.h"


typedef struct
{
    Coord* coords;
    int identifiant;
}CoordIdentifiant;

/// @brief Donne la distance en KM entre 2 point
/// @param lat et lon 
float distance_km(CoordIdentifiant* PointA, CoordIdentifiant* PointB);
