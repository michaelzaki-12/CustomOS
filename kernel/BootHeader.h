//
// Created by pirates on 16/11/23.
//

#ifndef PIRATESOS_BOOTHEADER_H
#define PIRATESOS_BOOTHEADER_H
#include <stdint.h>

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
} __attribute__((packed)) ssfn_font_t;

typedef struct {
    unsigned int               Type;
    unsigned int               Pad;
    uint64_t PhysicalStart;
    uint64_t   VirtualStart;
    uint64_t                NumberOfPages;
    uint64_t                Attribute;
} efi_memory_descriptor;

typedef struct {
    unsigned long long    framebuffer;
    unsigned int    width;
    unsigned int    height;
    unsigned int    pitch;
    unsigned int pixelsperscanline;
    int             argc;
    char            **argv;
    ssfn_font_t* font;
    efi_memory_descriptor *memory_map;
    unsigned long long desc_size, memory_map_size, map_key;
} bootparam_t;


#endif //PIRATESOS_BOOTHEADER_H
