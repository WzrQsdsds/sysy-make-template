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

// int nVisit(const koopa_raw_basic_block_t&);
// int nVisit(const koopa_raw_value_t&);
// int nVisit(const koopa_raw_return_t&);
// int nVisit(const koopa_raw_integer_t&);
// int nVisit(const koopa_raw_binary_t &);

static std::map<koopa_raw_value_t, int> mp;
static int regcnt = 0;
static koopa_raw_value_t regvalue[3] = {0,0,0};//t0,t1,a0
static int func_m_cnt = 512;

void set_reg(const koopa_raw_value_t &value, string reg) {
  if(reg == string("t0") && regvalue[0] == value) {
    return;
  }
  if(reg == string("t1") && regvalue[1] == value) {
    return;
  }
  if(reg == string("a0") && regvalue[2] == value) {
    return;
  }
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    std::cout << "li   " << reg << ", ";
    Visit(value);
    std::cout << endl;
  }
  else {
    std::cout <<  "lw   " << reg << ", " << mp[value] * 4 << "(sp)" << endl;
  }
  if(reg == string("t0")) {
    regvalue[0] = value;
  }
  if(reg == string("t1")) {
    regvalue[1] = value;
  }
  if(reg == string("a0")) {
    regvalue[2] = value;
  }
  return;
}

// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  // ...
  std::cout << "  .text" <<endl;
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
void Visit(const koopa_raw_function_t &func) {
  // 执行一些其他的必要操作
  // ... 
  std::cout << "  .globl main" << endl;
  std::cout << "main:" << endl;
  std::cout << "addi sp, sp, ";
  //nVisit(func->bbs);
  func_m_cnt = ((func_m_cnt + 15) / 16) * 16;
  std::cout << func_m_cnt << endl;
  // 访问所有基本块
  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  
  // 访问所有指令
  Visit(bb->insts);
}
/*
typedef enum {
  /// Zero initializer. KOOPA_RVT_ZERO_INIT,
  /// Undefined value.  KOOPA_RVT_UNDEF,
  /// Aggregate constant. KOOPA_RVT_AGGREGATE,
  /// Function argument reference.  KOOPA_RVT_FUNC_ARG_REF,
  /// Basic block argument reference.  KOOPA_RVT_BLOCK_ARG_REF,
  /// Global memory allocation.  KOOPA_RVT_GLOBAL_ALLOC,
  /// Pointer calculation.  KOOPA_RVT_GET_PTR,
  /// Element pointer calculation.  KOOPA_RVT_GET_ELEM_PTR,
  /// Conditional branch.  KOOPA_RVT_BRANCH,
  /// Unconditional jump.  KOOPA_RVT_JUMP,
  /// Function call.  KOOPA_RVT_CALL,
}*/
// union {
//     koopa_raw_aggregate_t aggregate;
//     koopa_raw_func_arg_ref_t func_arg_ref;
//     koopa_raw_block_arg_ref_t block_arg_ref;
//     koopa_raw_global_alloc_t global_alloc;
//     koopa_raw_get_ptr_t get_ptr;
//     koopa_raw_get_elem_ptr_t get_elem_ptr;
//     koopa_raw_branch_t branch;
//     koopa_raw_jump_t jump;
//     koopa_raw_call_t call;
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
      std::cout << "sw   t0, " << regcnt * 4 << "(sp)" << endl;
      mp[value] = regcnt;
      regcnt++;
      break;
    case KOOPA_RVT_STORE:
      //store
      Visit(kind.data.store);
      break;
    case KOOPA_RVT_LOAD:
      //Memory load.  
      Visit(kind.data.load);
      std::cout << "sw   t0, " << regcnt * 4 << "(sp)" << endl;
      mp[value] = regcnt;
      regcnt++;
      break;
    case KOOPA_RVT_ALLOC:
      mp[value] = regcnt;
      regcnt++;
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

// 访问对应类型指令的函数定义
void Visit(const koopa_raw_return_t &ret) {
  string reg = string("a0");
  set_reg(ret.value, reg);
  std::cout << "addi sp, sp, ";
  std::cout << func_m_cnt * (-1) << endl;
  std::cout << "ret" << endl;
}

void Visit(const koopa_raw_integer_t &integer) {
    std::cout <<integer.value;
}
// 视需求自行实现
//
// typedef struct {
//   /// Operator.
//   koopa_raw_binary_op_t op;
//   /// Left-hand side value.
//   koopa_raw_value_t lhs;
//   /// Right-hand side value.
//   koopa_raw_value_t rhs;
// } koopa_raw_binary_t; op : ne, eq, gt, lt, ge, le, add, sub, mul, div, mod, and, or, xor, shl, shr, sar
void Visit(const koopa_raw_binary_t &binary) {
  string reg1 = string("t0");
  string reg2 = string("t1");
  set_reg(binary.lhs, reg1);
  set_reg(binary.rhs, reg2);
  if (binary.op == 0) {//ne
    std::cout << "xor  t0, t0, t1" << endl;
    std::cout << "snez t0, t0" << endl;
  }
  else if(binary.op == 1){//eq
    std::cout << "xor  t0, t0, t1" << endl;
    std::cout << "seqz t0, t0" << endl;
  }
  else if (binary.op == 2) {//gt
    std::cout << "sgt  t0, t0, t1" << endl;
  }
  else if (binary.op == 3) {//lt
    std::cout << "slt  t0, t0, t1" << endl;
  }
  else if (binary.op == 4) {//ge
    std::cout << "slt  t0, t0, t1" << endl;
    std::cout << "seqz t0, t0" << endl;
  }
  else if (binary.op == 5) {//le
    std::cout << "sgt  t0, t0, t1" << endl;
    std::cout << "seqz t0, t0" << endl;
  }
  else if (binary.op == 6) {//add
    std::cout << "add  t0, t0, t1" << endl;
  }
  else if (binary.op == 7) {//sub
    std::cout << "sub  t0, t0, t1" << endl;
  }
  else if (binary.op == 8) {//mul
    std::cout << "mul  t0, t0, t1" << endl;
  }
  else if (binary.op == 9) {//div
    std::cout << "div  t0, t0, t1" << endl;
  }
  else if (binary.op == 10) {//mod
    std::cout << "rem  t0, t0, t1" << endl;
  }
  else if (binary.op == 11) {//and
    std::cout << "and  t0, t0, t1" << endl;
  }
  else if (binary.op == 12) {//or
    std::cout << "or   t0, t0, t1" << endl;
  }
  else if (binary.op == 13) {//xor
    std::cout << "xor t0, t0, t1" << endl;
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
  string reg = string("t0");
  set_reg(store.value, reg);
  std::cout << "sw   t0, " << mp[store.dest] * 4 << "(sp)" << endl;
}

// typedef struct {
//   /// Source.
//   koopa_raw_value_t src;
// } koopa_raw_load_t;
void Visit(const koopa_raw_load_t &load) {
  string reg = string("t0");
  set_reg(load.src, reg);
}