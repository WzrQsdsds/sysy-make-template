%code requires {
  #include <memory>
  #include <string>
  #include "ast.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串

%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况

%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN EQ NEQ GEQ LEQ LAND LOR
%token <str_val> IDENT CONST
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Number Exp PrimaryExp UnaryExp UnaryOp AddExp MulExp AddOp MulOp RelExp RelOp EqExp EqOp LAndExp LOrExp Decl ConstDecl BType ConstDef ConstInitVal BlockItem LVal ConstExp VarDecl VarDef InitVal

%%

// 开始符, CompUnit ::= FuncDef, 
//大括号后声明了解析完成后 parser 要做的事情
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    $$ = ast;
  }
  ;

////Block         ::= "{" {BlockItem} "}"; | '{' '}'
Block
  : '{' BlockItem '}' {
    auto ast = new BlockAST();
    ast->type = 1;
    ast->block_item = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new BlockAST();
    ast->type = 2;
    $$ = ast;
  }
  ;

// Stmt ::= LVal "=" Exp ";"
//        | Exp ";" | ";"
//        | Block
//        | "return" Exp ";" | return ";"
Stmt
  : LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->type = 0;
    ast->l_val = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new StmtAST();
    ast->type = 1;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ';' {
    auto ast = new StmtAST();
    ast->type = 2;
    $$ = ast;
  }
  | Block {
    auto ast = new StmtAST();
    ast->type = 3;
    ast->block = unique_ptr<BaseAST>($1);
  }
  | RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->type = 4;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | RETURN ';' {
    auto ast = new StmtAST();
    ast->type = 5;
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->l_or_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

//PrimaryExp    ::= "(" Exp ")" | LVal | Number;
PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->type = 0;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | LVal {
    auto ast = new PrimaryExpAST();
    ast->type = 1;
    ast->l_val = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST();
    ast->type = 2;
    ast->number = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    auto ast = new NumberAST();
    ast->int_const = $1;
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->type = 0;
    ast->primary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST();
    ast->type = 1;
    ast->unary_op = unique_ptr<BaseAST>($1);
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

UnaryOp
  : '+' {
    auto ast = new UnaryOpAST();
    ast->type = 0;
    $$ = ast;
  }
  | '-' {
    auto ast = new UnaryOpAST();
    ast->type = 1;
    $$ = ast;
  }
  | '!' {
    auto ast = new UnaryOpAST();
    ast->type = 2;
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp {
    auto ast = new MulExpAST();
    ast->type = 0;
    ast->unary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | MulExp MulOp UnaryExp {
    auto ast = new MulExpAST();
    ast->type = 1;
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->mul_op = unique_ptr<BaseAST>($2);
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

MulOp
  : '*' {
    auto ast = new MulOpAST();
    ast->type = 0;
    $$ = ast;
  } 
  | '/' {
    auto ast = new MulOpAST();
    ast->type = 1;
    $$ = ast;
  }
  | '%' {
    auto ast = new MulOpAST();
    ast->type = 2;
    $$ = ast;
  }
  ;
  
AddExp
  : MulExp {
    auto ast = new AddExpAST();
    ast->type = 0;
    ast->mul_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp AddOp MulExp{
    auto ast = new AddExpAST();
    ast->type = 1;
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->add_op = unique_ptr<BaseAST>($2);
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

AddOp
  : '+' {
    auto ast = new AddOpAST();
    ast->type = 0;
    $$ = ast;
  }
  | '-' {
    auto ast = new AddOpAST();
    ast->type = 1;
    $$ = ast;
  }
  ;

RelExp
  : AddExp {
    auto ast = new RelExpAST();
    ast->type = 0;
    ast->add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } 
  | RelExp RelOp AddExp {
    auto ast = new RelExpAST();
    ast->type = 1;
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->rel_op = unique_ptr<BaseAST>($2);
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

RelOp
  : '<' {
    auto ast = new RelOpAST();
    ast->type = 0;
    $$ = ast;
  }
  | '>' {
    auto ast = new RelOpAST();
    ast->type = 1;
    $$ = ast;
  }
  | LEQ {
    auto ast = new RelOpAST();
    ast->type = 2;
    $$ = ast;
  }
  | GEQ {
    auto ast = new RelOpAST();
    ast->type = 3;
    $$ = ast;
  }
  ;

EqExp
  : RelExp {
    auto ast = new EqExpAST();
    ast->type = 0;
    ast->rel_exp = unique_ptr<BaseAST>($1);
    $$ = ast;    
  }
  | EqExp EqOp RelExp {
    auto ast = new EqExpAST();
    ast->type = 1;
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->eq_op = unique_ptr<BaseAST>($2);
    ast->rel_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

EqOp
  : EQ {
    auto ast = new EqOpAST();
    ast->type = 0;
    $$ = ast;
  }
  | NEQ {
    auto ast = new EqOpAST();
    ast->type = 1;
    $$ = ast;
  }
  ;

LAndExp
  : EqExp {
    auto ast = new LAndExpAST();
    ast->type = 0;
    ast->eq_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LAndExp LAND EqExp {
    auto ast = new LAndExpAST();
    ast->type = 1;
    ast->l_and_exp = unique_ptr<BaseAST>($1);
    ast->eq_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
    auto ast = new LOrExpAST();
    ast->type = 0;
    ast->l_and_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp LOR LAndExp {
    auto ast = new LOrExpAST();
    ast->type = 1;
    ast->l_or_exp = unique_ptr<BaseAST>($1);
    ast->l_and_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

//lv4----------------------------------------------

Decl
  : ConstDecl {
    auto ast = new DeclAST();
    ast->type = 0;
    ast->const_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST();
    ast->type = 1;
    ast->var_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstDecl
  : CONST BType ConstDef ';' {
    auto ast = new ConstDeclAST();
    ast->b_type = unique_ptr<BaseAST>($2);
    ast->const_def = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

BType
: INT {
    auto ast = new FuncTypeAST();
    $$ = ast;
  }
  ;

//ConstDef ::= IDENT "=" ConstInitVal | IDENT "=" ConstInitVal "," ConstDef
ConstDef
: IDENT '=' ConstInitVal {
  auto ast = new ConstDefAST();
  ast->type = 0;
  ast->ident = *unique_ptr<std::string>($1);
  ast->const_init_val = unique_ptr<BaseAST>($3);
  $$ = ast;
} 
| IDENT '=' ConstInitVal ',' ConstDef {
  auto ast = new ConstDefAST();
  ast->type = 1;
  ast->ident = *unique_ptr<std::string>($1);
  ast->const_init_val = unique_ptr<BaseAST>($3);
  ast->const_def = unique_ptr<BaseAST>($5);
  $$ = ast;
}
;

ConstInitVal
: ConstExp {
  auto ast = new ConstInitValAST();
  ast->const_exp = unique_ptr<BaseAST>($1);
  $$ = ast;
}
;

VarDecl
: BType VarDef  ';' {
  auto ast = new VarDeclAST();
  ast->b_type = unique_ptr<BaseAST>($1);
  ast->var_def = unique_ptr<BaseAST>($2);
  $$ = ast;
}
;

VarDef
: IDENT {
  auto ast = new VarDefAST();
  ast->type = 0;
  ast->ident = *unique_ptr<std::string>($1);
  $$ = ast;
}
| IDENT '=' InitVal {
  auto ast = new VarDefAST();
  ast->type = 1;
  ast->ident = *unique_ptr<std::string>($1);
  ast->init_val = unique_ptr<BaseAST>($3);
  $$ = ast;
}
| IDENT ',' VarDef {
  auto ast = new VarDefAST();
  ast->type = 2;
  ast->ident = *unique_ptr<std::string>($1);
  ast->var_def = unique_ptr<BaseAST>($3);
  $$ = ast;
}
| IDENT '=' InitVal ',' VarDef {
  auto ast = new VarDefAST();
  ast->type = 3;
  ast->ident = *unique_ptr<std::string>($1);
  ast->init_val = unique_ptr<BaseAST>($3);
  ast->var_def = unique_ptr<BaseAST>($5);
  $$ = ast;
}
;

InitVal
: Exp {
  auto ast = new InitValAST();
  ast->exp = unique_ptr<BaseAST>($1);
  $$ = ast;
}
;

//Block         ::= "{" {BlockItem} "}";

//BlockItem ::= Decl | Stmt | Decl BlockItem | Stmt BlockItem
BlockItem
: Decl {
  auto ast = new BlockItemAST();
  ast->type = 0;
  ast->decl = unique_ptr<BaseAST>($1);
  $$ = ast;
}
| Stmt {
  auto ast = new BlockItemAST();
  ast->type = 1;
  ast->stmt = unique_ptr<BaseAST>($1);
  $$ = ast;
}
| Decl BlockItem {
  auto ast = new BlockItemAST();
  ast->type = 2;
  ast->decl = unique_ptr<BaseAST>($1);
  ast->block_item = unique_ptr<BaseAST>($2);
  $$ = ast;
}
| Stmt BlockItem {
  auto ast = new BlockItemAST();
  ast->type = 3;
  ast->stmt = unique_ptr<BaseAST>($1);
  ast->block_item = unique_ptr<BaseAST>($2);
  $$ = ast;
}
;

LVal
: IDENT {
  auto ast = new LValAST();
  ast->ident = *unique_ptr<std::string>($1);
  $$ =ast;
}
;

//PrimaryExp    ::= "(" Exp ")" | LVal | Number;

ConstExp
: Exp {
  auto ast = new ConstExpAST();
  ast->exp = unique_ptr<BaseAST>($1);
  $$ = ast;
}
;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s <<" "<< ast->Dump() << endl;
}