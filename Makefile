
#-----------------------------------------------------------
# Configuration
#-----------------------------------------------------------

MODULE_SOURCES = src/krdb.mod.c

#-----------------------------------------------------------
# Build
#-----------------------------------------------------------

KDIR ?= /lib/modules/`uname -r`/build
KBUILD_CMD = make -C $(KDIR) M=$(PWD)/build
MODULE_BUILD_SOURCES = $(patsubst src/%.c,build/%.c,$(MODULE_SOURCES))

all: build/krdb.ko

$(MODULE_BUILD_SOURCES): $(MODULE_SOURCES)
	cp -f $< $@

build/krdb.ko: $(MODULE_BUILD_SOURCES)
	$(KBUILD_CMD) modules

clean:
	$(KBUILD_CMD) clean

.PHONY: clean
