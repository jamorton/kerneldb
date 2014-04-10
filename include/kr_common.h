
#ifndef KR_COMMON_H
#define KR_COMMON_H

#define KR_NETLINK_USER 29

/**
 * Netlink commands for communication between clients and the kernel module.
 */

/* no operation, reserved */
#define KR_COMMAND_NOP    0x11

/**
 * Open the database.
 *  client->server data: DB path as null terminated string
 *  response data: 32-bit client id
 */
#define KR_COMMAND_OPEN   0x20

/**
 * Close the database
 */
#define KR_COMMAND_CLOSE  0x21

#define KR_COMMAND_PUT    0x30
#define KR_COMMAND_GET    0x31

/**
 * Run the in-kernel-module benchmark
 * data: DB path as null terminated string.
 */
#define KR_COMMAND_BENCH  0x40

#endif
