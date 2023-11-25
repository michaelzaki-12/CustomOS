//
// Created by pirates on 15/11/23.
//
#pragma once
#ifndef PIRATESOS_PAGEFRAMEALLOCATOR_H
#define PIRATESOS_PAGEFRAMEALLOCATOR_H
#include "EfiMemory.h"
#include <stdint.h>
#include "BitMap.h"
#include "BootHeader.h"

#define Page_Count  4096

class PageFrameAllocator{
public:
    void ReadMemoryMap(efi_memory_descriptor* memory_map, size_t memory_map_size, size_t memorymap_desc_size);
    void LockPage(void* address);
    void LockPages(void* address, uint64_t PageCount);
    void FreePage(void* address);
    void FreePages(void* address, uint64_t PageCount);
    void* RequestPage();
    uint64_t GetFreeMemory();
    uint64_t GetReservedMemory();
    uint64_t GetUsedMemory();

    BitMap bitmap;
private:
    void InitBitmap(size_t BitmapSize, void* BufferAddress);
    void ReservePage(void* address);
    void ReleasePage(void* address);
    void ReservePages(void* address, uint64_t PageCount);
    void ReleasePages(void* address, uint64_t PageCount);
};


#endif //PIRATESOS_PAGEFRAMEALLOCATOR_H
