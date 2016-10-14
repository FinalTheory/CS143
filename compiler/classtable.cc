#include <typeinfo>
#include <vector>
#include <sstream>
#include "cgen.h"
#include "classtable.h"
#include "emit.h"
#include "globals.h"

using std::max;
using std::pair;
using std::vector;
using std::stringstream;


extern int cgen_debug;

//////////////////////////////////////////////////////////////////////////////
//
//  ClassTable methods
//
//////////////////////////////////////////////////////////////////////////////

ClassTable::ClassTable(Classes classes)
    : intclasstag(-1),
      boolclasstag(-1),
      stringclasstag(-1),
      semant_errors(0),
      error_stream(cerr) {
  if (cgen_debug) { cout << "Building " << __func__ << endl; }
  enterscope();

  install_basic_classes();

  check_classes(classes);

  install_classes(classes);

  // TODO: remove this design
  SpecialClass.insert(prim_slot);

  check_inheritance();

  // Now finish first checking of classes, if there is error, just exit
  if (errors()) {
    cerr << "Compilation halted due to static semantic errors." << endl;
    exit(1);
  }

  build_inheritance_tree();

  setup_class_tags();
}

ClassTable::~ClassTable() {
  exitscope();
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

void ClassTable::check_inheritance() {
  this->AllSymbols.insert(Object);
  this->Graph.SetRoot(Object);
  // First, check if all classes are defined only once
  // And then insert them into set and graph nodes
  for (auto cur_class: classes_) {
    this->class_name2node[cur_class->name] = cur_class;
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
  for (auto cur_class: classes_) {
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

void ClassTable::setup_class_tags() {
  int index = 0;
  // Setup class tags using DFS
  setup_class_tags_helper(root(), index);
  intclasstag = Globals.classtag[Int];
  boolclasstag = Globals.classtag[Bool];
  stringclasstag = Globals.classtag[Str];
  classes_.sort([](const CgenNodeP& a, const CgenNodeP& b) {
    return Globals.classtag[a->name] < Globals.classtag[b->name];
  });
  if (cgen_debug) {
    cout << "Class tags:" << endl;
    for (auto cls: classes_) {
      cout << cls->name << ": " << Globals.classtag[cls->name]
           << " - " << Globals.subclasstag_max[cls->name] << endl;
    }
  }
}

int ClassTable::setup_class_tags_helper(CgenNodeP root, int& index) {
  auto classtag = index++;
  Globals.classtag[root->name] = classtag;
  for (auto nd: *root->get_children()) {
    classtag = max(setup_class_tags_helper(nd, index), classtag);
  }
  Globals.subclasstag_max[root->name] = classtag;
  return classtag;
}


void ClassTable::install_basic_classes() {

// The tree package uses these globals to annotate the classes built below.
  //curr_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

//
// A few special class names are installed in the lookup table but not
// the class list.  Thus, these classes exist, but are not part of the
// inheritance hierarchy.
// No_class serves as the parent of Object and the other special classes.
// SELF_TYPE is the self class; it cannot be redefined or inherited.
// prim_slot is a class known to the code generator.
//
  addid(No_class,
        new CgenNode(class_(No_class, No_class, nil_Features(), filename),
                     Basic, this));
  addid(SELF_TYPE,
        new CgenNode(class_(SELF_TYPE, No_class, nil_Features(), filename),
                     Basic, this));
  addid(prim_slot,
        new CgenNode(class_(prim_slot, No_class, nil_Features(), filename),
                     Basic, this));

//
// The Object class has no parent class. Its methods are
//        cool_abort() : Object    aborts the program
//        type_name() : Str        returns a string representation of class name
//        copy() : SELF_TYPE       returns a copy of the object
//
// There is no need for method bodies in the basic classes---these
// are already built in to the runtime system.
//
  install_class(
      new CgenNode(
          class_(Object,
                 No_class,
                 append_Features(
                     append_Features(
                         single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
                         single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
                     single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
                 filename),
          Basic, this));

//
// The IO class inherits from Object. Its methods are
//        out_string(Str) : SELF_TYPE          writes a string to the output
//        out_int(Int) : SELF_TYPE               "    an int    "  "     "
//        in_string() : Str                    reads a string from the input
//        in_int() : Int                         "   an int     "  "     "
//
  install_class(
      new CgenNode(
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
                 filename),
          Basic, this));

//
// The Int class has no methods and only a single attribute, the
// "val" for the integer.
//
  install_class(
      new CgenNode(
          class_(Int,
                 Object,
                 single_Features(attr(val, prim_slot, no_expr())),
                 filename),
          Basic, this));

//
// Bool also has only the "val" slot.
//
  install_class(
      new CgenNode(
          class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())), filename),
          Basic, this));

//
// The class Str has a number of slots and operations:
//       val                                  ???
//       str_field                            the string itself
//       length() : Int                       length of the string
//       concat(arg: Str) : Str               string concatenation
//       substr(arg: Int, arg2: Int): Str     substring
//
  install_class(
      new CgenNode(
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
                 filename),
          Basic, this));

}

// ClassTable::install_class
// ClassTable::install_classes
//
// install_classes enters a list of classes in the symbol table.
//
void ClassTable::install_class(CgenNodeP nd) {
  Symbol name = nd->get_name();
  // Add class to the list of classes
  // and the symbol table.
  // Notice that the class might be illegal
  classes_.push_back(nd);
  addid(name, nd);
}

void ClassTable::check_classes(Classes cs) {
  for (auto i = cs->first();
       cs->more(i);
       i = cs->next(i)) {
    class__class* cur_class = static_cast<class__class*>(cs->nth(i));
    if (cur_class->name == SELF_TYPE
        || cur_class->name == Object
        || cur_class->name == Str
        || cur_class->name == Bool
        || cur_class->name == Int) {
      this->semant_error(cur_class)
          << "Fatal error: redefinition of basic class \""
          << cur_class->name->get_string() << "\"." << endl;
    }
  }
}

void ClassTable::install_classes(Classes cs) {
  for (int i = cs->first(); cs->more(i); i = cs->next(i)) {
    install_class(new CgenNode(cs->nth(i), NotBasic, this));
  }
}

//
// ClassTable::build_inheritance_tree
//
void ClassTable::build_inheritance_tree() {
  for (auto iter: classes_) {
    set_relations(iter);
  }
}

//
// ClassTable::set_relations
//
// Takes a CgenNode and locates its, and its parent's, inheritance nodes
// via the class table.  Parent and child pointers are added as appropriate.
//
void ClassTable::set_relations(CgenNodeP nd) {
  CgenNode* parent_node = probe(nd->get_parent());
  nd->set_parentnd(parent_node);
  parent_node->add_child(nd);
}

void CgenNode::add_child(CgenNodeP n) {
  children.push_back(n);
}

void CgenNode::set_parentnd(CgenNodeP p) {
  assert(parentnd == NULL);
  assert(p != NULL);
  parentnd = p;
}

//***************************************************
//
//  Emit code to start the .data segment and to
//  declare the global names.
//
//***************************************************
void ClassTable::code_global_data(ostream& str) {
  Symbol main = idtable.lookup_string(MAINNAME);
  Symbol string = idtable.lookup_string(STRINGNAME);
  Symbol integer = idtable.lookup_string(INTNAME);
  Symbol boolc = idtable.lookup_string(BOOLNAME);

  str << "\t.data\n" << ALIGN;
  //
  // The following global names must be defined first.
  //
  str << GLOBAL << CLASSNAMETAB << endl;
  str << GLOBAL;
  emit_protobj_ref(main, str);
  str << endl;
  str << GLOBAL;
  emit_protobj_ref(integer, str);
  str << endl;
  str << GLOBAL;
  emit_protobj_ref(string, str);
  str << endl;
  str << GLOBAL;
  falsebool.code_ref(str);
  str << endl;
  str << GLOBAL;
  truebool.code_ref(str);
  str << endl;
  str << GLOBAL << INTTAG << endl;
  str << GLOBAL << BOOLTAG << endl;
  str << GLOBAL << STRINGTAG << endl;

  //
  // We also need to know the tag of the Int, String, and Bool classes
  // during code generation.
  //
  str << INTTAG << LABEL
      << WORD << intclasstag << endl;
  str << BOOLTAG << LABEL
      << WORD << boolclasstag << endl;
  str << STRINGTAG << LABEL
      << WORD << stringclasstag << endl;
}


//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************

void ClassTable::code_global_text(ostream& str) {
  str << GLOBAL << HEAP_START << endl
      << HEAP_START << LABEL
      << WORD << 0 << endl
      << "\t.text" << endl
      << GLOBAL;
  emit_init_ref(idtable.add_string("Main"), str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Int"), str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("String"), str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Bool"), str);
  str << endl << GLOBAL;
  emit_method_ref(idtable.add_string("Main"), idtable.add_string("main"), str);
  str << endl;
}

void ClassTable::code_bools(int boolclasstag, ostream& str) {
  falsebool.code_def(str, boolclasstag);
  truebool.code_def(str, boolclasstag);
}

void ClassTable::code_select_gc(ostream& str) {
  //
  // Generate GC choice constants (pointers to GC functions)
  //
  str << GLOBAL << "_MemMgr_INITIALIZER" << endl;
  str << "_MemMgr_INITIALIZER:" << endl;
  str << WORD << gc_init_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_COLLECTOR" << endl;
  str << "_MemMgr_COLLECTOR:" << endl;
  str << WORD << gc_collect_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_TEST" << endl;
  str << "_MemMgr_TEST:" << endl;
  str << WORD << (cgen_Memmgr_Test == GC_TEST) << endl;
}


//********************************************************
//
// Emit code to reserve space for and initialize all of
// the constants.  Class names should have been added to
// the string table (in the supplied code, is is done
// during the construction of the inheritance graph), and
// code for emitting string constants as a side effect adds
// the string's length to the integer table.  The constants
// are emmitted by running through the stringtable and inttable
// and producing code for each entry.
//
//********************************************************

void ClassTable::code_constants(ostream& str) {
  // Add constants that are required by the code generator.
  stringtable.add_string(EMPTY_STR);
  inttable.add_string(STR_ZERO);

  stringtable.code_string_table(str, stringclasstag);
  inttable.code_string_table(str, intclasstag);
  code_bools(boolclasstag, str);
}

void ClassTable::code_class_nameTab(ostream& str) {
  str << CLASSNAMETAB << LABEL;
  for (auto iter: classes_) {
    auto name = iter->get_name()->get_string();
    auto entry = stringtable.lookup_string(name);
    str << WORD;
    entry->code_ref(str);
    str << endl;
  }
}

void ClassTable::code_class_objTab(ostream& str) {
  str << CLASSOBJTAB << LABEL;
  for (auto iter: classes_) {
    auto cls_name = iter->get_name()->get_string();
    str << WORD << cls_name << PROTOBJ_SUFFIX << endl
        << WORD << cls_name << CLASSINIT_SUFFIX << endl;
  }
}

static int
code_proto_object_helper(CgenNodeP cls, ostream& s) {
  auto parent = cls->get_parentnd();
  // If this node have a parent
  // then it is just the Object class
  int size = DEFAULT_OBJFIELDS;
  if (parent->get_name() != No_class) {
    size = code_proto_object_helper(parent, s);
  }
  auto features = cls->features;
  for (auto i = features->first();
       features->more(i);
       i = features->next(i)) {
    auto cur_feature = features->nth(i);
    if (typeid(*cur_feature) == typeid(attr_class)) {
      size++;
      auto attr = static_cast<attr_class*>(cur_feature);
      s << WORD;
      if (attr->type_decl == Int) {
        inttable.lookup_string(STR_ZERO)->code_ref(s);
      } else if (attr->type_decl == Bool) {
        falsebool.code_ref(s);
      } else if (attr->type_decl == Str) {
        stringtable.lookup_string(EMPTY_STR)->code_ref(s);
      } else {
        s << STR_ZERO;
      }
      s << endl;
    }
  }
  return size;
}

void ClassTable::code_proto_object(ostream& str) {
  int idx = 0;
  for (auto cls: classes_) {
    stringstream ss;
    auto cls_name = cls->get_name()->get_string();
    auto size = code_proto_object_helper(cls, ss);
    str << WORD << "-1" << endl                       // -1 eye catcher
        << cls_name << PROTOBJ_SUFFIX << LABEL        // class label
        << WORD << idx << endl                        // class tag
        << WORD << size << endl                       // object size
        << WORD << cls_name << DISPTAB_SUFFIX << endl // dispatch table
        << ss.str();                                  // data attributes
    idx++;
  }
}

static void
code_dispatch_table_helper(Symbol class_name,
                           CgenNodeP cur,
                           vector<pair<Symbol, Symbol>>& disp_table) {
  auto parent = cur->get_parentnd();
  // If this node have a parent
  if (parent->get_name() != No_class) {
    code_dispatch_table_helper(class_name, parent, disp_table);
  }
  auto features = cur->features;
  for (auto i = features->first();
       features->more(i);
       i = features->next(i)) {
    auto cur_feature = features->nth(i);
    // if this feature is a method
    // then we add it into dispatch table
    if (typeid(*cur_feature) == typeid(method_class)) {
      auto method_name = static_cast<method_class*>(cur_feature)->name;
      int offset = -1;
      auto pair = std::make_pair(cur->get_name(), method_name);
      // We should first check if method with same name
      // already exists in the dispatch table
      // which means that this method is overwritten by its sub class
      for (int i = 0; i < disp_table.size(); i++) {
        auto& item = disp_table[i];
        if (item.second == method_name) {
          item = pair;
          offset = i;
          break;
        }
      }
      // If not found, then we append this method
      // at the end of dispatch table
      if (offset == -1) {
        disp_table.push_back(pair);
        offset = int(disp_table.size()) - 1;
      }
      // This is used to record methods of the class
      // So this parameter "class_name" is unchanged
      Globals.set_method_offset_for_class(
          class_name, method_name, offset);
    }
  }
}

void ClassTable::code_dispatch_table(ostream& str) {
  vector<pair<Symbol, Symbol>> disp_table;
  for (auto cls: classes_) {
    str << cls->get_name() << DISPTAB_SUFFIX << LABEL;
    // Notice that we pass the name of current class in
    code_dispatch_table_helper(cls->get_name(), cls, disp_table);
    for (auto& item: disp_table) {
      str << WORD;
      // We use this name to ref the absolute code address of a method
      emit_method_ref(item.first, item.second, str);
      str << endl;
    }
  }
}

static void method_call_on_init(int num_temp, ostream& s) {
  // Save previous stack pointer, frame pointer and return address
  emit_store(FP, 0, SP, s);
  emit_store(SELF, -1, SP, s);
  emit_store(RA, -2, SP, s);
  // Make $fp points to the return address
  emit_addiu(FP, SP, -(SAVED_REGS - 1) * WORD_SIZE, s);
  emit_addiu(SP, SP, -WORD_SIZE * (SAVED_REGS + num_temp), s);
  // Since the object address stores in $a0
  // we copy it to $s0 -- the self pointer
  emit_move(SELF, ACC, s);
}

static void
method_call_on_return(int num_temp,
                      int num_para,
                      ostream& s) {
  emit_load(RA, 0, FP, s);
  emit_load(SELF, 1, FP, s);
  emit_load(FP, 2, FP, s);
  emit_addiu(SP, SP, WORD_SIZE * (SAVED_REGS + num_temp + num_para), s);
  emit_return(s);
}

static void load_attr_for_class(CgenNodeP cls, int& offset) {
  auto parent = cls->get_parentnd();
  // If this node have a parent
  if (parent->get_name() != No_class) {
    load_attr_for_class(parent, offset);
  }

  auto features = cls->features;
  for (int i = features->first();
       features->more(i);
       i = features->next(i)) {
    auto feature = features->nth(i);
    if (typeid(*feature) == typeid(attr_class)) {
      auto attr = static_cast<attr_class*>(feature);
      Globals.env.addid(attr->name,
                        Globals.new_location(SELF, offset++));
    }
  }
}

void ClassTable::code_object_initializer(ostream& str) {
  for (auto cls: classes_) {
    Globals.set_current_class(cls->get_name());
    Globals.env.enterscope();

    int offset = DEFAULT_OBJFIELDS;
    load_attr_for_class(cls, offset);

    // First pass:
    // calculate number of temp locations
    int max_temp = 0;
    auto features = cls->features;
    for (int i = features->first();
         features->more(i);
         i = features->next(i)) {
      auto feature = features->nth(i);
      if (typeid(*feature) == typeid(attr_class)) {
        auto attr = static_cast<attr_class*>(feature);
        max_temp = max(max_temp, attr->init->temporaries());
      }
    }

    Globals.init_temp_allocator(max_temp);

    str << cls->get_name() << CLASSINIT_SUFFIX << LABEL;
    method_call_on_init(max_temp, str);

    auto parent = cls->get_parentnd();
    // If this node have a parent
    // then we first call <parent>_init
    if (parent->get_name() != No_class) {
      str << JAL;
      emit_init_ref(parent->get_name(), str);
      str << endl;
    }

    // Second pass: generate code to initialize attributes
    for (auto i = features->first();
         features->more(i);
         i = features->next(i)) {
      auto feature = features->nth(i);
      if (typeid(*feature) == typeid(attr_class)) {
        auto attr = static_cast<attr_class*>(feature);
        // Generate code for init expr
        // As for no_expr, it returns nothing
        // But we still need to check type here
        if (typeid(*attr->init) != typeid(no_expr_class)) {
          attr->init->code(str);
          // Now the result of this expr stores in $a0
          // we move it into the location of this attr
          auto loc = Globals.env.lookup(attr->name);
          assert(loc != nullptr);
          emit_store(ACC, loc->offset, loc->reg, str);
        }
      }
    }
    // We copy self pointer back
    // to ensure that $a0 is unmodified
    // But this is only for <class>_init
    emit_move(ACC, SELF, str);
    method_call_on_return(max_temp, 0, str);

    Globals.env.exitscope();
  }
}

void ClassTable::code_class_methods(ostream& str) {
  for (auto cls: classes_) {
    if (cls->basic()) { continue; }
    Globals.set_current_class(cls->get_name());
    Globals.env.enterscope();

    int offset = DEFAULT_OBJFIELDS;
    load_attr_for_class(cls, offset);

    auto features = cls->features;
    for (int i = features->first();
         features->more(i);
         i = features->next(i)) {
      auto feature = features->nth(i);
      if (typeid(*feature) == typeid(method_class)) {
        auto method = static_cast<method_class*>(feature);
        Globals.env.enterscope();

        // Add formal parameters into env
        // Notice how do we calculate offset here
        auto formals = method->formals;
        for (int k = formals->first(),
                 arg_offset = SAVED_REGS + formals->len() - 1;
             formals->more(k);
             k = formals->next(k), arg_offset--) {
          auto formal = static_cast<formal_class*>(formals->nth(k));
          Globals.env.addid(formal->name,
                            Globals.new_location(FP, arg_offset));
        }

        int max_temp = method->expr->temporaries();
        Globals.init_temp_allocator(max_temp);

        emit_method_ref(cls->get_name(), method->name, str);
        str << LABEL;
        method_call_on_init(max_temp, str);
        method->expr->code(str);
        method_call_on_return(max_temp, formals->len(), str);

        Globals.env.exitscope();
      }
    }

    Globals.env.exitscope();
  }
}

void ClassTable::code(ostream& s) {
  if (cgen_debug) { cout << "coding global data" << endl; }
  code_global_data(s);

  if (cgen_debug) { cout << "choosing gc" << endl; }
  code_select_gc(s);

  if (cgen_debug) { cout << "coding constants" << endl; }
  code_constants(s);

  if (cgen_debug) { cout << "coding for class_nameTab" << endl; }
  code_class_nameTab(s);

  if (cgen_debug) { cout << "coding for class_objTab" << endl; }
  code_class_objTab(s);

  if (cgen_debug) { cout << "coding for for dispatch tables" << endl; }
  code_dispatch_table(s);

  if (cgen_debug) { cout << "coding for prototype objects" << endl; }
  code_proto_object(s);

  if (cgen_debug) { cout << "coding global text" << endl; }
  code_global_text(s);

  if (cgen_debug) { cout << "coding object initializer" << endl; }
  code_object_initializer(s);

  if (cgen_debug) { cout << "coding class methods" << endl; }
  code_class_methods(s);
}

CgenNodeP ClassTable::root() {
  return probe(Object);
}

///////////////////////////////////////////////////////////////////////
//
// CgenNode methods
//
///////////////////////////////////////////////////////////////////////

CgenNode::CgenNode(Class_ nd, Basicness bstatus, ClassTableP ct) :
    class__class((const class__class&)*nd),
    parentnd(NULL),
    children(),
    basic_status(bstatus) {
  stringtable.add_string(name->get_string());          // Add class name to string table
}
