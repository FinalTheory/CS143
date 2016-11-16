#include <memory>
#include "instance.h"
#include "classtable.h"
#include "globals.h"
#include "intermediate.h"

extern char* curr_filename;

using std::make_shared;
using tac::Variable;
using tac::Instruction;
using tac::ExternalType;
using tac::TemporaryFactory;
using tac::VariableFactory;
using tac::CodeLabelFactory;

const char kEmptyStr[] = "";

const char kZeroStr[] = "0";

const char kClassObjTab[] = "class_objTab";

const int kDispathTableOffset = 2;

const int kWordSize = 4;

enum DispatchType {
  STATIC,
  DYNAMIC
};


namespace tac {

SymbolTable<Symbol, NonImmediate> env;

// static
int TemporaryFactory::id_ = 1;

Temporary TemporaryFactory::self_;

Temporary TemporaryFactory::retval_;

// static
Temporary TemporaryFactory::self() {
  if (!self_) { self_.reset(new TemporaryImpl(0)); }
  return self_;
};

// static
Temporary TemporaryFactory::retval() {
  if (!retval_) { retval_.reset(new TemporaryImpl(1)); }
  return retval_;
};

// static
TemporaryFactory::TemporarySet TemporaryFactory::vars_(
    [](const Temporary left, const Temporary right) {
      return left->index_ > right->index_;
    });

// static
Temporary TemporaryFactory::alloc() {
  if (vars_.size() == 0) {
    return Temporary(new TemporaryImpl(id_++));
  } else {
    auto res = vars_.top();
    vars_.pop();
    return res;
  }
}

// static
void TemporaryFactory::free(Temporary v) {
  vars_.push(v);
}

// static
int CodeLabelFactory::g_index_ = 0;

// static
int VariableFactory::g_index_ = 0;

// static
Variable VariableFactory::alloc() {
  return Variable(new VariableImpl(g_index_++));
}

// static
void VariableFactory::free(Variable v) {
  assert(v->offset_ == g_index_ - 1);
  g_index_--;
}

// static
void VariableFactory::reset() {
  g_index_ = 0;
}

void CodeSection::Serialize(ostream& s) {
  for (auto ptr: *this) {
    ptr->Serialize(s);
  }
}

}


Temporary assign_class::code(CodeSection& sec) {
  auto lhs = tac::env.lookup(name);
  auto rhs = expr->code(sec);
  sec.emit(New<tac::Assign>(lhs, rhs));
  return rhs;
}


static Temporary dispatch_impl(
    Expression expr,
    Expressions actual,
    Symbol name,
    Symbol obj_type,
    CodeSection& sec,
    DispatchType disp_type,
    int line_number) {
  auto label = CodeLabelFactory::alloc();
  // Evaluate the actual parameters
  // and push them onto stack
  for (auto i = actual->first();
       actual->more(i);
       i = actual->next(i)) {
    auto init = actual->nth(i);
    auto res = init->code(sec);
    sec.emit(New<tac::Push>(res));
    TemporaryFactory::free(res);
  }
  // Evaluate the object and save into obj
  auto obj = expr->code(sec);
  sec.emit(New<tac::BranchNonZero>(obj, label));

  // Exception handling, first allocate arguments
  auto arg1 = TemporaryFactory::alloc();
  auto arg2 = TemporaryFactory::alloc();
  // Load filename and line number info
  sec.emit(New<tac::Assign>(
      arg1, New<tac::StringConst>(
          stringtable.lookup_string(curr_filename)->get_index())));
  sec.emit(New<tac::Assign>(
      arg2, New<tac::Value>(line_number)));
  // Call the handler function
  sec.emit(New<tac::CallWith2Arg>(
      New<tac::ExternalLabel>(
          ExternalType::FUNC_DISPATCH_ABORT), arg1, arg2));
  // Free arguments
  TemporaryFactory::free(arg1);
  TemporaryFactory::free(arg2);

  // Call method
  sec.emit(label);
  // Get method offset in the dispatch table
  int offset = Globals.get_method_offset_for_class(obj_type, name);
  // Temporary to store dispatch address
  auto addr = TemporaryFactory::alloc();
  if (disp_type == DispatchType::STATIC) {
    // just load method address of this class
    sec.emit(New<tac::LoadAddress>(
        addr, New<tac::ClassDispTable>(obj_type), offset));
  } else if (disp_type == DispatchType::DYNAMIC) {
    // load dispatch table into addr
    sec.emit(New<tac::LoadAddress>(addr, obj, kDispathTableOffset));
    // load method address into addr
    sec.emit(New<tac::LoadAddress>(addr, addr, offset));
  } else {
    assert(false);
  }
  // Call this method
  sec.emit(New<tac::CallWith1Arg>(addr, obj));
  TemporaryFactory::free(addr);
  TemporaryFactory::free(obj);
  auto res = TemporaryFactory::alloc();
  // "retval" is volatile, we need to save it to new temporary
  sec.emit(New<tac::Assign>(res, TemporaryFactory::retval()));
  return res;
}


Temporary static_dispatch_class::code(CodeSection& sec) {
  return dispatch_impl(
      expr, actual, name,
      type_name, sec,
      DispatchType::STATIC,
      get_line_number());
}


Temporary dispatch_class::code(CodeSection& sec) {
  auto obj_type = expr->get_type();
  if (obj_type == SELF_TYPE) {
    obj_type = Globals.get_current_class();
  }
  return dispatch_impl(
      expr, actual, name,
      obj_type, sec,
      DispatchType::DYNAMIC,
      get_line_number());
}


Temporary cond_class::code(CodeSection& sec) {
  // Assume conditions are more likely to be false
  auto label_true = CodeLabelFactory::alloc();
  auto label_end = CodeLabelFactory::alloc();
  // evaluate condition
  auto val = pred->code(sec);
  sec.emit(New<tac::BranchZero>(val, label_true));
  auto false_val = else_exp->code(sec);
  sec.emit(New<tac::Assign>(val, false_val));
  TemporaryFactory::free(false_val);
  sec.emit(New<tac::Jump>(label_end));
  // label true:
  sec.emit(label_true);
  auto true_val = then_exp->code(sec);
  sec.emit(New<tac::Assign>(val, true_val));
  TemporaryFactory::free(true_val);
  // label end:
  sec.emit(label_end);
  return val;
}


Temporary loop_class::code(CodeSection& sec) {
  auto label_start = CodeLabelFactory::alloc();
  auto label_end = CodeLabelFactory::alloc();
  // loop start:
  sec.emit(label_start);
  // calculate loop condition
  auto val = pred->code(sec);
  // jump to end if false
  sec.emit(New<tac::BranchZero>(val, label_end));
  // val is no longer used, so free it
  TemporaryFactory::free(val);
  // evaluate loop body
  auto res = body->code(sec);
  // jump back to loop start
  sec.emit(New<tac::Jump>(label_start));
  // loop end:
  sec.emit(label_end);
  // load zero into return value
  auto zero = New<tac::StringConst>(
      inttable.lookup_string(kZeroStr)->get_index());
  sec.emit(New<tac::Assign>(res, zero));
  return res;
}


Temporary typcase_class::code(CodeSection& sec) {
  // Class information prepare
  typedef branch_class* BranchType;
  vector<BranchType> patterns;
  vector<tac::CodeLabel> labels;
  auto label_end = CodeLabelFactory::alloc();
  for (auto i = cases->first();
       cases->more(i);
       i = cases->next(i)) {
    auto cs = static_cast<branch_class*>(cases->nth(i));
    patterns.push_back(cs);
    labels.push_back(CodeLabelFactory::alloc());
  }
  auto label_abort = CodeLabelFactory::alloc();
  labels.push_back(label_abort);
  std::sort(patterns.begin(), patterns.end(),
            [](const BranchType& a, const BranchType& b) {
              return Globals.classtag[a->type_decl] >
                     Globals.classtag[b->type_decl];
            });
  // Code generation
  auto obj = expr->code(sec);
  sec.emit(New<tac::BranchNonZero>(obj, labels[0]));
  // Exception handling
  // First allocate arguments
  auto arg1 = TemporaryFactory::alloc();
  auto arg2 = TemporaryFactory::alloc();
  // Load filename and line number info
  sec.emit(New<tac::Assign>(
      arg1, New<tac::StringConst>(
          stringtable.lookup_string(curr_filename)->get_index())));
  sec.emit(New<tac::Assign>(
      arg2, New<tac::Value>(get_line_number())));
  // Call the handler function
  sec.emit(New<tac::CallWith2Arg>(
      New<tac::ExternalLabel>(
          ExternalType::FUNC_CASE_ABORT2), arg1, arg2));
  // Free arguments
  TemporaryFactory::free(arg1);
  TemporaryFactory::free(arg2);
  // Normal matching
  auto tag = TemporaryFactory::alloc();
  auto res = TemporaryFactory::alloc();
  for (auto i = 0; i < patterns.size(); i++) {
    auto cs = patterns[i];
    auto tag_min = Globals.classtag[cs->type_decl];
    auto tag_max = Globals.subclasstag_max[cs->type_decl];
    sec.emit(labels[i]);
    // Now the object still holds in "obj"
    // We should load the class tag after first label
    if (i == 0) { sec.emit(New<tac::LoadAddress>(tag, obj, 0)); }
    // Allocate a temp variable to do comparing
    auto tmp = TemporaryFactory::alloc();
    // If class tag is less than min value, we goto next label
    sec.emit(New<tac::LessThan>(
        tmp, tag, New<tac::Value>(tag_min)));
    sec.emit(New<tac::BranchNonZero>(tmp, labels[i + 1]));
    // If class tag is larger than max value, we also goto next label
    sec.emit(New<tac::LessThan>(
        tmp, New<tac::Value>(tag_max), tag));
    sec.emit(New<tac::BranchNonZero>(tmp, labels[i + 1]));
    TemporaryFactory::free(tmp);

    tac::env.enterscope();
    auto loc = VariableFactory::alloc();
    sec.emit(New<tac::Assign>(loc, obj));
    tac::env.addid(cs->name, loc);
    // Calculate result of expression
    auto tmp_res = cs->expr->code(sec);
    // Save the result
    sec.emit(New<tac::Assign>(res, tmp_res));
    // Release the result temporary
    TemporaryFactory::free(tmp_res);
    VariableFactory::free(loc);
    tac::env.exitscope();
    sec.emit(New<tac::Jump>(label_end));
  }

  TemporaryFactory::free(tag);
  TemporaryFactory::free(obj);
  return res;
}


Temporary block_class::code(CodeSection& sec) {
  auto prev = Temporary(nullptr);
  for (int i = body->first();
       body->more(i);
       i = body->next(i)) {
    // Free unused temporaries and only return the last one
    if (prev) { TemporaryFactory::free(prev); }
    prev = body->nth(i)->code(sec);
  }
  return prev;
}


Temporary let_class::code(CodeSection& sec) {
  tac::env.enterscope();
  // If there is no init-expr
  // we initialize this object with its default value
  auto loc = VariableFactory::alloc();
  if (typeid(*init) == typeid(no_expr_class)) {
    if (type_decl == Int) {
      sec.emit(New<tac::Assign>(
          loc, New<tac::StringConst>(
              inttable.lookup_string(kZeroStr)->get_index())));
    } else if (type_decl == Bool) {
      sec.emit(New<tac::Assign>(
          loc, New<tac::BoolConst>(false)));
    } else if (type_decl == Str) {
      sec.emit(New<tac::Assign>(
          loc, New<tac::StringConst>(
              stringtable.lookup_string(kEmptyStr)->get_index())));
    } else {
      // All objects are initialized to null by default
      sec.emit(New<tac::Assign>(
          loc, New<tac::Value>(0)));
    }
  } else {
    auto val = init->code(sec);
    sec.emit(New<tac::Assign>(loc, val));
  }
  tac::env.addid(identifier, loc);
  // Generate code in new scope
  auto res = body->code(sec);
  // Free the new allocated variable
  VariableFactory::free(loc);
  // and exit the scope
  tac::env.exitscope();
  return res;
}


template<typename OP>
Temporary BinaryArithFunc(Expression e1, Expression e2, CodeSection& sec) {
  auto val_a = e1->code(sec);
  auto val_b = e2->code(sec);
  auto res = TemporaryFactory::alloc();
  sec.emit(New<OP>(res, val_a, val_b));
  TemporaryFactory::free(val_a);
  TemporaryFactory::free(val_b);
  return res;
}


Temporary plus_class::code(CodeSection& sec) {
  return BinaryArithFunc<tac::Add>(e1, e2, sec);
}


Temporary sub_class::code(CodeSection& sec) {
  return BinaryArithFunc<tac::Sub>(e1, e2, sec);
}


Temporary mul_class::code(CodeSection& sec) {
  return BinaryArithFunc<tac::Mul>(e1, e2, sec);
}


Temporary divide_class::code(CodeSection& sec) {
  return BinaryArithFunc<tac::Div>(e1, e2, sec);
}


Temporary lt_class::code(CodeSection& sec) {
  return BinaryArithFunc<tac::LessThan>(e1, e2, sec);
}


Temporary leq_class::code(CodeSection& sec) {
  return BinaryArithFunc<tac::LessEqualTo>(e1, e2, sec);
}


Temporary eq_class::code(CodeSection& sec) {
  return BinaryArithFunc<tac::EqualTo>(e1, e2, sec);
}


template<typename OP>
Temporary UnaryArithFunc(Expression e1, CodeSection& sec) {
  auto val = e1->code(sec);
  auto res = TemporaryFactory::alloc();
  sec.emit(New<OP>(val, res));
  TemporaryFactory::free(val);
  return res;
}


Temporary neg_class::code(CodeSection& sec) {
  return UnaryArithFunc<tac::ArithNeg>(e1, sec);
}


Temporary comp_class::code(CodeSection& sec) {
  return UnaryArithFunc<tac::BoolNeg>(e1, sec);
}


Temporary isvoid_class::code(CodeSection& sec) {
  return UnaryArithFunc<tac::IsVoid>(e1, sec);
}


Temporary int_const_class::code(CodeSection& sec) {
  auto lhs = TemporaryFactory::alloc();
  auto rhs = New<tac::IntConst>(
      inttable.lookup_string(token->get_string())->get_index());
  sec.emit(New<tac::Assign>(lhs, rhs));
  return lhs;
}


Temporary string_const_class::code(CodeSection& sec) {
  auto lhs = TemporaryFactory::alloc();
  auto rhs = New<tac::StringConst>(
      stringtable.lookup_string(token->get_string())->get_index());
  sec.emit(New<tac::Assign>(lhs, rhs));
  return lhs;
}


Temporary bool_const_class::code(CodeSection& sec) {
  auto lhs = TemporaryFactory::alloc();
  auto rhs = New<tac::BoolConst>(bool(this->val));
  sec.emit(New<tac::Assign>(lhs, rhs));
  return lhs;
}


Temporary object_class::code(CodeSection& sec) {
  if (name == self) {
    return TemporaryFactory::self();
  } else {
    auto tmp = TemporaryFactory::alloc();
    sec.emit(New<tac::Assign>(tmp, tac::env.lookup(name)));
    return tmp;
  }
}


Temporary new__class::code(CodeSection& sec) {
  // For SELF_TYPE we need to use static dispatch
  if (type_name == SELF_TYPE) {
    auto entry = TemporaryFactory::alloc();
    auto offset = TemporaryFactory::alloc();
    // Load address of class_objTab
    sec.emit(New<tac::Assign>(
        entry, New<tac::GlobalSymbol>(kClassObjTab)));
    // Load object tag from self pointer
    sec.emit(New<tac::LoadAddress>(
        offset, TemporaryFactory::self(), 0));
    // calculate offset
    sec.emit(New<tac::Mul>(
        offset, offset, New<tac::Value>(2 * kWordSize)));
    sec.emit(New<tac::Add>(entry, entry, offset));
    TemporaryFactory::free(offset);

    auto proto_obj = TemporaryFactory::alloc();
    // Load proto object address
    sec.emit(New<tac::LoadAddress>(proto_obj, entry, 0));
    // Backup entry before we call function
    sec.emit(New<tac::Push>(entry));
    // call Object.copy
    sec.emit(New<tac::CallWith1Arg>(
        New<tac::ClassMethod>(Object, ::copy), proto_obj));
    TemporaryFactory::free(proto_obj);
    // Now the new object would be hold in "retval"
    // we should backup it as soon as possible
    auto obj = TemporaryFactory::alloc();
    sec.emit(New<tac::Assign>(obj, TemporaryFactory::retval()));
    // Restore entry after function return
    sec.emit(New<tac::Pop>(entry));
    // Load address and call Class_init function
    auto init_func = TemporaryFactory::alloc();
    sec.emit(New<tac::LoadAddress>(init_func, entry, 1));
    sec.emit(New<tac::CallWith1Arg>(init_func, obj));
    TemporaryFactory::free(init_func);
    TemporaryFactory::free(entry);
    sec.emit(New<tac::Assign>(obj, TemporaryFactory::retval()));
    return obj;
  } else {
    // Call Object.copy
    sec.emit(New<tac::CallWith1Arg>(
        New<tac::ClassMethod>(Object, ::copy),
        New<tac::ClassProto>(type_name)));
    auto obj = TemporaryFactory::alloc();
    sec.emit(New<tac::Assign>(obj, TemporaryFactory::retval()));
    // Call Class_init
    sec.emit(New<tac::CallWith1Arg>(
        New<tac::ClassInit>(type_name), obj));
    sec.emit(New<tac::Assign>(obj, TemporaryFactory::retval()));
    return obj;
  }
}


Temporary no_expr_class::code(CodeSection& sec) {
  // We do nothing here
  // This piece of code should never be reached
  assert(false);
  return Temporary(nullptr);
}
