#ifndef SEMANT_H_
#define SEMANT_H_

#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <assert.h>
#include <iostream>
#include <typeinfo>
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

using std::map;
using std::set;
using std::fill;
using std::pair;
using std::vector;
using std::make_pair;

#define TRUE 1
#define FALSE 0

class ClassTable;

typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance Nodes.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

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

  vector<Elem> *get_leaves() {
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
  vector<vector<int> > EdgeDown;
  vector<int> EdgeUp;
  map<Elem, int> pElem2Idx;
  set<Elem> SonOfAll;
};


class ClassTable {
 private:
  int semant_errors;

  void install_basic_classes();

  void check_inheritance();

  ostream &error_stream;
  set<Symbol> AllSymbols;
  InheritanceGraph<Symbol> Graph;
  Classes classes;
  // Map from class name to class node in AST
  map<Symbol, Class_> class_name2node;
  set<Symbol> SpecialClass;

 public:
  ClassTable(Classes);

  int errors() { return semant_errors; }

  ostream &semant_error();

  ostream &semant_error(Class_ c);

  ostream &semant_error(Symbol filename, tree_node *t);

  Symbol LCA(Symbol a, Symbol b) {
    return this->Graph.LCA(a, b);
  };

  vector<Symbol> *get_leaves() {
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

  Classes getClasses() {
    return this->classes;
  }
};


#endif

