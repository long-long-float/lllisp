#include "token.h"
#include <sstream>

namespace Lisp {
  std::string Token::str() {
    std::stringstream ss;
    ss << "Token(" << type << ", " << value << ")";
    return ss.str();
  }
}
