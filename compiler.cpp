#include "compiler.h"

#define EACH_CONS(var, init) for(Cons* var = regard<Cons>(init) ; typeid(*var) != typeid(Nil) ; var = (Cons*)regard<Cons>(var)->cdr)

namespace Lisp {
  Compiler::Compiler() : context(llvm::getGlobalContext()), module(new llvm::Module("top", context)), builder(llvm::IRBuilder<>(context)) {
    // int main()
    auto *funcType = llvm::FunctionType::get(builder.getInt32Ty(), false);
    current_func = mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);

    main_entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
    builder.SetInsertPoint(main_entry);

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

    root_env = cur_env = new Environment();
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

  llvm::Value* Compiler::compile_exprs(Cons *exprs) {
    llvm::Value *result = nullptr;
    EACH_CONS(expr, exprs) {
      result = compile_expr(expr->get(0));
    }
    return result;
  }

  llvm::Value* Compiler::compile_expr(Object* obj) {
    std::type_info const & id = typeid(*obj);
    if(id == typeid(Cons)) {
      auto list = (Cons*)obj;
      auto name = regard<Symbol>(list->get(0))->value;
      if(name == "print") {
        // TODO: list->get(1)の型を予め決定しておく
        // auto str = regard<String>(list->get(1))->value;
        auto str = compile_expr(list->get(1));

        builder.CreateCall(putsFunc, str);
        return nullptr; // TODO: 空のconsを返す
      }
      else if(name == "printn") {
        auto num = compile_expr(list->get(1));
        builder.CreateCall(printnFunc, num);
        return nullptr; // TODO: 空のconsを返す
      }
      else if(name == "setq") {
        auto val = compile_expr(list->get(2));
        auto val_name = regard<Symbol>(list->get(1))->value;
        // TODO: 型をInt以外使えるようにする
        auto var_pointer = builder.CreateAlloca(builder.getInt32Ty(), nullptr, val_name);
        builder.CreateStore(val, var_pointer);
        cur_env->set(val_name, var_pointer);
        return val;
      }
      else if(name == "defun") {
        auto func_name = regard<Symbol>(list->get(1))->value;
        auto args = regard<Cons>(list->get(2));
        auto body = regard<Cons>(list->tail(3));

        // TODO: 引数、返り値ともにint以外に対応させる
        std::vector<llvm::Type*> llvm_args;
        EACH_CONS(arg, args) {
          llvm_args.push_back(builder.getInt32Ty());
        }
        llvm::ArrayRef<llvm::Type*> llvm_args_ref(llvm_args);
        auto func_type = llvm::FunctionType::get(builder.getInt32Ty(), llvm_args_ref, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, func_name, module);

        Environment *env = new Environment();

        auto arg_itr = func->arg_begin();
        EACH_CONS(arg, args) {
          auto arg_name = regard<Symbol>(arg->get(0))->value;
          arg_itr->setName(arg_name);

          arg_itr++;
        }

        cur_env->set(func_name, func);

        cur_env = cur_env->down_env(env);

        auto entry = llvm::BasicBlock::Create(context, "entrypoint", func);
        builder.SetInsertPoint(entry);

        auto &vs_table = func->getValueSymbolTable();
        EACH_CONS(arg, args) {
          auto arg_name = regard<Symbol>(arg->get(0))->value;
          auto alloca = builder.CreateAlloca(builder.getInt32Ty(), 0, arg_name);
          builder.CreateStore(vs_table.lookup(arg_name), alloca);
          env->set(arg_name, alloca);
        }

        auto prev_func = current_func;
        current_func = func;

        auto result = compile_exprs(body);
        builder.CreateRet(result);

        current_func = prev_func;

        cur_env = cur_env->up_env();

        builder.SetInsertPoint(main_entry);
      }
      else if(name == "defmacro") {
      }
      else if(name == "atom") {
      }
      else if(name == "+") {
        auto n1 = compile_expr(list->get(1));
        auto n2 = compile_expr(list->get(2));
        return builder.CreateAdd(n1, n2);
      }
      else if(name == "-") {
        auto n1 = compile_expr(list->get(1));
        auto n2 = compile_expr(list->get(2));
        return builder.CreateSub(n1, n2);
      }
      else if(name == "*") {
        auto n1 = compile_expr(list->get(1));
        auto n2 = compile_expr(list->get(2));
        return builder.CreateMul(n1, n2);
      }
      else if(name == "=") {
        auto n1 = compile_expr(list->get(1));
        auto n2 = compile_expr(list->get(2));
        return builder.CreateICmpEQ(n1, n2);
      }
      else if(name == ">") {
        auto n1 = compile_expr(list->get(1));
        auto n2 = compile_expr(list->get(2));
        return builder.CreateICmpSGT(n1, n2);
      }
      else if(name == "mod") {
      }
      else if(name == "let") {
      }
      else if(name == "lambda") {
      }
      else if(name == "progn") {
        return compile_exprs(list->tail(1));
      }
      else if(name == "cond") {
        // TODO: n(>=2)の条件に対応
        auto condBB = llvm::BasicBlock::Create(module->getContext(), "cond", current_func);
        builder.CreateBr(condBB);
        builder.SetInsertPoint(condBB);

        auto cond = compile_expr(regard<Cons>(list->get(1))->get(0));
        auto thenBB = llvm::BasicBlock::Create(module->getContext(), "then", current_func);
        auto elseBB = llvm::BasicBlock::Create(module->getContext(), "else");
        auto mergeBB = llvm::BasicBlock::Create(module->getContext(), "endcond");

        builder.CreateCondBr(cond, thenBB, elseBB);

        // then
        builder.SetInsertPoint(thenBB);

        auto thenValue = compile_expr(regard<Cons>(list->get(1))->get(1));
        builder.CreateBr(mergeBB);

        thenBB = builder.GetInsertBlock();

        // else
        current_func->getBasicBlockList().push_back(elseBB);
        builder.SetInsertPoint(elseBB);

        auto elseValue = compile_exprs(regard<Cons>(list->get(2)));
        builder.CreateBr(mergeBB);

        elseBB = builder.GetInsertBlock();

        // endif
        current_func->getBasicBlockList().push_back(mergeBB);
        builder.SetInsertPoint(mergeBB);

        // TODO:
        if (!thenValue) {
          return elseValue;
        } else if (!elseValue) {
          return thenValue;
        } else {
          // auto phi = builder.CreatePHI(builder.getInt8PtrTy(), 2, "iftmp");
          auto phi = builder.CreatePHI(builder.getInt32Ty(), 2, "iftmp");
          phi->addIncoming(thenValue, thenBB);
          phi->addIncoming(elseValue, elseBB);

          return phi;
        }
      }
      else if(name == "cons") {
      }
      else if(name == "list") {
      }
      else {
        auto func = cur_env->get(name);

        if (!func) {
          throw std::logic_error("undefined function: " + name);
        }

        // TODO: 対象の関数の引数情報などを保存してチェックする
        auto args = list->tail(1);
        std::vector<llvm::Value*> callee_args;
        EACH_CONS(arg, args) {
          callee_args.push_back(compile_expr(arg->get(0)));
        }
        llvm::ArrayRef<llvm::Value*> callee_args_ref(callee_args);

        return builder.CreateCall((llvm::Function*)func, callee_args_ref);

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
      auto var_name = regard<Symbol>(obj)->value;
      auto var = cur_env->get(var_name);
      if (!var) {
        throw std::logic_error("undefined variable: " + var_name);
      }
      return builder.CreateLoad(var);
    }
    else if(id == typeid(String)) {
      return builder.CreateGlobalStringPtr(regard<String>(obj)->value.c_str());
    }
    else if(id == typeid(Integer)) {
      return builder.getInt32(regard<Integer>(obj)->value);
    } else {
      throw std::logic_error("unknown expr: " + obj->lisp_str());
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
