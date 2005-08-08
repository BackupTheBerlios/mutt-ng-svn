/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_ALIAS_H
#define _MUTT_ALIAS_H

#include "rfc822.h"

typedef struct alias {
  struct alias *self;           /* XXX - ugly hack */
  char *name;
  ADDRESS *addr;
  struct alias *next;
  short tagged;
  short del;
  short num;
} ALIAS;

void mutt_create_alias (ENVELOPE *, ADDRESS *);
int mutt_check_alias_name (const char *, char *);
ADDRESS *mutt_get_address (ENVELOPE *, char **);
ADDRESS *mutt_lookup_alias (const char *s);
ADDRESS *mutt_expand_aliases (ADDRESS *);
void mutt_expand_aliases_env (ENVELOPE *);
void mutt_free_alias (ALIAS **);
ADDRESS *alias_reverse_lookup (ADDRESS *);
int mutt_alias_complete (char *, size_t);
int mutt_addr_is_user (ADDRESS *);
void mutt_alias_menu (char *, size_t, ALIAS *);

#endif /* !_MUTT_ALIAS_H */
