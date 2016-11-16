#ifndef PROJECT_INTERMEDIATE_H
#define PROJECT_INTERMEDIATE_H

#include <vector>
#include <ostream>
#include <string>
#include <queue>
#include "macros.h"
#include "stringtab.h"

using std::vector;
using std::ostream;
using std::string;
using std::function;
using std::shared_ptr;
using std::priority_queue;


namespace tac {

class OperandImpl {
 public:
  OperandImpl() {}

  virtual void Serialize(ostream& s) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(OperandImpl);
};

using Operand = shared_ptr<OperandImpl>;


class InstructionImpl {
 public:
  InstructionImpl() {}

  virtual void Serialize(ostream& s) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(InstructionImpl);
};

using Instruction = shared_ptr<InstructionImpl>;


class CodeSection : public vector<Instruction> {
 public:
  void emit(const Instruction& instruction) {
    this->push_back(instruction);
  }

  void Serialize(ostream& s);

 private:
};


class ImmediateImpl : public OperandImpl {
 protected:
  ImmediateImpl() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(ImmediateImpl);
};

using Immediate = shared_ptr<ImmediateImpl>;


class NonImmediateImpl : public OperandImpl {
 protected:
  NonImmediateImpl(int offset) : offset_(offset) {}

  int offset_;
 private:
  friend class VariableFactory;
  DISALLOW_COPY_AND_ASSIGN(NonImmediateImpl);
};

using NonImmediate = shared_ptr<NonImmediateImpl>;


class AttributeImpl : public NonImmediateImpl {
 public:
  AttributeImpl(int offset) : NonImmediateImpl(offset) {}
};

using Attribute = shared_ptr<AttributeImpl>;


class FormalImpl : public NonImmediateImpl {
 public:
  FormalImpl(int offset) : NonImmediateImpl(offset) {}

};

using Formal = shared_ptr<FormalImpl>;


class VariableImpl : public NonImmediateImpl {
 public:
  void Serialize(ostream& s) override {}

 private:
  friend class VariableFactory;

  VariableImpl(int offset)
      : NonImmediateImpl(offset) {}
};

using Variable = shared_ptr<VariableImpl>;

class VariableFactory {
 public:
  static Variable alloc();

  static void free(Variable);

  static void reset();

 private:
  static int g_index_;
  DISALLOW_IMPLICIT_CONSTRUCTORS(VariableFactory);
};


class TemporaryImpl;

using Temporary = shared_ptr<TemporaryImpl>;

class TemporaryImpl : public ImmediateImpl {
 public:
  virtual void Serialize(ostream& s) override {}

 protected:
  TemporaryImpl(int index) : index_(index) {};

 private:
  friend class TemporaryFactory;

  int index_;
  DISALLOW_IMPLICIT_CONSTRUCTORS(TemporaryImpl);
};

class TemporaryFactory {
 public:
  static Temporary alloc();

  static void free(Temporary);

  static Temporary self();

  static Temporary retval();

 private:
  using TemporarySet = std::priority_queue<Temporary,
      std::vector<Temporary>, function<bool(const Temporary, const Temporary)>>;
  static TemporarySet vars_;
  static int id_;
  static Temporary self_;
  static Temporary retval_;
  DISALLOW_IMPLICIT_CONSTRUCTORS(TemporaryFactory);
};


class GlobalSymbolImpl : public ImmediateImpl {
 public:
  GlobalSymbolImpl(string name)
      : name_(name) {}

  void Serialize(ostream& s) override {}

 private:
  string name_;
};

using GlobalSymbol = shared_ptr<GlobalSymbolImpl>;


class ClassProtoImpl : public ImmediateImpl {
 public:
  ClassProtoImpl(Symbol cls)
      : class_(cls) {}

  void Serialize(ostream& s) override {}

 private:
  Symbol class_;
};

using ClassProto = shared_ptr<ClassProtoImpl>;


class ClassDispTableImpl : public ImmediateImpl {
 public:
  ClassDispTableImpl(Symbol cls)
      : class_(cls) {}

  void Serialize(ostream& s) override {}

 private:
  Symbol class_;
};

using ClassDispTable = shared_ptr<ClassDispTableImpl>;


class ConstImpl : public ImmediateImpl {
 public:
  ConstImpl() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(ConstImpl);
};

using Const = shared_ptr<ConstImpl>;


class ValueImpl : public ImmediateImpl {
 public:
  ValueImpl(int value)
      : value_(value) {}

  void Serialize(ostream& s) override {}

 private:
  int value_;
};

using Value = shared_ptr<ValueImpl>;


class StringConstImpl : public ConstImpl {
 public:
  StringConstImpl(int index) : index_(index) {}

  void Serialize(ostream& s) override {}

 private:
  int index_;
};

using StringConst = shared_ptr<StringConstImpl>;


class IntConstImpl : public ConstImpl {
 public:
  IntConstImpl(int index) : index_(index) {}

  void Serialize(ostream& s) override {}

 private:
  int index_;
};

using IntConst = shared_ptr<IntConstImpl>;


class BoolConstImpl : public ConstImpl {
 public:
  BoolConstImpl(bool val) : val_(val) {}

  void Serialize(ostream& s) override {}

 private:
  bool val_;
};

using BoolConst = shared_ptr<BoolConstImpl>;


class BinaryArithImpl : public InstructionImpl {
 public:
  BinaryArithImpl(Temporary result,
                  Immediate val_a,
                  Immediate val_b)
      : result_(result),
        val_a_(val_a),
        val_b_(val_b) {}

 protected:
  Immediate val_a_, val_b_;
  Temporary result_;
};


class LessThanImpl : public BinaryArithImpl {
 public:
  LessThanImpl(Temporary result,
               Immediate val_a,
               Immediate val_b)
      : BinaryArithImpl(result, val_a, val_b) {}

  void Serialize(ostream& s) override {}
};

using LessThan = shared_ptr<LessThanImpl>;


class LessEqualToImpl : public BinaryArithImpl {
 public:
  LessEqualToImpl(Temporary result,
                  Immediate val_a,
                  Immediate val_b)
      : BinaryArithImpl(result, val_a, val_b) {}

  void Serialize(ostream& s) override {}
};

using LessEqualTo = shared_ptr<LessEqualToImpl>;


class EqualToImpl : public BinaryArithImpl {
 public:
  EqualToImpl(Temporary result,
              Immediate val_a,
              Immediate val_b)
      : BinaryArithImpl(result, val_a, val_b) {}

  void Serialize(ostream& s) override {}
};

using EqualTo = shared_ptr<EqualToImpl>;


class AddImpl : public BinaryArithImpl {
 public:
  AddImpl(Temporary result,
          Immediate val_a,
          Immediate val_b)
      : BinaryArithImpl(result, val_a, val_b) {}

  void Serialize(ostream& s) override {}
};

using Add = shared_ptr<AddImpl>;


class SubImpl : public BinaryArithImpl {
 public:
  SubImpl(Temporary result,
          Immediate val_a,
          Immediate val_b)
      : BinaryArithImpl(result, val_a, val_b) {}

  void Serialize(ostream& s) override {}
};

using Sub = shared_ptr<SubImpl>;


class MulImpl : public BinaryArithImpl {
 public:
  MulImpl(Temporary result,
          Immediate val_a,
          Immediate val_b)
      : BinaryArithImpl(result, val_a, val_b) {}

  void Serialize(ostream& s) override {}
};

using Mul = shared_ptr<MulImpl>;


class DivImpl : public BinaryArithImpl {
 public:
  DivImpl(Temporary result,
          Immediate val_a,
          Immediate val_b)
      : BinaryArithImpl(result, val_a, val_b) {}

  void Serialize(ostream& s) override {}
};

using Div = shared_ptr<DivImpl>;


class UnaryArithImpl : public InstructionImpl {
 public:
  UnaryArithImpl(Temporary val, Temporary result)
      : val_(val), result_(result) {}

  void Serialize(ostream& s) override {}

 protected:
  Temporary val_;
  Temporary result_;
};


class IsVoidImpl : public UnaryArithImpl {
 public:
  IsVoidImpl(Temporary val, Temporary result)
      : UnaryArithImpl(val, result) {}
};

using IsVoid = shared_ptr<IsVoidImpl>;


class BoolNegImpl : public UnaryArithImpl {
 public:
  BoolNegImpl(Temporary val, Temporary result)
      : UnaryArithImpl(val, result) {}

  void Serialize(ostream& s) override {}
};

using BoolNeg = shared_ptr<BoolNegImpl>;


class ArithNegImpl : public UnaryArithImpl {
 public:
  ArithNegImpl(Temporary val, Temporary result)
      : UnaryArithImpl(val, result) {}

  void Serialize(ostream& s) override {}
};

using ArithNeg = shared_ptr<ArithNegImpl>;


class AssignImpl : public InstructionImpl {
 public:
  AssignImpl(Temporary lhs, Temporary rhs)
      : lhs_(lhs), rhs_(rhs) {}

  AssignImpl(Temporary lhs, Immediate rhs)
      : lhs_(lhs), rhs_(rhs) {}

  AssignImpl(Temporary lhs, NonImmediate rhs)
      : lhs_(lhs), rhs_(rhs) {}

  AssignImpl(NonImmediate lhs, Temporary rhs)
      : lhs_(lhs), rhs_(rhs) {}

  AssignImpl(NonImmediate lhs, Immediate rhs)
      : lhs_(lhs), rhs_(rhs) {}

  void Serialize(ostream& s) override {}

 private:
  Operand lhs_, rhs_;
};

using Assign = shared_ptr<AssignImpl>;


class LabelImpl : public ImmediateImpl {
 public:
  LabelImpl() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(LabelImpl);
};

using Label = shared_ptr<LabelImpl>;


class CodeLabelImpl : public LabelImpl,
                      public InstructionImpl {
 public:
  void Serialize(ostream& s) override {}

 protected:
  CodeLabelImpl(int index) : index_(index) {}

 private:
  friend class CodeLabelFactory;

  int index_;
  DISALLOW_COPY_AND_ASSIGN(CodeLabelImpl);
};

using CodeLabel = shared_ptr<CodeLabelImpl>;

class CodeLabelFactory {
 public:
  static CodeLabel alloc() {
    return CodeLabel(new CodeLabelImpl(g_index_++));
  }

 private:
  static int g_index_;
  DISALLOW_IMPLICIT_CONSTRUCTORS(CodeLabelFactory);
};


enum ExternalType {
  FUNC_TEST_EQUAL = 0,
  FUNC_CASE_ABORT,
  FUNC_CASE_ABORT2,
  FUNC_DISPATCH_ABORT,
};

class ExternalLabelImpl : public LabelImpl {
 public:
  ExternalLabelImpl(ExternalType type)
      : type_(type) {}

  void Serialize(ostream& s) override {}

 private:
  ExternalType type_;
};

using ExternalLabel = shared_ptr<ExternalLabelImpl>;


class ClassInitImpl : public LabelImpl {
 public:
  ClassInitImpl(Symbol cls)
      : class_(cls) {}

  void Serialize(ostream& s) override {}

 private:
  Symbol class_;
};

using ClassInit = shared_ptr<ClassInitImpl>;


class ClassMethodImpl : public LabelImpl {
 public:
  ClassMethodImpl(Symbol cls, Symbol method)
      : class_(cls), method_(method) {}

  void Serialize(ostream& s) override {}

 private:
  Symbol class_, method_;
};

using ClassMethod = shared_ptr<ClassMethodImpl>;


class JumpImpl : public InstructionImpl {
 public:
  JumpImpl(Label label)
      : label_(label) {}

  void Serialize(ostream& s) override {}

 private:
  Label label_;
};

using Jump = shared_ptr<JumpImpl>;


class CallWith1ArgImpl : public InstructionImpl {
 public:
  CallWith1ArgImpl(
      Immediate func,
      Immediate arg)
      : func_(func),
        arg_(arg) {}

  void Serialize(ostream& s) override {}

 private:
  Immediate func_;
  Immediate arg_;
};

// This operation is very dangerous
// after a function returns, all allocated temporaries
// might be overwritten, which should be kept in mind!
using CallWith1Arg = shared_ptr<CallWith1ArgImpl>;


class CallWith2ArgImpl : public InstructionImpl {
 public:
  CallWith2ArgImpl(Immediate func,
                   Temporary arg1,
                   Temporary arg2)
      : func_(func),
        arg1_(arg1),
        arg2_(arg2) {}

  void Serialize(ostream& s) override {}

 private:
  Immediate func_;
  Temporary arg1_, arg2_;
};

using CallWith2Arg = shared_ptr<CallWith2ArgImpl>;


class BranchImpl : public InstructionImpl {
 protected:
  BranchImpl(Temporary val, Label label)
      : val_(val), label_(label) {}

 private:
  Temporary val_;
  Label label_;
  DISALLOW_COPY_AND_ASSIGN(BranchImpl);
};


class BranchNonZeroImpl : public BranchImpl {
 public:
  BranchNonZeroImpl(Temporary val, Label label)
      : BranchImpl(val, label) {}

  void Serialize(ostream& s) override {}
};

using BranchNonZero = shared_ptr<BranchNonZeroImpl>;


class BranchZeroImpl : public BranchImpl {
 public:
  BranchZeroImpl(Temporary val, Label label)
      : BranchImpl(val, label) {}

  void Serialize(ostream& s) override {}
};

using BranchZero = shared_ptr<BranchZeroImpl>;


class LoadAddressImpl : public InstructionImpl {
 public:
  LoadAddressImpl(Temporary dest,
                  Immediate addr,
                  int offset)
      : dest_(dest),
        addr_(addr),
        offset_(offset) {}

  void Serialize(ostream& s) override {}

 private:
  int offset_;
  Temporary dest_;
  Immediate addr_;
};

using LoadAddress = shared_ptr<LoadAddressImpl>;


class PushImpl : public InstructionImpl {
 public:
  PushImpl(Temporary val)
      : val_(val) {}

  void Serialize(ostream& s) override {}

 private:
  Temporary val_;
};

using Push = shared_ptr<PushImpl>;


class PopImpl : public InstructionImpl {
 public:
  PopImpl(Temporary val)
      : val_(val) {}

  void Serialize(ostream& s) override {}

 private:
  Temporary val_;
};

using Pop = shared_ptr<PopImpl>;


class Return : public InstructionImpl {
 private:
  Operand ret_addr;
};


class Comment : public InstructionImpl {

};

}

#endif //PROJECT_INTERMEDIATE_H
