///////////////////////////////////////////////////////////////////////
//
//  Assembly Code Naming Conventions:
//
//     Dispatch table            <classname>_dispTab
//     Method entry point        <classname>.<method>
//     Class init code           <classname>_init
//     Abort method entry        <classname>.<method>.Abort
//     Prototype object          <classname>_protObj
//     Integer constant          int_const<Symbol>
//     String constant           str_const<Symbol>
//
///////////////////////////////////////////////////////////////////////

#include "stringtab.h"


#define WORD_SIZE     4
#define LOG_WORD_SIZE 2     // for logical shifts
#define SAVED_REGS    3     // in callee we save $fp, $s0, $ra

// Global names
#define CLASSNAMETAB         "class_nameTab"
#define CLASSOBJTAB          "class_objTab"
#define INTTAG               "_int_tag"
#define BOOLTAG              "_bool_tag"
#define STRINGTAG            "_string_tag"
#define HEAP_START           "heap_start"

// Naming conventions
#define DISPTAB_SUFFIX       "_dispTab"
#define METHOD_SEP           "."
#define CLASSINIT_SUFFIX     "_init"
#define PROTOBJ_SUFFIX       "_protObj"
#define OBJECTPROTOBJ        "Object" PROTOBJ_SUFFIX
#define INTCONST_PREFIX      "int_const"
#define STRCONST_PREFIX      "str_const"
#define BOOLCONST_PREFIX     "bool_const"

// Labels defined in runtime system
#define TEST_EQUAL   "equality_test"
#define CASE_ABORT   "_case_abort"
#define CASE_ABORT2  "_case_abort2"
#define DISP_ABORT   "_dispatch_abort"

#define STR_ZERO             "0"
#define EMPTY_STR            ""
#define EMPTYSLOT            0
#define LABEL                ":\n"
#define SEP                  "\t"


#define STRINGNAME (char *) "String"
#define INTNAME    (char *) "Int"
#define BOOLNAME   (char *) "Bool"
#define MAINNAME   (char *) "Main"

//
// information about object headers
//
#define DEFAULT_OBJFIELDS 3
#define TAG_OFFSET 0
#define SIZE_OFFSET 1
#define DISPTABLE_OFFSET 2

#define STRING_SLOTS      1
#define INT_SLOTS         1
#define BOOL_SLOTS        1

#define GLOBAL        "\t.globl\t"
#define ALIGN         "\t.align\t2\n"
#define WORD          "\t.word\t"

//
// register names
//
#define ZERO "$zero"    // Zero register
#define ACC  "$a0"    // Accumulator
#define A1   "$a1"    // For arguments to prim funcs
#define SELF "$s0"    // Ptr to self (callee saves)
#define S1   "$s1"    // Saved temporary 1
#define T0   "$t0"    // Temporary 0
#define T1   "$t1"    // Temporary 1
#define T2   "$t2"    // Temporary 2
#define T3   "$t3"    // Temporary 3
#define T4   "$t4"    // Temporary 4
#define SP   "$sp"    // Stack pointer
#define FP   "$fp"    // Frame pointer
#define RA   "$ra"    // Return address

//
// Opcodes
//
#define JALR  "\tjalr\t"
#define JAL   "\tjal\t"
#define RET   "\tjr\t" RA "\t"

#define SW    "\tsw\t"
#define LW    "\tlw\t"
#define LI    "\tli\t"
#define LA    "\tla\t"

#define MOVE  "\tmove\t"
#define NEG   "\tneg\t"
#define ADD   "\tadd\t"
#define ADDI  "\taddi\t"
#define ADDU  "\taddu\t"
#define ADDIU "\taddiu\t"
#define DIV   "\tdiv\t"
#define MUL   "\tmul\t"
#define SUB   "\tsub\t"
#define SLL   "\tsll\t"
#define BEQZ  "\tbeqz\t"
#define BRANCH   "\tb\t"
#define BEQ      "\tbeq\t"
#define BNE      "\tbne\t"
#define BLEQ     "\tble\t"
#define BLT      "\tblt\t"
#define BGT      "\tbgt\t"

extern int cgen_debug;

extern char* gc_init_names[];

extern char* gc_collect_names[];

//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////


class BoolConst;

void emit_string_constant(ostream& str, const char* s);

void emit_load(char* dest_reg, int offset, char* source_reg, ostream& s);

void emit_store(char* source_reg, int offset, char* dest_reg, ostream& s);

void emit_load_imm(char* dest_reg, int val, ostream& s);

void emit_load_address(char* dest_reg, char* address, ostream& s);

void emit_partial_load_address(char* dest_reg, ostream& s);

void emit_load_bool(char* dest, const BoolConst& b, ostream& s);

void emit_load_string(char* dest, StringEntry* str, ostream& s);

void emit_load_int(char* dest, IntEntry* i, ostream& s);

void emit_move(char* dest_reg, char* source_reg, ostream& s);

void emit_neg(char* dest, char* src1, ostream& s);

void emit_add(char* dest, char* src1, char* src2, ostream& s);

void emit_addu(char* dest, char* src1, char* src2, ostream& s);

void emit_addiu(char* dest, char* src1, int imm, ostream& s);

void emit_div(char* dest, char* src1, char* src2, ostream& s);

void emit_mul(char* dest, char* src1, char* src2, ostream& s);

void emit_sub(char* dest, char* src1, char* src2, ostream& s);

void emit_sll(char* dest, char* src1, int num, ostream& s);

void emit_jalr(char* dest, ostream& s);

void emit_jal(char* address, ostream& s);

void emit_return(ostream& s);

void emit_gc_assign(ostream& s);

void emit_disptable_ref(Symbol sym, ostream& s);

void emit_init_ref(Symbol sym, ostream& s);

void emit_label_ref(int l, ostream& s);

void emit_protobj_ref(Symbol sym, ostream& s);

void emit_method_ref(Symbol classname, Symbol methodname, ostream& s);

void emit_label_def(int l, ostream& s);

void emit_beqz(char* source, int label, ostream& s);

void emit_beq(char* src1, char* src2, int label, ostream& s);

void emit_bne(char* src1, char* src2, int label, ostream& s);

void emit_bleq(char* src1, char* src2, int label, ostream& s);

void emit_blt(char* src1, char* src2, int label, ostream& s);

void emit_blti(char* src1, int imm, int label, ostream& s);

void emit_bgti(char* src1, int imm, int label, ostream& s);

void emit_branch(int l, ostream& s);

//
// Push a register on the stack. The stack grows towards smaller addresses.
//
void emit_push(char* reg, ostream& str);

void emit_pop(char* reg, ostream& str);

//
// Fetch the integer value in an Int object.
// Emits code to fetch the integer value of the Integer object pointed
// to by register source into the register dest
//
void emit_fetch_int(char* dest, char* source, ostream& s);

//
// Emits code to store the integer value contained in register source
// into the Integer object pointed to by dest.
//
void emit_store_int(char* source, char* dest, ostream& s);

void emit_test_collector(ostream& s);

void emit_gc_check(char* source, ostream& s);
