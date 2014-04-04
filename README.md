
# krdb

## Project Layout

  * src/ - kernel module source files
    - src/module.c - kernel module entry point, netlink communication
    - src/user.c - keeps track of clients actively communicating with the kernel module
    - src/db.c - storage implementation. top-level put/get are implemented here
    - src/io.c - low-level block I/O system, and buffer manager
  * client/ - user-space client library
    - client/client.c - client API functions
    - client/conn.c - netlink connection management
    - client/main.c - test program. built to bin/krcl
  * include/ - common include files
    - include/kr_common.h - header file used by both client and kernel module
    - include/kr_client.h - client library API header

## Build/Run

  Note: on Ubuntu, you need the libnl-3-dev package.

  1. Run `make` to build the kernel module and client library (must be on linux)
  2. Run `sudo insmod bin/krdb.ko` to start the module
  3. Run `sudo rmmod krdb` to remove it. (Or restart)
