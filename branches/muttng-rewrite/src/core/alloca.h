/** @ingroup core_mem */
/**
 * @file core/alloca.h
 * @brief Interface: System-independent @c alloca() header inclusion.
 */
#ifndef CORE_ALLOCA__H
#define CORE_ALLOCA__H

#ifndef FREEBSD
#include <alloca.h>
#else
#include <stdlib.h>
#endif

#endif
