SHELL := /bin/bash

# configure

#ARCH=i386
ARCH=x86_64
#BOOT_TYPE=pc
BOOT_TYPE=efi
GRUB_TARGET=$(ARCH)-$(BOOT_TYPE)
ifeq ($(GRUB_TARGET),x86_64-pc)
	$(error "no such target:$(GRUB_TARGET)")
endif

#virtual mode
INSTALL_MODE=virtual
IMG_SIZE=512
IMG_FILE=gminios.img
ISO_FILE=gminios.iso

# basic dir and file
INC_DIRS=include/ include/arch/$(ARCH)/ include/arch/
KERNEL_SRC_DIRS=arch/$(ARCH) kernel driver fs
LIB_SRC_DIR=lib
OBJDIR=obj
KERNEL_FILE=$(OBJDIR)/kernel.bin
LIB_FILE=$(OBJDIR)/libgminios.a

# toolchain flags
CC=gcc
LD=ld
ASM=as
AR=ar
PERL=perl
CFLAGS= -c -mno-sse -mno-mmx -fno-builtin -ffreestanding -fno-stack-protector -fno-pic -Wall -Wextra -g -std=gnu99 $(addprefix -I,$(INC_DIRS))
CFLAGS+= -DARCH=$(ARCH)
LDFLAGS = -nostdlib -T $(OBJDIR)/$(PROJECT_DIR)/kernel.ld
ASMFLAGS = $(addprefix -I,$(INC_DIRS))

ifeq ($(ARCH), i386)
CFLAGS += -m32
ASMFLAGS += --32
LDFLAGS += -m elf_i386
else
ASMFLAGS += --64
endif

# qemu
QEMU=qemu-system-$(ARCH)

ifeq ($(BOOT_TYPE), pc)
QEMU_BIOS_OPT=
else
QEMU_BIOS_OPT=-bios /usr/share/qemu/OVMF.fd
endif

# disk & grub
ifeq ($(BOOT_TYPE),pc)
MK_DISK=fdisk
PARTITION_CONF=$(PROJECT_DIR)/partition_msdos.txt
else ifeq ($(BOOT_TYPE),efi)
MK_DISK=gdisk
PARTITION_CONF=$(PROJECT_DIR)/partition_gpt.txt
endif

# install
ROOT_PART=1
GRUB_PART=2

ifeq ($(BOOT_TYPE),efi)
ROOT_PART=2
GRUB_PART=3
endif