/** @ingroup core_rx */
/**
 * @file core/rx.c
 * @brief Implementation: Regular expressions
 */
#include "core_features.h"

#include "rx.h"

#include "mem.h"
#include "str.h"
#include "buffer.h"

#ifndef CORE_WANT_PCRE

/*
 * usual system regex
 */

#ifndef REG_WORDS
/**
 * this is a non-standard option supported by Solaris 2.5.x which allows
 * patterns of the form \<...\>
 */
#define REG_WORDS 0
#endif

rx_t *rx_compile (const char* pattern, buffer_t* error, int flags) {
  int f = REG_WORDS|REG_EXTENDED, err;
  rx_t *pp = mem_calloc (1, sizeof (rx_t));

  /* map RX_* flags to REG_* flags */
  if ((flags & RX_NO_CASE)) f|=REG_ICASE;
  if ((flags & RX_NEWLINE)) f|=REG_NEWLINE;

  pp->pattern = str_dup (pattern);
  pp->rx = mem_calloc (1, sizeof (regex_t));
  if ((err = regcomp(pp->rx, NONULL(pattern), f)) != 0) {
    if (error) {
      /*
       * error may be initialized to {0,0,0} -> add something to have
       * some space for regerror()
       */
      if (!error->size) buffer_add_ch(error,'\0');
      regerror(err,pp->rx,error->str,error->size);
    }
    rx_free (pp);
    mem_free (&pp);
  }
  return pp;
}

void rx_free (rx_t* rx) {
  if (!rx)
    return;
  mem_free(&rx->pattern);
  if (rx->rx)
    regfree(rx->rx);
  mem_free(&rx->rx);
}

int rx_eq (const rx_t* r1, const rx_t* r2) {
  if (!r1 || !r2)
    return !r1 && !r2;
  return str_eq (r1->pattern, r2->pattern);
}

int rx_exec (rx_t* rx, const char* str) {
  if (!rx || !rx->rx)
    return 0;
  return regexec(rx->rx,str,(size_t)0,(regmatch_t*)0,(int)0);
}

#else /* !CORE_WANT_PCRE */

/*
 * PCRE-based regex
 */

#endif
