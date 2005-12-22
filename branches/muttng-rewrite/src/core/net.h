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

#define MI_MAY_BE_IRREVERSIBLE  (1<<0)

int net_host_to_local (buffer_t* dst, buffer_t* src,
                       const char* local, unsigned short flags);

int net_local_to_host (buffer_t* dst, buffer_t* src, const char* local);

/**
 * Get libidn version.
 * @param dst Optional destination buffer.
 * @return Wether compiled with libidn support or not.
 */
int net_idna_version (buffer_t* dst);

#ifdef __cplusplus
}
#endif

#endif
