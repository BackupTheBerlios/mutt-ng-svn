/**
 * @file core/net.h
 * @brief Interface: Network-related routines
 *
 * This file is published under the GNU General Public License.
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

/**
 * flag for net_idn2local() specifying whether character set conversion
 * may be irreversible (e.g. from UTF-8 to IDN with non-latin1 chars to
 * local latin1)
 */
#define MI_MAY_BE_IRREVERSIBLE  (1<<0)

/**
 * Decode IDN to local character set.
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param local Local character set.
 * @param flags Flags.
 * @return Success.
 * @test net_tests::test_to_local().
 */
int net_idn2local (buffer_t* dst, buffer_t* src,
                   const char* local, unsigned short flags);

/**
 * Encode local hostname to IDN.
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param local Local character set.
 * @test net_tests::test_from_local().
 */
int net_local2idn (buffer_t* dst, buffer_t* src, const char* local);

/**
 * Get libidn version.
 * @param dst Optional destination buffer.
 * @return Wether compiled with libidn support or not.
 */
int net_idn_version (buffer_t* dst);

#ifdef __cplusplus
}
#endif

#endif
