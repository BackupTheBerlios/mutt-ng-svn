/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Generic Config Option
 */
#include "option.h"

Option::Option(const char* name_, const char* init_) : name(name_),init(init_) {}
Option::~Option(){}
const char* Option::getName() { return name; }
const char* Option::getInit() { return init; }
