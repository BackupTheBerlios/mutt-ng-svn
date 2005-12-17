/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/config_manager.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Configuration Manager
 */
#include "config_manager.h"

#include <algorithm>

#include "libmuttng/util/hash.h"

/** internal hash tables of options */
static Hash<Option*>* Options = NULL;

/**
 * Free up memory for an Option object. Used as callback with
 * Hash::map().
 * @param option Option.
 */
static void free_option(Option** option) {
  delete *option;
  *option = NULL;
}

ConfigManager::ConfigManager(){}
ConfigManager::~ConfigManager(){}

bool ConfigManager::init() {
  if (!Options)
    Options = new Hash<Option*>(1000,false,free_option);
  return true;
}

bool ConfigManager::cleanup() {
  if (Options) {
    delete Options;
    Options = NULL;
  }
  return true;
}

bool ConfigManager::reg(Option* option) {
  init();
  if (!option)
    return false;
  if (Options->exists(option->getName())) {
    delete option;
    return false;
  }
  Options->add(option->getName(),option);
  return true;
}

bool ConfigManager::set(const char* name, buffer_t* value, buffer_t* error) {
  init();
  Option* option;
  if (!(option = Options->find(name)))
    return false;
  return option->set(value->str,error);
}

Option* ConfigManager::get(const char* name) {
  return Options->find(name);
}

int ConfigManager::get(buffer_t* dst, buffer_t* name) {
  if (!dst || !name || !name->len) return 0;
  Option* option = Options->find(name->str);
  if (!option) return 0;
  option->query(dst);
  return 1;
}

/**
 * Used as callback for Hash::map(): simply add option name to vector.
 * @param key The name of the option.
 * @param option Unused: the option object itself.
 * @param moredata Destination vector.
 */
static void add(const char* key, Option* option, std::vector<const char*>* moredata) {
  (void) option;
  moredata->push_back(key);
}

std::vector<const char*>* ConfigManager::getAll() {
  std::vector<const char*>* ret = new std::vector<const char*>;
  Options->map<std::vector<const char*>* >(add,ret);
  return ret;
}
