#include <assert.h>
#include "emit.h"
#include "cgen.h"
#include "globals.h"

extern char* pad(int n);

//
// Explicit template instantiations.
// Comment out for versions of g++ prior to 2.7
//
template
class StringTable<IdEntry>;

template
class StringTable<StringEntry>;

template
class StringTable<IntEntry>;

Entry::Entry(const char* s, int l, int i)
    : str(s), len(l), index(i) {}

ostream& Entry::print(ostream& s) const {
  return s << "{" << str << ", " << len << ", " << index << "}\n";
}

ostream& operator<<(ostream& s, const Entry& sym) {
  return s << sym.get_string();
}


ostream& operator<<(ostream& s, Symbol sym) {
  return s << *sym;
}

const char* Entry::get_string() const {
  return str.c_str();
}

int Entry::get_index() const {
  return index;
}

int Entry::get_len() const {
  return len;
}

// A Symbol is a pointer to an Entry.  Symbols are stored directly
// as nodes of the abstract syntax tree defined by the cool-tree.aps.
// The APS package requires that copy and print (called dump) functions
// be defined for components of the abstract syntax tree.
//
Symbol copy_Symbol(const Symbol s) {
  return s;
}

void dump_Symbol(ostream& s, int n, Symbol sym) {
  s << pad(n) << sym << endl;
}

StringEntry::StringEntry(const char* s, int l, int i) : Entry(s, l, i) {}

IdEntry::IdEntry(const char* s, int l, int i) : Entry(s, l, i) {}

IntEntry::IntEntry(const char* s, int l, int i) : Entry(s, l, i) {}

IdTable idtable;
IntTable inttable;
StrTable stringtable;

//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);

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
  emit_string_constant(s, str.c_str());                                   // ascii string
  s << ALIGN;                                                             // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the
// stringtable.
//
void StrTable::code_string_table(ostream& s, int stringclasstag) {
  for (auto p: tbl) {
    p->code_def(s, stringclasstag);
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
  for (auto p: tbl) {
    p->code_def(s, intclasstag);
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
