//
// Created by pirates on 28/09/23.
//
#pragma once
#ifndef PIRATESOS_KERNEL_H
#define PIRATESOS_KERNEL_H

#include <stdint.h>
#include "BootHeader.h"


char* itoa(int value, char* result, int base) {
// check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }
    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );
// Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}


#if defined(__WATCOMC__) && __WATCOMC__ <= 1100
#define FIXCONST	__based(__segname("CONST2"))
#else
#define FIXCONST
#endif

char* ulltoa(uint64_t value, char *buf, int radix) {
    char 			tmp[64 + 1];		/* Lowest radix is 2, so 64-bits plus a null */
    char 			*p1 = tmp, *p2;
    static const char FIXCONST xlat[] = "0123456789abcdefghijklmnopqrstuvwxyz";


    do {
        *p1++ = xlat[value % (unsigned)radix];
    } while((value /= (unsigned)radix));

    for(p2 = buf; p1 != tmp; *p2++ = *--p1) {
        /* nothing to do */
    }
    *p2 = '\0';

    return buf;
}


#endif //PIRATESOS_KERNEL_H
