dd if=/dev/zero of=fat.iso bs=32k count=1440
mformat -i fat.iso -F ::
mmd -i fat.iso ::/EFI
mmd -i fat.iso ::/EFI/BOOT
mcopy -i fat.iso BOOTX64.EFI ::/EFI/BOOT
mcopy -i fat.iso font.sfn ::
mcopy -i fat.iso kernel.elf ::
mcopy -i fat.iso image.png ::
