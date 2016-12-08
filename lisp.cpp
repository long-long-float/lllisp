#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <stack>
#include <typeinfo>
#include <stdexcept>
#include <cstdlib>
#include <ctype.h>

#include <dlfcn.h>

#include "lisp.h"

#define PRINT_LINE (std::cout << "line: " << __LINE__ << std::endl)

// cons must be pure list
#define EACH_CONS(var, init) for(Cons* var = regard<Cons>(init) ; typeid(*var) != typeid(Nil) ; var = (Cons*)regard<Cons>(var)->cdr)

// for debug
using std::cout;
using std::endl;

namespace Lisp {
  class Parser {
  public:
    std::vector<Object*> parse(const std::string &code) {
      tokens = tokenize(code);

      if(false) { // NOTE: for debug
        for(auto tok : tokens) {
          std::cout << tok->str() << std::endl;
        }
      }

      std::vector<Object*> exprs;
      while(!tokens.empty()) {
        exprs.push_back(parse_expr());
      }
      return exprs;
    }

  private:
    std::list<Token*> tokens;

    inline Token* cur_token() {
      return tokens.empty() ? nullptr : tokens.front();
    }

    void consume_token() {
      if(!tokens.empty()) tokens.pop_front();
    }

    Cons* parse_list() {
      consume_token();
      if(cur_token()->type != TOKEN_SYMBOL) {
        //TODO: raise an error
      }

      auto first_cons = new Cons(new Nil(), new Nil(), cur_token()->loc);
      auto cur_cons   = first_cons;
      size_t count = 0;
      while(true) {
        if(tokens.empty()) {
          throw std::logic_error("unexpected end of code : expected ')'");
        }
        if(cur_token()->type == TOKEN_BRACKET_CLOSE) break;

        if(count != 0) {
          cur_cons->cdr = new Cons(new Nil(), new Nil(), cur_token()->loc);
          cur_cons = (Cons*)cur_cons->cdr;
        }
        cur_cons->car = parse_expr();

        count++;
      }

      return first_cons;
    }

    Object* parse_expr() {
      auto ctoken = cur_token();
      auto ttype = ctoken->type;
      if(ttype == TOKEN_BRACKET_OPEN) {
        auto ret = parse_list();
        consume_token();
        return ret;
      }

      consume_token();
      switch(ttype) {
        case TOKEN_BRACKET_OPEN: return nullptr; //not reached
        case TOKEN_SYMBOL:
          return new Symbol(ctoken->value, ctoken->loc);
        case TOKEN_STRING:
          return new String(ctoken->value, ctoken->loc);
        case TOKEN_INTEGER:
          return new Integer(ctoken->value, ctoken->loc);
        case TOKEN_NIL:
          return new Nil(ctoken->loc);
        case TOKEN_T:
          return new T(ctoken->loc);
        default:
          throw std::logic_error("unknown token: " + std::to_string(cur_token()->type));
      }
    }

    bool is_symbol(char c) {
      return c == '!' || ('#' <= c && c <= '\'') || ('*' <= c && c <= '/') ||
            ('<' <= c && c <= '@') || isalpha(c);
    }

    bool is_number(char c) {
      return isdigit(c);
    }

    std::list<Token*> tokenize(const std::string &code) {
      std::list<Token*> tokens;

      int lineno = 1, colno = 0;

      for(size_t i = 0 ; i < code.size() - 1 ; i++) { //TODO: -1を修正(EOFっぽい?)
        char ch = code[i];

        size_t ti = i; // for calcurating lineno

        if(ch == ';') { // comment
          while(code[i] != '\n' && i < code.size()) {
            i++;
          }
        }
        else if(ch == '(')
          tokens.push_back(new Token(TOKEN_BRACKET_OPEN, Location(lineno, colno)));
        else if(ch == ')')
          tokens.push_back(new Token(TOKEN_BRACKET_CLOSE, Location(lineno, colno)));
        else if(ch == '"') { // string
          i++;

          size_t token_len = 0;
          while(code[i + token_len] != '"') {
            token_len++;
            if(i + token_len >= code.size()) {
              //TODO: raise an error
            }
          }
          tokens.push_back(new Token(TOKEN_STRING, code.substr(i, token_len), Location(lineno, colno)));
          i += token_len;
        }
        else if(ch == ' ')
          ; // skip
        else if(ch == '\n') {
          lineno++;
          colno = 0;
        }
        else if((ch == '-' && is_number(code[i + 1])) || is_number(ch)) {
          size_t token_len = 0;

          if(ch == '-') token_len++;

          while(is_number(code[i + token_len])) {
            token_len++;
            if(i + token_len >= code.size()) {
              // TODO: raise an error
            }
          }

          std::string token_val = code.substr(i, token_len);
          tokens.push_back(new Token(TOKEN_INTEGER, token_val, Location(lineno, colno)));

          i += token_len - 1;
        }
        else { // symbol
          size_t token_len = 0;
          while(is_symbol(code[i + token_len])) {
            token_len++;
            if(i + token_len >= code.size()) {
              //TODO: raise an error
            }
          }

          std::string token_val = code.substr(i, token_len);
          TokenType token_type;
          if(token_val == "nil")
            token_type = TOKEN_NIL;
          else if(token_val == "t")
            token_type = TOKEN_T;
          else
            token_type = TOKEN_SYMBOL;

          if(token_type == TOKEN_SYMBOL)
            tokens.push_back(new Token(TOKEN_SYMBOL, token_val, Location(lineno, colno)));
          else
            tokens.push_back(new Token(token_type, Location(lineno, colno)));

          i += token_len - 1;
        }

        colno += i - ti + 1;
      }

      return tokens;
    }
  };

  std::vector<Object*> parse(std::string &code) {
    Parser p;
    return p.parse(code);
  }

  void clean_up() {
    for(auto obj : objects) {
      delete obj;
    }
  }
}

int main() {
  using namespace std;

  string code((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());
  auto ast = Lisp::parse(code);
  cout << ast[0]->lisp_str() << endl;

  Lisp::Compiler compiler;
  compiler.compile(ast);

  Lisp::clean_up();

  // compiler.get_module()->dump();

  // generate bitcode
  std::error_code error_info;
  llvm::raw_fd_ostream os("a.bc", error_info, llvm::sys::fs::OpenFlags::F_None);
  llvm::WriteBitcodeToFile(compiler.get_module(), os);

  return 0;
}
