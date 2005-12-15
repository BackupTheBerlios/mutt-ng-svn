/**
 * @file core/net.h
 * @brief Interface: Network-related routines
 */
#ifndef CORE_NET_H
#define CORE_NET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "buffer.h"

/**
 * Obtain host's DNS domain name from /etc/resolv.conf
 * @param dst Destination buffer.
 * @return Success.
 */
int net_dnsdomainname (buffer_t* dst);

#ifdef __cplusplus
}
#endif

#endif
