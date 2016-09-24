#pragma once

#include <list>

namespace Lisp {
  class GCObject;
  extern std::list<GCObject*> objects;

  class GCObject {
  public:
    bool mark_flag;

    GCObject() {
      objects.push_back(this);
    }

    virtual ~GCObject() {}

    virtual void mark() {
      mark_flag = true;
    }
  };
}
