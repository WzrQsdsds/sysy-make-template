#pragma once
#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include "ast.h"
#include "koopa.h"
#include "raw.h"
#include <map>

// // 函数声明略
void Visit(const koopa_raw_program_t&);
void Visit(const koopa_raw_slice_t&);
void Visit(const koopa_raw_function_t&);
void Visit(const koopa_raw_basic_block_t&);
void Visit(const koopa_raw_value_t&);
void Visit(const koopa_raw_return_t&);
void Visit(const koopa_raw_integer_t&);
void Visit(const koopa_raw_binary_t &);
void Visit(const koopa_raw_store_t &store);
void Visit(const koopa_raw_load_t &load);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_jump_t &jump);
void Visit(const koopa_raw_call_t &call);
void Visit(const koopa_raw_global_alloc_t &global_alloc);
void Visit(const koopa_raw_func_arg_ref_t &func_arg_ref);

// int nVisit(const koopa_raw_basic_block_t&);
// int nVisit(const koopa_raw_value_t&);
// int nVisit(const koopa_raw_return_t&);
// int nVisit(const koopa_raw_integer_t&);
// int nVisit(const koopa_raw_binary_t &);

static std::map<koopa_raw_value_t, int> mp;
static int regcnt = 0;
static int func_m_cnt = 512;
static int ra_reg_cnt;

void set_reg(const koopa_raw_value_t &value, string reg) {
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    std::cout << "li   " << reg << ", " ;
    Visit(value);
    std::cout << endl;
  }
  else {
    std::cout << "li   t4, " << mp[value] * 4 << endl;
    std::cout << "add  t4, t4, sp" << endl;
    std::cout <<  "lw   " << reg << ", (t4)" << endl;
  }
  return;
}

// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  // ...
  
  // 访问所有全局变量
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
// typedef struct {
//   /// Type of function.
//   koopa_raw_type_t ty;
//   /// Name of function.
//   const char *name;
//   /// Parameters.
//   koopa_raw_slice_t params;
//   /// Basic blocks, empty if is a function declaration.
//   koopa_raw_slice_t bbs;
// } koopa_raw_function_data_t;
void Visit(const koopa_raw_function_t &func) {
  // 执行一些其他的必要操作
  // ... 
    if(func->bbs.len > 0){
      std::cout << "  .text" <<endl;
      std::cout << "  .global " << func->name + 1 << endl;
      //nVisit(func->bbs);
      func_m_cnt = ((func_m_cnt + 15) / 16) * 16;
      std::cout << func->name + 1 << ":" << endl;
      std::cout << "addi sp, sp, " << (-1) * func_m_cnt << endl;
      std::cout << "li   t4, " << regcnt * 4 << endl;
      std::cout << "add  t4, t4, sp" << endl;
      std::cout << "sw   ra, (t4)" << endl;
      ra_reg_cnt = regcnt;
      regcnt++;
      Visit(func->params);
      // 访问所有基本块
      Visit(func->bbs);
    }
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  if(string(bb->name) != string("\%entry")){
    std::cout << bb->name + 1 << ":" << endl;
  }
  
  
  // 访问所有指令
  Visit(bb->insts);
}
/*
typedef enum {
  /// Zero initializer. KOOPA_RVT_ZERO_INIT, zero init
  /// Undefined value.  KOOPA_RVT_UNDEF, undef
  /// Aggregate constant. KOOPA_RVT_AGGREGATE, rvt aggregate
  /// Basic block argument reference.  KOOPA_RVT_BLOCK_ARG_REF, koopa rvt block arg ref
  /// Pointer calculation.  KOOPA_RVT_GET_PTR, rvt get ptr
  /// Element pointer calculation.  KOOPA_RVT_GET_ELEM_PTR, koopa rrt get elem ptr
}*/
// union {
//     koopa_raw_aggregate_t aggregate;
//     koopa_raw_block_arg_ref_t block_arg_ref;
//     koopa_raw_get_ptr_t get_ptr;
//     koopa_raw_get_elem_ptr_t get_elem_ptr;
//   } data;
// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      //二元算数
      Visit(kind.data.binary);
      std::cout << "li   t4, " << regcnt * 4 << endl;
      std::cout << "add  t4, t4, sp" << endl;
      std::cout << "sw   t0, (t4)" << endl;
      mp[value] = regcnt;
      regcnt++;
      break;
    case KOOPA_RVT_STORE:
      //Memory store
      Visit(kind.data.store);
      break;
    case KOOPA_RVT_LOAD:
      //Memory load.  
      Visit(kind.data.load);
      std::cout << "li   t4, " << regcnt * 4 << endl;
      std::cout << "add  t4, t4, sp" << endl;
      std::cout << "sw   t0, (t4)" << endl;
      mp[value] = regcnt;
      regcnt++;
      break;
    case KOOPA_RVT_ALLOC:
      mp[value] = regcnt;
      regcnt++;
      break;
    case KOOPA_RVT_BRANCH:
      Visit(kind.data.branch);
      break;
    case KOOPA_RVT_JUMP:
      Visit(kind.data.jump);
      break;
    case KOOPA_RVT_CALL:
      Visit(kind.data.call);
      std::cout << "li   t4, " << regcnt * 4 << endl;
      std::cout << "add  t4, t4, sp" << endl;
      std::cout << "sw   a0, (t4)" << endl;
      mp[value] = regcnt;
      regcnt ++;
      break;
    case KOOPA_RVT_GLOBAL_ALLOC:
      std::cout << "  .data" << endl;
      std::cout << "  .globl " << value->name + 1 << endl;
      std::cout << value->name + 1 << ":" << endl;
      Visit(kind.data.global_alloc);
      break;
    case KOOPA_RVT_FUNC_ARG_REF:
      Visit(kind.data.func_arg_ref);
      mp[value] = regcnt;
      regcnt ++;
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

// 访问对应类型指令的函数定义
void Visit(const koopa_raw_return_t &ret) {
  if (ret.value != 0) {
    string reg = string("a0");
    set_reg(ret.value, reg);
    std::cout << "li   t4, " << ra_reg_cnt * 4 << endl;
    std::cout << "add  t4, t4, sp" << endl;
    std::cout << "lw   ra, (t4)" << endl;
  }
  std::cout << "addi sp, sp, ";
  std::cout << func_m_cnt << endl;
  std::cout << "ret" << endl;
}

void Visit(const koopa_raw_integer_t &integer) {
    std::cout <<integer.value;
}
// typedef struct {
//   /// Operator.
//   koopa_raw_binary_op_t op;
//   /// Left-hand side value.
//   koopa_raw_value_t lhs;
//   /// Right-hand side value.
//   koopa_raw_value_t rhs;
// } koopa_raw_binary_t; op : ne, eq, gt, lt, ge, le, add, sub, mul, div, mod, and, or, xor, shl, shr, sar
void Visit(const koopa_raw_binary_t &binary) {
  string reg0 = string("t0");
  string reg1 = string("t1");
  string reg2 = string("t2");
  set_reg(binary.lhs, reg1);
  set_reg(binary.rhs, reg2);
  if (binary.op == 0) {//ne
    std::cout << "xor  t0, t1, t2" << endl;
    std::cout << "snez t0, t0" << endl;
  }
  else if(binary.op == 1){//eq
    std::cout << "xor  t0, t1, t2" << endl;
    std::cout << "seqz t0, t0" << endl;
  }
  else if (binary.op == 2) {//gt
    std::cout << "sgt  t0, t1, t2" << endl;
  }
  else if (binary.op == 3) {//lt
    std::cout << "slt  t0, t1, t2" << endl;
  }
  else if (binary.op == 4) {//ge
    std::cout << "slt  t0, t1, t2" << endl;
    std::cout << "seqz t0, t0" << endl;
  }
  else if (binary.op == 5) {//le
    std::cout << "sgt  t0, t1, t2" << endl;
    std::cout << "seqz t0, t0" << endl;
  }
  else if (binary.op == 6) {//add
    std::cout << "add  t0, t1, t2" << endl;
  }
  else if (binary.op == 7) {//sub
    std::cout << "sub  t0, t1, t2" << endl;
  }
  else if (binary.op == 8) {//mul
    std::cout << "mul  t0, t1, t2" << endl;
  }
  else if (binary.op == 9) {//div
    std::cout << "div  t0, t1, t2" << endl;
  }
  else if (binary.op == 10) {//mod
    std::cout << "rem  t0, t1, t2" << endl;
  }
  else if (binary.op == 11) {//and
    std::cout << "and  t0, t1, t2" << endl;
  }
  else if (binary.op == 12) {//or
    std::cout << "or   t0, t1, t2" << endl;
  }
  else if (binary.op == 13) {//xor
    std::cout << "xor t0, t1, t2" << endl;
  }
  else {//shl shr sar
    std::cout << "ERROR";
  }
}

// typedef struct {
//   /// Value.
//   koopa_raw_value_t value;
//   /// Destination.
//   koopa_raw_value_t dest;
// } koopa_raw_store_t;
void Visit(const koopa_raw_store_t &store) {
  if (store.value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    std::cout << "la   t0, " << store.value->name + 1 << endl;
    std::cout << "lw   t0, 0(t0)" << endl;

  }
  else if(store.value->kind.tag == KOOPA_RVT_GET_ELEM_PTR) {

  }
  else if(store.value->kind.tag == KOOPA_RVT_GET_PTR) {

  }else {
    string reg = string("t0");
    set_reg(store.value, reg);
  }

  if(store.dest->kind.tag==KOOPA_RVT_GLOBAL_ALLOC) {
      cout<<"la   t1, " << store.dest->name + 1 << endl;
      cout<<"sw   t0, 0(t1)"<<endl;
  }
  else if(store.dest->kind.tag==KOOPA_RVT_GET_ELEM_PTR) {
      // cout<<"   li t4, "<<M[(ull)sto_dest]<<endl;
      // cout<<"   add t4, t4, sp"<<endl;
      // cout<<"   lw t1, (t4)"<<endl;
      // cout<<"   sw t0, 0(t1)"<<endl;
  }
  else if(store.dest->kind.tag==KOOPA_RVT_GET_PTR) {
      // cout<<"   li t4, "<<M[(ull)sto_dest]<<endl;
      // cout<<"   add t4, t4, sp"<<endl;
      // cout<<"   lw t1, (t4)"<<endl;
      // cout<<"   sw t0, 0(t1)"<<endl;
  }
  else {
    std::cout << "li   t4, " << mp[store.dest] * 4 << endl;
    std::cout << "add  t4, t4, sp" << endl;
    std::cout << "sw   t0, (t4)" << endl;
  }
}

// typedef struct {
//   /// Source.
//   koopa_raw_value_t src;
// } koopa_raw_load_t;
void Visit(const koopa_raw_load_t &load) {
  if (load.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    std::cout << "la   t0, " << load.src->name + 1 << endl;
    std::cout << "lw   t0, 0(t0)" << endl;

  }
  else if(load.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR) {
      // cout<<"   li t4, "<< mp[load.src]<<endl;
      // cout<<"   lw t0, (t4)"<<endl;
      // cout<<"   lw t0, 0(t0)"<<endl;
      // cout<<"   li t4, "<< regcnt <<endl;
      // cout<<"   add t4, t4, sp"<<endl;
      // cout<<"   sw t0, (t4)"<<endl;
      // mp[(ull)value]=st;
      // st+=4;
  }
  else if(load.src->kind.tag == KOOPA_RVT_GET_PTR) {
      // cout<<"   li t4, "<< mp[load.src]<<endl;
      // cout<<"   add t4, t4, sp"<<endl;
      // cout<<"   lw t0, (t4)"<<endl;
      // cout<<"   lw t0, 0(t0)"<<endl;
      // cout<<"   li t4, "<<st<<endl;
      // cout<<"   add t4, t4, sp"<<endl;
      // cout<<"   sw t0, (t4)"<<endl;
      // M[(ull)value]=st;
      // st+=4;
  }else {
    string reg = string("t0");
    set_reg(load.src, reg);
  }
}

// typedef struct {
//   /// Condition.
//   koopa_raw_value_t cond;
//   /// Target if condition is `true`.
//   koopa_raw_basic_block_t true_bb;
//   /// Target if condition is `false`.
//   koopa_raw_basic_block_t false_bb;
//   /// Arguments of `true` target..
//   koopa_raw_slice_t true_args;
//   /// Arguments of `false` target..
//   koopa_raw_slice_t false_args;
// // } koopa_raw_branch_t;
void Visit(const koopa_raw_branch_t &branch) {
  string reg = string("t0");
  set_reg(branch.cond, reg);
  std::cout << "bnez t0, " << branch.true_bb->name + 1 << endl;
  std::cout << "j " << branch.false_bb->name + 1 << endl; 
}

// typedef struct {
//   /// Target.
//   koopa_raw_basic_block_t target;
//   /// Arguments of target..
//   koopa_raw_slice_t args;
// } koopa_raw_jump_t;
void Visit(const koopa_raw_jump_t &jump) {
  std::cout << "j " << jump.target->name + 1 << endl;
}

// typedef struct {
//   /// Callee.
//   koopa_raw_function_t callee;
//   /// Arguments.
//   koopa_raw_slice_t args;
// } koopa_raw_call_t;
void Visit(const koopa_raw_call_t &call) {
  int size = call.args.len;
  if(size <= 8) {
    for(int i = 0; i < size; i ++) {
      auto value = call.args.buffer[i];
      set_reg((koopa_raw_value_t)value, string("a") + to_string(i));
    }
    std::cout <<"call " << call.callee->name + 1 << endl;
  }
  else {
    int newspace = (size - 8) * 4;
    std::cout << "addi sp, sp, -" << newspace << endl;
    for( int i = 0; i < 8; i ++) {
      auto value = call.args.buffer[i];
      set_reg((koopa_raw_value_t)value, string("a") + to_string(i));
    }
    for(int i = 8; i < size; i ++) {
      auto value = call.args.buffer[i];
      set_reg((koopa_raw_value_t)value, string("t0"));
      std::cout << "li   t4, " << (i - 8) * 4 << endl;
      std::cout << "add  t4, t4, sp" << endl;
      std::cout << "sw   t0, (t4)" << endl;
    }
    std::cout << "call " << call.callee->name + 1 << endl;
    std::cout << "addi sp, sp, " << newspace << endl;
  }
}

// typedef struct {
//   /// Initializer.
//   koopa_raw_value_t init;
// } koopa_raw_global_alloc_t;
void Visit(const koopa_raw_global_alloc_t &global_alloc) {
  auto &kind = global_alloc.init->kind;
  if(kind.tag == KOOPA_RVT_INTEGER) {
      cout<<"   .word "<< kind.data.integer.value << endl;
      cout<<endl;
  }
  else if(kind.tag == KOOPA_RVT_AGGREGATE) {
      koopa_raw_value_t val = kind.data.global_alloc.init;
      //array
  }
  else if(kind.tag == KOOPA_RVT_ZERO_INIT) {
      //koopa_raw_type_t value=data->ty->data.pointer.base;
      int siz=4;
      //while(value->tag==KOOPA_RTT_ARRAY)
      // {
      //     array_size[(ull)data].push_back(value->data.array.len);
      //     siz*=value->data.array.len;
      //     value=value->data.array.base;
      // }
      cout<<"   .zero "<<siz<<endl;
  }
}

// typedef struct {
//   /// Index.
//   size_t index;
// } koopa_raw_func_arg_ref_t;
void Visit(const koopa_raw_func_arg_ref_t &func_arg_ref) {
  if(func_arg_ref.index < 8) {
    std::cout << "li   t4, " << regcnt * 4 << endl;
    std::cout << "add  t4, t4, sp" << endl;
    std::cout << "sw   a" << func_arg_ref.index << ", (t4)" << endl;
  }
  else {
    std::cout << "li   t4, " << func_m_cnt + (func_arg_ref.index - 8) * 4 << endl;
    std::cout << "add  t4, t4, sp" << endl;
    std::cout << "lw   t0, (t4)" << endl; 
    std::cout << "li   t4, " << regcnt * 4 << endl;
    std::cout << "add  t4, t4, sp" << endl;
    std::cout << "sw   t0, (t4)" << endl;
  }
}