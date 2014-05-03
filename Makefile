
#-----------------------------------------------------------
# Configuration
#-----------------------------------------------------------

MODULE_SOURCES = src/module.c src/db.c src/io.c src/bucket.c src/bench.c
MODULE_OUT = bin/krdb.ko

LIB_SOURCES = client/conn.c client/client.c
LIB_OUT = bin/libkrdb.so
LIB_CFLAGS = -Wall -Iinclude -O2 -fPIC -shared $(shell pkg-config --cflags --libs libnl-3.0)

CLIENT_SOURCES = client/main.c
CLIENT_OUT = bin/krcl
CLIENT_CFLAGS = -Wall -Iinclude -O2 -Lbin -lkrdb

#-----------------------------------------------------------
# Build
#-----------------------------------------------------------

KDIR ?= /lib/modules/`uname -r`/build
KBUILD_CMD = make -C $(KDIR) M=$(PWD)/build
KBUILD_OUT = build/krdb.ko
MODULE_BUILD_SOURCES = $(patsubst src/%.c,build/%.c,$(MODULE_SOURCES))

all: $(MODULE_OUT) $(LIB_OUT) $(CLIENT_OUT)

$(MODULE_OUT): $(MODULE_SOURCES)
	cp -f $(MODULE_SOURCES) build/
	$(KBUILD_CMD) modules
	mkdir -p bin
	cp -f build/krdb.ko $@

$(LIB_OUT): $(LIB_SOURCES)
	mkdir -p bin
	gcc -o $@ $^ $(LIB_CFLAGS)

$(CLIENT_OUT): $(CLIENT_SOURCES)
	mkdir -p bin
	gcc -o $@ $^ $(CLIENT_CFLAGS)

install: $(MODULE_OUT) $(LIB_OUT) $(CLIENT_OUT)
	insmod $(MODULE_OUT)
	install $(LIB_OUT) /usr/local/lib
	install $(CLIENT_OUT) /usr/local/bin
	ldconfig

clean:
	$(KBUILD_CMD) clean
	rm -f $(MODULE_OUT) $(LIB_OUT) $(MODULE_BUILD_SOURCES)

.PHONY: clean install
