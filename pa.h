#include "padatabase.h"
#include "va.h"

typedef void (*paStatementHandler)(paStatement statement, bool preDecent);

// Main routines
paModule paParseSourceFile(paModule outerModule, char *fileName);
paModule paParseCommand();
void paPreprocessDataTypes(paModule module);
void paWrapModuleWithL42Main(paModule module);
void paInterpreter(void);
paSharedlib paCompileModule(paModule module);
paSharedlib paCompileStatement(paStatement statement);
vaValue paRunSharedLib(paSharedlib sharedlib);
bool paPreprocessModule(paModule module);
bool paGenerateCCode(paModule module, char *fileName);
void paCreateBuiltins(void);

// Module methods.
paModule paModuleCreate(paModule outerModule, utSym name);

// Typedef methods.
paTypedef paTypedefCreate(paNamespace namespace, utSym name, paType type);

// MType/type methods.
paType paBasicTypeCreate(vaType type);
paType paIntTypeCreate(vaType type, uint8 width);
paType paIdentTypeCreate(utSym name);
paMType paTupleTypeCreate(void);
paType paListTypeCreate(paType type);
paType paObjectTypeCreate(paType type);
void paAddFieldToTuple(paMType tuple, paType type, utSym name,
    paExpr defaultExpr, vaString comment);
paType paHashTuple(paMType type);

// Enum and entry methods.
paEnum paEnumCreate(paNamespace namespace, utSym name);
paEntry paEntryCreate(paEnum Enum, utSym name, vaString comment);

// Statement methods.
paStatement paStatementCreate(paStaterule staterule);
paStatement paModuleStatementCreate(paStatement outerStatement, paModule module);
paStatement paCommentStatementCreate(paStatement outerStatement, vaString comment);
paStatement paTypedefStatementCreate(paStatement outerStatement, paTypedef Typedef);
paStatement paEnumStatementCreate(paStatement outerStatement, paEnum Enum);
paStatement paFunctionStatementCreate(paStatement outerStatement, paFunction function);
paStatement paExprStatementCreate(paStatement outerStatement, paExpr expr);
paStatement paReturnStatementCreate(paStatement outerStatement, paExpr expr);
void paClassStatementHandler(paStatement statement, bool preDecent);
void paDefStatementHandler(paStatement statement, bool preDecent);
void paDoStatementHandler(paStatement statement, bool preDecent);
void paWhileStatementHandler(paStatement statement, bool preDecent);
void paIfStatementHandler(paStatement statement, bool preDecent);
void paElseIfStatementHandler(paStatement statement, bool preDecent);
void paElseStatementHandler(paStatement statement, bool preDecent);
void paReturnStatementHandler(paStatement statement, bool preDecent);
void paEnumStatementHandler(paStatement statement, bool preDecent);
void paExprStatementHandler(paStatement statement, bool preDecent);

// Function methods.
paFunction paFunctionCreate(paNamespace namespace, utSym name, paType returnType,
    paType parameterType);

// Function pointer methods.
paFuncptr paFuncptrCreate(paModule module, utSym sym, paStatementHandler handler);

// Ident methods
paIdent paIdentCreate(paNamespace namespace, paIdentType type, utSym name);
paIdent paFindIdent(paExpr dotExpr);

// Namespace methods
paNamespace paNamespaceCreate(paIdent outerIdent);
void paCreateTopNamespace(void);

// Expresion methods.
paExpr paExprCreate(paExprType type);
paExpr paUnaryExprCreate(paExprType type, paExpr a);
paExpr paBinaryExprCreate(paExprType type, paExpr a, paExpr b);
paExpr paTrinaryExprCreate(paExprType type, paExpr a,
    paExpr b, paExpr c);
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

// Shortcuts and utilities
char *paCapitalize(char *string);
static inline char *paEnumGetName(paEnum Enum)
    {return paIdentGetName(paEnumGetIdent(Enum));}
static inline char *paEntryGetName(paEntry entry)
    {return paIdentGetName(paEntryGetIdent(entry));}
static inline char *paFunctionGetName(paFunction function)
    {return paIdentGetName(paFunctionGetIdent(function));}
static inline char *paTypedefGetName(paTypedef Typedef)
    {return paIdentGetName(paTypedefGetIdent(Typedef));}
static inline char *paNamespaceGetName(paNamespace namespace)
    {return paIdentGetName(paNamespaceGetIdent(namespace));}
static inline char *paFuncptrGetName(paFuncptr funcptr)
    {return paIdentGetName(paFuncptrGetIdent(funcptr));}
static inline paNamespace paFunctionGetNamespace(paFunction function)
    {return paIdentGetNamespace(paFunctionGetIdent(function));}

// Globals
extern paRoot paTheRoot;
extern paNamespace paTopNamespace;
extern paModule paTopModule;
extern paStatement paTopStatement;
extern paSyntax paL42Syntax;
extern paModule paBuiltinModule;
extern bool paQuit; // Set true to force interpreter to end

// Lex, Yacc stuff */
void paLexerStart(void);
void paLexerStop(void);
paModule paParse(paModule outerModule, utSym ModuleName);
paToken paLex(void);
void paPrintToken(paToken token);
void paPrintNodeExpr(paNodeExpr nodeExpr);
void paError(paToken token, char *message, ...);
void paExprError(paExpr expr, char *message, ...);

extern paSyntax paCurrentSyntax;
extern FILE *paFile;
extern uint32 paFileSize, paLineNum;

// Global symbols
extern utSym paIdentSym, paIntegerSym, paFloatSym, paStringSym, paBoolSym, paCharSym,
    paExprSym;
