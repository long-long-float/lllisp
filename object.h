#pragma once

#include "gc.h"
#include "location.h"

#include <string>
#include <cstdlib>
#include <sstream>

namespace Lisp {
  class Environment;

  class Object : public GCObject {
  public:
    Location loc;

    Object() {}
    Object(Location aloc) : loc(aloc) {}

    virtual std::string lisp_str() = 0;
  };

  class String : public Object {
  public:
    std::string value;

    String(std::string &avalue, Location aloc = Location()) : Object(aloc), value(avalue) {}

    std::string lisp_str();
  };

  class Integer : public Object {
  public:
    long value;

    Integer(std::string &avalue, Location aloc = Location())  : Object(aloc), value(std::atol(avalue.c_str())) {}
    Integer(long avalue, Location aloc = Location()) : Object(aloc), value(avalue) {}

    std::string lisp_str();
  };

  class Symbol : public Object {
  public:
    std::string value;

    Symbol(std::string avalue, Location aloc = Location()) : Object(aloc), value(avalue) {}

    std::string lisp_str();
  };

  class Nil : public Object {
  public:
    Nil(Location aloc = Location()) : Object(aloc) {}

    std::string lisp_str();
  };

  class T : public Object {
  public:
    T(Location aloc = Location()) : Object(aloc) {}

    std::string lisp_str();
  };

  class Cons : public Object {
  public:
    Object *car, *cdr;

    Cons(Object* acar, Object* acdr, Location aloc = Location())
     : Object(aloc), car(acar), cdr(acdr) {}

    void mark();

    std::string lisp_str();

    Object* get(size_t index);
    int find(Object *item);
    Cons* tail(size_t index);

    size_t size();

  private:
    std::string lisp_str_child(bool show_bracket);
  };

  class Lambda : public Object {
  public:
    Cons *args, *body;
    Environment *lexical_parent;

    Lambda(Cons *aargs, Cons *abody, Environment* alexical_parent, Location aloc = Location())
      : Object(aloc), args(aargs), body(abody), lexical_parent(alexical_parent) {}

    std::string lisp_str();

    void mark();
  };

  class Macro : public Object {
    Cons *args, *body;

    Object* expand_rec(Cons* src_args, Object* cur_body);

  public:
    Macro(Cons *aargs, Cons *abody, Location aloc = Location())
     : Object(aloc), args(aargs), body(abody) {}

    Object* expand(Cons* src_args);

    void mark();

    std::string lisp_str();
 };

}
