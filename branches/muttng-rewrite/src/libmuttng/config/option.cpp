#include "option.h"

Option::Option(const char* name_, const char* init_) : name(name_),init(init_) {}
Option::~Option(){}
const char* Option::getName() { return name; }
const char* Option::getInit() { return init; }
