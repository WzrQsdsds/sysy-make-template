#pragma once
#include<memory>
#include<string>
#include <cassert>
#include <cstdio>
#include <iostream>
using namespace std;

class BaseAST {
    public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
};

//CompUnit  ::= FuncDef;
class CompUnitAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override {
        //std::cout << "compUnitAST { ";
        func_def->Dump();
        //std::cout << " }";
    }
};



//FuncDef   ::= FuncType IDENT "(" ")" Block;
class FuncDefAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override {
        //std::cout << "FuncDefAST { ";
        std::cout << "fun @" << ident;
        std::cout << "(): ";
        func_type->Dump();
        std::cout << " ";
        block->Dump();
        //std::cout << " }";
    }
};



//FuncType  ::= "int";
class FuncTypeAST : public BaseAST {
    public:
    int type;
    void Dump() const override {
        //std::cout << "FuncTypeAST { ";
        if(type) {
            std::cout<<"i32";
        }
        else std::cout<<"ivoid";
        //std::cout << " }";
    }

};

//Block     ::= "{" Stmt "}";
class BlockAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> stmt;
    void Dump() const override {
        //std::cout << "BlockAST { ";
        std::cout << "{ \%entry: ";
        stmt->Dump();
        std::cout << " }";
        //std::cout<<" }";
    }
};


//Stmt      ::= "return" Number ";";
//->Stmt        ::= "return" Exp ";";
class StmtAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    void Dump() const override {
        std::cout << "ret ";
        exp->Dump();
    }
};

// Exp         ::= UnaryExp;
class ExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> unary_exp;
    void Dump() const override {
        unary_exp->Dump();
    }
};

// PrimaryExp  ::= "(" Exp ")" | Number;
class PrimaryExpAST : public BaseAST {
    public:
    int type; //type == 0 | type == 1
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> number;
    void Dump() const override {
        if(type == 0) {

        }
        else if (type == 1) {

        }
    }
};

//Number    ::= INT_CONST;
class NumberAST : public BaseAST {
    public:
    int int_const;
    void Dump() const override {
        std::cout << int_const;
    }
};

// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
class UnaryExpAST : public BaseAST {
    public:
    int type; //type == 0 | type == 1
    std::unique_ptr<BaseAST> primary_exp;
    std::unique_ptr<BaseAST> unary_op;
    std::unique_ptr<BaseAST> unary_exp;


};

// UnaryOp     ::= "+" | "-" | "!";
class UnaryOpAST : public BaseAST {
    public:
    int type; //type == 0 + | type == 1 - | type == 2 !
};