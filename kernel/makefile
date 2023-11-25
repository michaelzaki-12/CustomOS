ARCH = x86_64
TARGET = ../kernel.elf
SRCS = $(wildcard *.cpp)
OUT = $(wildcard *.o)

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
	$(CXX) -ffreestanding -fno-stack-protector -fno-stack-check  -std=c++20 -O3 -D__$(ARCH)__ -I. -c $(SRCS)
	$(LD) -nostdlib  -Tlinker.ld $(OUT) -o $(TARGET)

clean:
	rm *.o $(TARGET) 2>/dev/null || true
