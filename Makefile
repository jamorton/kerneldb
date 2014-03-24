
#-----------------------------------------------------------
# Configuration
#-----------------------------------------------------------

MODULE_SOURCES = src/krdb.mod.c
MODULE_OUT = bin/krdb.ko

CLIENT_SOURCES = client/conn.c client/client.c client/main.c
CLIENT_OUT = bin/krcl
CLIENT_CFLAGS = -Wall --std=c99 -Iinclude $(shell pkg-config --cflags --libs libnl-3.0)

#-----------------------------------------------------------
# Build
#-----------------------------------------------------------

KDIR ?= /lib/modules/`uname -r`/build
KBUILD_CMD = make -C $(KDIR) M=$(PWD)/build
KBUILD_OUT = build/krdb.ko
MODULE_BUILD_SOURCES = $(patsubst src/%.c,build/%.c,$(MODULE_SOURCES))

all: $(MODULE_OUT) $(CLIENT_OUT)

$(MODULE_BUILD_SOURCES): $(MODULE_SOURCES)
	cp -f $< $@

$(MODULE_OUT): $(MODULE_BUILD_SOURCES)
	$(KBUILD_CMD) modules
	mkdir -p bin
	cp -f build/krdb.ko $@

$(CLIENT_OUT): $(CLIENT_SOURCES)
	mkdir -p bin
	gcc -o $@ $^ $(CLIENT_CFLAGS)

clean:
	$(KBUILD_CMD) clean
	rm -f $(MODULE_OUT) $(CLIENT_OUT)

.PHONY: clean
