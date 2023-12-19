ARCH = x86_64
TARGET = ../kernel.elf
SRCS = $(wildcard *.cpp)
MemorySRCS = $(wildcard ./Memory/*.cpp)
OUT = $(wildcard *.o)
MemoryOUT = $(wildcard ./Memory/*.o)

all: $(TARGET)

ifeq ($(USE_GCC),)
CXX = clang++ -target $(ARCH)-elf
LD = ld.lld
else
CC = $(ARCH)-elf-gcc
LD = $(ARCH)-elf-ld
endif

# UEFI wastes lots and lots of memory. Link our "kernel" at an address (32M) which isn't used by UEFI
$(TARGET): $(SRCS)
	$(CXX) -ffreestanding -fno-stack-protector -fno-stack-check -Wall  -std=c++20 -O0 -D__$(ARCH)__ -I. -c $(SRCS) $(MemorySRCS)
	$(LD) -nostdlib  -Tlinker.ld $(OUT) $(MemoryOUT) -o $(TARGET)

run:
	qemu-system-x86_64 -drive file=../fat.iso -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="../OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="../OVMF_VARS-pure-efi.fd" -net none
clean:
	rm *.o $(TARGET) 2>/dev/null || true
