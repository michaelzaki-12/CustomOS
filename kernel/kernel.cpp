#include "kernel.h"
#include "EfiMemory.h"
#include "Memory/PageFrameAllocator.h"
#include "Memory/PageMapIndexer.h"
#include "Memory/Paging.h"
#include "Memory/PageTableManager.h"
#include "memory.h"


extern "C" void *memcpy(void *dest, const void *src, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        ((char*)dest)[i] = ((char*)src)[i];
    }
}

bootparam_t *bootp;
extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

const char *types[] = {
        "EfiReservedMemoryType",
        "EfiLoaderCode",
        "EfiLoaderData",
        "EfiBootServicesCode",
        "EfiBootServicesData",
        "EfiRuntimeServicesCode",
        "EfiRuntimeServicesData",
        "EfiConventionalMemory",
        "EfiUnusableMemory",
        "EfiACPIReclaimMemory",
        "EfiACPIMemoryNVS",
        "EfiMemoryMappedIO",
        "EfiMemoryMappedIOPortSpace",
        "EfiPalCode"
};

void printString(int x, int y, char *s) {
    unsigned char *ptr, *chr, *frg;
    unsigned int c;
    unsigned long long int o, p;
    int i, j, k, l, m, n;
    while (*s) {
        if ((*s & 128) != 0) {
            if (!(*s & 32)) {
                c = ((*s & 0x1F) << 6) | (*(s + 1) & 0x3F);
                s += 1;
            } else if (!(*s & 16)) {
                c = ((*s & 0xF) << 12) | ((*(s + 1) & 0x3F) << 6) | (*(s + 2) & 0x3F);
                s += 2;
            } else if (!(*s & 8)) {
                c = ((*s & 0x7) << 18) | ((*(s + 1) & 0x3F) << 12) | ((*(s + 2) & 0x3F) << 6) | (*(s + 3) & 0x3F);
                s += 3;
            } else c = 0;
        } else c = *s;
        s++;
        if (c == '\r') {
            x = 0;
            continue;
        } else if (c == '\n') {
            x = 0;
            y += bootp->font->height;
            continue;
        }
        for (ptr = (unsigned char *) bootp->font + bootp->font->characters_offs, chr = 0, i = 0; i < 0x110000; i++) {
            if (ptr[0] == 0xFF) {
                i += 65535;
                ptr++;
            } else if ((ptr[0] & 0xC0) == 0xC0) {
                j = (((ptr[0] & 0x3F) << 8) | ptr[1]);
                i += j;
                ptr += 2;
            } else if ((ptr[0] & 0xC0) == 0x80) {
                j = (ptr[0] & 0x3F);
                i += j;
                ptr++;
            } else {
                if ((unsigned int) i == c) {
                    chr = ptr;
                    break;
                }
                ptr += 6 + ptr[1] * (ptr[0] & 0x40 ? 6 : 5);
            }
        }
        if (!chr) continue;
        ptr = chr + 6;
        o = (unsigned long long int) bootp->framebuffer + y * bootp->pitch + x * 4;
        for (i = n = 0; i < chr[1]; i++, ptr += chr[0] & 0x40 ? 6 : 5) {
            if (ptr[0] == 255 && ptr[1] == 255) continue;
            frg = (unsigned char *) bootp->font +
                  (chr[0] & 0x40 ? ((ptr[5] << 24) | (ptr[4] << 16) | (ptr[3] << 8) | ptr[2]) :
                   ((ptr[4] << 16) | (ptr[3] << 8) | ptr[2]));
            if ((frg[0] & 0xE0) != 0x80) continue;
            o += (int) (ptr[1] - n) * bootp->pitch;
            n = ptr[1];
            k = ((frg[0] & 0x1F) + 1) << 3;
            j = frg[1] + 1;
            frg += 2;
            for (m = 1; j; j--, n++, o += bootp->pitch)
                for (p = o, l = 0; l < k; l++, p += 4, m <<= 1) {
                    if (m > 0x80) {
                        frg++;
                        m = 1;
                    }
                    if (*frg & m) *((unsigned int *) p) = 0xFFFF00;
                }
        }
        x += chr[4] + 1;
        y += chr[5];
    }
}


extern "C" int main(bootparam_t *bootpar) {
    bootp = bootpar;
    uint64_t mMapEntries = bootpar->memory_map_size / bootpar->desc_size;

    for (uint64_t x = 0; x < bootpar->width; x++) {
        for (uint64_t y = 0; y < bootpar->height; y++) {
            *((uint32_t *) (bootpar->framebuffer + 4 * bootpar->pixelsperscanline * y + 4 * x)) = 0xFFFFFFFF;
        }
    }

    printString(10, 10, (char *) "Hello from \"kernel\".");
    char *B[1024];
    uint64_t KernelSize = (uint64_t) &_KernelEnd - (uint64_t) &_KernelStart;
    uint64_t KernelPages = ceil((uint64_t) KernelSize, 0x1000);
    printString(10, 30, (char*)"Working!!!");
    PageFrameAllocator GlobalFrameAllocator = PageFrameAllocator();

    printString(50, 30, (char*)"Working!!!");

    GlobalFrameAllocator.LockPages(&_KernelStart, KernelPages);
    printString(50, 50, (char*)"Working!!!");


    GlobalFrameAllocator.ReadMemoryMap(bootpar->memory_map, bootpar->memory_map_size, bootpar->desc_size);
    printString(50, 70, (char*)"Working!!!");
    PageTable *PML4 = (PageTable *) GlobalFrameAllocator.RequestPage();
    printString(50, 90, (char*)"Working!!!");

    memset(PML4, 0, 0x1000);
    printString(50, 110, (char*)"Working!!!");

    PageTableManager pageTableManager = PageTableManager(PML4);
    printString(50, 130, (char*)"Working!!!");
    printString(10, 350, ulltoa(GetMemoryMap(bootpar->memory_map, mMapEntries, bootpar->desc_size) / 1024 / 1024, (char*)B, 10));
    for (uint64_t i = 0; i < GetMemoryMap(bootpar->memory_map, mMapEntries, bootpar->desc_size); i += 0x1000){
        pageTableManager.MapMemory((void*)i, (void*)i);
    }
    printString(120, 150, (char*)"Working!!!");

    uint64_t FBBase = (uint64_t)bootpar->framebuffer;
    uint64_t FBSize = (uint64_t)bootpar->framebuffersize + 0x1000;
    GlobalFrameAllocator.LockPages((void*)FBBase, FBSize / 0x1000 + 1);
    for (uint64_t i = FBBase; i < FBBase + FBSize; i += 4096){
        pageTableManager.MapMemory((void*)i, (void*)i);
    }
    printString(50, 150, (char*)"Working!!!");
    asm("mov %0, %%cr3" : : "r" (PML4));
    printString(50, 170, (char*)"Working!!!");
    memset((void *)bootpar->framebuffer, 0, bootpar->framebuffersize);
    printString(50, 190, (char*)"Working!!!");

    pageTableManager.MapMemory((void*)0x600000000, (void*)0x80000);
    printString(50, 210, (char*)"Working!!!");
    uint64_t* test = (uint64_t*)0x600000000;
    *test = 26;

    printString(10, 230, ulltoa(*test, (char*)B, 10));
    return 0;

}
