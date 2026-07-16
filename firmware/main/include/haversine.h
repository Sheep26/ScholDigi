#ifndef HAVERSINE_H
#define HAVERSINE_H

#include <exercise.h>

#define EARTH_RAD 6356752.3
#define M_PI 3.14159265358979

double hav(double theta);
double hav2(double lat1, double lat2, double lng1, double lng2);

double distanceBetweenPoints(exercise_point_t *point1, exercise_point_t *point2);

#endif