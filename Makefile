PROJECT_DIR=project
include $(PROJECT_DIR)/conf.mk

INS_DEV=$(shell sudo losetup -f)

KERNEL_SRC=$(foreach dir, $(KERNEL_SRC_DIRS), $(wildcard $(dir)/*.[cS]))
KERNEL_OBJS=$(addprefix $(OBJDIR)/,$(patsubst %.c, %.o,$(patsubst %.S, %.o, $(KERNEL_SRC))))
LIB_SRC=$(wildcard $(LIB_SRC_DIR)/*.c)
LIB_OBJS=$(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(LIB_SRC)))

OBJDIRS= $(KERNEL_SRC_DIRS) $(LIB_SRC_DIR)
all: prepare $(KERNEL_FILE)

prepare:
	@mkdir -p mnt/
	@mkdir -p $(OBJDIR)/

install: $(KERNEL_FILE) $(IMG_FILE)
	@if [[ "$(INSTALL_MODE)" == "virtual" ]]; then	\
		make $(IMG_FILE) && \
		sudo losetup -P $(INS_DEV) $(IMG_FILE); 	\
	fi
	bash $(PROJECT_DIR)/install-start.sh $(INS_DEV) $(ROOT_PART) ext2 $(GRUB_PART) $(GRUB_TARGET)
	sudo cp $(KERNEL_FILE) mnt/
	sudo umount mnt
	@if [[ "$(INSTALL_MODE)" == "virtual" ]]; then	\
		sudo losetup -d $(INS_DEV); 	\
	fi


run:
	$(QEMU) -hda $(IMG_FILE) $(QEMU_BIOS_OPT) -serial file:gminios.log  -device VGA -monitor stdio 
#$(QEMU) gminios.iso -bios /usr/share/qemu/OVMF.fd -monitor stdio

# run-iso: $(ISO_FILE)
# 	$(QEMU) $(ISO_FILE) -serial file:gminios.log  -device VGA -monitor stdio $(QEMU_BIOS_OPT)

gdb: $(IMG_FILE)
	$(QEMU) $(IMG_FILE) -serial stdio -gdb tcp::1234 -S

dump_kernel:
	objdump -D $(KERNEL_FILE) > $(OBJDIR)/kernel.txt

clean: 
	rm -rf $(OBJDIR)/*

# iso:
# 	mkdir -p mnt/boot
# 	mkdir -p mnt/boot/grub
# 	cp ${KERNEL_FILE} mnt/boot/gminios.kernel
# 	cp boot/grub_iso.cfg mnt/boot/grub/grub.cfg
# 	grub-mkrescue -o gminios.iso mnt
# 	rm -r mnt/*
$(IMG_FILE): $(PARTITION_CONF)
	dd if=/dev/zero of=$@ bs=1048576 count=$(IMG_SIZE)
	$(MK_DISK) $@ < $(PARTITION_CONF)

iso:
	mkdir -p mnt/boot
	mkdir -p mnt/boot/grub
	cp ${KERNEL_FILE} mnt/kernel
	echo 'menuentry "GMiniOS" {multiboot2 /kernel boot}' > mnt/boot/grub/grub.cfg
	grub-mkrescue -o gminios.iso mnt
	rm -r mnt/*

$(LIB_FILE): $(LIB_OBJS)
	$(AR) r -o $@ $^

$(KERNEL_FILE): $(KERNEL_OBJS) $(LIB_FILE) $(OBJDIR)/$(PROJECT_DIR)/kernel.ld $(OBJDIR)/.vars.LDFLAGS
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS) $(LIB_FILE)

$(OBJDIR)/%.o: %.S $(OBJDIR)/.vars.ASMFLAGS
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
# $(ASM) $(ASMFLAGS) -o $@ $< -vv

$(OBJDIR)/%.o: %.c $(OBJDIR)/.vars.CFLAGS
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.psf
	@mkdir -p $(@D)
	objcopy -O elf32-i386 -B i386 -I binary $< $@

$(OBJDIR)/%.ld: %.ld
	@mkdir -p $(@D)
	$(CC) -v -x c -E -P $(CFLAGS) -o $@ $<

$(OBJDIR)/.vars.%: FORCE
	@echo "$($*)" | cmp -s $@ || echo "$($*)" > $@
.PRECIOUS: $(OBJDIR)/.vars.%
.PHONY: FORCE

# This magic automatically generates makefile dependencies
# for header files included from C source files we compile,
# and keeps those dependencies up-to-date every time we recompile.
# See 'mergedep.pl' for more information.
$(OBJDIR)/.deps: $(foreach dir, $(OBJDIRS), $(wildcard $(OBJDIR)/$(dir)/*.d))
	@$(PERL) mergedep.pl $@ $^

-include $(OBJDIR)/.deps

.PHONY : all clean install prepare