#pragma once

#include <string>

namespace Lisp {
  struct Location {
    int lineno, colno;

    Location() : lineno(-1), colno(-1) {}
    Location(size_t l, size_t c) : lineno(l), colno(c) {}

    std::string str() {
      return "line: " + (lineno >= 0 ? std::to_string(lineno) : "?") +
             " col: " + (colno >= 0 ? std::to_string(colno) : "?");
    }
  };
}
