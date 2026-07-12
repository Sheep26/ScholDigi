/*
* This file is to store a struct which defines a location points for an exercise and store the exercise struct.
* Said struct will contain latatude, longatue, altitude, time and speed.
* Said struct will also be a part of a linked list and contain the pointer for the next trip point.
* 
* The exercise struct will contain a pointer to the start of the linked list which contains all the points.
* It will also contain the start time, stop time, start location, end location, start altitude, end altitude.
*/

#ifndef EXERCISE_POINT_H
#define EXERCISE_POINT_H

#include <time.h>

struct exercise_point;

typedef struct exercise_point {
    double lat;
    double lng;
    double alt;
    struct tm time;
    double speed;

    struct exercise_point *next;
} exercise_point_t;

typedef struct {
    double start_lat;
    double start_lng;
    double start_alt;
    struct tm start_time;

    double end_lat;
    double end_lng;
    double end_alt;
    struct tm end_time;

    exercise_point_t *list;
    exercise_point_t *last;
} exercise_t;

void addExercisePoint(exercise_t *exercise, exercise_point_t *point);

#endif