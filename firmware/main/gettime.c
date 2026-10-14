#include <gettime.h>

struct tm getTime() {
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    return timeinfo;
}

time_t getTimeEpoch() {
    time_t now;
    time(&now);

    return now;
}