#include "codatabase.h"
#include "va.h"

typedef void (*coStatementHandler)(coStatement statement, bool preDecent);

// Main routines
coModule coParseSourceFile(coModule outerModule, char *fileName);
coModule coParseCommand();
void coPreprocessDataTypes(coModule module);
void coWrapModuleWithL42Main(coModule module);
void coInterpreter(void);
coSharedlib coCompileModule(coModule module);
coSharedlib coCompileStatement(coStatement statement);
vaValue coRunSharedLib(coSharedlib sharedlib);
bool coPreprocessModule(coModule module);
bool coGenerateCCode(coModule module, char *fileName);
void coCreateBuiltins(void);

// Module methods.
coModule coModuleCreate(coModule outerModule, utSym name);

// Typedef methods.
coTypedef coTypedefCreate(coNamespace namespace, utSym name, coType type);

// MType/type methods.
coType coBasicTypeCreate(vaType type);
coType coIntTypeCreate(vaType type, uint8 width);
coType coIdentTypeCreate(utSym name);
coMType coTupleTypeCreate(void);
coType coListTypeCreate(coType type);
coType coObjectTypeCreate(coType type);
void coAddFieldToTuple(coMType tuple, coType type, utSym name,
    coExpr defaultExpr, vaString comment);
coType coHashTuple(coMType type);

// Enum and entry methods.
coEnum coEnumCreate(coNamespace namespace, utSym name);
coEntry coEntryCreate(coEnum Enum, utSym name, vaString comment);

// Statement methods.
coStatement coStatementCreate(coStaterule staterule);
coStatement coModuleStatementCreate(coStatement outerStatement, coModule module);
coStatement coCommentStatementCreate(coStatement outerStatement, vaString comment);
coStatement coTypedefStatementCreate(coStatement outerStatement, coTypedef Typedef);
coStatement coEnumStatementCreate(coStatement outerStatement, coEnum Enum);
coStatement coFunctionStatementCreate(coStatement outerStatement, coFunction function);
coStatement coExprStatementCreate(coStatement outerStatement, coExpr expr);
coStatement coReturnStatementCreate(coStatement outerStatement, coExpr expr);
void coClassStatementHandler(coStatement statement, bool preDecent);
void coDefStatementHandler(coStatement statement, bool preDecent);
void coDoStatementHandler(coStatement statement, bool preDecent);
void coWhileStatementHandler(coStatement statement, bool preDecent);
void coIfStatementHandler(coStatement statement, bool preDecent);
void coElseIfStatementHandler(coStatement statement, bool preDecent);
void coElseStatementHandler(coStatement statement, bool preDecent);
void coReturnStatementHandler(coStatement statement, bool preDecent);
void coEnumStatementHandler(coStatement statement, bool preDecent);
void coExprStatementHandler(coStatement statement, bool preDecent);

// Function methods.
coFunction coFunctionCreate(coNamespace namespace, utSym name, coType returnType,
    coType parameterType);

// Function pointer methods.
coFuncptr coFuncptrCreate(coModule module, utSym sym, coStatementHandler handler);

// Ident methods
coIdent coIdentCreate(coNamespace namespace, coIdentType type, utSym name);
coIdent coFindIdent(coExpr dotExpr);

// Namespace methods
coNamespace coNamespaceCreate(coIdent outerIdent);
void coCreateTopNamespace(void);

// Expresion methods.
coExpr coExprCreate(coExprType type);
coExpr coUnaryExprCreate(coExprType type, coExpr a);
coExpr coBinaryExprCreate(coExprType type, coExpr a, coExpr b);
coExpr coTrinaryExprCreate(coExprType type, coExpr a,
    coExpr b, coExpr c);
coExpr coValueExprCreate(vaValue value);
coExpr coIdentExprCreate(utSym sym);
coExpr coOperatorExprCreate(coOperator operator);
void coPrintExpr(coExpr expr);

// Syntax
void coPrintElement(coElement element);
void coPrintPattern(coPattern pattern);
void coPrintStaterule(coStaterule staterule);
void coPrintNoderule(coNoderule noderule);
void coPrintOperator(coOperator operator);
void coPrintPrecedenceGroup(coPrecedenceGroup precedenceGroup);
void coPrintSyntax(coSyntax syntax);

// Shortcuts and utilities
char *coCapitalize(char *string);
static inline char *coEnumGetName(coEnum Enum)
    {return coIdentGetName(coEnumGetIdent(Enum));}
static inline char *coEntryGetName(coEntry entry)
    {return coIdentGetName(coEntryGetIdent(entry));}
static inline char *coFunctionGetName(coFunction function)
    {return coIdentGetName(coFunctionGetIdent(function));}
static inline char *coTypedefGetName(coTypedef Typedef)
    {return coIdentGetName(coTypedefGetIdent(Typedef));}
static inline char *coNamespaceGetName(coNamespace namespace)
    {return coIdentGetName(coNamespaceGetIdent(namespace));}
static inline char *coFuncptrGetName(coFuncptr funcptr)
    {return coIdentGetName(coFuncptrGetIdent(funcptr));}
static inline coNamespace coFunctionGetNamespace(coFunction function)
    {return coIdentGetNamespace(coFunctionGetIdent(function));}

// Globals
extern coRoot coTheRoot;
extern coNamespace coTopNamespace;
extern coModule coTopModule;
extern coStatement coTopStatement;
extern coSyntax coL42Syntax;
extern coModule coBuiltinModule;
extern bool coQuit; // Set true to force interpreter to end

// Lex, Yacc stuff */
void coLexerStart(void);
void coLexerStop(void);
coModule coParse(coModule outerModule, utSym ModuleName);
coToken coLex(void);
void coPrintToken(coToken token);
void coPrintNodeExpr(coNodeExpr nodeExpr);
void coError(coToken token, char *message, ...);
void coExprError(coExpr expr, char *message, ...);

extern coSyntax coCurrentSyntax;
extern FILE *coFile;
extern uint32 coFileSize, coLineNum;

// Global symbols
extern utSym coIdentSym, coIntegerSym, coFloatSym, coStringSym, coBoolSym, coCharSym,
    coExprSym;
