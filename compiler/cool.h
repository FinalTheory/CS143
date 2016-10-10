#ifndef _COOL_H_
#define _COOL_H_

#include "cool-io.h"

/* a type renaming */
typedef int Boolean;

class Entry;

typedef Entry* Symbol;

Boolean copy_Boolean(Boolean);

void assert_Boolean(Boolean);

void dump_Boolean(ostream&, int, Boolean);

Symbol copy_Symbol(Symbol);

void assert_Symbol(Symbol);

void dump_Symbol(ostream&, int, Symbol);

#include "tree.h"

typedef class Program_class* Program;
typedef class Class__class* Class_;
typedef class Feature_class* Feature;
typedef class Formal_class* Formal;
typedef class Expression_class* Expression;
typedef class Case_class* Case;
typedef list_node<Class_> Classes_class;
typedef Classes_class* Classes;
typedef list_node<Feature> Features_class;
typedef Features_class* Features;
typedef list_node<Formal> Formals_class;
typedef Formals_class* Formals;
typedef list_node<Expression> Expressions_class;
typedef Expressions_class* Expressions;
typedef list_node<Case> Cases_class;
typedef Cases_class* Cases;

#endif
