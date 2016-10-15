#include "environment.h"
#include "object.h"

namespace Lisp {
  bool Environment::exists_local(key &name) {
    return locals.find(name) != locals.end();
  }

  Environment* Environment::get_env_by_name(key &name) {
    if(exists_local(name)) return this;
    else {
      Environment* ret;
      if(lexical_parent && (ret = lexical_parent->get_env_by_name(name))) return ret;
      if(parent && (ret = parent->get_env_by_name(name))) return ret;
      return nullptr;
    }
  }

  void Environment::set(key &name, Object* val) {
    auto env = get_env_by_name(name);
    if(env) env->locals[name] = val;
    else locals[name] = val;
  }

  Object* Environment::get(key &name) {
    auto env = get_env_by_name(name);
    if(env) return env->locals[name];
    return nullptr;
  }

  Environment* Environment::down_env(Environment *new_env) {
    child = new_env;
    new_env->parent = this;
    return new_env;
  }

  Environment* Environment::up_env() {
    auto parent_env = parent;
    parent_env->child = nullptr;
    return parent_env;
  }

  void Environment::set_lexical_parent(Environment *alexical_parent) {
    lexical_parent = alexical_parent;
  }

  void Environment::mark() {
    if(mark_flag) return;
    GCObject::mark();

    for(auto& kv : locals) {
      kv.second->mark();
    }
    if(child) child->mark();
  }
}
