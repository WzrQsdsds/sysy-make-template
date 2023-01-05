#pragma once
#include<memory>
#include<string>
#include <cassert>
#include <cstdio>
#include <iostream>
using namespace std;

static int now = -1;

class BaseAST {
    public:
    int type;
    virtual ~BaseAST() = default;
    virtual string Dump() const = 0;
};

//CompUnit  ::= FuncDef;
class CompUnitAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_def;

    string Dump() const override {
        func_def->Dump();
        return string("compunit");
    }
};



//FuncDef   ::= FuncType IDENT "(" ")" Block;
class FuncDefAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    string Dump() const override {
        std::cout << "fun @" << ident;
        std::cout << "(): ";
        func_type->Dump();
        std::cout << " ";
        block->Dump();
        return string("funcdef");
    }
};



//FuncType  ::= "int";
class FuncTypeAST : public BaseAST {
    public:
    string Dump() const override {
        std::cout<<"i32";
        return string("i32");
        
    }

};

//Block     ::= "{" Stmt "}";
class BlockAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> stmt;
    string Dump() const override {
        std::cout << "{ \%entry: ";
        string ret = stmt->Dump();
        std::cout << " }";
        return ret;
    }
};

//->Stmt        ::= "return" Exp ";";
class StmtAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    string Dump() const override {
        string ret = exp->Dump();
        std::cout << "ret " << ret;
        return ret;
    }
};

// Exp         ::= UnaryExp;->LOrExp
class ExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> l_or_exp;
    string Dump() const override {
        string ret = l_or_exp->Dump();
        return ret;
    }
};

// PrimaryExp  ::= "(" Exp ")" | Number;
//-> PrimaryExp    ::= "(" Exp ")" | LVal | Number;
class PrimaryExpAST : public BaseAST {
    public:
    //type == 0 | type == 1
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> number;
    string Dump() const override {
        if(type == 0){
            string ret = exp->Dump();
            return ret;
        }            
        else if(type == 1){
            return number->Dump();
        }
        return string("");
    }
};

//Number    ::= INT_CONST;
class NumberAST : public BaseAST {
    public:
    int int_const;
    string Dump() const override {
        return to_string(int_const);
    }
};

// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
class UnaryExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> primary_exp;
    std::unique_ptr<BaseAST> unary_op;
    std::unique_ptr<BaseAST> unary_exp;
    string Dump() const override {
        if(type == 0){
            string ret = primary_exp->Dump();
            return ret;
        }
        else if(type == 1){
            string ret = unary_exp->Dump();
            if(unary_op->type == 0){
                return ret;
            }
            else if(unary_op->type == 1){
                now++;
                std::cout << "%" << now << " = sub 0, " << ret << endl;
                return string("%")+to_string(now);
            }
            else if(unary_op->type == 2){
                now++;
                std::cout << "%" << now << " = eq 0, " << ret << endl;
                return string("%")+to_string(now);
            }
        }
        return "";
    }
};

//UnaryOp     ::= "+" | "-" | "!";
class UnaryOpAST : public BaseAST {
    public:
    string Dump() const override {
        return string("");
    }
};

//MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
class MulExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> unary_exp;
    std::unique_ptr<BaseAST> mul_exp;
    std::unique_ptr<BaseAST> mul_op;
    string Dump() const override {
        if(type == 0){
            string ret = unary_exp->Dump();
            return ret;
        }
        else if(type == 1){
            string ret1 = mul_exp->Dump();
            string ret2 = unary_exp->Dump();
            now++;
            if(mul_op->type == 0){
                std::cout << "%" << now << " = mul " << ret1 << ", " << ret2 << endl;
            }
            else if(mul_op->type == 1){
                std::cout << "%" << now << " = div " << ret1 << ", " << ret2 << endl;
            }
            else if(mul_op->type == 2){
                std::cout << "%" << now << " = mod " << ret1 << ", " << ret2 << endl;
            }
            return string("%")+to_string(now);
        }
        return string("ERROR");
    }
};

class MulOpAST : public BaseAST {
    public:
    string Dump() const override {
        return string("");
    }
};

//AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
class AddExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> mul_exp;
    std::unique_ptr<BaseAST> add_op;
    std::unique_ptr<BaseAST> add_exp;
    string Dump() const override {
        if(type == 0){
            string ret = mul_exp->Dump();
            return ret;
        }
        else if(type == 1){
            string ret1 = add_exp->Dump();
            string ret2 = mul_exp->Dump();
            now++;
            if(add_op->type == 0){
                std::cout << "%" << now << " = add " << ret1 << ", " << ret2 <<endl;
            }
            else if(add_op->type == 1){
                std::cout << "%" << now << " = sub " << ret1 << ", " << ret2 <<endl;
            }
            return string("%")+to_string(now);
        }
        return string("ERROR");
    }
};

class AddOpAST : public BaseAST {
    public:
    string Dump() const override {
        return string("");
    }
};

//RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
class RelExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> add_exp;
    std::unique_ptr<BaseAST> rel_exp;
    std::unique_ptr<BaseAST> rel_op;
    string Dump() const override {
        if(type == 0) {
            string ret = add_exp->Dump();
            return ret;
        }
        else if (type == 1) {
            string ret1 = rel_exp->Dump();
            string ret2 = add_exp->Dump();
            now++;
            if(rel_op->type == 0) {
                std::cout << "%" << now <<" = lt " << ret1 <<", " << ret2 << endl;
            }
            else if (rel_op->type == 1) {
                std::cout << "%" << now <<" = gt " << ret1 <<", " << ret2 << endl;
            }
            else if (rel_op->type == 2) {
                std::cout << "%" << now <<" = le " << ret1 <<", " << ret2 << endl;
            }
            else if (rel_op->type == 3) {
                std::cout << "%" << now <<" = ge " << ret1 <<", " << ret2 << endl;
            }
            return string("%")+to_string(now);
        }
        return string("ERROR");
    }
};

class RelOpAST : public BaseAST {
    public:
    string Dump() const override {
        return string("");
    }
};

//EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
class EqExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> rel_exp;
    std::unique_ptr<BaseAST> eq_exp;
    std::unique_ptr<BaseAST> eq_op;
    string Dump() const override {
        if(type == 0) {
            string ret = rel_exp->Dump();
            return ret;
        }
        else if (type == 1) {
            string ret1 = eq_exp->Dump();
            string ret2 = rel_exp->Dump();
            now++;
            if (eq_op->type == 0) {
                std::cout << "%" << now <<" = eq " << ret1 <<", " << ret2 << endl;
            }
            else if (eq_op->type == 1) {
                std::cout << "%" << now <<" = ne " << ret1 <<", " << ret2 << endl;
            }
            return string("%")+to_string(now);
        }
        return string("ERROR");
    }
};

class EqOpAST : public BaseAST {
    public:
    string Dump() const override {
        return string("");
    }
};

//LAndExp     ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> eq_exp;
    std::unique_ptr<BaseAST> l_and_exp;
    string Dump() const override {
        if (type == 0) {
            string ret = eq_exp->Dump();
            return ret;
        }
        else if (type == 1) {
            string ret1 = l_and_exp->Dump();
            string ret2 = eq_exp->Dump();
            now++;
            std::cout << "%" << now <<" = ne " << ret1 <<", 0" << endl;
            now++;
            std::cout << "%" << now <<" = ne " << ret2 <<", 0" << endl;
            now++;
            std::cout << "%" << now <<" = and %" << (now - 2) <<", %" << (now - 1) << endl;
            return string("%")+to_string(now);
        }
        return string("ERROR");
    }
};

//LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> l_and_exp;
    std::unique_ptr<BaseAST> l_or_exp;
    string Dump() const override {
        if (type == 0) {
            string ret = l_and_exp->Dump();
            return ret;
        }
        else if (type == 1) {
            string ret1 = l_or_exp->Dump();
            string ret2 = l_and_exp->Dump();
            now++;
            std::cout << "%" << now <<" = ne " << ret1 <<", 0" << endl;
            now++;
            std::cout << "%" << now <<" = ne " << ret2 <<", 0" << endl;
            now++;
            std::cout << "%" << now <<" = or %" << (now - 2) <<", %" << (now - 1) << endl;
            return string("%")+to_string(now);
        }
        return string("ERROR");
    }
};


//lv4:----------------------------------------------------------

// Decl          ::= ConstDecl;
class DeclAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> const_decl;
    string Dump() const override {

    }
};

// ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
class ConstDeclAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> b_type;
    std::unique_ptr<BaseAST> const_def;
}

// BType         ::= "int";


// ConstDef      ::= IDENT "=" ConstInitVal;


// ConstInitVal  ::= ConstExp;

// Block         ::= "{" {BlockItem} "}";


// BlockItem     ::= Decl | Stmt;


// LVal          ::= IDENT;


// ConstExp      ::= Exp;