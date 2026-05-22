#include "search_trough.h"

double distance_km_direct(Coord* pointa, Coord* pointb)
{
    double lat1 = DEG_TO_RAD(pointa->lat);
    double lon1 = DEG_TO_RAD(pointa->lon);
    double lat2 = DEG_TO_RAD(pointb->lat);
    double lon2 = DEG_TO_RAD(pointb->lon);

    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;

    double a = sin(dlat / 2.0) * sin(dlat / 2.0) +
        cos(lat1) * cos(lat2) *
        sin(dlon / 2.0) * sin(dlon / 2.0);
    
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    return EARTH_RADIUS_KM * c;
}

int point_le_plus_proche(CoordGraph* graph, Coord* cible)
{
    int best_index = -1;
    double distance_min = INFINITY;

    for (int i = 0; i < graph->count; i++)
    {
        double distance = distance_km_direct(cible, &graph->coords[i]);

        if (distance < distance_min)
        {
            distance_min = distance;
            best_index = i;
        }
    }

    assert(best_index >= 0 && best_index < graph->count);
    return best_index;
}


int splitCSV(char* line, char* fields[], int maxFields)
{
    int count = 0;

    fields[count++] = line;

    while (*line && count < maxFields)
    {
        if (*line == ',')
        {
            *line = '\0';
            fields[count++] = line + 1;
        }

        line++;
    }

    return count;
}

int loadCommunes(Localisation** localisations, int* count)
{
    FILE* file = fopen(COMMUNES_PATH, "r");

    if (!file)
    {
        perror("Erreur ouverture fichier");
        return -1;
    }

    char line[LINE_SIZE];

    // Ignore l'en-tête
    if (!fgets(line, LINE_SIZE, file))
    {
        fclose(file);
        return -1;
    }

    int capacity = 20000;
    int ignored = 0;

    *count = 0;

    *localisations = malloc(sizeof(Localisation) * capacity);

    if (!(*localisations))
    {
        fclose(file);
        return -1;
    }

    while (fgets(line, LINE_SIZE, file))
    {
        char nom[255] = "";
        char latStr[64] = "";
        char lonStr[64] = "";
        int regionId = 0;

        char* fields[16];

        int nbFields = splitCSV(line, fields, 16);

        if (nbFields < 15)
            continue;

        strncpy(nom, fields[1], sizeof(nom) - 1);
        nom[sizeof(nom) - 1] = '\0';

        strncpy(latStr, fields[5], sizeof(latStr) - 1);
        latStr[sizeof(latStr) - 1] = '\0';

        strncpy(lonStr, fields[6], sizeof(lonStr) - 1);
        lonStr[sizeof(lonStr) - 1] = '\0';

        regionId = atoi(fields[0]);

        // Ignore les communes sans coordonnées
        if (latStr[0] == '\0' || lonStr[0] == '\0')
        {
            ignored++;
            continue;
        }

        // Agrandissement du tableau si nécessaire
        if (*count >= capacity)
        {
            capacity *= 2;

            Localisation* temp =
                realloc(*localisations,
                    sizeof(Localisation) * capacity);

            if (!temp)
            {
                fclose(file);
                return -1;
            }

            *localisations = temp;
        }

        Localisation* loc = &(*localisations)[*count];

        loc->coordid = malloc(sizeof(CoordIdentifiant));

        if (!loc->coordid)
        {
            fclose(file);
            return -1;
        }

        loc->coordid->coords = malloc(sizeof(Coord));

        if (!loc->coordid->coords)
        {
            free(loc->coordid);
            fclose(file);
            return -1;
        }

        strcpy(loc->name, nom);

        loc->id = regionId;

        loc->coordid->coords->lat = atof(latStr);
        loc->coordid->coords->lon = atof(lonStr);

        (*count)++;
    }

    fclose(file);

    printf("Communes chargees : %d\n", *count);
    printf("Communes ignorees : %d\n", ignored);

    return 0;
}

int indice_from_id(int id, Localisation* local)
{
    for (int i = 0; i < local->size; i++)
    {
        if (local[i].id == id)
        {
            return i;
        }
    }
    return -1;
}
