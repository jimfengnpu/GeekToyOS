PROJECT_DIR=project
include $(PROJECT_DIR)/conf.mk

INS_DEV=$(shell sudo losetup -f)

KERNEL_SRC=$(foreach dir, $(KERNEL_SRC_DIRS), $(wildcard $(SRCDIR)/$(dir)/*.[cS])) 
KERNEL_OBJS=$(subst $(SRCDIR)/, $(OBJDIR)/, $(patsubst %.c, %.o,$(patsubst %.S, %.o, $(KERNEL_SRC)))) $(OBJDIR)/$(PROJECT_DIR)/font.o
LIB_SRC=$(wildcard $(SRCDIR)/$(LIB_SRC_DIR)/*.c)
LIB_OBJS=$(subst $(SRCDIR)/, $(OBJDIR)/, $(patsubst %.c, %.o, $(LIB_SRC)))


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
	bash $(PROJECT_DIR)/install.sh $(INS_DEV) $(ROOT_PART) ext2 $(GRUB_PART) $(GRUB_TARGET)
	sudo cp $(KERNEL_FILE) mnt/
	sudo umount mnt
	@if [[ "$(INSTALL_MODE)" == "virtual" ]]; then	\
		sudo losetup -d $(INS_DEV); 	\
	fi

iso: $(ISO_FILE)

run:
	$(QEMU) -cdrom $(ISO_FILE) $(QEMU_OPT) -serial file:kernel.log  -device VGA -monitor stdio -d cpu_reset -D qemu.log

gdb: $(IMG_FILE)
	$(QEMU) -cdrom $(ISO_FILE) $(QEMU_OPT) -serial file:kernel.log  -device VGA -monitor stdio -d cpu_reset -D qemu.log -gdb tcp::1234 -S

dump_kernel:
	objdump -D $(KERNEL_FILE) > $(OBJDIR)/kernel.txt

clean: 
	rm -rf $(OBJDIR)/*

$(IMG_FILE): $(PARTITION_CONF) $(PROJECT_DIR)/conf.mk
	dd if=/dev/zero of=$@ bs=1048576 count=$(IMG_SIZE)
	$(MK_DISK) $@ < $(PARTITION_CONF)

$(ISO_FILE): $(KERNEL_FILE) $(PARTITION_CONF) $(PROJECT_DIR)/conf.mk
	mkdir -p mnt/boot
	mkdir -p mnt/boot/grub
	cp ${KERNEL_FILE} mnt/boot/kernel.bin
	cp $(PROJECT_DIR)/grub_iso.cfg mnt/boot/grub/grub.cfg
	grub-mkrescue $(GRUB_DATA) -o $(ISO_FILE) mnt
	rm -r mnt/*

$(LIB_FILE): $(LIB_OBJS)
	$(AR) r -o $@ $^

$(KERNEL_FILE): $(KERNEL_OBJS) $(LIB_FILE) $(OBJDIR)/$(PROJECT_DIR)/kernel.ld $(OBJDIR)/.vars.LDFLAGS
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS) $(LIB_FILE)

$(OBJDIR)/%.o: $(SRCDIR)/%.S $(OBJDIR)/.vars.ASMFLAGS
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<
# $(ASM) $(ASMFLAGS) -o $@ $< -vv

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)/.vars.CFLAGS
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.psf
	@mkdir -p $(@D)
	cp $< $(@D)
	cd $(@D); objcopy $(OBJCOPYFLAGS) -I binary $(notdir $<) $(notdir $@)

$(OBJDIR)/%.ld: %.ld
	@mkdir -p $(@D)
	$(CC) -x c -E -P $(CFLAGS) -o $@ $<

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