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
    uint8_t* Buffer;
    bool Get(uint8_t index);
    void Set(uint8_t index, bool value);


};


#endif //PIRATESOS_BITMAP_H
