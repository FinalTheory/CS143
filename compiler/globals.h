#ifndef PROJECT_GLOBALS_H
#define PROJECT_GLOBALS_H

#include <map>
#include "symtab.h"
#include "stringtab.h"


using std::map;
using std::pair;


//
// Three symbols from the semantic analyzer (semant.cc) are used.
// If e : No_type, then no code is generated for e.
// Special code is generated for new SELF_TYPE.
// The name "self" also generates code different from other references.
//
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
extern Symbol
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


class globals_impl {
 public:
  typedef struct location_impl {
    char* reg;
    int offset;

    location_impl(char* _reg, int _offset)
        : reg(_reg), offset(_offset) {}
  } * Location;

  static Location new_location(char* reg, int offset) {
    return new location_impl(reg, offset);
  }

  void set_current_class(Symbol class_name) {
    current_class = class_name;
  }

  Symbol get_current_class() {
    return current_class;
  }

  int new_label() {
    return label_index++;
  }

  void init_temp_allocator(int _max_temp) {
    max_temp = _max_temp;
    temp_offset = -1;
  }

  Location alloc_temp_loc();

  void free_temp_loc() {
    assert(abs(temp_offset) >= 1);
    temp_offset++;
  }

  void set_method_offset_for_class(
      Symbol class_name,
      Symbol method_name,
      int offset) {
    auto key = std::make_pair(class_name, method_name);
    method_offset[key] = offset;
  }

  int get_method_offset_for_class(
      Symbol class_name,
      Symbol method_name
  ) {
    auto key = std::make_pair(class_name, method_name);
    assert(method_offset.find(key) !=
           method_offset.end());
    return method_offset[key];
  }

 public:
  map<Symbol, int> classtag, subclasstag_max;
  SymbolTable<Symbol, location_impl> env;

  globals_impl() {
    initialize_constants();
  }

 private:
  int max_temp;
  int temp_offset;
  int label_index = 0;
  Symbol current_class;
  map<pair<Symbol, Symbol>, int> method_offset;

  // Initializing the predefined symbols.
  static void initialize_constants(void);
};

extern globals_impl Globals;

#endif //PROJECT_GLOBALS_H
