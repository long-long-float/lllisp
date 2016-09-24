#pragma once

#include "gc.h"
#include "location.h"
#include <string>

namespace Lisp {
  enum TokenType{
    TOKEN_BRACKET_OPEN,
    TOKEN_BRACKET_CLOSE,
    TOKEN_SYMBOL,
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_NIL,
    TOKEN_T,
  };

  class Object;

  class Token : public GCObject {
  public:
    TokenType type;
    std::string value;
    Location loc;

    Token(TokenType atype, Location aloc)
     : type(atype), value(std::string()), loc(aloc) {}
    Token(TokenType atype, std::string avalue, Location aloc)
     : type(atype), value(avalue), loc(aloc) {}

    std::string str();
  };
}
