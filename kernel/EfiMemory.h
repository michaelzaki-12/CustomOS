//
// Created by pirates on 15/11/23.
//
#pragma once
#ifndef PIRATESOS_EFIMEMORY_H
#define PIRATESOS_EFIMEMORY_H
#include "BootHeader.h"
#include <stdint.h>


uint64_t GetMemoryMap(efi_memory_descriptor *mMap, uint64_t MapEntries, uint64_t desc_size);

#endif //PIRATESOS_EFIMEMORY_H
