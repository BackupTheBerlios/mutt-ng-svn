/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef rfc822_h
#define rfc822_h

/* possible values for RFC822Error */
enum {
  ERR_MEMORY = 1,
  ERR_MISMATCH_PAREN,
  ERR_MISMATCH_QUOTE,
  ERR_BAD_ROUTE,
  ERR_BAD_ROUTE_ADDR,
  ERR_BAD_ADDR_SPEC
};

typedef struct address_t {
#ifdef EXACT_ADDRESS
  char *val;                    /* value of address as parsed */
#endif
  char *personal;               /* real name of address */
  char *mailbox;                /* mailbox and host address */
  int group;                    /* group mailbox? */
  struct address_t *next;
} ADDRESS;

void rfc822_free_address (ADDRESS **);
void rfc822_qualify (ADDRESS *, const char *);
ADDRESS *rfc822_parse_adrlist (ADDRESS *, const char *s);
ADDRESS *rfc822_cpy_adr (ADDRESS * addr);
ADDRESS *rfc822_cpy_adr_real (ADDRESS * addr);
ADDRESS *rfc822_append (ADDRESS ** a, ADDRESS * b);
void rfc822_write_address (char *, size_t, ADDRESS *, int);
void rfc822_write_address_single (char *, size_t, ADDRESS *, int);
void rfc822_free_address (ADDRESS ** addr);
void rfc822_cat (char *, size_t, const char *, const char *);

extern int RFC822Error;
extern const char *RFC822Errors[];

#define rfc822_error(x) RFC822Errors[x]
#define rfc822_new_address() calloc(1,sizeof(ADDRESS))

#endif /* rfc822_h */
