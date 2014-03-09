
#ifndef KR_COMMON_H
#define KR_COMMON_H

#define KR_NETLINK_USER 29

/**
 * Netlink commands for communication between clients and the kernel module.
 * Commands are send as raw bytes, with the first byte being the command ID.
 */

/* no operation, reserved */

#define KR_COMMAND_NOP    0

/**
 * Open the database.
 *  client->server data: DB path as null terminated string
 *  response data: 32-bit client id
 */
#define KR_COMMAND_OPEN   1

/**
 * Close the database
 */
#define KR_COMMAND_CLOSE  2
#define KR_COMMAND_SET    3
#define KR_COMMAND_GET    4

#endif
