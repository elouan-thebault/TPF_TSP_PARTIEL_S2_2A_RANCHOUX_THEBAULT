#include "search_trough.h"

double distance_km(CoordIdentifiant* PointA, CoordIdentifiant* PointB)
{
    lat1 = DEG_TO_RAD(PointA->coords->lat);
    lon1 = DEG_TO_RAD(PointA->coords->lon);
    lat2 = DEG_TO_RAD(PointB->coords->lat);
    lon2 = DEG_TO_RAD(PointB->coords->lon);

    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;

    double a = sin(dlat / 2.0) * sin(dlat / 2.0) +
        cos(lat1) * cos(lat2) *
        sin(dlon / 2.0) * sin(dlon / 2.0);

    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    return EARTH_RADIUS_KM * c;
}
