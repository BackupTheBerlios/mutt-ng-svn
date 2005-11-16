#ifndef CORE_ALLOCA__H
#define CORE_ALLOCA__H

/**
 * @file core/alloca.h
 * @brief Interface: System-independent @c alloca() header inclusion.
 */
#ifndef FREEBSD
#include <alloca.h>
#else
#include <stdlib.h>
#endif

#endif
