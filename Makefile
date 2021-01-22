
CC       := gcc
LD       := ld
OBJCOPY  := objcopy
CFLAGS   := -Wall -Wextra -Iinclude -MMD -MP -O2
ASFLAGS  := -D__ASSEMBLY__ -Iinclude -MMD -MP

SOURCES  := $(wildcard boot/*.S) $(wildcard payload/*.c) bminstall.c
OBJECTS  := $(SOURCES:%.S=%.o)
OBJECTS  := $(OBJECTS:%.c=%.o)
DEPENDS  := $(OBJECTS:%.o=%.d)

TARGETS         := boot/boot.bin bminstall
PAYLOAD_TARGETS := payload/tlb_after_sipi payload/smp_wakeup_test
PAYLOAD_OBJECTS := $(patsubst %.c,%.o,$(wildcard payload/*.c))
PAYLOAD_OBJECTS := $(filter-out $(PAYLOAD_TARGETS:=.o),$(PAYLOAD_OBJECTS))

LDFLAGS_Darwin  := -pie -static -arch i386 -dead_strip -e _startup32
LDFLAGS_Linux   := -pie -static -melf_i386 -s --gc-sections --no-dynamic-linker \
-e startup32 -T payload/script.ld

all: $(OBJECTS) $(TARGETS) $(PAYLOAD_TARGETS)

clean:
	@rm -f $(DEPENDS) $(OBJECTS) $(TARGETS) $(PAYLOAD_TARGETS)


payload/lapic.o: CFLAGS += -mgeneral-regs-only
payload/%.o: CFLAGS += -m32 -fno-builtin -ffreestanding -Werror=format -fno-asynchronous-unwind-tables
$(PAYLOAD_TARGETS): % : %.o
$(PAYLOAD_TARGETS): $(PAYLOAD_OBJECTS)
	$(LD) $(LDFLAGS_$(shell uname -s)) $@.o $(PAYLOAD_OBJECTS) -o $@ && ./genpayload.sh $@


boot/boot.o: ASFLAGS += -m16
boot/boot.bin:boot/boot.o
	$(call remreloc,$<)
	$(OBJCOPY) -O binary -j .text $< $@

define remreloc
	@[ $(shell uname -s) = Darwin ] && \
	$(OBJCOPY) --remove-relocations .text $(1) || true
endef

-include $(DEPENDS)
.PHONY: all clean
