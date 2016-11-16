// The symbol table package.
//
// Create a symbol table with :
// SymbolTable<thing to look up on, info to store> name();
//
// You must enter a scope before adding anything to the symbol table.

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include <memory>
#include <map>
#include <list>
#include "cool-io.h"

using std::unique_ptr;
using std::shared_ptr;

// TODO: edit comments below
// SymbolTable<SYM,DAT> describes a symbol table mapping symbols of
//    type `SYM' to data of type `DAT *'.  It is implemented as a
//    list of lists of `SymtabEntry<SYM,DAT> *'.  The inner list is
//    a scope, a mapping from symbols to data, and the outer list is
//    a list of scopes. 
//
//    `scope_list_' points to the current_ top scope.
//
//    `enterscope' makes the table point to a new scope whose parent
//       is the scope it pointed to previously.
//
//    `exitscope' makes the table point to the parent scope of the
//        current_ scope.  Note that the old child scope is not
//        deallocated.  One may save the state of a symbol table
//        at a given point by copying it with `operator ='
//
//    `addid(s,i)' adds a symbol table entry to the current_ scope of
//        the symbol table mapping symbol `s' to data `d'.  The old
//        top scope isn't modified; a new scope is created whose
//        entry list is the new entry followed by the old entry list,
//        and whose tail is the old top scope's parent.  The table
//        is made to point to this new scope.
//
//    `lookup(s)' looks for the symbol `s', starting at the top scope
//        and proceeding down the list of scopes until either an
//        entry is found whose `get_id()' equals `s', or the end of
//        the root scope is reached.  It returns the data item
//        associated with the entry, or NULL if no such entry exists.
//
//    
//    `probe(s)' checks the top scope for an entry whose `get_id()'
//        equals `s', and returns the entry's `get_info()' if
//        found, and NULL otherwise.
//
//    `dump()' prints the symbols in the symbol table.
//

template<class SYM, class DAT>
class SymbolTable {
  using ScopeImpl = std::map<SYM, DAT>;
  using Scope = unique_ptr<ScopeImpl>;
  using ScopeListImpl = std::list<Scope>;
  using ScopeList = unique_ptr<ScopeListImpl>;
  using ScopePointer = typename ScopeListImpl::iterator;
 private:
  ScopeList scope_list_;
  ScopePointer current_;

 public:
  SymbolTable()
      : scope_list_(new ScopeListImpl),
        current_(scope_list_->end()) {}

  void fatal_error(char* msg) {
    cerr << msg << "\n";
    exit(-1);
  }

  // Enter a new scope.  A symbol table is organized as a list of
  // std::map. The head of the list is the innermost scope, the tail
  // holds the outer scopes.  A scope must be entered before anything
  // can be added to the table.
  void enterscope() {
    scope_list_->push_front(Scope(new ScopeImpl));
    current_ = scope_list_->begin();
  }

  // Pop the first scope off of the symbol table.
  void exitscope() {
    // It is an error to exit a scope that doesn't exist.
    if (current_ == scope_list_->end()) {
      fatal_error("exitscope: Can't remove scope from an empty symbol table.");
    }
    scope_list_->pop_front();
    current_ = scope_list_->begin();
  }

  // Add an item to the symbol table.
  void addid(SYM s, DAT i) {
    // There must be at least one scope to add a symbol.
    if (current_ == scope_list_->end()) {
      fatal_error("addid: Can't add a symbol without a scope.");
    }
    (*current_)->insert(std::make_pair(s, i));
  }

  // Lookup an item through all scopes of the symbol table.  If found
  // it returns the associated information field, if not returns empty DAT.
  DAT lookup(SYM s) {
    for (auto& scope: *scope_list_) {
      auto iter = scope->find(s);
      if (iter != scope->end()) {
        return (*iter).second;
      }
    }
    return DAT();
  }

  // probe the symbol table.  Check the top scope (only) for the item
  // 's'.  If found, return the information field. If not return empty DAT.
  DAT probe(SYM s) {
    if (current_ == scope_list_->end()) {
      fatal_error("probe: No scope in symbol table.");
    }
    auto iter = (*current_)->find(s);
    if (iter != (*current_)->end()) {
      return (*iter).second;
    } else {
      return DAT();
    }
  }

  // Prints out the contents of the symbol table
  void dump() {
    for (auto& scope: *scope_list_) {
      cerr << "\nScope: \n";
      for (auto& item: *scope) {
        cerr << "  " << item.first << endl;
      }
    }
  }

};

#endif

