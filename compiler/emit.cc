#include "emit.h"
#include "cgen.h"


char* gc_init_names[] =
    {"_NoGC_Init", "_GenGC_Init", "_ScnGC_Init"};
char* gc_collect_names[] =
    {"_NoGC_Collect", "_GenGC_Collect", "_ScnGC_Collect"};


void emit_load(char* dest_reg, int offset, char* source_reg, ostream& s) {
  s << LW << dest_reg << " " << offset * WORD_SIZE << "(" << source_reg << ")"
    << endl;
}

void emit_store(char* source_reg, int offset, char* dest_reg, ostream& s) {
  s << SW << source_reg << " " << offset * WORD_SIZE << "(" << dest_reg << ")"
    << endl;
}

void emit_load_imm(char* dest_reg, int val, ostream& s) { s << LI << dest_reg << " " << val << endl; }

void emit_load_address(char* dest_reg, char* address, ostream& s) {
  s << LA << dest_reg << " " << address << endl;
}

void emit_partial_load_address(char* dest_reg, ostream& s) { s << LA << dest_reg << " "; }

void emit_load_bool(char* dest, const BoolConst& b, ostream& s) {
  emit_partial_load_address(dest, s);
  b.code_ref(s);
  s << endl;
}

void emit_load_string(char* dest, StringEntry* str, ostream& s) {
  emit_partial_load_address(dest, s);
  str->code_ref(s);
  s << endl;
}

void emit_load_int(char* dest, IntEntry* i, ostream& s) {
  emit_partial_load_address(dest, s);
  i->code_ref(s);
  s << endl;
}

void emit_move(char* dest_reg, char* source_reg, ostream& s) {
  s << MOVE << dest_reg << " " << source_reg << endl;
}

void emit_neg(char* dest, char* src1, ostream& s) { s << NEG << dest << " " << src1 << endl; }

void emit_add(char* dest, char* src1, char* src2, ostream& s) {
  s << ADD << dest << " " << src1 << " " << src2 << endl;
}

void emit_addu(char* dest, char* src1, char* src2, ostream& s) {
  s << ADDU << dest << " " << src1 << " " << src2 << endl;
}

void emit_addiu(char* dest, char* src1, int imm, ostream& s) {
  s << ADDIU << dest << " " << src1 << " " << imm << endl;
}

void emit_div(char* dest, char* src1, char* src2, ostream& s) {
  s << DIV << dest << " " << src1 << " " << src2 << endl;
}

void emit_mul(char* dest, char* src1, char* src2, ostream& s) {
  s << MUL << dest << " " << src1 << " " << src2 << endl;
}

void emit_sub(char* dest, char* src1, char* src2, ostream& s) {
  s << SUB << dest << " " << src1 << " " << src2 << endl;
}

void emit_sll(char* dest, char* src1, int num, ostream& s) {
  s << SLL << dest << " " << src1 << " " << num << endl;
}

void emit_jalr(char* dest, ostream& s) { s << JALR << "\t" << dest << endl; }

void emit_jal(char* address, ostream& s) { s << JAL << address << endl; }

void emit_return(ostream& s) { s << RET << endl; }

void emit_gc_assign(ostream& s) { s << JAL << "_GenGC_Assign" << endl; }

void emit_disptable_ref(Symbol sym, ostream& s) { s << sym << DISPTAB_SUFFIX; }

void emit_init_ref(Symbol sym, ostream& s) { s << sym << CLASSINIT_SUFFIX; }

void emit_label_ref(int l, ostream& s) { s << "label" << l; }

void emit_protobj_ref(Symbol sym, ostream& s) { s << sym << PROTOBJ_SUFFIX; }

void emit_method_ref(Symbol classname, Symbol methodname, ostream& s) {
  s << classname << METHOD_SEP << methodname;
}

void emit_label_def(int l, ostream& s) {
  emit_label_ref(l, s);
  s << ":" << endl;
}

void emit_beqz(char* source, int label, ostream& s) {
  s << BEQZ << source << " ";
  emit_label_ref(label, s);
  s << endl;
}

void emit_beq(char* src1, char* src2, int label, ostream& s) {
  s << BEQ << src1 << " " << src2 << " ";
  emit_label_ref(label, s);
  s << endl;
}

void emit_bne(char* src1, char* src2, int label, ostream& s) {
  s << BNE << src1 << " " << src2 << " ";
  emit_label_ref(label, s);
  s << endl;
}

void emit_bleq(char* src1, char* src2, int label, ostream& s) {
  s << BLEQ << src1 << " " << src2 << " ";
  emit_label_ref(label, s);
  s << endl;
}

void emit_blt(char* src1, char* src2, int label, ostream& s) {
  s << BLT << src1 << " " << src2 << " ";
  emit_label_ref(label, s);
  s << endl;
}

void emit_blti(char* src1, int imm, int label, ostream& s) {
  s << BLT << src1 << " " << imm << " ";
  emit_label_ref(label, s);
  s << endl;
}

void emit_bgti(char* src1, int imm, int label, ostream& s) {
  s << BGT << src1 << " " << imm << " ";
  emit_label_ref(label, s);
  s << endl;
}

void emit_branch(int l, ostream& s) {
  s << BRANCH;
  emit_label_ref(l, s);
  s << endl;
}

void emit_push(char* reg, ostream& str) {
  emit_store(reg, 0, SP, str);
  emit_addiu(SP, SP, -WORD_SIZE, str);
}

void emit_pop(char* reg, ostream& str) {
  emit_load(reg, 1, SP, str);
  emit_addiu(SP, SP, WORD_SIZE, str);
}

void emit_fetch_int(char* dest, char* source, ostream& s) { emit_load(dest, DEFAULT_OBJFIELDS, source, s); }

void emit_store_int(char* source, char* dest, ostream& s) { emit_store(source, DEFAULT_OBJFIELDS, dest, s); }

void emit_test_collector(ostream& s) {
  emit_push(ACC, s);
  emit_move(ACC, SP, s); // stack end
  emit_move(A1, ZERO, s); // allocate nothing
  s << JAL << gc_collect_names[cgen_Memmgr] << endl;
  emit_addiu(SP, SP, 4, s);
  emit_load(ACC, 0, SP, s);
}

void emit_gc_check(char* source, ostream& s) {
  if (source != (char*)A1) { emit_move(A1, source, s); }
  s << JAL << "_gc_check" << endl;
}
