#include <uefi.h>
#include "BootLoader.h"
#include "kernel/BootHeader.h"
/**
 * Classic Hello World example
 */
/**
 * Display string using a bitmap font without the SSFN library
 */
void printString(int x, int y, char *s)
{
    unsigned char *ptr, *chr, *frg;
    unsigned int c;
    uintptr_t o, p;
    int i, j, k, l, m, n;
    while(*s) {
        c = 0; s += mbtowc((wchar_t*)&c, (const char*)s, 4);
        if(c == '\r') { x = 0; continue; } else
        if(c == '\n') { x = 0; y += font->height; continue; }
        for(ptr = (unsigned char*)font + font->characters_offs, chr = 0, i = 0; i < 0x110000; i++) {
            if(ptr[0] == 0xFF) { i += 65535; ptr++; }
            else if((ptr[0] & 0xC0) == 0xC0) { j = (((ptr[0] & 0x3F) << 8) | ptr[1]); i += j; ptr += 2; }
            else if((ptr[0] & 0xC0) == 0x80) { j = (ptr[0] & 0x3F); i += j; ptr++; }
            else { if((unsigned int)i == c) { chr = ptr; break; } ptr += 6 + ptr[1] * (ptr[0] & 0x40 ? 6 : 5); }
        }
        if(!chr) continue;
        ptr = chr + 6; o = (uintptr_t)lfb + y * pitch + x * 4;
        for(i = n = 0; i < chr[1]; i++, ptr += chr[0] & 0x40 ? 6 : 5) {
            if(ptr[0] == 255 && ptr[1] == 255) continue;
            frg = (unsigned char*)font + (chr[0] & 0x40 ? ((ptr[5] << 24) | (ptr[4] << 16) | (ptr[3] << 8) | ptr[2]) :
                                          ((ptr[4] << 16) | (ptr[3] << 8) | ptr[2]));
            if((frg[0] & 0xE0) != 0x80) continue;
            o += (int)(ptr[1] - n) * pitch; n = ptr[1];
            k = ((frg[0] & 0x1F) + 1) << 3; j = frg[1] + 1; frg += 2;
            for(m = 1; j; j--, n++, o += pitch)
                for(p = o, l = 0; l < k; l++, p += 4, m <<= 1) {
                    if(m > 0x80) { frg++; m = 1; }
                    if(*frg & m) *((unsigned int*)p) = 0xFFFFFF;
                }
        }
        x += chr[4]+1; y += chr[5];
    }
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    efi_status_t status;
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t *gop = NULL;
    FILE *f;
    long int size;
    bootparam_t bootp;


    FILE *elfF;
    char *buff;
    long int elfSize;
    Elf64_Ehdr *elf;
    Elf64_Phdr *phdr;
    uintptr_t entry;
    int i;

    efi_memory_descriptor_t *memory_map = NULL; //, *mement;
    uintn_t memory_map_size=0, map_key=0, desc_size=0;
    /*
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
*/
    /* load the file */
    if((elfF = fopen("kernel.elf", "r"))) {
        fseek(elfF, 0, SEEK_END);
        elfSize = ftell(elfF);
        fseek(elfF, 0, SEEK_SET);
        buff = malloc(elfSize + 1);
        if(!buff) {
            fprintf(stderr, "unable to allocate memory\n");
            return 1;
        }
        fread(buff, elfSize, 1, elfF);
        fclose(elfF);
    } else {
        fprintf(stderr, "Unable to open file\n");
        return 0;
    }

    /* set up boot parameters passed to the "kernel" */
    memset(&bootp, 0, sizeof(bootparam_t));
    status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
    if(!EFI_ERROR(status) && gop) {
        status = gop->SetMode(gop, 0);
        ST->ConOut->Reset(ST->ConOut, 0);
        ST->StdErr->Reset(ST->StdErr, 0);
        if(EFI_ERROR(status)) {
            fprintf(stderr, "unable to set video mode\n");
            return 0;
        }
    } else {
        fprintf(stderr, "unable to get graphics output protocol\n");
        return 0;
    }


    /* is it a valid ELF executable for this architecture? */
    elf = (Elf64_Ehdr *)buff;
    if(!memcmp(elf->e_ident, ELFMAG, SELFMAG) &&    /* magic match? */
       elf->e_ident[EI_CLASS] == ELFCLASS64 &&     /* 64 bit? */
       elf->e_ident[EI_DATA] == ELFDATA2LSB &&     /* LSB? */
       elf->e_type == ET_EXEC &&                   /* executable object? */
       elf->e_machine == EM_MACH &&                /* architecture match? */
       elf->e_phnum > 0) {                         /* has program headers? */
        /* load segments */
        for(phdr = (Elf64_Phdr *)(buff + elf->e_phoff), i = 0;
            i < elf->e_phnum;
            i++, phdr = (Elf64_Phdr *)((uint8_t *)phdr + elf->e_phentsize)) {
            if(phdr->p_type == PT_LOAD) {
                printf("ELF segment %p %d bytes (bss %d bytes)\n", phdr->p_vaddr, phdr->p_filesz,
                       phdr->p_memsz - phdr->p_filesz);
                memcpy((void*)phdr->p_vaddr, buff + phdr->p_offset, phdr->p_filesz);
                memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
            }
        }
        entry = elf->e_entry;
    } else {
        fprintf(stderr, "not a valid ELF executable for this architecture\n");
        return 0;
    }
    /* free resources */
    free(buff);


    /* get the memory map */
    status = BS->GetMemoryMap(&memory_map_size, NULL, &map_key, &desc_size, NULL);
    if(status != EFI_BUFFER_TOO_SMALL || !memory_map_size) goto err;
    /* in worst case malloc allocates two blocks, and each block might split a record into three, that's 4 additional records */
    memory_map_size += 4 * desc_size;
    memory_map = (efi_memory_descriptor_t*)malloc(memory_map_size);
    if(!memory_map) {
        fprintf(stderr, "unable to allocate memory\n");
        return 1;
    }
    status = BS->GetMemoryMap(&memory_map_size, memory_map, &map_key, &desc_size, NULL);
    if(EFI_ERROR(status)) {
        err:    fprintf(stderr, "Unable to get memory map\n");
        return 0;
    }

//    printf("Address              Size Type\n");
//    for(mement = memory_map; (uint8_t*)mement < (uint8_t*)memory_map + memory_map_size;
//        mement = NextMemoryDescriptor(mement, desc_size)) {
//        printf("%016x %8d %02x %s\n", mement->PhysicalStart, mement->NumberOfPages, mement->Type, types[mement->Type]);
//    }


    /* load font */
    if((f = fopen("font.sfn", "r"))) {
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        font = (ssfn_font_t*)malloc(size + 1);
        if(!font) {
            fprintf(stderr, "unable to allocate memory\n");
            return 1;
        }
        fread(font, size, 1, f);
        fclose(f);
    } else {
        fprintf(stderr, "Unable to load font\n");
        return 0;
    }

    /* set video mode */
    status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
    if(!EFI_ERROR(status) && gop) {
        status = gop->SetMode(gop, 0);
        ST->ConOut->Reset(ST->ConOut, 0);
        ST->StdErr->Reset(ST->StdErr, 0);
        if(EFI_ERROR(status)) {
            fprintf(stderr, "unable to set video mode\n");
            return 0;
        }
        /* set up destination buffer */
        lfb = (unsigned char*)gop->Mode->FrameBufferBase;
        width = gop->Mode->Information->HorizontalResolution;
        height = gop->Mode->Information->VerticalResolution;
        pitch = sizeof(unsigned int) * gop->Mode->Information->PixelsPerScanLine;
    } else {
        fprintf(stderr, "unable to get graphics output protocol\n");
        return 0;
    }
    bootp.framebuffer = (unsigned int*)gop->Mode->FrameBufferBase;
    bootp.width = gop->Mode->Information->HorizontalResolution;
    bootp.height = gop->Mode->Information->VerticalResolution;
    bootp.pitch = sizeof(unsigned int) * gop->Mode->Information->PixelsPerScanLine;
    bootp.font = (kssfn_font_t*)font;


    printf("Hello World!\n");
    /* display multilingual text */
   printString(10, 10, "Hello 多种语言 Многоязычный többnyelvű World!");

    /* execute the "kernel" */
    printf("ELF entry point %p\n", entry);
    (*((int(* __attribute__((sysv_abi)))(bootparam_t *))(entry)))(&bootp);
//    printf("ELF returned %d\n", i);
    printString(10, 50, "Loading the Kernel Successfully");

    /* free resources exit */
    free(font);
    free(memory_map);


    while(1 == 1);
    return 0;
}
