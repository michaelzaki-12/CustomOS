//
// Created by pirates on 15/11/23.
//
#include "PageFrameAllocator.h"

uint64_t FreeMemory, UsedMemory, ReservedMemory;
bool Initilaized = false;

uint64_t ceil(const uint32_t x, const uint32_t y){
    uint64_t z = x;
    return (z + y - 1) / y;
}

void PageFrameAllocator::ReadMemoryMap(efi_memory_descriptor *memory_map, size_t memory_map_size, size_t memorymap_desc_size) {
    if(Initilaized) return;
    Initilaized = true;

    uint64_t * LargestFreeMemSeg = NULL;
    size_t LargestFreeMemSegSize = 0;

    uint64_t MemoryMapEntries = memory_map_size / memorymap_desc_size;
    for(int  i = 0; i < MemoryMapEntries; i++){
        efi_memory_descriptor* desc = (efi_memory_descriptor*)((uint64_t)memory_map + (i * memorymap_desc_size));
        if(desc->Type == 7){ // type Conventional memory
            if(desc->NumberOfPages * Page_Count > LargestFreeMemSegSize){
                LargestFreeMemSeg = (uint64_t*)desc->PhysicalStart;
                LargestFreeMemSegSize = desc->NumberOfPages * Page_Count;
            }
        }
    }

    uint64_t memorysize = GetMemoryMap(memory_map, MemoryMapEntries, memorymap_desc_size);
    FreeMemory = memorysize;
    uint64_t BitmapSize = ceil(memorysize / Page_Count , 8);

    InitBitmap(BitmapSize, LargestFreeMemSeg);
    /*Lock Pages*/

    LockPages(bitmap.Buffer, ceil(bitmap.size , Page_Count));

    /*Reserve Pages of unusable/Reserved Memory */

    for(int  i = 0; i < MemoryMapEntries; i++) {
        efi_memory_descriptor *desc = (efi_memory_descriptor *) ((uint64_t) memory_map + (i * memorymap_desc_size));
        if(desc->Type != 7){ // It's not a Conventional Memory
            ReservePages((void*)desc->PhysicalStart, desc->NumberOfPages);
        }
    }

}

void PageFrameAllocator::InitBitmap(size_t BitmapSize, void *BufferAddress) {
    bitmap.size = BitmapSize;
    bitmap.Buffer = (uint8_t*)BufferAddress;
    for(int i = 0; i < BitmapSize; i++){
        *(uint8_t*)(bitmap.Buffer + i) = 0;
    }
}

void PageFrameAllocator::LockPage(void *address) {
    uint64_t index = (uint64_t)address / Page_Count;
    if(bitmap.Get(index) == true) return;
    bitmap.Set(index, true);
    FreeMemory -= Page_Count;
    UsedMemory += Page_Count;
}

void PageFrameAllocator::LockPages(void *address, uint64_t PageCount) {
    for(uint64_t i = 0; i < PageCount; i++){
        LockPage((void*)((uint64_t)address + (i * Page_Count)));
    }
}

void PageFrameAllocator::FreePage(void *address) {
    uint64_t index = (uint64_t )address / Page_Count;
    if(bitmap.Get(index) == false) return;
    bitmap.Set(index, false);
    FreeMemory += Page_Count;
    UsedMemory -= Page_Count;
}

void PageFrameAllocator::FreePages(void *address, uint64_t PageCount) {
    for(int i = 0; i < PageCount; i++){
        FreePage((void*)((uint64_t)address + (i * Page_Count)));
    }
}

void PageFrameAllocator::ReservePage(void *address) {
    uint64_t index = (uint64_t )address / Page_Count;
    if(bitmap.Get(index) == true) return;
    bitmap.Set(index, true);
    FreeMemory -= Page_Count;
    ReservedMemory += Page_Count;
}
void PageFrameAllocator::ReservePages(void *address, uint64_t PageCount) {
    for(int i = 0; i < PageCount; i++){
        ReservePage((void*)((uint64_t)address + (i * Page_Count)));
    }
}

void PageFrameAllocator::ReleasePage(void *address) {
    uint64_t index = (uint64_t )address / Page_Count;
    if(bitmap.Get(index) == false) return;
    bitmap.Set(index, false);
    FreeMemory += Page_Count;
    ReservedMemory -= Page_Count;
}
void PageFrameAllocator::ReleasePages(void *address, uint64_t PageCount) {
    for(int i = 0; i < PageCount; i++){
        ReleasePage((void*)((uint64_t)address + (i * Page_Count)));
    }
}

uint64_t PageFrameAllocator::GetFreeMemory() {
    return FreeMemory;
}

uint64_t PageFrameAllocator::GetReservedMemory() {
    return ReservedMemory;
}

uint64_t PageFrameAllocator::GetUsedMemory() {
    return UsedMemory;
}

void *PageFrameAllocator::RequestPage() {
    for(uint64_t index = 0; index < (bitmap.size * 8); index++){
        if(bitmap.Get(index) == true) continue;
        LockPage((void*)(index * Page_Count));
        return (void*)(index * Page_Count);
    }
    return nullptr; /*Need A Driver For Copying from the Memory To A Disk*/
}

