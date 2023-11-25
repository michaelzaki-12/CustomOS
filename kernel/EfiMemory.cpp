//
// Created by pirates on 16/11/23.
//
#include "EfiMemory.h"

uint64_t GetMemoryMap(efi_memory_descriptor *mMap, uint64_t MapEntries, uint64_t desc_size){
        static uint64_t MemInBytes;
        if(MemInBytes > 0) return MemInBytes;
        for(unsigned int i = 0; i < MapEntries; i++){
            efi_memory_descriptor* desc = (efi_memory_descriptor*)((uint64_t)mMap + (i * desc_size));
            MemInBytes += desc->NumberOfPages * 4096;
        }
        return MemInBytes;
}
