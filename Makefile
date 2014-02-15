
KDIR ?= /lib/modules/`uname -r`/build
KBUILD_CMD = make -C $(KDIR) M=$(PWD)/build

.PHONY: module clean
module:
	cp -f src/lkm.c build
	$(KBUILD_CMD) modules

clean:
	$(KBUILD_CMD) clean
