#ifndef PROJECT_CLASSTABLE_H
#define PROJECT_CLASSTABLE_H

#include <list>
#include <vector>
#include <set>
#include <map>
#include "cool-tree.h"
#include "symtab.h"

using std::set;
using std::map;
using std::vector;

class CgenNode;

typedef CgenNode* CgenNodeP;

class ClassTable;

typedef ClassTable* ClassTableP;

enum Basicness {
  Basic, NotBasic
};


class CgenNode : public class__class {
 private:
  CgenNodeP parentnd;                        // Parent of class
  std::list<CgenNodeP> children;                  // Children of class
  Basicness basic_status;                    // `Basic' if class is basic
  // `NotBasic' otherwise

 public:
  CgenNode(Class_ c,
           Basicness bstatus,
           ClassTableP class_table);

  void add_child(CgenNodeP child);

  std::list<CgenNodeP>* get_children() { return &children; }

  void set_parentnd(CgenNodeP p);

  CgenNodeP get_parentnd() { return parentnd; }

  int basic() { return (basic_status == Basic); }
};


template<typename Elem>
class InheritanceGraph {
 public:
  void SetRoot(Elem root) {
    // The first element of vector would be root
    // So When using set root, the hole tree should be initialized
    this->Init();
    this->AddNode(root);
  }

  void AddNode(Elem p) {
    vector<int> emptyVec;
    this->Nodes.push_back(p);
    pElem2Idx[p] = this->Nodes.size() - 1;
    EdgeUp.push_back(this->Nodes.size() - 1);
    EdgeDown.push_back(emptyVec);
  };

  bool AddEdge(Elem father, Elem son) {
    if (this->pElem2Idx.find(father) == this->pElem2Idx.end() ||
        this->pElem2Idx.find(son) == this->pElem2Idx.end()) {
      return false;
    }
    auto u = this->pElem2Idx[father];
    auto v = this->pElem2Idx[son];
    this->EdgeDown[u].push_back(v);
    this->EdgeUp[v] = u;
    return true;
  };

  bool CheckCircle() {
    // Check if all nodes are from a root node
    // Check if there is circle in Inheritance Nodes
    vector<bool> flags;
    flags.resize(Nodes.size());
    for (auto i = 0; i < this->Nodes.size(); i++) {
      auto u = i;
      fill(flags.begin(), flags.end(), false);
      while (u != 0 && u != this->EdgeUp[u] && !flags[this->EdgeUp[u]]) {
        u = this->EdgeUp[u];
        flags[u] = true;
      }
      if (u != 0) { return false; }
    }
    size_t sum_edges = 0;
    for (auto i = 0; i < this->EdgeDown.size(); i++) {
      sum_edges += this->EdgeDown[i].size();
    }
    return sum_edges == this->Nodes.size() - 1;
  };

  Elem LCA(Elem a, Elem b) {
    auto u = this->pElem2Idx[a];
    auto v = this->pElem2Idx[b];
    int len1 = 0, len2 = 0;
    while (u != 0) {
      u = EdgeUp[u];
      len1++;
    }
    while (v != 0) {
      v = EdgeUp[v];
      len2++;
    }
    u = this->pElem2Idx[a];
    v = this->pElem2Idx[b];
    if (len1 > len2) {
      int n = len1 - len2;
      while (n--) { u = EdgeUp[u]; }
    } else if (len1 < len2) {
      int n = len2 - len1;
      while (n--) { v = EdgeUp[v]; }
    }
    while (u != v) {
      u = EdgeUp[u];
      v = EdgeUp[v];
    }
    return this->Nodes[u];
  };

  // Seems that this method is useless
  int compare(Elem a, Elem b) {
    auto u = this->pElem2Idx[a];
    auto v = this->pElem2Idx[b];
    while (u != 0) {
      u = EdgeUp[u];
      if (u == v) { return -1; }
    }
    u = this->pElem2Idx[a];
    while (v != 0) {
      v = EdgeUp[v];
      if (u == v) { return 1; }
    }
    return 0;
  };

  bool ConformTo(Elem a, Elem b) {
    if (this->SonOfAll.find(a) != this->SonOfAll.end()) { return true; }
    if (this->SonOfAll.find(b) != this->SonOfAll.end()) { return false; }
    auto u = this->pElem2Idx[a];
    auto v = this->pElem2Idx[b];
    if (v == 0) { return true; }
    while (u != 0) {
      if (u == v) { return true; }
      u = EdgeUp[u];
    }
    return false;
  };

  vector<Elem>* get_leaves() {
    auto res = new vector<Elem>;
    for (auto i = 0; i < this->Nodes.size(); i++) {
      if (this->EdgeDown[i].size() == 0) { res->push_back(Nodes[i]); }
    }
    return res;
  };

  Elem get_father(Elem son) {
    return this->Nodes[this->EdgeUp[this->pElem2Idx[son]]];
  };

  void AddSon(Elem son) {
    this->SonOfAll.insert(son);
  }

  bool is_leave(Elem node) {
    return this->EdgeDown[this->pElem2Idx[node]].size() == 0;
  }

 private:
  void Init() {
    this->Nodes.clear();
    this->EdgeDown.clear();
    this->EdgeUp.clear();
    this->pElem2Idx.clear();
    this->SonOfAll.clear();
  }

  vector<Elem> Nodes;
  vector<vector<int>> EdgeDown;
  vector<int> EdgeUp;
  map<Elem, int> pElem2Idx;
  set<Elem> SonOfAll;
};


class ClassTable : public SymbolTable<Symbol, CgenNode> {
  //
  // Methods for code generation
  //
 public:
  ClassTable(Classes);

  ~ClassTable();

  void code(ostream& s);

  CgenNodeP root();

 private:
  std::list<CgenNodeP> classes_;

  int stringclasstag;
  int intclasstag;
  int boolclasstag;

  // The following methods emit code for
  // constants and global declarations.

  void code_global_data(ostream&);

  void code_global_text(ostream&);

  void code_bools(int, ostream&);

  void code_select_gc(ostream&);

  void code_constants(ostream&);

  void code_class_nameTab(ostream&);

  void code_class_objTab(ostream&);

  void code_proto_object(ostream&);

  void code_dispatch_table(ostream&);

  void code_object_initializer(ostream&);

  void code_class_methods(ostream&);

// The following creates an inheritance graph from
// a list of classes.  The graph is implemented as
// a tree of `CgenNode', and class names are placed
// in the base class symbol table.

  void install_basic_classes();

  void install_class(CgenNodeP nd);

  // Check if basic classes are redefined
  void check_classes(Classes cs);

  void install_classes(Classes cs);

  void build_inheritance_tree();

  void set_relations(CgenNodeP nd);

  void setup_class_tags();

  int setup_class_tags_helper(CgenNodeP root, int& index);

  //
  // Methods for semant
  //
 public:
  int errors() { return semant_errors; }

  ostream& semant_error();

  ostream& semant_error(Class_ c);

  ostream& semant_error(Symbol filename, tree_node* t);

  Symbol LCA(Symbol a, Symbol b) {
    return this->Graph.LCA(a, b);
  };

  vector<Symbol>* get_leaves() {
    return this->Graph.get_leaves();
  };

  bool is_leave(Symbol node) {
    return this->Graph.is_leave(node);
  }

  Symbol get_father(Symbol son) {
    return this->Graph.get_father(son);
  };

  bool ConformTo(Symbol a, Symbol b) {
    return this->Graph.ConformTo(a, b);
  };

  Class_ GetNode(Symbol name) {
    return this->class_name2node[name];
  };

  bool HaveClass(Symbol name) {
    return this->class_name2node.find(name) != this->class_name2node.end() ||
           this->SpecialClass.find(name) != this->SpecialClass.end();
  }

  std::list<CgenNodeP> getClasses() {
    return classes_;
  }

 private:
  void check_inheritance();

  ostream& error_stream;
  int semant_errors;
  set<Symbol> AllSymbols;
  InheritanceGraph<Symbol> Graph;
  // Map from class name to class node in AST
  map<Symbol, Class_> class_name2node;
  set<Symbol> SpecialClass;
};

#endif //PROJECT_CLASSTABLE_H
