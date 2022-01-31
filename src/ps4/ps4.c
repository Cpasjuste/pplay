//
// Created by cpasjuste on 27/01/2022.
//

#include "time.h"

int __clock_gettime(clockid_t c, struct timespec *t) {
    return clock_gettime(c, t);
}