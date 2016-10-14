#include "cool-io.h"

#define TRUE 1
#define FALSE 0

//
// Garbage collection options
//

extern enum Memmgr { GC_NOGC, GC_GENGC, GC_SNCGC } cgen_Memmgr;

extern enum Memmgr_Test { GC_NORMAL, GC_TEST } cgen_Memmgr_Test;

extern enum Memmgr_Debug { GC_QUICK, GC_DEBUG } cgen_Memmgr_Debug;


class BoolConst {
 private:
  int val;
 public:
  BoolConst(int);

  void code_def(ostream&, int boolclasstag);

  void code_ref(ostream&) const;
};

extern BoolConst falsebool;
extern BoolConst truebool;
