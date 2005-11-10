/**
 * @file muttng/config/global_variables.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief UI internal configuration variables
 */
#ifndef MUTTNG_CONFIG_GLOBAL_VARIABLES_H
#define MUTTNG_CONFIG_GLOBAL_VARIABLES_H

#if SET_COMMAND_CPP
/** modifier for actual storage of variables */
#define WHERE static
/** set initial value for static and none for extern */
#define INITVAL(X)      =X
#else
/** modifier for actual storage of variables */
#define WHERE extern
/** set initial value for static and none for extern */
#define INITVAL(X)
#endif

/** no comment */
WHERE const char* Foo INITVAL(NULL);

#endif /* !MUTTNG_CONFIG_GLOBAL_VARIABLES_H */
