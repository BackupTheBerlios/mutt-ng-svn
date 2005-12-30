/** @ingroup core_rx */
/**
 * @file core/rx.c
 * @brief Implementation: Regular expressions
 */
#include <string.h>

#include "core_features.h"

#include "rx.h"
#include "mem.h"
#include "str.h"
#include "buffer.h"
#include "intl.h"
#include "alloca.h"

#include <pcre.h>

typedef struct pcreregex_t {
  pcre* rx;
  pcre_extra* extra;
} pcreregex_t;

static int pcre_flags (int flags) {
  int f = PCRE_EXTENDED;
  if ((flags & RX_NO_CASE)) f|=PCRE_CASELESS;
  if ((flags & RX_NEWLINE)) f|=PCRE_MULTILINE;
  return f;
}

rx_t *rx_compile (const char* pattern, buffer_t* error, int flags) {
  int f = 0, erroff=0;
  rx_t *pp = mem_calloc (1, sizeof (rx_t));
  const char* errptr = NULL;
  pcreregex_t* regex;

  /* map RX_* flags to PCRE_* flags */
  f = pcre_flags(flags);

  pp->pattern = str_dup (pattern);
  regex = mem_calloc(1,sizeof(pcreregex_t));
  pp->rx = (void*)regex;
  if (!(regex->rx = pcre_compile(NONULL(pattern),0,&errptr,&erroff,NULL))) {
    if (error) {
      buffer_add_str(error,_("Error in pattern: \""),-1);
      buffer_add_str(error,errptr,-1);
      buffer_add_str(error,_("\" at '"),-1);
      buffer_add_str(error,pattern+erroff,-1);
      buffer_add_ch(error,'\'');
    }
    rx_free (pp);
    mem_free (&pp);
  }
  return pp;
}

void rx_free (rx_t* rx) {
  pcreregex_t* regex;
  if (!rx)
    return;
  mem_free(&rx->pattern);
  regex = (pcreregex_t*)rx->rx;
  if (regex)
    pcre_free(regex->rx);
  mem_free(&rx->rx);
}

int rx_eq (const rx_t* r1, const rx_t* r2) {
  if (!r1 || !r2)
    return !r1 && !r2;
  return str_eq (r1->pattern, r2->pattern);
}

int rx_match (rx_t* rx, const char* str) {
  pcreregex_t* regex;
  if (!rx || !rx->rx)
    return 0;
  regex = (pcreregex_t*)rx->rx;
  return pcre_exec(regex->rx,regex->extra,str,str_len(str),
                   0,0,NULL,0)==0;
}

int rx_exec (rx_t* rx, const char* str, rx_match_t* matches, size_t matchcnt) {
  int* m = NULL;
  size_t i,j;
  int rc;
  pcreregex_t* regex;

  for (i=0; i<matchcnt; i++)
    matches[i].start = matches[i].end = (size_t)-1;

  if (!rx || !rx->rx)
    return 0;

  regex = (pcreregex_t*)rx->rx;
  m = alloca(matchcnt*sizeof(int)*3);

  if ((rc = pcre_exec(regex->rx,regex->extra,str,str_len(str),
                      0,0,m,matchcnt*sizeof(int)*3))==0) {
    for (i=0,j=0; i<(matchcnt*3); i+=3,j++) {
      matches[j].start = (signed)m[i];
      matches[j].end = (signed)m[i+1];
    }
  }
  return rc==0;
}

int rx_version (buffer_t* dst) {
  if (!dst) return 1;
  buffer_add_str(dst,"pcre ",5);
  buffer_add_str(dst,pcre_version(),-1);
  return 1;
}
