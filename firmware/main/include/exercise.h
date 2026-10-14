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
#include <ssd1306.h>

struct exercise_point;

typedef struct exercise_point {
    double lat;
    double lng;
    double alt;
    time_t time;
    double speed;

    struct exercise_point *next;
} exercise_point_t; // 48 bytes - 4

typedef struct {
    double lat;
    double lng;
    double alt;
    time_t time;
    double speed;
} exercise_point_send_t; // 44 bytes

typedef struct {
    double distance;
    double avg_speed;
    double top_speed;
    double alt_diff;
    double avg_alt;
    double min_alt;
    double max_alt;
    time_t start;
    time_t stop;

    exercise_point_t *list;
    exercise_point_t *last;
} exercise_t; // 80 bytes - 8

typedef struct {
    double distance;
    double avg_speed;
    double top_speed;
    double alt_diff;
    double avg_alt;
    double min_alt;
    double max_alt;
    time_t start;
    time_t stop;
} exercise_send_t; // 72 bytes

void setup_display_handle(ssd1306_handle_t handle);
void addExercisePoint(exercise_t *exercise, exercise_point_t *point);
exercise_t *getExercise();
void toggleExercise();

#endif