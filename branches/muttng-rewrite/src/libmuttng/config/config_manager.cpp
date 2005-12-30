/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/config_manager.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Configuration Manager
 */
#include "config_manager.h"
#include "core/str.h"
#include "libmuttng/util/hash.h"

/** internal hash table of options */
static Hash<Option*>* Options = NULL;
/** internal hash table of features */
static Hash<void*>* Features = NULL;

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
  if (!Features)
    Features = new Hash<void*>(20,true,NULL);
  return true;
}

bool ConfigManager::cleanup() {
  if (Options) {
    delete Options;
    Options = NULL;
  }
  if (Features) {
    delete Features;
    Features = NULL;
  }
  return true;
}

Option* ConfigManager::regOption(Option* option) {
  init();
  if (!option)
    return NULL;
  if (Options->exists(option->getName())) {
    delete option;
    return NULL;
  }
  Options->add(option->getName(),option);
  return option;
}

void ConfigManager::regFeature(const char* name) {
  init();
  if (!name && !*name) return;
  size_t len = str_len(name)+8;
  char* p = (char*)mem_malloc(len+1);
  memcpy(p,"feature_",8);
  memcpy(p+8,name,len-8);
  p[len] = '\0';
  Features->add(p,NULL);
  mem_free(&p);
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

std::vector<const char*>* ConfigManager::getOptions() {
  init();
  return Options->getKeys();
}

std::vector<const char*>* ConfigManager::getFeatures() {
  init();
  return Features->getKeys();
}
