/*
 * Copyright notice from original mutt:
 * Copyright (C) 2004 g10 Code GmbH
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "crypt-mod.h"
#include "lib/mem.h"

/* A type an a variable to keep track of registered crypto modules. */
typedef struct crypt_module *crypt_module_t;

struct crypt_module {
  crypt_module_specs_t specs;
  crypt_module_t next, *prevp;
};

static crypt_module_t modules;

/* Register a new crypto module. */
void crypto_module_register (crypt_module_specs_t specs)
{
  crypt_module_t module_new = safe_malloc (sizeof (*module_new));

  module_new->specs = specs;
  module_new->next = modules;
  if (modules)
    modules->prevp = &module_new->next;
  modules = module_new;
}

/* Return the crypto module specs for IDENTIFIER.  This function is
   usually used via the CRYPT_MOD_CALL[_CHECK] macros. */
crypt_module_specs_t crypto_module_lookup (int identifier)
{
  crypt_module_t module = modules;

  while (module && (module->specs->identifier != identifier))
    module = module->next;

  return module ? module->specs : NULL;
}
