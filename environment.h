#pragma once

#include "gc.h"

#include <string>
#include <map>

namespace Lisp {
  class Object;

  class Environment : public GCObject {
    typedef std::string key;

    std::map<key, Object*> locals;

    Environment *parent, *child;
    Environment *lexical_parent;

    bool exists_local(key &name);

    Environment* get_env_by_name(key &name);
  public:

    Environment() : parent(nullptr), child(nullptr), lexical_parent(nullptr) {}

    void set(key &name, Object* val);
    Object* get(key &name);

    Environment* down_env(Environment *new_env);
    Environment* up_env();

    void set_lexical_parent(Environment *alexical_parent);

    void mark();
  };
}
