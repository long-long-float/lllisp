#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>
#include <string>

#include "object.h"
#include "environment.h"

namespace Lisp {
  class Error : public std::logic_error {
  public:
    Error(std::string msg, Location loc) : std::logic_error(msg + " @ " + loc.str()) {}
  };

  class NameError : public Error {
  public:
    NameError(Symbol *sym) : Error("undefined local variable " + sym->value, sym->loc) {}
  };

  class TypeError : public Error  {
  public:
    TypeError(Object* obj, std::string expected_type) :
      Error(obj->lisp_str() + " is not " + expected_type, obj->loc) {}
  };

  class Compiler {
  public:
    Compiler();
    ~Compiler();

    void compile(std::vector<Object*> &ast);

    llvm::Module* getModule() { return module; }

  private:
    llvm::LLVMContext &context;
    llvm::Module *module;
    llvm::IRBuilder<> builder;

    llvm::Function *mainFunc;
    llvm::Constant *putsFunc;
    llvm::Constant *printnFunc;

    llvm::Function *current_func;

    llvm::BasicBlock *main_entry;

    llvm::Value* compile_exprs(Cons* exprs);
    llvm::Value* compile_expr(Object* obj);

    Environment *root_env, *cur_env;

    template<typename T> T* regard(Object* expr);
  };

  std::string compile(std::vector<Object*> &ast);
}
