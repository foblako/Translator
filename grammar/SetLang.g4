grammar SetLang;

program: statement* EOF;

statement:
      assignment ';'                               #assignStmt
    | 'print' '(' expr ')' ';'                     #printStmt
    | 'if' '(' expr ')' block ('else' block)?      #ifStmt
    | 'while' '(' expr ')' block                   #whileStmt
    ;

block: '{' statement* '}';

assignment: ID '=' expr;

expr:
      expr ('*' | '/') expr                        #mulDivExpr
    | expr ('+' | '-') expr                        #addSubExpr
    | expr ('==' | '!=') expr                      #eqNeqExpr
    | '{' (expr (',' expr)*)? '}'                  #setExpr
    | INT                                          #intExpr
    | ID                                           #idExpr
    | '(' expr ')'                                 #parensExpr
    ;

INT: [0-9]+;
ID: [a-zA-Z_][a-zA-Z0-9_]*;

WS: [ \t\r\n]+ -> skip;
LINE_COMMENT: '//' ~[\r\n]* -> skip;
