#include "main.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
/**
 * Display string using a bitmap font without the SSFN library
 */

uint64_t GetMemoryMap(efi_memory_descriptor_t* mMap, uint64_t MapEntries, uint64_t desc_size){
    static uint64_t MemInBytes;
    if(MemInBytes > 0) return MemInBytes;
    for(unsigned int i =0; i < MapEntries; i++){
        efi_memory_descriptor_t* desc = (efi_memory_descriptor_t*)((uint64_t)mMap + (i * desc_size));
        MemInBytes += desc->NumberOfPages * 4096;
    }
    return MemInBytes;
}


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
    FILE *f;

    FILE* Font;
    long int FontSize;

    FILE *PNGFile;
    unsigned char *PNGbuff;
    uint32_t *data;
    int w, h, l;
    long int PNGsize;
    stbi__context s;
    stbi__result_info ri;

    /*Memory Map Variables*/
    efi_memory_descriptor_t *memory_map = NULL, *mement;
    uintn_t memory_map_size=0, map_key=0, desc_size=0;
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


    char *buff;
    long int size;
    Elf64_Ehdr *elf;
    Elf64_Phdr *phdr;
    uintptr_t entry;
    int R;
    efi_status_t status;
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t *gop = NULL;
    efi_gop_mode_info_t *info = NULL;
    uintn_t isiz = sizeof(efi_gop_mode_info_t), currentMode;

    bootparam_t bootp;
    memset(&bootp, 0, sizeof(bootparam_t));

    status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
    if(!EFI_ERROR(status) && gop) {
        /* if mode given on command line, set it */
        if(argc > 1) {
            status = gop->SetMode(gop, atoi(argv[1]));
            /* changing the resolution might mess up ConOut and StdErr, better to reset them */
            ST->ConOut->Reset(ST->ConOut, 0);
            ST->StdErr->Reset(ST->StdErr, 0);
            if(EFI_ERROR(status)) {
                fprintf(stderr, "unable to set video mode\n");
                return 0;
            }
        }
        /* we got the interface, get current mode */
        status = gop->QueryMode(gop, gop->Mode ? gop->Mode->Mode : 0, &isiz, &info);
        if(status == EFI_NOT_STARTED || !gop->Mode) {
            status = gop->SetMode(gop, 0);
            ST->ConOut->Reset(ST->ConOut, 0);
            ST->StdErr->Reset(ST->StdErr, 0);
        }
        if(EFI_ERROR(status)) {
            fprintf(stderr, "unable to get current video mode\n");
            return 0;
        }
        currentMode = gop->Mode->Mode;
        /* iterate on modes and print info */
        for(unsigned int i = 0; i < gop->Mode->MaxMode; i++) {
            status = gop->QueryMode(gop, i, &isiz, &info);
            if(info->VerticalResolution == 1080 && info->HorizontalResolution == 1920) gop->SetMode(gop, i);
            if(EFI_ERROR(status) || info->PixelFormat > PixelBitMask) continue;
            printf(" %c%3d. %4d x%4d (pitch %4d fmt %d r:%06x g:%06x b:%06x)\n",
                   i == currentMode ? '*' : ' ', i,
                   info->HorizontalResolution, info->VerticalResolution, info->PixelsPerScanLine, info->PixelFormat,
                   info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor?0xff:(
                           info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff0000:(
                                   info->PixelFormat==PixelBitMask?info->PixelInformation.RedMask:0)),
                   info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor ||
                   info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff00:(
                           info->PixelFormat==PixelBitMask?info->PixelInformation.GreenMask:0),
                   info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor?0xff0000:(
                           info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff:(
                                   info->PixelFormat==PixelBitMask?info->PixelInformation.BlueMask:0)));
        }
    } else
        fprintf(stderr, "unable to get graphics output protocol\n");


    /* load image */
    if((PNGFile = fopen("image.png", "r"))) {
        fseek(PNGFile, 0, SEEK_END);
        PNGsize = ftell(PNGFile);
        fseek(PNGFile, 0, SEEK_SET);
        PNGbuff = (unsigned char*)malloc(PNGsize);
        if(!PNGbuff) {
            fprintf(stderr, "unable to allocate memory\n");
            return 1;
        }
        fread(PNGbuff, PNGsize, 1, PNGFile);
        fclose(PNGFile);
        ri.bits_per_channel = 8;
        s.read_from_callbacks = 0;
        s.img_buffer = s.img_buffer_original = PNGbuff;
        s.img_buffer_end = s.img_buffer_original_end = PNGbuff + PNGsize;
        data = (uint32_t*)stbi__png_load(&s, &w, &h, &l, 4, &ri);
        if(!data) {
            fprintf(stdout, "Unable to decode png: %s\n", stbi__g_failure_reason);
            return 0;
        }
    } else {
        fprintf(stderr, "Unable to load image\n");
        return 0;
    }


    /* load the file */
    if((f = fopen("kernel.elf", "r"))) {
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        buff = malloc(size + 1);
        if(!buff) {
            fprintf(stderr, "unable to allocate memory\n");
            return 1;
        }
        fread(buff, size, 1, f);
        fclose(f);
    } else {
        fprintf(stderr, "Unable to open file\n");
        return 0;
    }
    /* load font */
    if((Font = fopen("font.sfn", "r"))) {
        fseek(Font, 0, SEEK_END);
        FontSize = ftell(Font);
        fseek(Font, 0, SEEK_SET);
        font = (ssfn_font_t*)malloc(FontSize + 1);
        if(!font) {
            fprintf(stderr, "unable to allocate memory\n");
            return 1;
        }
        fread(font, FontSize, 1, Font);
        fclose(Font);
    } else {
        fprintf(stderr, "Unable to load font\n");
        return 0;
    }

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

    printf("Address              Size Type\n");
    for(mement = memory_map; (uint8_t*)mement < (uint8_t*)memory_map + memory_map_size;
        mement = NextMemoryDescriptor(mement, desc_size)) {
        printf("%016x %8d %02x %s\n", mement->PhysicalStart, mement->NumberOfPages, mement->Type, types[mement->Type]);
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
        for(phdr = (Elf64_Phdr *)(buff + elf->e_phoff), R = 0;
            R < elf->e_phnum;
            R++, phdr = (Elf64_Phdr *)((uint8_t *)phdr + elf->e_phentsize)) {
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

    bootp.framebuffer = gop->Mode->FrameBufferBase;
    bootp.width = gop->Mode->Information->HorizontalResolution;
    bootp.height = gop->Mode->Information->VerticalResolution;
    bootp.pitch =  sizeof(unsigned int) * gop->Mode->Information->PixelsPerScanLine;

    bootp.memory_map = (efi_memory_descriptor*)memory_map;
    bootp.desc_size = desc_size;
    bootp.memory_map_size = memory_map_size;
    bootp.map_key = map_key;

    bootp.pixelsperscanline = gop->Mode->Information->PixelsPerScanLine;
    bootp.font = font;
    //printf("Memory Size In Bytes = %8d\n", GetMemoryMap(memory_map, MapEntries, desc_size));
    /* execute the "kernel" */
    //printf("ELF entry point %p\n", entry);
    (*((int(* __attribute__((sysv_abi)))(bootparam_t*))(entry)))(&bootp);
    //printf("ELF returned %d\n", R);
    //printf("Vertical Resolution x%4d\r\n", gop->Mode->Information->VerticalResolution);
    //printf("Horizontol Resolution x%4d\r\n", gop->Mode->Information->HorizontalResolution);
    //printf("%8d\n", memory_map_size);
    /* png is RGBA, but UEFI needs BGRA */
    if(gop->Mode->Information->PixelFormat == PixelBlueGreenRedReserved8BitPerColor ||
       (gop->Mode->Information->PixelFormat == PixelBitMask && gop->Mode->Information->PixelInformation.BlueMask != 0xff0000)) {
        for(l = 0; l < w * h; l++)
            data[l] = ((data[l] & 0xff) << 16) | (data[l] & 0xff00) | ((data[l] >> 16) & 0xff);
    }
    /* display image */
    gop->Blt(gop, data, EfiBltBufferToVideo, 0, 0, (gop->Mode->Information->HorizontalResolution - w) / 2,
             (gop->Mode->Information->VerticalResolution - h) / 2, w, h, 0);

    /* free resources exit */
    free(data);
    free(PNGbuff);


    /* free resources exit */
    free(font);
    if(exit_bs()) {
        fprintf(stderr,
                "Ph'nglui mglw'nafh Chtulu R'lyeh wgah'nagl fhtagn\n"
                "(Hastur has a hold on us and won't let us go)\n");
        return 0;
    }

    while(1 == 1);
    return 0;
}
