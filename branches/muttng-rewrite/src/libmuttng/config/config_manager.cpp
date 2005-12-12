#include "config_manager.h"

#include "libmuttng/util/hash.h"

static Hash<Option*>* Options = NULL;

static void free_option(HASH_ITEMTYPE* p) {
  Option* option = (Option*) p;
  delete option;
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
  if (Options->exists(option->getName()))
    return false;
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

static void add(const char* key, Option* option, std::vector<const char*>* moredata) {
  (void) option;
  moredata->push_back(key);
}

std::vector<const char*>* ConfigManager::getAll() {
  std::vector<const char*>* ret = new std::vector<const char*>;
  Options->map<std::vector<const char*>* >(add,ret);
  return ret;
}
