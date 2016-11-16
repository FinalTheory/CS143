#ifndef _STRINGTAB_H_
#define _STRINGTAB_H_

#include <assert.h>
#include <string.h>
#include <string>
#include <set>
#include <memory>
#include <algorithm>
#include "cool-io.h"

#define MAXSIZE 1000000

using std::min;
using std::string;
using std::function;
using std::shared_ptr;
using std::make_shared;

class Entry;

typedef Entry* Symbol;

extern ostream& operator<<(ostream& s, const Entry& sym);

extern ostream& operator<<(ostream& s, Symbol sym);

/////////////////////////////////////////////////////////////////////////
//
//  String Table Entries
//
/////////////////////////////////////////////////////////////////////////

class Entry {
 protected:
  string str;     // the string it self
  int len;       // the length of the string (without trailing \0)
  int index;     // a unique index for each string
 public:
  Entry(const char* s, int l, int i);

  bool operator<(Entry& other) {
    return str < other.str;
  }

  ostream& print(ostream& s) const;

  // Return the str and len components of the Entry.
  const char* get_string() const;

  int get_len() const;

  // Return the unique index of string
  int get_index() const;
};

//
// There are three kinds of string table entries:
//   a true string, an string representation of an identifier, and 
//   a string representation of an integer.
//
// Having separate tables is convenient for code generation.  Different
// data definitions are generated for string constants (StringEntry) and 
// integer  constants (IntEntry).  Identifiers (IdEntry) don't produce
// static data definitions.
//
// code_def and code_ref are used by the code to produce definitions and
// references (respectively) to constants.  
//
class StringEntry : public Entry {
 public:
  void code_def(ostream& str, int stringclasstag);

  void code_ref(ostream& str);

  StringEntry(const char* s, int l, int i);
};

class IdEntry : public Entry {
 public:
  IdEntry(const char* s, int l, int i);
};

class IntEntry : public Entry {
 public:
  void code_def(ostream& str, int intclasstag);

  void code_ref(ostream& str);

  IntEntry(const char* s, int l, int i);
};

typedef StringEntry* StringEntryP;
typedef IdEntry* IdEntryP;
typedef IntEntry* IntEntryP;

//////////////////////////////////////////////////////////////////////////
//
//  String Tables
//
//////////////////////////////////////////////////////////////////////////

template<class Elem>
class StringTable {
 protected:
  using ElemP = shared_ptr<Elem>;
  // a string table is a STL set
  std::set<ElemP, function<bool(ElemP, ElemP)>> tbl;
  // the current index
  int index;
 public:
  StringTable() : tbl([](ElemP a, ElemP b) {
    return *a < *b;
  }), index(0) {}

  // The following methods each add a string to the string table.
  // Only one copy of each string is maintained.
  // Returns a pointer to the string table entry with the string.

  // add the prefix of s of length maxchars
  Elem* add_string(const char* s, int maxchars);

  // add the (null terminated) string s
  Elem* add_string(const char* s);

  // add the string representation of an integer
  Elem* add_int(int i);

  // lookup an element using its index
  Elem* lookup(int index);

  // lookup an element using its string
  Elem* lookup_string(const char* s);
};

class IdTable : public StringTable<IdEntry> {
};

class StrTable : public StringTable<StringEntry> {
 public:
  void code_string_table(ostream&, int classtag);
};

class IntTable : public StringTable<IntEntry> {
 public:
  void code_string_table(ostream&, int classtag);
};

//
// A string table is implemented a linked list of Entrys.  Each Entry
// in the list has a unique string.
//

template<class Elem>
Elem* StringTable<Elem>::add_string(const char* s) {
  return add_string(s, MAXSIZE);
}

//
// Add a string requires two steps.  First, the set is searched; if the
// string is found, a pointer to the existing Entry for that string is
// returned.  If the string is not found, a new Entry is created and added
// to the list.
//
template<class Elem>
Elem* StringTable<Elem>::add_string(
    const char* s, int maxchars) {
  int len = min((int)strlen(s), maxchars);
  auto key = make_shared<Elem>(s, len, index + 1);
  auto iter = tbl.find(key);
  if (iter == tbl.end()) {
    index++;
    tbl.insert(key);
    return key.get();
  } else {
    return iter->get();
  }
}

//
// To look up a string, the set is scanned until a matching Entry is located.
// If no such entry is found, an assertion failure occurs.  Thus, this function
// is used only for strings that one expects to find in the table.
//
template<class Elem>
Elem* StringTable<Elem>::lookup_string(const char* s) {
  auto key = make_shared<Elem>(s, strlen(s), index);
  auto iter = tbl.find(key);
  if (iter != tbl.end()) { return iter->get(); }
  assert(false);   // fail if string is not found
  return nullptr; // to avoid compiler warning
}

//
// lookup is similar to lookup_string, but uses the index of the string
// as the key.
//
template<class Elem>
Elem* StringTable<Elem>::lookup(int ind) {
  for (auto ptr: tbl) {
    if (ptr->get_index() == ind) {
      return ptr.get();
    }
  }
  assert(false);  // fail if string is not found
  return nullptr; // to avoid compiler warning
}

//
// add_int adds the string representation of an integer to the list.
//
template<class Elem>
Elem* StringTable<Elem>::add_int(int i) {
  static char buf[32];
  snprintf(buf, 32, "%d", i);
  return add_string(buf);
}

extern IdTable idtable;
extern IntTable inttable;
extern StrTable stringtable;

#endif
