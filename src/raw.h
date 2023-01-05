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
int Visit(const koopa_raw_integer_t&);
int Visit(const koopa_raw_binary_t &);

const string reg[] = {"x0","t0","t1","t2","t3","t4","t5","t6","a0","a1","a2","a3","a4","a5","a6","a7"};
static int use = 0;
map<koopa_raw_value_t, int> mp;


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
    std::cout << "  .globl main"<<endl;
  // 访问所有基本块
  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  std::cout << "main:"<<endl;
  // 访问所有指令
  Visit(bb->insts);
}
/*
typedef enum {
  /// Integer constant. KOOPA_RVT_INTEGER, ***
  /// Zero initializer. KOOPA_RVT_ZERO_INIT,
  /// Undefined value.  KOOPA_RVT_UNDEF,
  /// Aggregate constant. KOOPA_RVT_AGGREGATE,
  /// Function argument reference.  KOOPA_RVT_FUNC_ARG_REF,
  /// Basic block argument reference.  KOOPA_RVT_BLOCK_ARG_REF,
  /// Local memory allocation.  KOOPA_RVT_ALLOC,
  /// Global memory allocation.  KOOPA_RVT_GLOBAL_ALLOC,
  /// Memory load.  KOOPA_RVT_LOAD,
  /// Memory store.  KOOPA_RVT_STORE,
  /// Pointer calculation.  KOOPA_RVT_GET_PTR,
  /// Element pointer calculation.  KOOPA_RVT_GET_ELEM_PTR,
  /// Binary operation.  KOOPA_RVT_BINARY,
  /// Conditional branch.  KOOPA_RVT_BRANCH,
  /// Unconditional jump.  KOOPA_RVT_JUMP,
  /// Function call.  KOOPA_RVT_CALL,
  /// Function return.  KOOPA_RVT_RETURN, ***
}*/
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
      Visit(kind.data.binary);
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

// 访问对应类型指令的函数定义
void Visit(const koopa_raw_return_t &ret) {
    Visit(ret.value);
    std::cout << "ret";
}

int Visit(const koopa_raw_integer_t &integer) {
    std::cout << "li " << reg[use];
    std::cout <<integer.value <<endl;
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
// } koopa_raw_binary_t;
int Visit(const koopa_raw_binary_t &binary) {
  std::cout << binary.op<< " ";//eq 1 sub 7
  if(binary.op == 1){
    std::cout << "eq ";
  }
  else if(binary.op == 7){
    std::cout << "sub ";
  }
  Visit(binary.lhs);
  std::cout<<"!!!";
  Visit(binary.rhs);
}