/*
 * Copyright notice from original mutt:
 * Copyright (C) 1999-2000 Tommi Komulainen <Tommi.Komulainen@iki.fi>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _MUTT_SSL_H_
#define _MUTT_SSL_H_ 1

#include "mutt_socket.h"

#ifdef USE_SSL
int mutt_ssl_starttls (CONNECTION * conn);

extern int ssl_socket_setup (CONNECTION * conn);
#endif
#ifdef USE_GNUTLS
int mutt_gnutls_starttls (CONNECTION * conn);

extern int mutt_gnutls_socket_setup (CONNECTION * conn);
#endif
#endif /* _MUTT_SSL_H_ */
