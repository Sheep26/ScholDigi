#include <haversine.h>
#include <math.h>

/*
This function houses an implementation for the haversine formula.
The haversine formula is used to calculate the distance between two points on a globe with respect to their latadue and longatude.
*/

double degToRad(double deg) {
    return deg * M_PI / 180;
}

double hav(double theta) {
    return sin(theta/2) * sin(theta/2);
}

double hav2(double lat1, double lat2, double lng1, double lng2) {
    return hav(lat2 - lat1) + cos(lat1) * cos(lat2) * hav(lng2 - lng1);
}

double distanceBetweenPoints(exercise_point_t *point1, exercise_point_t *point2) {
    return 2.0 * EARTH_RAD * asin(sqrt(hav2(degToRad(point1->lat), degToRad(point2->lat), degToRad(point1->lng), degToRad(point2->lng))));
}