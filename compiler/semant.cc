#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <typeinfo>
#include "semant.h"
#include "utilities.h"
#include "symtab.h"

extern int semant_debug;
extern char* curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;

//
// Additional methods defined for the tree package
// To make the god damn "protected" fields happy
//

//Symbol class__class::get_parent() {
//  return this->parent;
//}
//
//Symbol class__class::get_name() {
//  return this->name;
//}
//
//Features class__class::get_features() {
//  return this->features;
//}
//
//Formals method_class::get_formals() {
//  return this->formals;
//}
//
//Symbol method_class::get_return_type() {
//  return this->return_type;
//}
//
//Symbol method_class::get_name() {
//  return this->name;
//}
//
//Symbol attr_class::get_name() {
//  return this->name;
//}
//
//Symbol formal_class::get_name() {
//  return this->name;
//}
//
//Symbol formal_class::get_type_decl() {
//  return this->type_decl;
//}
//
//Symbol attr_class::get_type_decl() {
//  return this->type_decl;
//}
//
//Expression method_class::get_expr() {
//  return this->expr;
//}
//
//Expression attr_class::get_init_expr() {
//  return this->init;
//}
//
//Symbol branch_class::get_name() {
//  return this->name;
//}
//
//Symbol branch_class::get_type_decl() {
//  return this->type_decl;
//}
//
//Expression branch_class::get_expr() {
//  return this->expr;
//}

//#ifdef NO_FLEX
//// Make flex happy
//// if no -fl is enabled
//extern "C" int yywrap( void ) {
//    return 1;
//}
//#endif

//
// Environments for Type Checking, stands for O, M, C
//

ClassTable* classtable;
// Record all Object IDs using provided ID table
SymbolTable<Symbol, Entry> ObjectIDs;
// A global pointer to current class
class__class* Current_Class;
// Record methods of each class, key is class name
map<Symbol, map<Symbol, pair<Formals, Symbol> > > Methods;

//
// Initializing the predefined symbols.
//

// Meaningful IDs are Bool, Int, String, Object, IO, No_type
// Note that these five types are basic types, with a additional one for error handling
// When assign values to the "type" field of Expression node, only these basic types are used
static void initialize_constants(void) {
  arg = idtable.add_string("arg");
  arg2 = idtable.add_string("arg2");
  Bool = idtable.add_string("Bool");
  concat = idtable.add_string("concat");
  cool_abort = idtable.add_string("abort");
  copy = idtable.add_string("copy");
  Int = idtable.add_string("Int");
  in_int = idtable.add_string("in_int");
  in_string = idtable.add_string("in_string");
  IO = idtable.add_string("IO");
  length = idtable.add_string("length");
  Main = idtable.add_string("Main");
  main_meth = idtable.add_string("main");
  //   _no_class is a symbol that can't be the name of any
  //   user-defined class.
  No_class = idtable.add_string("_no_class");
  No_type = idtable.add_string("_no_type");
  Object = idtable.add_string("Object");
  out_int = idtable.add_string("out_int");
  out_string = idtable.add_string("out_string");
  prim_slot = idtable.add_string("_prim_slot");
  self = idtable.add_string("self");
  SELF_TYPE = idtable.add_string("SELF_TYPE");
  Str = idtable.add_string("String");
  str_field = idtable.add_string("_str_field");
  substr = idtable.add_string("substr");
  type_name = idtable.add_string("type_name");
  val = idtable.add_string("_val");
}

// This is the construct functions
ClassTable::ClassTable(Classes classes) : semant_errors(0), error_stream(cerr) {
  this->classes = classes;
  this->install_basic_classes();
  this->check_inheritance();
}

void ClassTable::install_basic_classes() {

  // The tree package uses these globals to annotate the classes built below.
  // curr_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

  // The following demonstrates how to create dummy parse trees to
  // refer to basic Cool classes.  There's no need for method
  // bodies -- these are already built into the runtime system.

  // IMPORTANT: The results of the following expressions are
  // stored in local variables.  You will want to do something
  // with those variables at the end of this method to make this
  // code meaningful.

  //
  // The Object class has no parent class. Its methods are
  //        abort() : Object    aborts the program
  //        type_name() : Str   returns a string representation of class name
  //        copy() : SELF_TYPE  returns a copy of the object
  //
  // There is no need for method bodies in the basic classes---these
  // are already built in to the runtime system.

  Class_ Object_class =
      class_(Object,
             No_class,
             append_Features(
                 append_Features(
                     single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
                     single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
                 single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
             filename);

  //
  // The IO class inherits from Object. Its methods are
  //        out_string(Str) : SELF_TYPE       writes a string to the output
  //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
  //        in_string() : Str                 reads a string from the input
  //        in_int() : Int                      "   an int     "  "     "
  //
  Class_ IO_class =
      class_(IO,
             Object,
             append_Features(
                 append_Features(
                     append_Features(
                         single_Features(method(out_string, single_Formals(formal(arg, Str)),
                                                SELF_TYPE, no_expr())),
                         single_Features(method(out_int, single_Formals(formal(arg, Int)),
                                                SELF_TYPE, no_expr()))),
                     single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
                 single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
             filename);

  //
  // The Int class has no methods and only a single attribute, the
  // "val" for the integer.
  //
  Class_ Int_class =
      class_(Int,
             Object,
             single_Features(attr(val, prim_slot, no_expr())),
             filename);

  //
  // Bool also has only the "val" slot.
  //
  Class_ Bool_class =
      class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())), filename);

  //
  // The class Str has a number of slots and operations:
  //       val                                  the length of the string
  //       str_field                            the string itself
  //       length() : Int                       returns length of the string
  //       concat(arg: Str) : Str               performs string concatenation
  //       substr(arg: Int, arg2: Int): Str     substring selection
  //
  Class_ Str_class =
      class_(Str,
             Object,
             append_Features(
                 append_Features(
                     append_Features(
                         append_Features(
                             single_Features(attr(val, Int, no_expr())),
                             single_Features(attr(str_field, prim_slot, no_expr()))),
                         single_Features(method(length, nil_Formals(), Int, no_expr()))),
                     single_Features(method(concat,
                                            single_Formals(formal(arg, Str)),
                                            Str,
                                            no_expr()))),
                 single_Features(method(substr,
                                        append_Formals(single_Formals(formal(arg, Int)),
                                                       single_Formals(formal(arg2, Int))),
                                        Str,
                                        no_expr()))),
             filename);

  // Check if basic classes are redefined
  for (auto i = this->classes->first(); this->classes->more(i); i = this->classes->next(i)) {
    class__class* cur_class = static_cast<class__class*>(this->classes->nth(i));
    if (cur_class->name == SELF_TYPE
        || cur_class->name == Object
        || cur_class->name == Str
        || cur_class->name == Bool
        || cur_class->name == Int) {
      this->semant_error(cur_class)
          << "Fatal error: redefinition of basic class \"" << cur_class->name->get_string() << "\"." << endl;
    }
  }

  // Finally, append all these classes to the classes list
  this->classes = append_Classes(single_Classes(IO_class), this->classes);
  this->classes = append_Classes(single_Classes(Str_class), this->classes);
  this->classes = append_Classes(single_Classes(Bool_class), this->classes);
  this->classes = append_Classes(single_Classes(Int_class), this->classes);
  this->classes = append_Classes(single_Classes(Object_class), this->classes);
  this->SpecialClass.insert(prim_slot);
}

void ClassTable::check_inheritance() {
  this->AllSymbols.insert(Object);
  this->Graph.SetRoot(Object);
  // First, check if all classes are defined only once
  // And then insert them into set and graph nodes
  for (auto i = this->classes->first(); this->classes->more(i); i = this->classes->next(i)) {
    class__class* cur_class = static_cast<class__class*>(this->classes->nth(i));
    this->class_name2node[cur_class->name] = this->classes->nth(i);
    if (cur_class->name == Object) { continue; }
    if (this->AllSymbols.find(cur_class->name) == this->AllSymbols.end()) {
      this->AllSymbols.insert(cur_class->name);
      this->Graph.AddNode(cur_class->name);
    } else {
      // Report fatal error and return false
      this->semant_error(cur_class) << "Fatal error: class name redefined." << endl;
      return;
    }
  }
  // Then Add Edges to the graph
  for (auto i = this->classes->first(); this->classes->more(i); i = this->classes->next(i)) {
    class__class* cur_class = static_cast<class__class*>(this->classes->nth(i));
    if (cur_class->name == Object) { continue; }
    // Notice to check if this class have parent
    if (this->AllSymbols.find(cur_class->get_parent()) == this->AllSymbols.end()) {
      this->semant_error(cur_class) << "Fatal error: unknown parent of class " <<
                                    cur_class->name << endl;
      return;
    }
    // Avoid self-loop
    if (cur_class->get_parent() == cur_class->name) {
      this->semant_error(cur_class) << "Fatal error: class inherited from itself." << endl;
      return;
    }
    // Check if class could be inherited
    if (cur_class->get_parent() == Str
        || cur_class->get_parent() == Int
        || cur_class->get_parent() == Bool
        || cur_class->get_parent() == SELF_TYPE) {
      this->semant_error(cur_class)
          << "Fatal error: class \"" << cur_class->name->get_string()
          << "\" cannot inherit from class \"" << cur_class->get_parent()->get_string()
          << "\"." << endl;
      return;
    }
    this->Graph.AddEdge(cur_class->get_parent(), cur_class->name);
  }
  if (!this->Graph.CheckCircle()) {
    this->semant_error() << "Fatal error: class inheritance graph has a circle." << endl;
    return;
  }
  this->Graph.AddSon(No_type);
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c) {
  return semant_error(static_cast<class__class*>(c)->get_filename(), c);
}

ostream& ClassTable::semant_error(Symbol filename, tree_node* t) {
  error_stream << filename << ":" << t->get_line_number() << ": ";
  return semant_error();
}

ostream& ClassTable::semant_error() {
  semant_errors++;
  return error_stream;
}

//
// Set proper type information for each type of node
//
void assign_class::type_check() {
  if (this->type != NULL) { return; }
  this->expr->type_check();
  auto T_ = this->expr->get_type();
  if (T_ == SELF_TYPE) { T_ = Current_Class->name; }
  auto T = ObjectIDs.lookup(this->name);
  if (T == SELF_TYPE) { T = Current_Class->name; }
  // If assign to self, then raise error and set type to No_type and return
  if (this->name == self) {
    this->type = No_type;
    classtable->semant_error(Current_Class)
        << "Error: assign expression to keyword \"self\" is not allowed." << endl;
    return;
  }
  if (T == NULL) {
    this->type = No_type;
    classtable->semant_error(Current_Class)
        << "Error: identifer \"" << this->name->get_string()
        << "\" is used without being declared." << endl;
    return;
  }
  if (!classtable->ConformTo(T_, T)) {
    this->type = No_type;
    classtable->semant_error(Current_Class)
        << "Error: expression doesn't match type \""
        << T->get_string() << "\" of identifer \""
        << this->name->get_string() << "\"." << endl;
    return;
  }
  this->type = this->expr->get_type();
}

void static_dispatch_class::type_check() {
  if (this->type != NULL) { return; }
  // 1) Type check all actual parameter expressions
  for (auto i = this->actual->first(); this->actual->more(i); i = this->actual->next(i)) {
    this->actual->nth(i)->type_check();
    if (this->actual->nth(i)->get_type() == No_type) {
      this->type = No_type;
    }
  }

  // 2) Check if sub expression has No type
  this->expr->type_check();
  auto raw_type = this->expr->get_type();
  if (raw_type == SELF_TYPE) { raw_type = Current_Class->name; }
  auto convert_type = this->type_name;

  if (raw_type == No_type) {
    this->type = No_type;
  }

  if (this->type != NULL) { return; }

  if (!classtable->HaveClass(convert_type)) {
    classtable->semant_error(Current_Class)
        << "Error: type \"" << convert_type->get_string() << "\" is not defined." << endl;
    this->type = No_type;
    return;
  }

  if (!classtable->ConformTo(raw_type, convert_type)) {
    classtable->semant_error(Current_Class)
        << "Error: expression type \"" << raw_type->get_string()
        << "\" should conform to dispatch type \""
        << convert_type->get_string() << "\"." << endl;
    this->type = No_type;
    return;
  }

  // 3) Check if method is not defined
  if (Methods[convert_type].find(this->name) == Methods[convert_type].end()) {
    classtable->semant_error(Current_Class)
        << "Error: method \"" << this->name->get_string()
        << "\" of class \"" << convert_type->get_string()
        << "\" is not defined." << endl;
    this->type = No_type;
    return;
  }

  Formals formals = Methods[convert_type][this->name].first;
  Symbol return_type = Methods[convert_type][this->name].second;

  // 4) Check if number of parameters are matched
  if (formals->len() != this->actual->len()) {
    classtable->semant_error(Current_Class)
        << "Error: number of formal parameters and actual parameters does not match." << endl;
    this->type = No_type;
    return;
  }

  // 5) Check type conform of parameters
  for (auto i = this->actual->first(); this->actual->more(i); i = this->actual->next(i)) {
    this->actual->nth(i)->type_check();
    auto T = this->actual->nth(i)->get_type();
    if (T == SELF_TYPE) { T = Current_Class->name; }
    // Note that this type is guaranteed to be exist and not to be SELF_TYPE
    auto T_ = static_cast<formal_class*>(formals->nth(i))->type_decl;
    if (!classtable->ConformTo(T, T_)) {
      classtable->semant_error(Current_Class)
          << "Error: type of actual parameter \""
          << T->get_string() << "\" does not conform to type \""
          << T_->get_string() << "\" of formal parameter \""
          << static_cast<formal_class*>(formals->nth(i))->name->get_string()
          << "\"." << endl;
      this->type = No_type;
      return;
    }
  }

  // 6) Get the return type
  if (return_type == SELF_TYPE) {
    this->type = this->expr->get_type();
  } else {
    this->type = return_type;
  }
}

void dispatch_class::type_check() {
  if (this->type != NULL) { return; }
  // 1) Type check all actual parameter expressions
  for (auto i = this->actual->first(); this->actual->more(i); i = this->actual->next(i)) {
    this->actual->nth(i)->type_check();
    if (this->actual->nth(i)->get_type() == No_type) {
      this->type = No_type;
    }
  }

  // 2) Check if sub expression has No type
  this->expr->type_check();
  auto T0_ = this->expr->get_type();
  if (T0_ == SELF_TYPE) { T0_ = Current_Class->name; }
  if (T0_ == No_type) { this->type = No_type; }

  if (this->type != NULL) { return; }

  // 3) Check if method is not defined
  if (Methods[T0_].find(this->name) == Methods[T0_].end()) {
    classtable->semant_error(Current_Class)
        << "Error: method \"" << this->name->get_string()
        << "\" of class \"" << T0_->get_string()
        << "\" is not defined." << endl;
    this->type = No_type;
    return;
  }

  Formals formals = Methods[T0_][this->name].first;
  Symbol return_type = Methods[T0_][this->name].second;

  // 4) Check if number of parameters are matched
  if (formals->len() != this->actual->len()) {
    classtable->semant_error(Current_Class)
        << "Error: number of formal parameters and actual parameters does not match." << endl;
    this->type = No_type;
    return;
  }

  // 5) Check type conform of parameters
  for (auto i = this->actual->first(); this->actual->more(i); i = this->actual->next(i)) {
    this->actual->nth(i)->type_check();
    auto T = this->actual->nth(i)->get_type();
    if (T == SELF_TYPE) { T = Current_Class->name; }
    // Note that this type is guaranteed to be exist and not to be SELF_TYPE
    auto T_ = static_cast<formal_class*>(formals->nth(i))->type_decl;
    if (!classtable->ConformTo(T, T_)) {
      classtable->semant_error(Current_Class)
          << "Error: type of actual parameter \""
          << T->get_string() << "\" does not conform to type \""
          << T_->get_string() << "\" of formal parameter \""
          << static_cast<formal_class*>(formals->nth(i))->name->get_string()
          << "\"." << endl;
      this->type = No_type;
      return;
    }
  }

  // 6) Get the return type
  if (return_type == SELF_TYPE) {
    this->type = this->expr->get_type();
  } else {
    this->type = return_type;
  }
}

void cond_class::type_check() {
  if (this->type != NULL) { return; }
  // Type check all sub expr
  this->pred->type_check();
  this->then_exp->type_check();
  this->else_exp->type_check();
  if (this->pred->get_type() != Bool) {
    classtable->semant_error(Current_Class)
        << "Error: condition type in \"if\" statement should be Bool." << endl;
  }
  auto T0 = this->then_exp->get_type();
  auto T1 = this->else_exp->get_type();
  if (T0 == SELF_TYPE) { T0 = Current_Class->name; }
  if (T1 == SELF_TYPE) { T1 = Current_Class->name; }
  this->type = classtable->LCA(T0, T1);
}

void loop_class::type_check() {
  if (this->type != NULL) { return; }
  this->pred->type_check();
  this->body->type_check();
  if (this->pred->get_type() != Bool) {
    classtable->semant_error(Current_Class)
        << "Error: condition type in \"while\" statement should be Bool." << endl;
  }
  this->type = Object;
}

void typcase_class::type_check() {
  if (this->type != NULL) { return; }
  // First check if all types are well defined or illegal
  using std::set;
  set<Symbol> all_cases;
  for (auto i = this->cases->first(); this->cases->more(i); i = this->cases->next(i)) {
    auto branch = static_cast<branch_class*>(this->cases->nth(i));
    if (all_cases.find(branch->type_decl) == all_cases.end()) {
      all_cases.insert(branch->type_decl);
    } else {
      classtable->semant_error(Current_Class)
          << "Error: duplicate branch \""
          << branch->type_decl->get_string()
          << "\" in case statement." << endl;
      this->type = No_type;
      return;
    }
    if (branch->type_decl == SELF_TYPE) {
      classtable->semant_error(Current_Class)
          << "Error: \"SELF_TYPE\" is not allowed in case branch." << endl;
      this->type = No_type;
      return;
    }
    if (!classtable->HaveClass(branch->type_decl)) {
      classtable->semant_error(Current_Class)
          << "Error: type \"" << branch->type_decl->get_string()
          << "\" in case branch is not defined" << endl;
      this->type = No_type;
      return;
    }
  }
  // Type check all sub expressions
  this->expr->type_check();
  if (this->expr->get_type() == No_type) {
    this->type = No_type;
  }
  for (auto i = this->cases->first();
       this->cases->more(i);
       i = this->cases->next(i)) {
    auto branch = static_cast<branch_class*>(this->cases->nth(i));
    ObjectIDs.enterscope();

    ObjectIDs.addid(branch->name, branch->type_decl);
    branch->expr->type_check();

    ObjectIDs.exitscope();
    if (branch->expr->get_type() == No_type) { this->type = No_type; }
  }
  // If error occured, then just exit
  if (this->type != NULL) { return; }
  // Finally, get the type of the case expression
  // Note that this type checking use the expression type, not the declare type
  if (this->cases->len() == 1) {
    this->type = static_cast<branch_class*>(
        this->cases->nth(this->cases->first()))->expr->get_type();
  } else {
    auto first = this->cases->first();
    auto second = this->cases->next(first);
    this->type = classtable->LCA(static_cast<branch_class*>(
                                     this->cases->nth(first))->expr->get_type(),
                                 static_cast<branch_class*>(
                                     this->cases->nth(second))->expr->get_type());
    for (auto i = this->cases->first();
         this->cases->more(i);
         i = this->cases->next(i)) {
      auto branch = static_cast<branch_class*>(this->cases->nth(i));
      this->type = classtable->LCA(branch->expr->get_type(), this->type);
    }
  }
}

void block_class::type_check() {
  // After parsing, the expr block is guaranteed to have at least one expr
  if (this->type != NULL) { return; }
  for (auto i = this->body->first(); this->body->more(i); i = this->body->next(i)) {
    this->body->nth(i)->type_check();
    // The type of block expr is the last expr
    if (!this->body->more(this->body->next(i))) {
      this->type = this->body->nth(i)->get_type();
      break;
    }
  }
  if (this->type == NULL) {
    classtable->semant_error(Current_Class)
        << "Error: block expressions do not have a type. BUG in compiler!!!" << endl;
    this->type = No_type;
  }
}

void let_class::type_check() {
  if (this->type != NULL) { return; }
  // First, get the real declare type
  Symbol T0_ = this->type_decl;
  if (T0_ == SELF_TYPE) { T0_ = Current_Class->name; }

  // Then, the initialization e1 is type checked in an environment without a new definition.
  this->init->type_check();
  auto T1 = this->init->get_type();
  if (T1 == SELF_TYPE) { T1 = Current_Class->name; }
  if (!classtable->HaveClass(T0_)) {
    classtable->semant_error(Current_Class)
        << "Error: type \"" << T0_->get_string()
        << "\" is used without being defined." << endl;
    this->type = No_type;
  }
  // Check if "self" is used as ID
  if (this->identifier == self) {
    classtable->semant_error(Current_Class)
        << "Error: key word \"self\" could not be used in \"let\" statement." << endl;
    this->type = No_type;
  }
  // If T1 is No_type, then there won't be type mismatch
  if (!classtable->ConformTo(T1, T0_)) {
    classtable->semant_error(Current_Class)
        << "Error: init expression does not match type \""
        << T0_->get_string() << "\" of variable \""
        << this->identifier->get_string() << "\"." << endl;
    this->type = No_type;
  }
  // The type of let is the type of the body.
  ObjectIDs.enterscope();
  // Note that if type declaration is SELF_TYPE, then just put it into environment
  // Do not convert into actual type!
  ObjectIDs.addid(this->identifier, this->type_decl);
  this->body->type_check();
  if (this->type == NULL) { this->type = this->body->get_type(); }
  ObjectIDs.exitscope();
}

void plus_class::type_check() {
  if (this->type != NULL) { return; }
  this->e1->type_check();
  this->e2->type_check();
  if (this->e1->get_type() == No_type || this->e2->get_type() == No_type) {
    this->type = No_type;
    return;
  }
  if (this->e1->get_type() == Int && this->e2->get_type() == Int) {
    this->type = Int;
  } else {
    classtable->semant_error(Current_Class)
        << "Error: expressions should have same type Int for operator \"+\"." << endl;
    this->type = No_type;
  }
}

void sub_class::type_check() {
  if (this->type != NULL) { return; }
  this->e1->type_check();
  this->e2->type_check();
  if (this->e1->get_type() == No_type || this->e2->get_type() == No_type) {
    this->type = No_type;
    return;
  }
  if (this->e1->get_type() == Int && this->e2->get_type() == Int) {
    this->type = Int;
  } else {
    classtable->semant_error(Current_Class)
        << "Error: expressions should have same type Int for operator \"-\"." << endl;
    this->type = No_type;
  }
}

void mul_class::type_check() {
  if (this->type != NULL) { return; }
  this->e1->type_check();
  this->e2->type_check();
  if (this->e1->get_type() == No_type || this->e2->get_type() == No_type) {
    this->type = No_type;
    return;
  }
  if (this->e1->get_type() == Int && this->e2->get_type() == Int) {
    this->type = Int;
  } else {
    classtable->semant_error(Current_Class)
        << "Error: expressions should have same type Int for operator \"*\"." << endl;
    this->type = No_type;
  }
}

void divide_class::type_check() {
  if (this->type != NULL) { return; }
  this->e1->type_check();
  this->e2->type_check();
  if (this->e1->get_type() == No_type || this->e2->get_type() == No_type) {
    this->type = No_type;
    return;
  }
  if (this->e1->get_type() == Int && this->e2->get_type() == Int) {
    this->type = Int;
  } else {
    classtable->semant_error(Current_Class)
        << "Error: expressions should have same type Int for operator \"/\"." << endl;
    this->type = No_type;
  }
}

void neg_class::type_check() {
  if (this->type != NULL) { return; }
  this->e1->type_check();
  if (this->e1->get_type() == No_type) {
    this->type = No_type;
    return;
  }
  if (this->e1->get_type() != Int) {
    classtable->semant_error(Current_Class)
        << "Error: expression should have type Int." << endl;
    this->type = No_type;
  } else {
    this->type = Int;
  }
}

void lt_class::type_check() {
  if (this->type != NULL) { return; }
  this->e1->type_check();
  this->e2->type_check();
  if (this->e1->get_type() == No_type || this->e2->get_type() == No_type) {
    this->type = No_type;
    return;
  }
  if (this->e1->get_type() == Int && this->e2->get_type() == Int) {
    this->type = Bool;
  } else {
    classtable->semant_error(Current_Class)
        << "Error: expressions should have same type Int for operator \"<\"." << endl;
    this->type = No_type;
  }
}

void eq_class::type_check() {
  if (this->type != NULL) { return; }
  this->e1->type_check();
  this->e2->type_check();
  if (this->e1->get_type() == No_type || this->e2->get_type() == No_type) {
    this->type = No_type;
    return;
  }
  if (this->e1->get_type() == Str && this->e2->get_type() == Str) {
    this->type = Bool;
  } else if (this->e1->get_type() == Int && this->e2->get_type() == Int) {
    this->type = Bool;
  } else if (this->e1->get_type() == Bool && this->e2->get_type() == Bool) {
    this->type = Bool;
  } else if ((this->e1->get_type() != Int
              && this->e1->get_type() != Str
              && this->e1->get_type() != Bool) &&
             (this->e2->get_type() != Int
              && this->e2->get_type() != Str
              && this->e2->get_type() != Bool)) {
    this->type = Bool;
  } else {
    classtable->semant_error(Current_Class)
        << "Error: basic type (Int, String, Bool) could only compare with same basic type." << endl;
    this->type = No_type;
  }
}

void leq_class::type_check() {
  if (this->type != NULL) { return; }
  this->e1->type_check();
  this->e2->type_check();
  if (this->e1->get_type() == No_type || this->e2->get_type() == No_type) {
    this->type = No_type;
    return;
  }
  if (this->e1->get_type() == Int && this->e2->get_type() == Int) {
    this->type = Bool;
  } else {
    classtable->semant_error(Current_Class)
        << "Error: expressions should have same type Int for operator \"<=\"." << endl;
    this->type = No_type;
  }
}

void comp_class::type_check() {
  if (this->type != NULL) { return; }
  this->e1->type_check();
  if (this->e1->get_type() == No_type) {
    this->type = No_type;
    return;
  }
  if (this->e1->get_type() != Bool) {
    classtable->semant_error(Current_Class)
        << "Error: expression should have type Bool." << endl;
    this->type = No_type;
  } else {
    this->type = Bool;
  }
}

void int_const_class::type_check() {
  if (this->type != NULL) { return; }
  this->type = Int;
}

void bool_const_class::type_check() {
  if (this->type != NULL) { return; }
  this->type = Bool;
}

void string_const_class::type_check() {
  if (this->type != NULL) { return; }
  this->type = Str;
}

void new__class::type_check() {
  if (this->type != NULL) { return; }
  if (this->type_name != SELF_TYPE) {
    if (classtable->HaveClass(this->type_name)) {
      this->type = this->type_name;
    } else {
      classtable->semant_error(Current_Class)
          << "Error: type \"" << this->type_name->get_string()
          << "\" in \"new\" operation is not defined." << endl;
      this->type = No_type;
    }
  } else {
    this->type = SELF_TYPE;
  }
}

void isvoid_class::type_check() {
  if (this->type != NULL) { return; }
  this->e1->type_check();
  this->type = Bool;
}

void no_expr_class::type_check() {
  if (this->type != NULL) { return; }
  this->type = No_type;
}

void object_class::type_check() {
  if (this->type != NULL) { return; }
  this->type = ObjectIDs.lookup(this->name);
  // If not found, error handling
  if (this->type == NULL) {
    classtable->semant_error(Current_Class)
        << "Error: identifer \"" << this->name->get_string()
        << "\" is used without being declared." << endl;
    this->type = No_type;
  }
}

void class__class::type_check() {
  Current_Class = this;

  ObjectIDs.enterscope();
  // 1. Add "self" identifier into hole environment
  ObjectIDs.addid(self, SELF_TYPE);

  // 2. Collect class attributes (including any inherited) into Object environment
  Symbol cur_node = this->name;
  while (classtable->get_father(cur_node) != cur_node) {
    auto cur_class = static_cast<class__class*>(classtable->GetNode(cur_node));
    Features cur_features = cur_class->features;
    for (auto i = cur_features->first(); cur_features->more(i); i = cur_features->next(i)) {
      Feature cur_feature = cur_features->nth(i);
      if (typeid(*cur_feature) == typeid(attr_class)) {
        attr_class* cur_attr = static_cast<attr_class*>(cur_feature);
        if (cur_attr->type_decl == SELF_TYPE) {
          ObjectIDs.addid(cur_attr->name, cur_class->name);
        } else {
          ObjectIDs.addid(cur_attr->name, cur_attr->type_decl);
        }
      }
    }
    // Look up to its father
    cur_node = classtable->get_father(cur_node);
  }

  // 3. Start analyzing methods
  for (auto i = this->features->first(); this->features->more(i); i = this->features->next(i)) {
    Feature cur_feature = this->features->nth(i);
    if (typeid(*cur_feature) == typeid(method_class)) {
      ObjectIDs.enterscope();

      method_class* cur_method = static_cast<method_class*>(cur_feature);
      // 1) Add formals into environment
      Formals formals = cur_method->formals;
      for (auto k = formals->first(); formals->more(k); k = formals->next(k)) {
        auto cur_formal = static_cast<formal_class*>(formals->nth(k));
        ObjectIDs.addid(cur_formal->name, cur_formal->type_decl);
      }
      // 2) Type check the expression of this method
      cur_method->expr->type_check();

      // 3) Then type check the method itself
      auto T0 = cur_method->return_type;
      auto T0_ = cur_method->expr->get_type();

      // Special case for SELF_TYPE
      // It couldn't be that for method in class A,
      // declare type is SELF_TYPE and expression type is A
      // Others are OK
      // This is because SELF_TYPE may refer to any type conform to A
      if (T0 == SELF_TYPE || T0_ == SELF_TYPE) {
        if (T0 != SELF_TYPE && T0_ == SELF_TYPE) { T0_ = Current_Class->name; }
        if (T0 == SELF_TYPE && T0_ == SELF_TYPE) {
          T0_ = Current_Class->name;
          T0 = Current_Class->name;
        }
      }
      if (!classtable->ConformTo(T0_, T0) || (T0 == SELF_TYPE || T0_ == SELF_TYPE)) {
        classtable->semant_error(Current_Class)
            << "Error: expression type \"" << T0_->get_string() << "\" of method \""
            << cur_method->name->get_string() << "\" doesn't conform to its declared type \""
            << T0->get_string() << "\"." << endl;
      }

      ObjectIDs.exitscope();
    } else if (typeid(*cur_feature) == typeid(attr_class)) {
      // 4) Type check the attributes
      attr_class* cur_attr = static_cast<attr_class*>(cur_feature);
      cur_attr->init->type_check();
      auto T0 = cur_attr->type_decl;
      auto T1 = cur_attr->init->get_type();
      if (T0 == SELF_TYPE) { T0 = Current_Class->name; }
      if (T1 == SELF_TYPE) { T1 = Current_Class->name; }
      if (!classtable->ConformTo(T1, T0)) {
        classtable->semant_error(Current_Class) << "Error: init expression type \""
                                                << T1->get_string() << "\" doesn't conform to declared type \""
                                                << T0->get_string() << "\" of attribute \""
                                                << cur_attr->name->get_string() << "\"." << endl;
      }
    } else {
      classtable->semant_error(Current_Class)
          << "Fatal error: unmatched feature type. BUG in compiler!!!" << endl;
    }
  }

  // Exit current scope before analyzing next class
  ObjectIDs.exitscope();
}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant() {
  initialize_constants();

  /* ClassTable constructor may do some semantic analysis */
  classtable = new ClassTable(classes);

  //
  // Now finish first checking of classes, if there is error, just exit
  //
  if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }

  using std::set;
  using std::map;
  // Record method names in each class
  set<Symbol> hash_methods;
  // Record attr names in each class
  set<Symbol> hash_attributes;
  // Record formal names in each method
  set<Symbol> hash_formals;
  // Record all attribute names in each class, map class name to vector of attributes
  map<Symbol, vector<Symbol> > get_attr;

  //
  // First pass: collect info of all methods and attributes
  //

  // Record methods of each class, key is class name
  map<Symbol, vector<pair<Symbol, pair<Formals, Symbol> > > > class_methods;

  for (auto i = classtable->getClasses()->first();
       classtable->getClasses()->more(i);
       i = classtable->getClasses()->next(i)) {
    hash_methods.clear();
    hash_attributes.clear();
    Current_Class = static_cast<class__class*>(classtable->getClasses()->nth(i));
    Features cur_features = Current_Class->features;
    for (auto j = cur_features->first(); cur_features->more(j); j = cur_features->next(j)) {
      Feature cur_feature = cur_features->nth(j);
      if (typeid(*cur_feature) == typeid(method_class)) {
        method_class* cur_method = static_cast<method_class*>(cur_feature);
        Formals formals = cur_method->formals;
        hash_formals.clear();
        for (auto k = formals->first(); formals->more(k); k = formals->next(k)) {
          auto cur_formal = static_cast<formal_class*>(formals->nth(k));
          if (hash_formals.find(cur_formal->name) == hash_formals.end()) {
            hash_formals.insert(cur_formal->name);
          } else {
            classtable->semant_error(Current_Class) << "Error: formal \""
                                                    << cur_formal->name->get_string() << "\" of method \""
                                                    << cur_method->name->get_string() << "\" is redefined." << endl;
          }
        }
        if (hash_methods.find(cur_method->name) != hash_methods.end()) {
          classtable->semant_error(Current_Class) << "Error: class method \""
                                                  << cur_method->name->get_string() << "\" is redefined." << endl;
          continue;
        }
        for (auto k = cur_method->formals->first();
             cur_method->formals->more(k);
             k = cur_method->formals->next(k)) {
          if (static_cast<formal_class*>(cur_method->formals->nth(k))->name == self) {
            classtable->semant_error(Current_Class)
                << "Error: keyword \"self\" could not be used as formal in method \""
                << cur_method->name->get_string() << "\" of class \""
                << Current_Class->name->get_string() << "\"." << endl;
          }
        }
        hash_methods.insert(cur_method->name);
        class_methods[Current_Class->name].push_back(
            make_pair(cur_method->name,
                      make_pair(cur_method->formals,
                                cur_method->return_type)));
      } else if (typeid(*cur_feature) == typeid(attr_class)) {
        // Collect attributes of each class
        attr_class* cur_attr = static_cast<attr_class*>(cur_feature);
        if (cur_attr->name == self) {
          classtable->semant_error(Current_Class)
              << "Error: keyword \"self\" could not be used as class attribute in class \""
              << Current_Class->name->get_string() << "\"." << endl;
        }
        if (hash_attributes.find(cur_attr->name) == hash_attributes.end()) {
          hash_attributes.insert(cur_attr->name);
        } else {
          classtable->semant_error(Current_Class) << "Error: local class attribute \""
                                                  << cur_attr->name->get_string() << "\" is redefined." << endl;
        }
        get_attr[Current_Class->name].push_back(cur_attr->name);
      } else {
        classtable->semant_error(Current_Class)
            << "Fatal error: unknown AST tree node. BUG in compiler!!!" << endl;
        exit(1);
      }
    }
  }

  if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }

  // Record all attributes in classes from a inheritance tree leave to root
  set<Symbol> all_attr;
  // Record all methods in classes from a inheritance tree leave to root
  // This maps a method name to its formals and return type
  map<Symbol, pair<Formals, Symbol> > all_methods;
  // Get all leave nodes according to inheritance tree, these are class names
  vector<Symbol>* all_leaves = classtable->get_leaves();

  //
  // Second pass:
  // 1. Check if all attributes are defined unique in inheritance graph
  // 2. Check if methods are properly overwritten
  //

  for (auto i = 0; i < all_leaves->size(); i++) {
    all_attr.clear();
    Symbol leave = (*all_leaves)[i];
    // Check first condition from leaves of the inheritance graph, and go up to the root
    while (classtable->get_father(leave) != leave) {
      for (auto j = 0; j < get_attr[leave].size(); j++) {
        if (all_attr.find(get_attr[leave][j]) == all_attr.end()) {
          all_attr.insert(get_attr[leave][j]);
        } else {
          classtable->semant_error(classtable->GetNode(leave))
              << "Error: attribute \"" << get_attr[leave][j]->get_string()
              << "\" in class \"" << static_cast<class__class*>(classtable->GetNode(leave))->name->get_string()
              << "\" is overwritten." << endl;
        }
      }
      leave = classtable->get_father(leave);
    }

    // Check if formals are same when overwriting methods
    all_methods.clear();
    leave = (*all_leaves)[i];
    while (true) {
      for (auto j = 0; j < class_methods[leave].size(); j++) {
        if (all_methods.find(class_methods[leave][j].first) == all_methods.end()) {
          all_methods[class_methods[leave][j].first] = class_methods[leave][j].second;
        } else {
          Formals super_formals = class_methods[leave][j].second.first;
          Formals sub_formals = all_methods[class_methods[leave][j].first].first;
          Symbol super_return_type = class_methods[leave][j].second.second;
          Symbol sub_return_type = all_methods[class_methods[leave][j].first].second;
          if (super_return_type != sub_return_type) {
            classtable->semant_error(classtable->GetNode(leave)) << "Error: method \""
                                                                 << class_methods[leave][j].first->get_string()
                                                                 << "\" in class \""
                                                                 << static_cast<class__class*>(classtable->GetNode(
                                                                     leave))->name->get_string()
                                                                 << "\" is overwritten with different return type."
                                                                 << endl;
          }
          if (super_formals->len() != sub_formals->len()) {
            classtable->semant_error(classtable->GetNode(leave)) << "Error: method \""
                                                                 << class_methods[leave][j].first->get_string()
                                                                 << "\" in class \""
                                                                 << static_cast<class__class*>(classtable->GetNode(
                                                                     leave))->name->get_string()
                                                                 << "\" is overwritten with different number of formals."
                                                                 << endl;
          }
          for (auto m = super_formals->first(), n = sub_formals->first();
               super_formals->more(m) && sub_formals->more(n);
               m = super_formals->next(m), n = sub_formals->next(n)) {
            auto super_formal = static_cast<formal_class*>(super_formals->nth(m));
            auto sub_formal = static_cast<formal_class*>(sub_formals->nth(n));
            if (super_formal->type_decl != sub_formal->type_decl) {
              classtable->semant_error(classtable->GetNode(leave))
                  << "Error: type of formal \"" << super_formal->name->get_string()
                  << "\" in method \"" << class_methods[leave][j].first->get_string() << "\" of class \""
                  << static_cast<class__class*>(classtable->GetNode(leave))->name->get_string()
                  << "\" is changed when overwriting method in subclass." << endl;
              break;
            }
          }
        }
      }
      if (leave == Object) { break; }
      leave = classtable->get_father(leave);
    }

  }

  if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }

  //
  // Third pass:
  // 1. Collect all methods of each class
  // 2. Check if all types in class attributes and method formals are defined
  //
  Main = NULL;
  main_meth = NULL;
  Methods.clear();
  for (auto x = classtable->getClasses()->first();
       classtable->getClasses()->more(x);
       x = classtable->getClasses()->next(x)) {
    all_methods.clear();
    // Set global current class
    Current_Class = static_cast<class__class*>(classtable->getClasses()->nth(x));
    Symbol leave = Current_Class->name;
    while (true) {
      for (auto j = 0; j < class_methods[leave].size(); j++) {
        if (all_methods.find(class_methods[leave][j].first) == all_methods.end()) {
          all_methods[class_methods[leave][j].first] = class_methods[leave][j].second;
        }
      }
      if (leave == Object) { break; }
      leave = classtable->get_father(leave);
    }
    Methods[Current_Class->name] = all_methods;

    for (auto i = Current_Class->features->first();
         Current_Class->features->more(i);
         i = Current_Class->features->next(i)) {
      Feature cur_feature = Current_Class->features->nth(i);
      if (typeid(*cur_feature) == typeid(method_class)) {
        method_class* cur_method = static_cast<method_class*>(cur_feature);
        // 0) Check if Main class and main method are defined
        if (strcmp(cur_method->name->get_string(), "main") == 0
            && strcmp(Current_Class->name->get_string(), "Main") == 0) {
          Main = Current_Class->name;
          main_meth = cur_method->name;
        }
        // 1) Add formals into environment
        Formals formals = cur_method->formals;
        for (auto k = formals->first(); formals->more(k); k = formals->next(k)) {
          auto cur_formal = static_cast<formal_class*>(formals->nth(k));
          if (cur_formal->type_decl == SELF_TYPE) {
            classtable->semant_error(Current_Class)
                << "Error: \"SELF_TYPE\" could not be used in formal declaration." << endl;
          } else {
            if (!classtable->HaveClass(cur_formal->type_decl)) {
              classtable->semant_error(Current_Class)
                  << "Error: type \"" << cur_formal->type_decl->get_string()
                  << "\" is used without being defined." << endl;
            }
          }
        }
        Symbol T0;
        if (cur_method->return_type != SELF_TYPE) {
          T0 = cur_method->return_type;
        } else {
          T0 = Current_Class->name;
        }
        if (!classtable->HaveClass(T0)) {
          classtable->semant_error(Current_Class)
              << "Error: type \"" << T0->get_string()
              << "\" is used without being defined." << endl;
        }
      } else if (typeid(*cur_feature) == typeid(attr_class)) {
        attr_class* cur_attr = static_cast<attr_class*>(cur_feature);
        if (cur_attr->type_decl != SELF_TYPE) {
          if (!classtable->HaveClass(cur_attr->type_decl)) {
            classtable->semant_error(Current_Class)
                << "Error: type \"" << cur_attr->type_decl->get_string()
                << "\" is used without being defined." << endl;
          }
        }
      } else {
        classtable->semant_error(Current_Class)
            << "Fatal error: unmatched feature type. BUG in compiler!!!" << endl;
      }
    }
  }

  if (Main == NULL || main_meth == NULL) {
    classtable->semant_error()
        << "Class Main is not defined." << endl;
  }

  if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }

  //
  // Forth pass: type checking for each class
  //
  for (auto i = this->classes->first();
       this->classes->more(i);
       i = this->classes->next(i)) {
    this->classes->nth(i)->type_check();
  }

  if (classtable->errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }
}
