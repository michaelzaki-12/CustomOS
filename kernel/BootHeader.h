//
// Created by pirates on 20/11/23.
//

#ifndef PIRATESOS_BOOTHEADER_H
#define PIRATESOS_BOOTHEADER_H
#include <stdint.h>
/* Scalable Screen Font (https://gitlab.com/bztsrc/scalable-font2) */
typedef struct {
    unsigned char  magic[4];
    unsigned int   size;
    unsigned char  type;
    unsigned char  features;
    unsigned char  width;
    unsigned char  height;
    unsigned char  baseline;
    unsigned char  underline;
    unsigned short fragments_offs;
    unsigned int   characters_offs;
    unsigned int   ligature_offs;
    unsigned int   kerning_offs;
    unsigned int   cmap_offs;
} __attribute__((packed)) kssfn_font_t;

typedef struct {
    unsigned int    *framebuffer;
    unsigned int    width;
    unsigned int    height;
    unsigned int    pitch;
    kssfn_font_t* font;
} bootparam_t;

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






#endif //PIRATESOS_BOOTHEADER_H
