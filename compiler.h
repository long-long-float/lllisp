#pragma once

namespace Lisp {
  class Compiler {
  public:
    Compiler() : context(llvm::getGlobalContext()), module(new llvm::Module("top", context)), builder(llvm::IRBuilder<>(context)) {
      // int main()
      auto *funcType = llvm::FunctionType::get(builder.getInt32Ty(), false);
      mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);

      // int puts(char*)
      std::vector<llvm::Type*> putsArgs;
      putsArgs.push_back(builder.getInt8PtrTy());
      llvm::ArrayRef<llvm::Type*> putsArgsRef(putsArgs);
      auto *putsType = llvm::FunctionType::get(builder.getInt32Ty(), putsArgsRef, false);
      putsFunc = module->getOrInsertFunction("puts", putsType);
    }

    ~Compiler() {
      delete module;
    }

    std::string compile(std::vector<Object*> &ast) {
      for (auto &object : ast) {
        object->compile_expr(object);
      }
      return "";
    }

  private:
    llvm::LLVMContext &context;
    llvm::Module *module;
    llvm::IRBuilder<> builder;

    llvm::Function *mainFunc;
    llvm::Constant *putsFunc;

    void compile_expr(Object* expr) {
      std::type_info const & id = typeid(*obj);
      if(id == typeid(Cons)) {
        auto list = (Cons*)obj;
        auto name = regard<Symbol>(list->get(0))->value;
        if(name == "print") {

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
        }
      }
      else if(id == typeid(Symbol)) {
      }
    }
  }

  std::string compile(std::vector<Object*> &ast) {
    Compiler c;
    return c.compile(ast);
  }
}
