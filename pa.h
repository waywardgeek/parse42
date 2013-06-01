#include "padatabase.h"

typedef void (*paStatementHandler)(paStatement statement);

// Main routines
paStatement paParseSourceFile(paSyntax syntax, char *fileName);
void paCreateBuiltins(void);

// Statement methods.
paStatement paStatementCreate(paStatement outerStatement, paStaterule staterule);
paStatement paCommentStatementCreate(paStatement outerStatement, vaString comment);

// Expresion methods.
paExpr paExprCreate(paExprType type);
paExpr paValueExprCreate(vaValue value);
paExpr paIdentExprCreate(utSym sym);
paExpr paOperatorExprCreate(paOperator operator);
void paPrintExpr(paExpr expr);

// Syntax
void paSyntaxStart(void);
void paSyntaxStop(void);
paSyntax paSyntaxCreate(utSym name);
void paProcessSyntaxStatement(paSyntax targetSyntax, paStatement statement);
paPrecedenceGroup paPrecedenceGroupCreate(paSyntax syntax, paOperator operator);
paStaterule paStateruleCreate(paSyntax syntax, utSym name, bool hasBlock,
    paExpr patternExpr);
paNoderule paNoderuleCreate(paSyntax syntax, utSym sym, paExpr ruleExpr);
paPattern paPatternCreate(paSyntax syntax, paExpr patternExpr);
paNodeExpr paTokenNodeExprCreate(paNodeExprType type);
paNodeExpr paSubruleNodeExprCreate(char *ruleName);
paNodelist paNodelistCreate(paNoderule noderule, paExprType type, utSym sym);
void paCheckNoderules(paSyntax syntax);
void paSetOperatorPrecedence(paSyntax syntax);
void paPrintElement(paElement element);
void paPrintPattern(paPattern pattern);
void paPrintStaterule(paStaterule staterule);
void paPrintNoderule(paNoderule noderule);
void paPrintOperator(paOperator operator);
void paPrintPrecedenceGroup(paPrecedenceGroup precedenceGroup);
void paPrintSyntax(paSyntax syntax);

// Globals
extern paRoot paTheRoot;

// Parsing stuff
void paLexerStart(void);
void paLexerStop(void);
paStatement paParse();
paToken paLex(void);
void paPrintToken(paToken token);
void paPrintNodeExpr(paNodeExpr nodeExpr);
void paError(paToken token, char *message, ...);
void paExprError(paExpr expr, char *message, ...);

extern paSyntax paParseSyntax, paCurrentSyntax;
extern FILE *paFile;
extern uint32 paFileSize, paLineNum;

// Global symbols
extern utSym paIdentSym, paIntegerSym, paFloatSym, paStringSym, paBoolSym, paCharSym,
    paExprSym;
