/**
 * @file core/alloca.h
 * @brief Interface: System-independent @c alloca() header inclusion.
 */
#ifndef FREEBSD
#include <alloca.h>
#else
#include <stdlib.h>
#endif
