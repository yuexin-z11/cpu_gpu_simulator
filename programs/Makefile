# Rebuilds programs/bin/*.elf. Needs a RISC-V GCC that targets rv32im,
# e.g. riscv64-unknown-elf-gcc. Precompiled binaries are not yet available.

CROSS   ?= riscv64-unknown-elf-
CC      := $(CROSS)gcc
CFLAGS  := -march=rv32im -mabi=ilp32 -O2 -ffreestanding -nostdlib -static \
           -fno-builtin -Isrc
LDFLAGS := -T src/common/link.ld

PROGRAMS := vecadd vecadd_strided branchy branchy_aligned matmul

all: $(PROGRAMS:%=bin/%.elf)

bin/%.elf: src/%.c src/common/crt0.S src/common/socsim.h src/common/link.ld
	$(CC) $(CFLAGS) $(LDFLAGS) src/common/crt0.S $< -o $@

clean:
	rm -f bin/*.elf

.PHONY: all clean
