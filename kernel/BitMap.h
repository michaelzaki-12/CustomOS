//
// Created by pirates on 06/10/23.
//
#pragma once
#ifndef PIRATESOS_BITMAP_H
#define PIRATESOS_BITMAP_H
#include <stdint.h>
#include <stddef.h>


class BitMap{
public:
    size_t size;
    inline static uint8_t* Buffer;
    static bool Get(uint8_t index);
    static void Set(uint8_t index, bool value);


};


#endif //PIRATESOS_BITMAP_H
