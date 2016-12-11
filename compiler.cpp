#include "compiler.h"

#include <iostream>

#define EACH_CONS(var, init) for(Cons* var = regard<Cons>(init) ; typeid(*var) != typeid(Nil) ; var = (Cons*)regard<Cons>(var)->cdr)

namespace Lisp {
  Compiler::Compiler() : context(llvm::getGlobalContext()), module(new llvm::Module("top", context)), builder(llvm::IRBuilder<>(context)) {
    // int main()
    auto *funcType = llvm::FunctionType::get(builder.getInt32Ty(), false);
    current_func = mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);

    main_entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
    builder.SetInsertPoint(main_entry);

    auto struct_type = llvm::StructType::create(context, "ilist");
    std::vector<llvm::Type*> members {
      builder.getInt32Ty(),
      llvm::PointerType::getUnqual(struct_type),
    };
    struct_type->setBody(members);
    ilist_ptr_type = llvm::PointerType::getUnqual(struct_type);

    // int puts(char*)
    std::vector<llvm::Type*> putsArgs { builder.getInt8PtrTy() };
    putsFunc = define_function("puts", putsArgs, builder.getInt32Ty());

    // ilist* cons(int32_t car, ilist *cdr) {
    auto ilist_sym = new Symbol("ilist");
    auto ilist_ptr_type = get_llvm_type(ilist_sym);
    std::vector<llvm::Type*> consArgs { builder.getInt32Ty(), ilist_ptr_type };
    consFunc = define_function("cons", consArgs, ilist_ptr_type);

    // ilist* nil()
    nilFunc = define_function("nil", std::vector<llvm::Type*>(), ilist_ptr_type);

    // bool nilq(ilist*)
    std::vector<llvm::Type*> nilqArgs { ilist_ptr_type };
    nilqFunc = define_function("nilq", std::vector<llvm::Type*>(), builder.getInt1Ty());

    // void printn(int)
    std::vector<llvm::Type*> printnArgs{ builder.getInt32Ty() };
    printnFunc = define_function("printn", printnArgs, builder.getVoidTy());

    // void printl(ilist*)
    std::vector<llvm::Type*> printlArgs{ ilist_ptr_type };
    printlFunc = define_function("printl", printlArgs, builder.getVoidTy());

    // char* itoa(int)
    std::vector<llvm::Type*> itoaArgs{ builder.getInt32Ty() };
    itoaFunc = define_function("itoa", itoaArgs, builder.getInt8PtrTy());

    root_env = cur_env = new Environment();
  }

  Compiler::~Compiler() {
    delete module;
  }

  llvm::Constant* Compiler::define_function(std::string name, std::vector<llvm::Type*> arg_types, llvm::Type* result_type) {
    llvm::ArrayRef<llvm::Type*> args_ref(arg_types);
    auto *func_type = llvm::FunctionType::get(result_type, args_ref, false);
    return module->getOrInsertFunction(name, func_type);
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

  llvm::Type* Compiler::get_llvm_type(Symbol *name) {
    if (name->value == "int") {
      return builder.getInt32Ty();
    } else if (name->value == "string") {
      return builder.getInt8PtrTy();
      // TODO: add intlist
    } else if (name->value == "ilist") {
      return ilist_ptr_type;
    } else {
      throw Error("undefined type " + name->value, name->loc);
    }
  }

  llvm::Value* Compiler::create_list(Object* values) {
    if (typeid(*values) == typeid(Nil)) {
      return builder.CreateCall(nilFunc, std::vector<llvm::Value*>());
    } else {
      auto cons = regard<Cons>(values);
      std::vector<llvm::Value*> args { compile_expr(cons->car), create_list(cons->cdr) };
      llvm::ArrayRef<llvm::Value*> args_ref(args);
      return builder.CreateCall(consFunc, args);
    }
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
        return str; // TODO: 空のconsを返す
      }
      else if(name == "printn") {
        auto num = compile_expr(list->get(1));
        builder.CreateCall(printnFunc, num);
        return num; // TODO: 空のconsを返す
      }
      else if(name == "printl") {
        auto xs = compile_expr(list->get(1));
        builder.CreateCall(printlFunc, xs);
        return xs; // TODO: 空のconsを返す
      }
      else if(name == "itoa") {
        auto num = compile_expr(list->get(1));
        return builder.CreateCall(itoaFunc, num);
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
        auto args      = regard<Cons>(list->get(2));
        auto arg_types = regard<Cons>(list->get(3));
        auto ret_type  = regard<Symbol>(list->get(4));
        auto body      = regard<Cons>(list->tail(5));

        std::vector<llvm::Type*> llvm_args;
        EACH_CONS(arg_type, arg_types) {
          llvm_args.push_back(get_llvm_type(regard<Symbol>(arg_type->get(0))));
        }
        llvm::ArrayRef<llvm::Type*> llvm_args_ref(llvm_args);
        auto func_type = llvm::FunctionType::get(get_llvm_type(ret_type), llvm_args_ref, false);
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
        auto arg_type = arg_types;
        EACH_CONS(arg, args) {
          auto arg_name = regard<Symbol>(arg->get(0))->value;
          auto alloca = builder.CreateAlloca(get_llvm_type(regard<Symbol>(arg_type->get(0))), 0, arg_name);
          builder.CreateStore(vs_table.lookup(arg_name), alloca);
          env->set(arg_name, alloca);

          arg_type = (Cons*)arg_type->cdr;
        }

        auto prev_func = current_func;
        current_func = func;

        auto result = compile_exprs(body);
        builder.CreateRet(result);

        current_func = prev_func;

        cur_env = cur_env->up_env();

        builder.SetInsertPoint(main_entry);
      }
      // else if(name == "defmacro") {
      // }
      // else if(name == "atom") {
      // }
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
        auto n1 = compile_expr(list->get(1));
        auto n2 = compile_expr(list->get(2));
        return builder.CreateSRem(n1, n2);
      }
      // else if(name == "let") {
      // }
      // else if(name == "lambda") {
      // }
      else if(name == "progn") {
        return compile_exprs(list->tail(1));
      }
      else if(name == "cond") {
        auto ret_type = regard<Symbol>(list->get(1));
        auto condBB = llvm::BasicBlock::Create(module->getContext(), "cond", current_func);
        builder.CreateBr(condBB);
        builder.SetInsertPoint(condBB);

        using BB_Value = std::pair<llvm::BasicBlock*, llvm::Value*>;

        BB_Value elseBB;
        auto mergeBB = llvm::BasicBlock::Create(module->getContext(), "endcond");

        std::vector<BB_Value> thenBBs;

        EACH_CONS(val, list->tail(2)) {
          auto cond_expr_pair = regard<Cons>(val->get(0));

          if (cond_expr_pair->size() == 1) { // else
            elseBB.second = compile_expr(cond_expr_pair->get(0));
            builder.CreateBr(mergeBB);

            // elseBB = builder.GetInsertBlock();

            // cond must be fisished with one expr
            break;

          } else { // then
            auto cond = compile_expr(cond_expr_pair->get(0));
            auto thenBB = llvm::BasicBlock::Create(module->getContext(), "then");
            auto _elseBB = llvm::BasicBlock::Create(module->getContext(), "else");
            elseBB.first = _elseBB;

            builder.CreateCondBr(cond, thenBB, _elseBB);

            // %then
            current_func->getBasicBlockList().push_back(thenBB);
            builder.SetInsertPoint(thenBB);

            auto thenValue = compile_expr(cond_expr_pair->get(1));
            thenBB = builder.GetInsertBlock();
            thenBBs.push_back(BB_Value(thenBB, thenValue));
            builder.CreateBr(mergeBB);

            // %else
            current_func->getBasicBlockList().push_back(_elseBB);
            builder.SetInsertPoint(_elseBB);

            // thenBBs[thenBBs.size() - 1] = builder.GetInsertBlock();
          }
        }

        current_func->getBasicBlockList().push_back(mergeBB);
        builder.SetInsertPoint(mergeBB);

        auto phi = builder.CreatePHI(get_llvm_type(ret_type), 2, "condtmp");
        for (auto thenBB : thenBBs) {
          phi->addIncoming(thenBB.second, thenBB.first);
        }
        phi->addIncoming(elseBB.second, elseBB.first);

        return phi;
      }
      else if(name == "cons") {
        auto car = compile_expr(list->get(1));
        auto cdr = compile_expr(list->get(2));

        std::vector<llvm::Value*> args { car, cdr };
        return builder.CreateCall(consFunc, args);
      }
      else if(name == "car") {
        auto xs = compile_expr(list->get(1));
        return builder.CreateExtractValue(builder.CreateLoad(xs), 0);
      }
      else if(name == "cdr") {
        auto xs = compile_expr(list->get(1));
        return builder.CreateExtractValue(builder.CreateLoad(xs), 1);
      }
      else if(name == "list") {
        return create_list(list->tail(1));
      }
      else if(name == "nil?") {
        auto xs = compile_expr(list->get(1));
        return builder.CreateCall(nilqFunc, xs);
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
    else if(id == typeid(Nil)) {
      return builder.CreateCall(nilFunc);
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
