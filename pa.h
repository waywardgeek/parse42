#include "padatabase.h"

typedef void (*paStatementHandler)(paStatement statement, bool preDecent);

// Main routines
paStatement paParseSourceFile(char *fileName);
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
void paPrintElement(paElement element);
void paPrintPattern(paPattern pattern);
void paPrintStaterule(paStaterule staterule);
void paPrintNoderule(paNoderule noderule);
void paPrintOperator(paOperator operator);
void paPrintPrecedenceGroup(paPrecedenceGroup precedenceGroup);
void paPrintSyntax(paSyntax syntax);

// Function pointer methods.
paFuncptr paFuncptrCreate(utSym sym, paStatementHandler handler);

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

extern paSyntax paL42Syntax, paCurrentSyntax;
extern FILE *paFile;
extern uint32 paFileSize, paLineNum;

// Global symbols
extern utSym paIdentSym, paIntegerSym, paFloatSym, paStringSym, paBoolSym, paCharSym,
    paExprSym;
