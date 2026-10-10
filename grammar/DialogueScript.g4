grammar DialogueScript;

// ===== Парсер (синтаксис) =====
program
    : startDecl? branchDecl* EOF
    ;

startDecl
    : 'start' IDENTIFIER
    ;

branchDecl
    : IDENTIFIER '{' statement* '}'
    ;

statement
    : npcStmt
    | choiceStmt
    | ifStmt
    | gotoStmt
    | actionStmt
    ;

npcStmt
    : 'npc' STRING ':' STRING
    ;

choiceStmt
    : 'choice' STRING '{' option+ '}'
    ;

option
    : 'option' STRING '->' IDENTIFIER
    ;

ifStmt
    : 'if' condition '{' statement* '}' ('else' '{' statement* '}')?
    ;

condition
    : 'has_item' '(' STRING ')'
    | 'has_flag' '(' STRING ')'
    ;

gotoStmt
    : 'goto' IDENTIFIER
    ;

actionStmt
    : 'give_item' '(' STRING ')'
    | 'take_item' '(' STRING ',' INT ')'
    | 'set_flag' '(' STRING ',' BOOL ')'
    ;

// ===== Лексер (токены) =====
BOOL       : 'true' | 'false' ;
IDENTIFIER : [a-zA-Z_][a-zA-Z_0-9]* ;
STRING     : '"' (~["\r\n])* '"' ;
INT        : [0-9]+ ;

WS         : [ \t\r\n]+ -> skip ;
LINE_COMMENT : '//' ~[\r\n]* -> skip ;
BLOCK_COMMENT : '/*' .*? '*/' -> skip ;
