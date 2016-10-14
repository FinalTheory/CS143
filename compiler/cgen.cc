//**************************************************************
//
// Code generator SKELETON
//
// Read the comments carefully. Make sure to
//    initialize the base class tags in
//       `ClassTable::ClassTable'
//
//    Add the label for the dispatch tables to
//       `IntEntry::code_def'
//       `StringEntry::code_def'
//       `BoolConst::code_def'
//
//    Add code to emit everyting else that is needed
//       in `ClassTable::code'
//
//
// The files as provided will produce code to begin the code
// segments, declare globals, and emit constants.  You must
// fill in the rest.
//
//**************************************************************
#include <typeinfo>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include "globals.h"
#include "emit.h"
#include "cgen.h"
#include "classtable.h"

using std::max;
using std::map;
using std::pair;
using std::vector;
using std::stringstream;


#define CODE_START \
  if (cgen_debug) s << "# Code start for " << typeid(*this).name() << endl

#define CODE_END \
  if (cgen_debug) s << "# Code end for " << typeid(*this).name() << endl

extern char* curr_filename;

extern ClassTable* classtable;

//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);


//*********************************************************
//
// Define method for code generation
//
// This is the method called by the compiler driver
// `cgtest.cc'. cgen takes an `ostream' to which the assembly will be
// emmitted, and it passes this and the class list of the
// code generator tree to the constructor for `ClassTable'.
// That constructor performs all of the work of the code
// generator.
//
//*********************************************************

void program_class::cgen(ostream& os) {
  // spim wants comments to start with '#'
  os << "# start of generated code\n";

  classtable->code(os);

  os << "\n# end of generated code\n";
}

///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Cool has three kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// All string constants are listed in the global "stringtable" and have
// type StringEntry.  StringEntry methods are defined both for String
// constant definitions and references.
//
// All integer constants are listed in the global "inttable" and have
// type IntEntry.  IntEntry methods are defined for Int
// constant definitions and references.
//
// Since there are only two Bool values, there is no need for a table.
// The two booleans are represented by instances of the class BoolConst,
// which defines the definition and reference methods for Bools.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s) {
  s << STRCONST_PREFIX << index;
}

//
// Emit code for a constant String.
// You should fill in the code naming the dispatch table.
//

void StringEntry::code_def(ostream& s, int stringclasstag) {
  IntEntryP lensym = inttable.add_int(len);

  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);
  s << LABEL                                                              // label
    << WORD << stringclasstag << endl                                     // tag
    << WORD << (DEFAULT_OBJFIELDS + STRING_SLOTS + (len + 4) / 4) << endl // size
    << WORD << Str->get_string() << DISPTAB_SUFFIX << endl;               // dispatch table

  s << WORD;
  lensym->code_ref(s);
  s << endl;                                                              // string length
  emit_string_constant(s, str);                                           // ascii string
  s << ALIGN;                                                             // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the 
// stringtable.
//
void StrTable::code_string_table(ostream& s, int stringclasstag) {
  for (List<StringEntry>* l = tbl; l; l = l->tl()) {
    l->hd()->code_def(s, stringclasstag);
  }
}

//
// Ints
//
void IntEntry::code_ref(ostream& s) {
  s << INTCONST_PREFIX << index;
}

//
// Emit code for a constant Integer.
// You should fill in the code naming the dispatch table.
//

void IntEntry::code_def(ostream& s, int intclasstag) {
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);
  s << LABEL                                               // label
    << WORD << intclasstag << endl                         // class tag
    << WORD << (DEFAULT_OBJFIELDS + INT_SLOTS) << endl     // object size
    << WORD << Int->get_string() << DISPTAB_SUFFIX << endl // dispatch table
    << WORD << str << endl;                                // integer value
}


//
// IntTable::code_string_table
// Generate an Int object definition for every Int constant in the
// inttable.
//
void IntTable::code_string_table(ostream& s, int intclasstag) {
  for (List<IntEntry>* l = tbl; l; l = l->tl()) {
    l->hd()->code_def(s, intclasstag);
  }
}


//
// Bools
//
BoolConst::BoolConst(int i) : val(i) { assert(i == 0 || i == 1); }

void BoolConst::code_ref(ostream& s) const {
  s << BOOLCONST_PREFIX << val;
}

//
// Emit code for a constant Bool.
// You should fill in the code naming the dispatch table.
//

void BoolConst::code_def(ostream& s, int boolclasstag) {
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);
  s << LABEL                                                // label
    << WORD << boolclasstag << endl                         // class tag
    << WORD << (DEFAULT_OBJFIELDS + BOOL_SLOTS) << endl     // object size
    << WORD << Bool->get_string() << DISPTAB_SUFFIX << endl // dispatch table
    << WORD << val << endl;                                 // value (0 or 1)
}


//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `cool-tree.h'  Sample code for
//   constant integers, strings, and booleans are provided.
//
//*****************************************************************

void assign_class::code(ostream& s) {
  CODE_START;
  auto loc = Globals.env.lookup(name);
  assert(loc != nullptr);
  expr->code(s);
  // Now result stores in $a0
  // we save it into our location
  emit_store(ACC, loc->offset, loc->reg, s);
  CODE_END;
}

int assign_class::temporaries() {
  return expr->temporaries();
}

static void
dispatch_impl(Expression expr,
              Expressions actual,
              Symbol name,
              Symbol obj_type,
              ostream& s,
              std::string load_table) {
  auto label = Globals.new_label();
  // Evaluate the actual parameters
  // and push them onto stack
  for (auto i = actual->first();
       actual->more(i);
       i = actual->next(i)) {
    auto init = actual->nth(i);
    init->code(s);
    emit_push(ACC, s);
  }
  // Evaluate the object and save into $a0
  expr->code(s);
  emit_bne(ACC, ZERO, label, s);
  emit_load_string(ACC, stringtable.lookup_string(curr_filename), s);
  emit_load_imm(T1, 1, s);
  emit_jal(DISP_ABORT, s);

  emit_label_def(label, s);
  // Load dispatch table
  s << load_table;
  // Load method address
  int offset = Globals.get_method_offset_for_class(obj_type, name);
  emit_load(T0, offset, T0, s);
  // Jump to the method definition
  emit_jalr(T0, s);
  // When returned, value would be saved in $a0
  // we DON't need to pop all those variables in stack
  // that would be done at the callee side
}

void static_dispatch_class::code(ostream& s) {
  CODE_START;
  stringstream ss;
  emit_partial_load_address(T0, ss);
  ss << type_name << DISPTAB_SUFFIX << endl;
  dispatch_impl(expr, actual, name,
                type_name, s, ss.str());
  CODE_END;
}

int static_dispatch_class::temporaries() {
  int max_temp = expr->temporaries();
  for (auto i = actual->first();
       actual->more(i);
       i = actual->next(i)) {
    max_temp = max(max_temp, actual->nth(i)->temporaries());
  }
  return max_temp;
}

void dispatch_class::code(ostream& s) {
  CODE_START;
  stringstream ss;
  emit_load(T0, DISPTABLE_OFFSET, ACC, ss);
  auto obj_type = expr->get_type();
  if (obj_type == SELF_TYPE) {
    obj_type = Globals.get_current_class();
  }
  dispatch_impl(expr, actual, name,
                obj_type, s, ss.str());
  CODE_END;
}

int dispatch_class::temporaries() {
  int max_temp = expr->temporaries();
  for (auto i = actual->first();
       actual->more(i);
       i = actual->next(i)) {
    max_temp = max(max_temp, actual->nth(i)->temporaries());
  }
  return max_temp;
}

void cond_class::code(ostream& s) {
  CODE_START;
  auto label_true = Globals.new_label();
  auto label_false = Globals.new_label();
  auto label_end = Globals.new_label();
  pred->code(s);
  // Extract boolean value from Bool object
  emit_fetch_int(T0, ACC, s);
  emit_beqz(T0, label_false, s);
  emit_label_def(label_true, s);
  then_exp->code(s);
  emit_branch(label_end, s);
  emit_label_def(label_false, s);
  else_exp->code(s);
  emit_label_def(label_end, s);
  CODE_END;
}

int cond_class::temporaries() {
  return max(pred->temporaries(),
             max(then_exp->temporaries(),
                 else_exp->temporaries()));
}

void loop_class::code(ostream& s) {
  CODE_START;
  auto label_start = Globals.new_label();
  auto label_end = Globals.new_label();

  emit_label_def(label_start, s);
  pred->code(s);
  emit_fetch_int(T0, ACC, s);
  emit_beqz(T0, label_end, s);

  body->code(s);
  emit_branch(label_start, s);

  emit_label_def(label_end, s);
  emit_move(ACC, ZERO, s);
  CODE_END;
}

int loop_class::temporaries() {
  return max(pred->temporaries(), body->temporaries());
}

void typcase_class::code(ostream& s) {
  CODE_START;
  typedef branch_class *BranchType;
  vector<BranchType> patterns;
  vector<int> labels;
  auto label_end = Globals.new_label();
  for (auto i = cases->first();
       cases->more(i);
       i = cases->next(i)) {
    auto cs = static_cast<branch_class *>(cases->nth(i));
    patterns.push_back(cs);
    labels.push_back(Globals.new_label());
  }
  auto label_abort = Globals.new_label();
  labels.push_back(label_abort);
  std::sort(patterns.begin(), patterns.end(),
            [](const BranchType& a, const BranchType& b) {
              return Globals.classtag[a->type_decl] >
                     Globals.classtag[b->type_decl];
            });
  expr->code(s);
  // Now $a0 holds the evaluated expr
  // We should check if it is void (NULL)
  emit_bne(ACC, ZERO, labels[0], s);
  emit_load_string(ACC, stringtable.lookup_string(curr_filename), s);
  emit_load_imm(T1, 1, s);
  emit_jal(CASE_ABORT2, s);
  for (auto i = 0; i < patterns.size(); i++) {
    auto cs = patterns[i];
    auto tag_min = Globals.classtag[cs->type_decl];
    auto tag_max = Globals.subclasstag_max[cs->type_decl];
    emit_label_def(labels[i], s);
    if (i == 0) { emit_load(T2, 0, ACC, s); }
    emit_blti(T2, tag_min, labels[i + 1], s);
    emit_bgti(T2, tag_max, labels[i + 1], s);

    Globals.env.enterscope();
    auto loc = Globals.alloc_temp_loc();

    Globals.env.addid(cs->name, loc);
    emit_store(ACC, loc->offset, loc->reg, s);
    cs->expr->code(s);

    Globals.free_temp_loc();
    Globals.env.exitscope();

    emit_branch(label_end, s);
  }
  emit_label_def(label_abort, s);
  emit_jal(CASE_ABORT, s);
  emit_label_def(label_end, s);
  CODE_END;
}

int typcase_class::temporaries() {
  int max_temp = expr->temporaries();
  for (auto i = cases->first();
       cases->more(i);
       i = cases->next(i)) {
    auto cs = static_cast<branch_class *>(cases->nth(i));
    max_temp = max(max_temp, cs->expr->temporaries() + 1);
  }
  return max_temp;
}

void block_class::code(ostream& s) {
  CODE_START;
  for (int i = body->first();
       body->more(i);
       i = body->next(i)) {
    body->nth(i)->code(s);
  }
  CODE_END;
}

int block_class::temporaries() {
  int max_temp = 0;
  for (int i = body->first();
       body->more(i);
       i = body->next(i)) {
    auto expr = body->nth(i);
    max_temp = max(max_temp, expr->temporaries());
  }
  return max_temp;
}

void let_class::code(ostream& s) {
  CODE_START;
  // If there is no init-expr
  // we initialize this object with its default value
  if (typeid(*init) == typeid(no_expr_class)) {
    if (type_decl == Int) {
      emit_partial_load_address(ACC, s);
      inttable.lookup_string(STR_ZERO)->code_ref(s);
      s << endl;
    } else if (type_decl == Bool) {
      emit_partial_load_address(ACC, s);
      falsebool.code_ref(s);
      s << endl;
    } else if (type_decl == Str) {
      emit_partial_load_address(ACC, s);
      stringtable.lookup_string(EMPTY_STR)->code_ref(s);
      s << endl;
    } else {
      emit_load_imm(ACC, 0, s);
    }
  } else {
    // Otherwise we generate code for it
    init->code(s);
  }
  // Now the result of init-expr stores in $a0
  // We need to allocate a memory space for it
  Globals.env.enterscope();
  auto loc = Globals.alloc_temp_loc();
  Globals.env.addid(identifier, loc);
  // And store the result of init-expr into this location
  emit_store(ACC, loc->offset, loc->reg, s);
  body->code(s);
  Globals.free_temp_loc();
  Globals.env.exitscope();
  CODE_END;
}

int let_class::temporaries() {
  return max(init->temporaries(), body->temporaries() + 1);
}

typedef void (* binary_operator)(char* dest, char* src1, char* src2, ostream& s);

static void
binary_calc_impl(binary_operator op,
                 Expression e1,
                 Expression e2,
                 ostream& s) {
  // generate code for left expression
  e1->code(s);
  // then the return value (object) is saved in $a0
  // with unmodified stack values
  emit_push(ACC, s);
  e2->code(s);
  // Now $a0 holds a ref to an Int object
  // This is just the right hand side of plus operation
  // So we use this object to create a new one as our result
  s << JAL;
  emit_method_ref(Object, copy, s);
  s << endl;
  // lhs: T0, rhs: ACC
  emit_pop(T0, s);
  // lhs: T1, rhs: T2
  emit_fetch_int(T1, T0, s);
  emit_fetch_int(T2, ACC, s);
  // result in: T3
  op(T1, T1, T2, s);
  emit_store_int(T1, ACC, s);
}

void plus_class::code(ostream& s) {
  CODE_START;
  binary_calc_impl(emit_add, e1, e2, s);
  CODE_END;
}

int plus_class::temporaries() {
  return max(e1->temporaries(), e2->temporaries() + 1);
}

void sub_class::code(ostream& s) {
  CODE_START;
  binary_calc_impl(emit_sub, e1, e2, s);
  CODE_END;
}

int sub_class::temporaries() {
  return max(e1->temporaries(), e2->temporaries() + 1);
}

void mul_class::code(ostream& s) {
  CODE_START;
  binary_calc_impl(emit_mul, e1, e2, s);
  CODE_END;
}

int mul_class::temporaries() {
  return max(e1->temporaries(), e2->temporaries() + 1);
}

void divide_class::code(ostream& s) {
  CODE_START;
  binary_calc_impl(emit_div, e1, e2, s);
  CODE_END;
}

int divide_class::temporaries() {
  return max(e1->temporaries(), e2->temporaries() + 1);
}

void neg_class::code(ostream& s) {
  CODE_START;
  // Eval the expression
  e1->code(s);
  // Copy result into new object
  s << JAL;
  emit_method_ref(Object, ::copy, s);
  s << endl;
  // Fetch the value in object
  emit_fetch_int(T0, ACC, s);
  // Do neg calculation
  emit_neg(T0, T0, s);
  // Store result into object
  emit_store_int(T0, ACC, s);
  CODE_END;
}

int neg_class::temporaries() {
  return e1->temporaries();
}

typedef void (* binary_comparator)(char* src1, char* src2, int label, ostream& s);

static void
binary_compare_impl(binary_comparator op,
                    Expression e1,
                    Expression e2,
                    ostream& s) {
  // Notice that we only allow compare between
  // integers, not objects
  e1->code(s);
  emit_push(ACC, s);
  e2->code(s);
  emit_pop(T0, s);
  // extract lhs and rhs
  emit_fetch_int(T1, T0, s);
  emit_fetch_int(T2, ACC, s);
  auto label = Globals.new_label();
  emit_load_bool(ACC, truebool, s);
  op(T1, T2, label, s);
  emit_load_bool(ACC, falsebool, s);
  emit_label_def(label, s);
  // Do nothing below, just put a label here
}

void lt_class::code(ostream& s) {
  CODE_START;
  binary_compare_impl(emit_blt, e1, e2, s);
  CODE_END;
}

int lt_class::temporaries() {
  return max(e1->temporaries(), e2->temporaries() + 1);
}

void leq_class::code(ostream& s) {
  CODE_START;
  binary_compare_impl(emit_bleq, e1, e2, s);
  CODE_END;
}

int leq_class::temporaries() {
  return max(e1->temporaries(), e2->temporaries() + 1);
}

void eq_class::code(ostream& s) {
  CODE_START;
  auto label = Globals.new_label();
  e1->code(s);
  emit_push(ACC, s);
  e2->code(s);
  emit_move(T2, ACC, s);
  emit_pop(T1, s);
  emit_load_bool(ACC, truebool, s);
  emit_beq(T1, T2, label, s);
  emit_load_bool(A1, falsebool, s);
  emit_jal(TEST_EQUAL, s);
  emit_label_def(label, s);
  CODE_END;
}

int eq_class::temporaries() {
  return max(e1->temporaries(), e2->temporaries() + 1);
}

void comp_class::code(ostream& s) {
  CODE_START;
  auto label = Globals.new_label();
  e1->code(s);
  emit_fetch_int(T0, ACC, s);
  emit_load_bool(ACC, truebool, s);
  emit_beqz(T0, label, s);
  emit_load_bool(ACC, falsebool, s);
  emit_label_def(label, s);
  CODE_END;
}

int comp_class::temporaries() {
  return e1->temporaries();
}

void int_const_class::code(ostream& s) {
  CODE_START;
  //
  // Need to be sure we have an IntEntry *, not an arbitrary Symbol
  //
  emit_load_int(ACC, inttable.lookup_string(token->get_string()), s);
  CODE_END;
}

int int_const_class::temporaries() {
  return 0;
}

void string_const_class::code(ostream& s) {
  CODE_START;
  emit_load_string(ACC, stringtable.lookup_string(token->get_string()), s);
  CODE_END;
}

int string_const_class::temporaries() {
  return 0;
}

void bool_const_class::code(ostream& s) {
  CODE_START;
  emit_load_bool(ACC, BoolConst(val), s);
  CODE_END;
}

int bool_const_class::temporaries() {
  return 0;
}

void new__class::code(ostream& s) {
  CODE_START;
  if (type_name == SELF_TYPE) {
    // Load address of class_objTab
    emit_load_address(T1, CLASSOBJTAB, s);
    // Load object tag from self pointer
    emit_load(T2, 0, SELF, s);
    // calculate the offset
    emit_sll(T2, T2, LOG_WORD_SIZE + 1, s);
    // add offset to base address
    emit_addu(T1, T1, T2, s);
    // load <class>_protObj into $a0
    emit_load(ACC, 0, T1, s);
    // backup the address
    emit_push(T1, s);
    // call Object.copy
    s << JAL;
    emit_method_ref(Object, ::copy, s);
    s << endl;
    // load address of <class>_init
    // at this time $a0 holds ref to the object
    emit_pop(T1, s);
    emit_load(T1, 1, T1, s);
    // call <class>_init
    emit_jalr(T1, s);
  } else {
    // Load proto object address into $a0
    emit_partial_load_address(ACC, s);
    emit_protobj_ref(type_name, s);
    s << endl;
    // call Object.copy
    s << JAL;
    emit_method_ref(Object, ::copy, s);
    s << endl;
    // call <class>_init
    s << JAL;
    emit_init_ref(type_name, s);
    s << endl;
  }
  CODE_END;
}

int new__class::temporaries() {
  // TODO: is this true?
  return 1;
}

void isvoid_class::code(ostream& s) {
  CODE_START;
  auto label = Globals.new_label();
  e1->code(s);
  emit_move(T0, ACC, s);
  emit_load_bool(ACC, truebool, s);
  emit_beqz(T0, label, s);
  emit_load_bool(ACC, falsebool, s);
  emit_label_def(label, s);
  CODE_END;
}

int isvoid_class::temporaries() {
  return e1->temporaries();
}

void no_expr_class::code(ostream& s) {
  CODE_START;
  // We do nothing here
  // This piece of code should never be reached
  assert(false);
  CODE_END;
}

int no_expr_class::temporaries() {
  return 0;
}

void object_class::code(ostream& s) {
  CODE_START;
  if (name == self) {
    emit_move(ACC, SELF, s);
  } else {
    auto loc = Globals.env.lookup(name);
    assert(loc != nullptr);
    emit_load(ACC, loc->offset, loc->reg, s);
  }
  CODE_END;
}

int object_class::temporaries() {
  return 0;
}
