#include "object.h"
#include "environment.h"

#include <typeinfo>

namespace Lisp {
  std::string String::lisp_str() { return '"' + value + '"'; }

  std::string Integer::lisp_str() { return std::to_string(value); }

  std::string Symbol::lisp_str() { return ':' + value; }

  std::string Nil::lisp_str() { return "nil"; }

  std::string T::lisp_str() { return "T"; }

  void Cons::mark() {
    Object::mark();
    car->mark(); cdr->mark();
  }

  std::string Cons::lisp_str() {
    return lisp_str_child(true);
  }

  Object* Cons::get(size_t index) {
    if(index == 0) return car;
    else {
      if(typeid(*cdr) == typeid(Cons)) {
        return ((Cons*)cdr)->get(index - 1);
      }
      else {
        //TODO: raise range error
        return nullptr;
      }
    }
  }

  int Cons::find(Object *item) {
    int index = 0;
    for(Cons* cc = this ; typeid(*cc) != typeid(Nil) ; cc = (Cons*)cc->cdr) {
      if(cc->car->lisp_str() == item->lisp_str()) return index;
      index++;
    }
    return -1;
  }

  Cons* Cons::tail(size_t index) {
    if(index <= 1) return (Cons*)cdr; //TODO: regard使う
    else return (Cons*)tail(index - 1)->cdr;
  }

  std::string Cons::lisp_str_child(bool show_bracket) {
    std::stringstream ss;

    if(show_bracket) ss << '(';

    if(typeid(*car) == typeid(Cons)) {
      ss << ((Cons*)car)->lisp_str_child(true);
    }
    else {
      ss << car->lisp_str();
    }

    if(typeid(*cdr) == typeid(Cons)) {
      ss << " " << ((Cons*)cdr)->lisp_str_child(false);
    }
    else if(typeid(*cdr) != typeid(Nil)){
      ss << " . "; // ドット対
      ss << cdr->lisp_str();
    }

    if(show_bracket) ss << ')';

    return ss.str();
  }

  std::string Lambda::lisp_str() {
    std::stringstream ss;
    ss << "(lambda " << args->lisp_str() << " " << body->lisp_str() << ")";
    return ss.str();
  }

  void Lambda::mark() {
    GCObject::mark();

    args->mark();
    body->mark();
    if(lexical_parent) lexical_parent->mark();
  }

  Object* Macro::expand_rec(Cons* src_args, Object* cur_body) {
    const std::type_info& typei = typeid(*cur_body);
    if(typei == typeid(Symbol)) {
      auto name = (Symbol*)cur_body;
      auto index  = args->find(name);
      return index != -1 ? src_args->get(index) : name;
    }
    else if(typei == typeid(Cons)) {
      auto cons = (Cons*)cur_body;
      return new Cons(expand_rec(src_args, cons->car), expand_rec(src_args, cons->cdr));
    }
    //TODO: 他の型にも対応
    return cur_body;
  }

  Object* Macro::expand(Cons* src_args) {
    return expand_rec(src_args, body);
  }

  void Macro::mark() {
    GCObject::mark();

    args->mark();
    body->mark();
  }

  std::string Macro::lisp_str() {
    std::stringstream ss;
    ss << "(macro " << args->lisp_str() << " " << body->lisp_str() << ")";
    return ss.str();
  }
}
