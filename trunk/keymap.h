/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000,2 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef KEYMAP_H
#define KEYMAP_H

#include "mapping.h"

/* maximal length of a key binding sequence used for buffer in km_bindkey */
#define MAX_SEQ 8

/* flags for km_dokey() */
#define M_KM_UNBUFFERED 1       /* don't read from the key buffer */

/* type for key storage, the rest of mutt works fine with int type */
typedef short keycode_t;

void km_bind (char *, int, int, char *, char *);
void km_bindkey (char *, int, int);
int km_dokey (int);

/* entry in the keymap tree */
struct keymap_t {
  char *macro;                  /* macro expansion (op == OP_MACRO) */
  char *descr;                  /* description of a macro for the help menu */
  struct keymap_t *next;        /* next key in map */
  short op;                     /* operation to perform */
  short eq;                     /* number of leading keys equal to next entry */
  short len;                    /* length of key sequence (unit: sizeof (keycode_t)) */
  keycode_t *keys;              /* key sequence */
};

char *km_keyname (int);
int km_expand_key (char *, size_t, struct keymap_t *);
struct keymap_t *km_find_func (int, int);
void km_init (void);
void km_error_key (int);
void mutt_what_key (void);

enum {
  MENU_ALIAS,
  MENU_ATTACH,
  MENU_COMPOSE,
  MENU_EDITOR,
  MENU_FOLDER,
  MENU_GENERIC,
  MENU_MAIN,
  MENU_PAGER,
  MENU_POST,
  MENU_QUERY,


  MENU_PGP,
  MENU_SMIME,

#ifdef CRYPT_BACKEND_GPGME
  MENU_KEY_SELECT_PGP,
  MENU_KEY_SELECT_SMIME,
#endif

#ifdef MIXMASTER
  MENU_MIX,
#endif



  MENU_MAX
};

/* the keymap trees (one for each menu) */
extern struct keymap_t *Keymaps[];

/* dokey() records the last real key pressed  */
extern int LastKey;

extern struct mapping_t Menus[];

struct binding_t {
  char *name;                   /* name of the function */
  int op;                       /* function id number */
  char *seq;                    /* default key binding */
};

struct binding_t *km_get_table (int menu);

extern struct binding_t OpGeneric[];
extern struct binding_t OpPost[];
extern struct binding_t OpMain[];
extern struct binding_t OpAttach[];
extern struct binding_t OpPager[];
extern struct binding_t OpCompose[];
extern struct binding_t OpBrowser[];
extern struct binding_t OpEditor[];
extern struct binding_t OpQuery[];
extern struct binding_t OpAlias[];

extern struct binding_t OpPgp[];

extern struct binding_t OpSmime[];

#ifdef MIXMASTER
extern struct binding_t OpMix[];
#endif

#include "keymap_defs.h"

#endif /* KEYMAP_H */
