#include "compiler.h"

namespace Lisp {
  Compiler::Compiler() : context(llvm::getGlobalContext()), module(new llvm::Module("top", context)), builder(llvm::IRBuilder<>(context)) {
    // int main()
    auto *funcType = llvm::FunctionType::get(builder.getInt32Ty(), false);
    mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);

    auto *entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
    builder.SetInsertPoint(entry);

    // int puts(char*)
    std::vector<llvm::Type*> putsArgs;
    putsArgs.push_back(builder.getInt8PtrTy());
    llvm::ArrayRef<llvm::Type*> putsArgsRef(putsArgs);
    auto *putsType = llvm::FunctionType::get(builder.getInt32Ty(), putsArgsRef, false);
    putsFunc = module->getOrInsertFunction("puts", putsType);

    // void printn(int)
    std::vector<llvm::Type*> printnArgs{ builder.getInt32Ty() };
    llvm::ArrayRef<llvm::Type*> printnArgsRef(printnArgs);
    auto *printnType = llvm::FunctionType::get(builder.getVoidTy(), printnArgsRef, false);
    printnFunc = module->getOrInsertFunction("printn", printnType);
  }

  Compiler::~Compiler() {
    delete module;
  }

  void Compiler::compile(std::vector<Object*> &ast) {
    for (auto &object : ast) {
      compile_expr(object);
    }

    builder.CreateRet(builder.getInt32(0));
  }

  llvm::Value* Compiler::compile_expr(Object* obj) {
    std::type_info const & id = typeid(*obj);
    if(id == typeid(Cons)) {
      auto list = (Cons*)obj;
      auto name = regard<Symbol>(list->get(0))->value;
      if(name == "print") {
        auto str = regard<String>(list->get(1))->value;
        auto const_str = builder.CreateGlobalStringPtr(str.c_str());
        builder.CreateCall(putsFunc, const_str);
        return nullptr; // TODO: 空のconsを返す
      }
      else if(name == "printn") {
        auto num = compile_expr(list->get(1));
        builder.CreateCall(printnFunc, num);
        return nullptr; // TODO: 空のconsを返す
      }
      else if(name == "type") {
      }
      else if(name == "tail") {
      }
      else if(name == "setq") {
      }
      else if(name == "defmacro") {
      }
      else if(name == "atom") {
      }
      else if(name == "+") {
        auto n1 = regard<Integer>(list->get(1))->value;
        auto n2 = regard<Integer>(list->get(2))->value;
        return builder.CreateAdd(builder.getInt32(n1), builder.getInt32(n2));
      }
      else if(name == "-") {
      }
      else if(name == "*") {
      }
      else if(name == "=") {
      }
      else if(name == ">") {
      }
      else if(name == "mod") {
      }
      else if(name == "let") {
      }
      else if(name == "lambda") {
      }
      else if(name == "cond") {
      }
      else if(name == "cons") {
      }
      else if(name == "list") {
      }
      else {
        throw std::logic_error("undefined function: " + name);
        /*
        auto obj = evaluate(list->get(0));
        if(typeid(*obj) == typeid(Lambda)) {
          Lambda* lambda = (Lambda*)obj;

          Environment *env = new Environment();
          size_t index = 1;
          EACH_CONS(cc, lambda->args) {
            if(typeid(*cc->car) == typeid(Nil)) break; //TODO なんとかする
            auto name = regard<Symbol>(cc->car)->value;
            env->set(name, evaluate(list->get(index)));

            index++;
          }
          env->set_lexical_parent(lambda->lexical_parent);

          cur_env = cur_env->down_env(env);

          Object* ret;
          EACH_CONS(cc, lambda->body) {
            ret = evaluate(cc->car);
          }

          cur_env = cur_env->up_env();
          return ret;
        }
        else if(typeid(*obj) == typeid(Macro)) {
          Macro* mac = (Macro*)obj;

          auto expanded = mac->expand(list->tail(1));
          return evaluate(expanded);
        }
        else {
          throw std::logic_error("undefined function: " + name);
        }
        */
      }
    }
    else if(id == typeid(Symbol)) {
    }

    return nullptr;
  }

  template<typename T> bool instance_of(Object *expr) {
    return typeid(*expr) != typeid(T);
  }

  template<typename T> T* Compiler::regard(Object* expr) {
    if(typeid(*expr) != typeid(T)) {
      throw TypeError(expr, std::string(typeid(T).name()));
    }
    return (T*)expr;
  }
}
