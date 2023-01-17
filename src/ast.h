#pragma once
#include<memory>
#include<string>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <stack>
using namespace std;

static int now = -1;
static int l_cal_cnt = 0;

typedef struct {
    int type;
    int value;
    std::string str;
}Symbol;

typedef std::map<std::string, Symbol> Symbolmap;

static Symbolmap globalsymbol;
static std::map <std::string, std::vector<Symbolmap>> funcsymbolmap;

static std::map <std::string, int> func_type_map;

static string cur_func_name = string("");

static Symbol *find_ident(std::string ident) {
    std::vector<Symbolmap> &smap = funcsymbolmap[cur_func_name]; 
    for(int i = smap.size()-1; i >= 0; i--) {
        auto iter = (smap[i]).find(ident);
        if(iter != smap[i].end()) {
            return &(iter->second);
        }
    }
    auto iter = globalsymbol.find(ident);
    if (iter != globalsymbol.end()){
        return &(iter->second);
    }
    std::cout << "error";
    return 0;
}
static void insert_ident(std::string ident, Symbol &s) {
    if (cur_func_name == string("")) {
        globalsymbol[ident] = s;
        return;
    }
    std::vector<Symbolmap> &smap = funcsymbolmap[cur_func_name]; 
    (smap.back())[ident] = s;
    return ;
}

static int depth = 0;

static int backend = 0;
static int if_cnt = 0;
static stack<int> ifstack;

class BaseAST {
    public:
    int type;
    virtual ~BaseAST() = default;
    virtual string Dump() const = 0;
    virtual int Calc() const = 0;
};

//CompUnit -> Unit
class CompUnitAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> unit;
    string Dump() const override {        
        std::cout << "decl @getint(): i32" << endl;
        func_type_map[string("getint")] = 1;
        std::cout << "decl @getch(): i32" << endl;
        func_type_map[string("getch")] = 1;
        std::cout << "decl @getarray(*i32): i32" << endl;
        func_type_map[string("getarray")] = 1;
        std::cout << "decl @putint(i32)" << endl;
        func_type_map[string("putint")] = 0;
        std::cout << "decl @putch(i32)" << endl;
        func_type_map[string("putch")] = 0;
        std::cout << "decl @putarray(i32, *i32)" << endl;
        func_type_map[string("putarray")] = 0;
        std::cout << "decl @starttime()" << endl;
        func_type_map[string("starttime")] = 0;
        std::cout << "decl @stoptime()" << endl;
        func_type_map[string("stoptime")] = 0;
        unit->Dump();
        return string("");
    }
    int Calc() const override {
        return 0;
    }
}
;

static int functype;
//Unit    ::= [Unit] (Decl | FuncDef)
// Decl | FuncDef | Decl Unit | FuncDef Unit
//ConstDecl | FuncType FunOrVarDef | ConstDecl Unit | FuncType FunOrVarDef Unit
class UnitAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> unit;
    std::unique_ptr<BaseAST> fun_or_var_decl;
    std::unique_ptr<BaseAST> const_decl;
    string Dump() const override {
        if (type == 0) {
            const_decl->Dump();
            return string("");
        }
        else if (type == 1) {
            fun_or_var_decl->Dump();
            return string("");
        }
        else if (type == 2) {
            const_decl->Dump();
            unit->Dump();
            return string("");
        }
        else if (type == 3) {
            fun_or_var_decl->Dump();
            unit->Dump();
            return string("");
        }
        return string("");
    }
    int Calc() const override {
        return 0;
    }
}
;
//FunOrVarDecl -> FuncType FunOrVarDef
class FunOrVarDeclAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_type;
    std::unique_ptr<BaseAST> fun_or_var_def;
    string Dump() const override {
        functype = func_type->type;
        fun_or_var_def->Dump();
        return string("");
    }
    int Calc() const override {
        return 0;
    }
}
;

//FunOrVar -> FuncDef | VarDef ";"
class FunOrVarDefAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_def;
    std::unique_ptr<BaseAST> var_def;
    string Dump() const override {
        if(type == 0) {
            func_def->Dump();
        }
        else var_def->Dump();
        return string("");
    }
    int Calc() const override {
        return 0;
    }
}
;

//FuncDef   ::= IDENT "(" FuncFParams ")" Block; | IDENT "(" ")" Block; 
class FuncDefAST : public BaseAST {
    public:
    std::string ident;
    std::unique_ptr<BaseAST> func_f_params;
    std::unique_ptr<BaseAST> block;

    string Dump() const override {
        backend = 0;
        cur_func_name = ident;
        std::cout << "fun @" << ident;
        std::cout << "(";
        if (type == 0) {
            func_f_params->Dump();
        }
        std::cout << ")";
        if(functype == 1){
            std::cout << ": i32";
        }
        func_type_map[ident] = functype;
        std::cout << " {\n\%entry: \n";
        if (type == 0) {
            func_f_params->Calc();
        }
        block->Dump();
        if(functype == 0 && backend == 0) {
            std::cout << "ret" << endl;
        }
        std::cout << "}" << endl;
        cur_func_name = string("");
        return string("");
    }
    int Calc() const override {
        return 0;
    }
};

//FuncType    ::= "void" | "int";
class FuncTypeAST : public BaseAST {
    public:
    string Dump() const override {
        if(type == 0)
            return string("");
        else if (type == 1) {
            return string(": i32");
        }
        return string("");
    }
    int Calc() const override {
        return 0;
    }
};

//-> Block ::= "{" BlockItem "}" | "{" "}"
class BlockAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> block_item;
    string Dump() const override {
        if(backend == 1) {
            return string("");
        }
        if(type == 2){
            return string("");
        }
        Symbolmap *fsl = new Symbolmap;
        std::vector<Symbolmap> &smap = funcsymbolmap[cur_func_name]; 
        smap.push_back(*fsl);
        depth++;
        string ret = block_item->Dump();
        smap.pop_back();
        return ret;
    }
    int Calc() const override {
        return 0;
    }
};

// Stmt ::= LVal "=" Exp ";"
//        | Exp ";" | ";"
//        | Block
//        | "return" Exp ";"; | return ";"
//  | "if" "(" Exp ")" Stmt "else" Stmt | "if" "(" Exp ")" Stmt
// | "while" "(" Exp ")" Stmt
class StmtAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> l_val;
    std::unique_ptr<BaseAST> block;
    std::unique_ptr<BaseAST> stmt1;
    std::unique_ptr<BaseAST> stmt2;
    string Dump() const override {
        if(backend == 1) {
            return string("");
        }
        if(type == 0){
            string ident = l_val->Dump();
            //Symbol s = symbt.globalsymbol[ident];
            Symbol *s = find_ident(ident);
            s->value = exp->Calc();
            //symbt.globalsymbol[ident] = s;
            string ret = exp->Dump();
            std::cout << "store "<< ret << ", @" << s->str << endl;
            return ident; 
        }
        else if (type == 1) {
            string ret = exp->Dump();
            return ret;
        }
        else if(type == 2) {
            return string("");
        }
        else if (type == 3) {
            string ret = block->Dump();
            return ret;
        }
        else if(type == 4){
            string ret = exp->Dump();
            std::cout << "ret " << ret << endl;
            backend = 1;
            return ret;
        }
        else if(type == 5) {
            std::cout << "ret 0" << endl;
            backend = 1;
            return string("");
        }
        else if(type == 6) {
            string ret = exp->Dump();
            string ifthen = "\%then_" + to_string(if_cnt);
            string ifelse = "\%else_" + to_string(if_cnt);
            string ifend = "\%end_" + to_string(if_cnt);
            if_cnt ++;
            std::cout << "br " << ret << ", " << ifthen << " , " << ifelse << endl;
            std::cout << ifthen << ":" << endl;
            stmt1->Dump();
            if(backend == 0){
                std::cout << "jump "<< ifend << endl;
            }
            else {
                backend = 0;
            }
            std::cout << ifelse << ":" << endl;
            stmt2->Dump();
            if(backend == 0){
                std::cout << "jump " << ifend << endl;
            }
            else {
                backend = 0;
            }
            std::cout << ifend << ":" << endl;\
            return string("");
        }
        else if(type == 7) {
            string ret = exp->Dump();
            string ifthen = "\%then_" + to_string(if_cnt);
            string ifend = "\%end_" + to_string(if_cnt);
            if_cnt ++;
            std::cout << "br " << ret << ", " << ifthen << ", " << ifend << endl;
            std::cout << ifthen << ":" << endl;
            stmt1->Dump();
            if(backend == 0){
                std::cout << "jump " << ifend << endl;
            }
            else {
                backend = 0;
            }
            std::cout << ifend << ":" << endl;
            return string("");
        }
        else if(type == 8) {
            string while_entry = "\%while_entry_" + to_string(if_cnt);
            string while_body = "\%while_body_" + to_string(if_cnt);
            string while_end = "\%end_" + to_string(if_cnt);
            ifstack.push(if_cnt);
            if_cnt ++;
            std::cout << "jump " << while_entry <<endl;
            std::cout << while_entry <<":" <<endl;
            string ret = exp->Dump();
            std::cout << "br " << ret << ", " << while_body << ", " << while_end <<endl;
            std::cout << while_body << ":" << endl; 
            stmt1->Dump();
            if (backend == 0) {
                std::cout << "jump " << while_entry << endl;
            }
            else {
                backend = 0;
            }
            ifstack.pop();
            std::cout << while_end << ":" <<endl;
            return string("");
        }
        else if(type == 9) {//break
            int iftemp = ifstack.top();
            std::cout << "jump %end_" << iftemp <<endl;
            backend = 1;
            return string("");
        }
        else if(type == 10) {
            int iftemp = ifstack.top();
            backend = 1;
            std::cout << "jump %while_entry_" << iftemp << endl;
            return string("");
        }
            
        return string("ERROR");
    }
    int Calc() const override {
        return exp->Calc();
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
    int Calc() const override {
        return l_or_exp->Calc();
    }
};

//-> PrimaryExp    ::= "(" Exp ")" | LVal | Number;
class PrimaryExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> l_val;
    std::unique_ptr<BaseAST> number;
    string Dump() const override {
        if(type == 0){
            string ret = exp->Dump();
            return ret;
        }
        else if(type == 1) {
            string ident = l_val->Dump();
            //Symbol s = symbt.globalsymbol[ident];
            Symbol *s = find_ident(ident);
            if(s->type == 0) {
                return to_string(l_val->Calc());
            }
            else if(s->type == 1) {
                now++;
                std::cout << "%" << now << " = load @" << s->str <<endl;
                return string("%") + to_string(now);
            }
        }
        else if(type == 2){
            return number->Dump();
        }
        return string("");
    }
    int Calc() const override {
        if (type == 0) {
            return exp->Calc();
        }
        else if(type == 1) {
            return l_val->Calc();
        }
        else if(type == 2) {
            return number->Calc();
        }
        return 0;
    }
};

//Number    ::= INT_CONST;
class NumberAST : public BaseAST {
    public:
    int int_const;
    string Dump() const override {
        return to_string(int_const);
    }
    int Calc() const override {
        return int_const;
    }
};

// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp; | IDENT "("  ")" | IDENT "(" FuncRParams ")"
class UnaryExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> primary_exp;
    std::unique_ptr<BaseAST> unary_op;
    std::unique_ptr<BaseAST> unary_exp;
    std::string ident;
    std::unique_ptr<BaseAST> func_r_params;
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
        else if (type == 2) {
            if(func_type_map[ident] == 1){
                now ++;
                std::cout << "%" << now << " = ";
            }
            
            std::cout << "call @" << ident << "()" << endl;
            if(func_type_map[ident] == 1)
                return string("%") + to_string(now);
            else return string("");
        }
        else if (type == 3) {
            string ret = func_r_params->Dump();
            if(func_type_map[ident] == 1){
                now ++;
                std::cout << "%" << now << " = ";
            }
            std::cout << "call @" << ident << "(";
            std::cout << ret;
            std::cout << ")" << endl;
            if(func_type_map[ident] == 1)
                return string("%") + to_string(now);
            else return string("");
        }
        return string("");
    }
    int Calc() const override {
        if(type == 0){
            return primary_exp->Calc();
        }
        else if(type == 1){
            if(unary_op->type == 0){
                return unary_exp->Calc();
            }
            else if(unary_op->type == 1){
                return -unary_exp->Calc();
            }
            else if(unary_op->type == 2){
                return !unary_exp->Calc();
            }
        }
        else if (type == 2) {
            return 0;
        }
        else if (type == 3) {
            return 0;
        }
        return 0;
    }
};
//UnaryOp     ::= "+" | "-" | "!";
class UnaryOpAST : public BaseAST {
    public:
    string Dump() const override {
        return string("");
    }
    int Calc() const override {
        return 0;
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
    int Calc() const override {
        if(type == 0){
            return unary_exp->Calc();
        }
        else if(type == 1){
            if(mul_op->type == 0){
                return mul_exp->Calc() * unary_exp->Calc();
            }
            else if(mul_op->type == 1){
                return mul_exp->Calc() / unary_exp->Calc();
            }
            else if(mul_op->type == 2){
                return mul_exp->Calc() % unary_exp->Calc();
            }
        }
        return 0;
    }
};

class MulOpAST : public BaseAST {
    public:
    string Dump() const override {
        return string("");
    }
    int Calc() const override {
        return 0;
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
    int Calc() const override {
        if(type == 0) {
            return mul_exp->Calc();
        }
        else if(type == 1) {
            if(add_op->type == 0){
                return add_exp->Calc() + mul_exp->Calc();
            }
            else if(add_op->type == 1){
                return add_exp->Calc() - mul_exp->Calc();
            }
        }
        return 0;
    }
};

class AddOpAST : public BaseAST {
    public:
    string Dump() const override {
        return string("");
    }
    int Calc() const override {
        return 0;
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
    int Calc() const override {
        if(type == 0) {
            return add_exp->Calc();
        }
        else if(type == 1) {
            if(rel_op->type == 0) {
                return rel_exp->Calc() < add_exp->Calc();
            }
            else if (rel_op->type == 1) {
                return rel_exp->Calc() > add_exp->Calc();
            }
            else if (rel_op->type == 2) {
                return rel_exp->Calc() <= add_exp->Calc();
            }
            else if (rel_op->type == 3) {
                return rel_exp->Calc() >= add_exp->Calc();
            }
        }
        return 0;
    }
};

class RelOpAST : public BaseAST {
    public:
    string Dump() const override {
        return string("");
    }
    int Calc() const override {
        return 0;
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
    int Calc() const override {
        if(type == 0) {
            return rel_exp->Calc();
        }
        else if(type == 1) {
            if(eq_op->type == 0){
                return eq_exp->Calc() == rel_exp->Calc();
            }
            else if(eq_op->type == 1) {
                return eq_exp->Calc() != rel_exp->Calc();
            }
        }
        return 0;
    }
};

class EqOpAST : public BaseAST {
    public:
    string Dump() const override {
        return string("");
    }
    int Calc() const override {
        return 0;
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
            string reseult = string("l_cal_cnt_") + to_string(l_cal_cnt);
            l_cal_cnt++;
            Symbol s;
            s.type = 1;
            s.value = 0;
            s.str = reseult + "_" + to_string(depth);
            std::cout << "@" << s.str << " = alloc i32" << endl; 
            std::cout << "store 0, @" << s.str << endl;
            insert_ident(reseult,s);

            now++;
            std::cout << "%" << now <<" = ne " << ret1 <<", 0" << endl;
            string br = string("%") + to_string(now);
            string ifthen = "\%then_" + to_string(if_cnt);
            string ifend = "\%end_" + to_string(if_cnt);
            if_cnt ++;
            std::cout << "br " << br << ", " << ifthen << ", " << ifend << endl;

            std::cout << ifthen << ":" << endl;
            string ret2 = eq_exp->Dump();
            now++;
            std::cout << "%" << now <<" = ne " << ret2 <<", 0" << endl;
            std::cout << "store %" << now << ", @" << s.str << endl;
            std::cout << "jump " << ifend << endl;

            std::cout << ifend << ":" << endl;
            now++;
            std::cout << "\%" << now << " = load @" << s.str << endl;
            
            return string("%")+to_string(now);
        }
        return string("ERROR");
    }
    int Calc() const override {
        if(type == 0) {
            return eq_exp->Calc();
        }
        else if (type == 1) {
            if (l_and_exp->Calc() == 0) {
                return 0;
            }
            return l_and_exp->Calc() && eq_exp->Calc();
        }
        return 0;
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
            string reseult = string("l_cal_cnt_") + to_string(l_cal_cnt);
            l_cal_cnt++;
            Symbol s;
            s.type = 1;
            s.value = 0;
            s.str = reseult + "_" + to_string(depth);
            std::cout << "@" << s.str << " = alloc i32" << endl; 
            std::cout << "store 1, @" << s.str << endl;
            insert_ident(reseult,s);

            now++;
            std::cout << "%" << now <<" = eq " << ret1 <<", 0" << endl;
            string br = string("%") + to_string(now);
            string ifthen = "\%then_" + to_string(if_cnt);
            string ifend = "\%end_" + to_string(if_cnt);
            if_cnt ++;
            std::cout << "br " << br << ", " << ifthen << ", " << ifend << endl;

            std::cout << ifthen << ":" << endl;
            string ret2 = l_and_exp->Dump();
            now++;
            std::cout << "%" << now <<" = ne " << ret2 <<", 0" << endl;
            std::cout << "store %" << now << ", @" << s.str << endl;
            std::cout << "jump " << ifend << endl;

            std::cout << ifend << ":" << endl;
            now++;
            std::cout << "\%" << now << " = load @" << s.str << endl;
            
            return string("%")+to_string(now);
        }
        return string("ERROR");
    }
    int Calc() const override {
        if (type == 0) {
            return l_and_exp->Calc();
        }
        else if(type == 1) {
            if(l_or_exp->Calc() != 0) {
                return 1;
            }
            return l_or_exp->Calc() || l_and_exp->Calc();
        }
        std::cout << "ERROR";
        return 0;
    }
};

// Decl          ::= ConstDecl;
//Decl ::= ConstDecl | VarDecl
class DeclAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> const_decl;
    std::unique_ptr<BaseAST> var_decl;
    string Dump() const override {
        if(backend == 1) {
            return string("");
        }
        string ret = string("");
        if(type == 0) ret = const_decl->Dump();
        else if (type == 1) ret = var_decl->Dump();
        return ret;
    }
    int Calc() const override {
        return 0;
    }
};

// ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
//->ConstDecl ::= "const" BType ConstDef ";";
class ConstDeclAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> b_type;
    std::unique_ptr<BaseAST> const_def;
    string Dump() const override {
        string ret = const_def->Dump();
        return ret;
    }
    int Calc() const override {
        return 0;
    }
};

// BType         ::= "int";
class BTypeAST : public BaseAST {
    public:
    string Dump() const override {
        return string("");
    }
    int Calc() const override {
        return 0;
    }
};


//->ConstDef ::= IDENT "=" ConstInitVal | IDENT "=" ConstInitVal "," ConstDef
class ConstDefAST : public BaseAST {
    public:
    std::string ident;
    std::unique_ptr<BaseAST> const_init_val;
    std::unique_ptr<BaseAST> const_def;
    string Dump() const override {
        Symbol s;
        s.type = 0;
        s.value = const_init_val->Calc();
        insert_ident(ident, s);
        if(type == 1) {
            const_def->Dump();
        }
        return string("");
    }
    int Calc() const override {
        return 0;
    }
};


// ConstInitVal  ::= ConstExp;
class ConstInitValAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> const_exp;
    string Dump() const override {
        return string("");
    }
    int Calc() const override {
        return const_exp->Calc();
    }
};

//VarDecl       ::= BType VarDef {"," VarDef} ";";
class VarDeclAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> b_type;
    std::unique_ptr<BaseAST> var_def;
    string Dump() const override {
        var_def->Dump();
        return string("");
    }
    int Calc() const override {
        return 0;
    }

};

//VarDef        ::= IDENT | IDENT "=" InitVal;
//->              = IDENT | IDENT "=" InitVal | IDENT "," VarDef | IDENT "=" InitVal "," VarDef
class VarDefAST : public BaseAST {
    public:
    std::string ident;
    std::unique_ptr<BaseAST> init_val;
    std::unique_ptr<BaseAST> var_def;
    string Dump() const override {
        if(cur_func_name == string("")){
            Symbol s;
            s.type = 1;
            s.str = ident;
            s.value = 0;
            std::cout << "global @" << s.str << " = alloc i32";
            if(type == 1 || type == 3) {
                int ret = init_val->Calc();
                s.value = ret;
                std::cout << ", " << ret << endl;
            }
            else {
                std::cout << ", zeroinit" << endl;
            }
            insert_ident(ident, s);
            if(type == 2 || type == 3) {
                var_def->Dump();
            }
            return string("");
        }
        Symbol s;
        s.type = 1;
        s.value = 0;
        s.str = ident  + "_" + to_string(depth);
        std::cout << "@" << s.str <<" = alloc i32" <<endl;
        if(type == 1 || type == 3) {
            string ret = init_val->Dump();
            std::cout << "store " << ret << ", @" << s.str << endl;
        }
        insert_ident(ident, s);
        if(type == 2 || type == 3) {
            var_def->Dump();
        }
        return string("");
    }
    int Calc() const override {
        return 0;
    }
};

//InitVal       ::= Exp;
class InitValAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    string Dump() const override {
        string ret = exp->Dump();
        return ret;
    }
    int Calc() const override {
        return exp->Calc();
    }
};

// BlockItem ::= Decl | Stmt | Decl BlockItem | Stmt BlockItem
class BlockItemAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> decl;
    std::unique_ptr<BaseAST> stmt;
    std::unique_ptr<BaseAST> block_item;
    string Dump() const override {
        if (type == 0) {
            string ret = decl->Dump();
            return ret;
        }
        else if (type == 1) {
            string ret = stmt->Dump();
            return ret;
        }
        else if (type == 2) {
            string ret1 = decl->Dump();
            string ret2 = block_item->Dump();
            return string("");
        }
        else if (type == 3) {
            string ret1 = stmt->Dump();
            string ret2 = block_item->Dump();
            return string("");
        }
        return string("ERROR");
    }
    int Calc() const override {
        return 0;
    }
};


// LVal          ::= IDENT;
class LValAST : public BaseAST {
    public:
    std::string ident;
    string Dump() const override {
        return ident;
    }
    int Calc() const override {
        //Symbol s = symbt.globalsymbol[ident];
        Symbol *s = find_ident(ident);
        return s->value;
    }
};


// ConstExp      ::= Exp;
class ConstExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    string Dump() const override {
        return string("");
    }
    int Calc() const override {
        return exp->Calc();
    }
};

//FuncFParams ::= FuncFParam;
class FuncFParamsAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_f_param;
    string Dump() const override {
        func_f_param->Dump();
        return string("");
    }
    int Calc() const override {
        func_f_param->Calc();
        return 0;
    }

};

//FuncFParam  ::= BType IDENT | BType IDENT "," FuncFParam;
class FuncFParamAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> b_type;
    std::string ident;
    std::unique_ptr<BaseAST> func_f_param;
    string Dump() const override {
        if (type == 0) {
            std::cout << "@" << ident <<": i32";
        }
        else if (type == 1) {
            std::cout << "@" << ident <<": i32, ";
            func_f_param->Dump(); 
        }
        return string("");
    }
    int Calc() const override {
        auto &smap = funcsymbolmap[cur_func_name];
        Symbolmap fsl;
        smap.push_back(fsl);
        Symbol s;
        s.type = 1;
        s.value = 0;
        s.str = ident  + "_" + to_string(depth);
        std::cout << "@" << s.str <<" = alloc i32" <<endl;
        std::cout << "store @" << ident << ", @" << s.str << endl;
        insert_ident(ident, s);
        if(type == 1) {
            func_f_param->Calc();
        }
        return 0;
    }
};

//FuncRParams ::= Exp {"," Exp};
//FuncRParams = Exp | Exp ',' FuncRParams
class FuncRParamsAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> func_r_params;
    string Dump() const override {
        string ret1 = exp->Dump();
        if(type == 1) {
            string ret2 =  func_r_params->Dump();
            return ret1 + string(", ") + ret2;
        }
        return ret1;
    }
    int Calc() const override {
        return 0;
    }
}
;