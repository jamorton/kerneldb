
#ifndef KR_COMMON_H
#define KR_COMMON_H

#define KR_NETLINK_USER 29

/**
 * Netlink commands for communication between clients and the kernel module.
 * Command format:
 *   First byte: command id (below)
 *
 */

/* no operation, reserved */
#define KR_COMMAND_NOP    0
#define KR_COMMAND_OPEN   1
#define KR_COMMAND_CLOSE  2
#define KR_COMMAND_SET    3
#define KR_COMMAND_GET    4

#endif
