//
// Created by pirates on 16/12/23.
//
#include "memory.h"


void memset(void *start, uint8_t value, uint64_t num) {
    for (uint64_t i = 0; i < num; i++) {
        *(uint8_t *) ((uint64_t) start + i) = value;
    }

}