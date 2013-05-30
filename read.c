#include "pa.h"

FILE *paFile;
// Must be set before parsing so that the parser knows where to add stuff.
uint32 paFileSize, paLineNum;
paSyntax paL42Syntax, paCurrentSyntax;
utSym paIdentSym, paIntegerSym, paFloatSym, paStringSym, paBoolSym, paCharSym, paExprSym;

static paPrecedenceGroup paCurrentPrecedenceGroup;

// Print out an error message and exit.
void paNodeExprError(
    paNodeExpr nodeExpr,
    char *message,
    ...)
{
    char *buff;
    va_list ap;

    va_start(ap, message);
    buff = utVsprintf((char *)message, ap);
    va_end(ap);
    utError("Line %d: %s", paNodeExprGetLineNum(nodeExpr), buff);
}

// Print a node expr.
void paPrintNodeExpr(
    paNodeExpr nodeExpr)
{
    paNodeExpr subNodeExpr;
    bool firstTime;

    switch(paNodeExprGetType(nodeExpr)) {
    case PA_NODEEXPR_IDENT: printf("ident"); break;
    case PA_NODEEXPR_EXPR:printf("expr"); break;
    case PA_NODEEXPR_INTEGER: printf("integer"); break;
    case PA_NODEEXPR_FLOAT: printf("float"); break;
    case PA_NODEEXPR_STRING: printf("string"); break;
    case PA_NODEEXPR_CHAR:printf("char"); break;
    case PA_NODEEXPR_CONSTIDENT:
        printf("\"%s\"", utSymGetName(paNodeExprGetSym(nodeExpr)));
        break;
    case PA_NODEEXPR_NODERULE:
        printf("%s", utSymGetName(paNodeExprGetSym(nodeExpr)));
        break;
    case PA_NODEEXPR_OPERATOR:
        printf("%s(", utSymGetName(paNodeExprGetSym(nodeExpr)));
        firstTime = true;
        paForeachNodeExprNodeExpr(nodeExpr, subNodeExpr) {
            if(!firstTime) {
                printf(" ");
            }
            paPrintNodeExpr(subNodeExpr);
            firstTime = false;
        } paEndNodeExprNodeExpr;
        printf(")");
        break;
    case PA_NODEEXPR_LISTOPERATOR:
        printf("%s[", utSymGetName(paNodeExprGetSym(nodeExpr)));
        paPrintNodeExpr(paNodeExprGetFirstNodeExpr(nodeExpr));
        printf("]");
        break;
    default:
        utExit("Unknown node expr type");
    }
}

// Print an element.
void paPrintElement(
    paElement element)
{
    if(paElementIsKeyword(element)) {
        printf("\"%s\"", utSymGetName(paElementGetSym(element)));
    } else {
        paPrintNodeExpr(paElementGetNodeExpr(element));
    }
}

// Print a pattern.
void paPrintPattern(
    paPattern pattern)
{
    paElement element;
    bool isFirst = true;

    paForeachPatternElement(pattern, element) {
        if(!isFirst) {
            printf(" ");
        }
        paPrintElement(element);
        isFirst = false;
    } paEndPatternElement;
    printf("\n");
}

// Print the staterule.
void paPrintStaterule(
    paStaterule staterule)
{
    utSym subSyntaxSym = paStateruleGetSubSyntaxSym(staterule);
    paFuncptr handlerFuncptr = paStateruleGetHandlerFuncptr(staterule);
    utSym sym;

    // TODO: print full use expression
    if(paStateruleHasBlock(staterule)) {
        printf("blockstatement ");
    } else {
        printf("statement ");
    }
    printf("%s", utSymGetName(paStateruleGetSym(staterule)));
    if(paStateruleGetNumBeforeSym(staterule) > 0) {
        printf(" before");
        paForeachStateruleBeforeSym(staterule, sym) {
            printf(" %s", utSymGetName(sym));
        } paEndStateruleBeforeSym;
    }
    if(paStateruleGetNumAfterSym(staterule) > 0) {
        printf(" after");
        paForeachStateruleAfterSym(staterule, sym) {
            printf(" %s", utSymGetName(sym));
        } paEndStateruleAfterSym;
    }
    if(handlerFuncptr != paFuncptrNull) {
        printf(" handler %s", paFuncptrGetName(handlerFuncptr));
    }
    if(subSyntaxSym != utSymNull) {
        printf(" uses %s", utSymGetName(subSyntaxSym));
    } 
    printf(": ");
    paPrintPattern(paStateruleGetPattern(staterule));
}

// Print a node rule.
void paPrintNoderule(
    paNoderule noderule)
{
    paNodeExpr nodeExpr;
    bool firstTime = true;

    printf("%s: ", paNoderuleGetName(noderule));
    paForeachNoderuleNodeExpr(noderule, nodeExpr) {
        if(!firstTime) {
            printf(" | ");
        }
        paPrintNodeExpr(nodeExpr);
        firstTime = false;
    } paEndNoderuleNodeExpr;
    printf("\n");
}

// Print the operator.
void paPrintOperator(
    paOperator operator)
{
    switch(paOperatorGetType(operator)) {
    case PA_OP_LEFT: printf("left"); break;
    case PA_OP_RIGHT: printf("right"); break;
    case PA_OP_NONASSOCIATIVE: printf("nonassociative"); break;
    case PA_OP_MERGE: printf("merge"); break;
    default:
        utExit("Unknown operator type");
    }
    printf(" %s: ", paOperatorGetName(operator));
    paPrintPattern(paOperatorGetPattern(operator));
}

// Return the number of operators in the precedence group.
static uint32 countPrecedenceGroupOperators(
    paPrecedenceGroup precedenceGroup)
{
    paOperator operator;
    uint32 numOperators = 0;

    paForeachPrecedenceGroupOperator(precedenceGroup, operator) {
        numOperators++;
    } paEndPrecedenceGroupOperator;
    return numOperators;
}

// Print a precedence group.
void paPrintPrecedenceGroup(
    paPrecedenceGroup precedenceGroup)
{
    paOperator operator;

    if(countPrecedenceGroupOperators(precedenceGroup) == 1) {
        paPrintOperator(paPrecedenceGroupGetFirstOperator(precedenceGroup));
    } else {
        printf("group\n");
        paForeachPrecedenceGroupOperator(precedenceGroup, operator) {
            printf("\t\t");
            paPrintOperator(operator);
        } paEndPrecedenceGroupOperator;
    }
}

// Print out the syntax.
void paPrintSyntax(
    paSyntax syntax)
{
    paStaterule staterule;
    paNoderule noderule;
    paPrecedenceGroup precedenceGroup;

    printf("syntax %s\n", paSyntaxGetName(syntax));
    paForeachSyntaxStaterule(syntax, staterule) {
        printf("\t");
        paPrintStaterule(staterule);
    } paEndSyntaxStaterule;
    paForeachSyntaxNoderule(syntax, noderule) {
        printf("\t");
        paPrintNoderule(noderule);
    } paEndSyntaxNoderule;
    paForeachSyntaxPrecedenceGroup(syntax, precedenceGroup) {
        printf("\t");
        paPrintPrecedenceGroup(precedenceGroup);
    } paEndSyntaxPrecedenceGroup;
}

// Create a new Syntax object.
paSyntax paSyntaxCreate(
    utSym sym)
{
    paSyntax syntax = paRootFindSyntax(paTheRoot, sym);

    if(syntax == paSyntaxNull) {
        syntax = paSyntaxAlloc();
        paSyntaxSetSym(syntax, sym);
        paRootAppendSyntax(paTheRoot, syntax);
    }
    return syntax;
}

// Create a Noderule object for matching node exprs.
static paNoderule noderuleCreate(
    paSyntax syntax,
    char *name,
    ...)
{
    paNoderule noderule = paNoderuleAlloc();
    va_list ap;
    paNodeExpr arg;

    paNoderuleSetSym(noderule, utSymCreate(name));
    paSyntaxAppendNoderule(syntax, noderule);
    va_start(ap, name); 
    arg = va_arg(ap, paNodeExpr);
    while(arg != paNodeExprNull) {
        paNoderuleAppendNodeExpr(noderule, arg);
        arg = va_arg(ap, paNodeExpr);
    }
    va_end(ap);
    return noderule;
}

// Create an operator node expr, with a list of sub-node exprs.
static paNodeExpr opNodeExprCreate(
    char *operatorName,
    ...)
{
    paNodeExpr nodeExpr = paNodeExprAlloc();
    va_list ap;
    paNodeExpr arg;

    paNodeExprSetType(nodeExpr, PA_NODEEXPR_OPERATOR);
    paNodeExprSetSym(nodeExpr, utSymCreate(operatorName));
    va_start(ap, operatorName); 
    arg = va_arg(ap, paNodeExpr);
    while(arg != paNodeExprNull) {
        paNodeExprAppendNodeExpr(nodeExpr, arg);
        arg = va_arg(ap, paNodeExpr);
    }
    va_end(ap);
    return nodeExpr;
}

// Create an identifier token node expr.
static paNodeExpr paTokenNodeExprCreate(
    paNodeExprType type)
{
    paNodeExpr nodeExpr = paNodeExprAlloc();

    paNodeExprSetType(nodeExpr, type);
    return nodeExpr;
}

// Create a sub-node rule node expr.
static paNodeExpr paSubruleNodeExprCreate(
    char *ruleName)
{
    paNodeExpr nodeExpr = paNodeExprAlloc();

    paNodeExprSetType(nodeExpr, PA_NODEEXPR_NODERULE);
    paNodeExprSetSym(nodeExpr, utSymCreate(ruleName));
    return nodeExpr;
}

// Create a list operator node expr.
static paNodeExpr listNodeExprCreate(
    char *operatorName,
    paNodeExpr subNodeExpr)
{
    paNodeExpr nodeExpr = paNodeExprAlloc();

    paNodeExprSetType(nodeExpr, PA_NODEEXPR_LISTOPERATOR);
    paNodeExprSetSym(nodeExpr, utSymCreate(operatorName));
    paNodeExprAppendNodeExpr(nodeExpr, subNodeExpr);
    return nodeExpr;
}

// Create a new element.
static paElement elementCreate(
    paSyntax syntax,
    paPattern pattern,
    char *text)
{
    paElement element = paElementAlloc();
    paKeyword keyword;
    paNodeExpr nodeExpr = paNodeExprNull;
    utSym sym = utSymCreate(text);

    paElementSetSym(element, sym);
    if(!strcmp(text, "expr")) {
        nodeExpr = paTokenNodeExprCreate(PA_NODEEXPR_EXPR);
    } else if(!strcmp(text + strlen(text) - 4, "Expr")) {
        nodeExpr = paSubruleNodeExprCreate(text);
    } else if(!strcmp(text, "ident")) {
        nodeExpr = paTokenNodeExprCreate(PA_NODEEXPR_IDENT);
    } else {
        paElementSetIsKeyword(element, true);
        keyword = paSyntaxFindKeyword(syntax, sym);
        if(keyword == paKeywordNull) {
            keyword = paKeywordAlloc();
            paKeywordSetSym(keyword, sym);
            paSyntaxAppendKeyword(syntax, keyword);
        }
        paKeywordAppendElement(keyword, element);
    }
    paElementSetNodeExpr(element, nodeExpr);
    paPatternAppendElement(pattern, element);
    return element;
}

// Create a new statement parsing rule.
static paStaterule stateruleCreate(
    paSyntax syntax,
    paStatementHandler handler,
    char *name,
    bool hasBlock,
    ...)
{
    paStaterule staterule = paStateruleAlloc();
    paPattern pattern = paPatternAlloc();
    paElement element;
    paFuncptr handlerFuncptr = paFuncptrNull;
    va_list ap;
    char *arg;

    paStateruleInsertPattern(staterule, pattern);
    paStateruleSetSym(staterule, utSymCreate(name));
    if(handler != NULL) {
        handlerFuncptr = paFuncptrCreate(utSymCreateFormatted("%sHandler", name), handler);
        paStateruleSetHandlerFuncptr(staterule, handlerFuncptr);
    }
    paStateruleSetHasBlock(staterule, hasBlock);
    va_start(ap, hasBlock); 
    arg = va_arg(ap, char *);
    while(arg != NULL) {
        element = elementCreate(syntax, pattern, arg);
        if(paElementIsKeyword(element)) {
            paStateruleAppendSignature(staterule, paElementGetKeyword(element));
        } else {
            // Apend NULL keywords where exprs go.
            paStateruleAppendSignature(staterule, paKeywordNull);
        }
        arg = va_arg(ap, char *);
    }
    va_end(ap);
    paSyntaxAppendStaterule(syntax, staterule);
    return staterule;
}

// Create a new precedence group.
paPrecedenceGroup paPrecedenceGroupCreate(
    paSyntax syntax,
    paOperator operator)
{
    paPrecedenceGroup precedenceGroup = paPrecedenceGroupAlloc();
    paPrecedenceGroup prevGroup = paPrecedenceGroupNull, nextGroup;

    if(operator != paOperatorNull) {
        nextGroup = paOperatorGetPrecedenceGroup(operator);
        prevGroup = paPrecedenceGroupGetPrevSyntaxPrecedenceGroup(nextGroup);
    }
    if(prevGroup != paPrecedenceGroupNull) {
        paSyntaxInsertAfterPrecedenceGroup(syntax, prevGroup, precedenceGroup);
    } else {
        paSyntaxAppendPrecedenceGroup(syntax, precedenceGroup);
    }
    return precedenceGroup;
}

// Create a new operator.
static paOperator operatorCreate(
    paSyntax syntax,
    paOperatorType type,
    char *name,
    ...)
{
    paPrecedenceGroup precedenceGroup = paPrecedenceGroupCreate(syntax, paOperatorNull);
    paOperator operator = paOperatorAlloc();
    paPattern pattern = paPatternAlloc();
    paElement element;
    va_list ap;
    char *arg;

    paOperatorInsertPattern(operator, pattern);
    paOperatorSetType(operator, type);
    paOperatorSetSym(operator, utSymCreate(name));
    paSyntaxAppendOperator(syntax, operator);
    paPrecedenceGroupAppendOperator(precedenceGroup, operator);
    va_start(ap, name); 
    arg = va_arg(ap, char *);
    while(arg != NULL) {
        element = elementCreate(syntax, pattern, arg);
        arg = va_arg(ap, char *);
    }
    va_end(ap);
    return operator;
}

// Also check that all non-keywords are the identifier "expr".
static void checkOperatorSyntax(
    paOperator operator)
{
    paElement element;
    utSym exprSym = utSymCreate("expr");

    paForeachPatternElement(paOperatorGetPattern(operator), element) {
        if(paElementGetKeyword(element) == paKeywordNull &&
                paElementGetSym(element) != exprSym) {
            utError("Invalid element '%s' in operator %s.  Only strings and expr are allowed.",
                utSymGetName(paElementGetSym(element)), paOperatorGetName(operator));
        }
    } paEndPatternElement;
}

// Determine if the operator has two exprs side by side, with no keyword
// between.  Also check that all non-keywords are the identifier "expr".
static bool operatorConcatenatesExprs(
    paOperator operator)
{
    paElement element;
    bool lastWasExpr = false;
    bool concatenatesExprs = false;
    bool hasKeywords = false;

    paForeachPatternElement(paOperatorGetPattern(operator), element) {
        if(paElementGetKeyword(element) == paKeywordNull) {
            if(lastWasExpr) {
                concatenatesExprs = true;
            }
            lastWasExpr = true;
        } else {
            hasKeywords = true;
        }
    } paEndPatternElement;
    return !hasKeywords && concatenatesExprs;
}

// Determine the expression type and expected symbol of expressions matching the node
// expression.
static paExprType findNodeExprTypeAndSym(
    paNodeExpr nodeExpr,
    utSym *symPtr)
{
    paExprType type;
    utSym sym = utSymNull;

    switch(paNodeExprGetType(nodeExpr)) {
    case PA_NODEEXPR_IDENT:
        type = PA_EXPR_IDENT;
        sym = paIdentSym;
        break;
    case PA_NODEEXPR_EXPR:
        type = PA_EXPR_IDENT;
        sym = paExprSym;
        break;
    case PA_NODEEXPR_INTEGER:
        type = PA_EXPR_VALUE;
        sym = paIntegerSym;
        break;
    case PA_NODEEXPR_FLOAT:
        type = PA_EXPR_VALUE;
        sym = paFloatSym;
        break;
    case PA_NODEEXPR_STRING:
        type = PA_EXPR_VALUE;
        sym = paStringSym;
        break;
    case PA_NODEEXPR_CHAR:
        type = PA_EXPR_VALUE;
        sym = paCharSym;
        break;
    case PA_NODEEXPR_CONSTIDENT:
        type = PA_EXPR_IDENT;
        sym = paNodeExprGetSym(nodeExpr);
        break;
    case PA_NODEEXPR_OPERATOR:
    case PA_NODEEXPR_LISTOPERATOR:
        type = PA_EXPR_OPERATOR;
        sym = paNodeExprGetSym(nodeExpr);
        break;
    default:
        utExit("Unknown node expr type");
    }
    *symPtr = sym;
    return type;
}

// Create a new nodelist object.
static paNodelist paNodelistCreate(
    paNoderule noderule,
    paExprType type,
    utSym sym)
{
    paNodelist nodelist = paNodelistAlloc();

    paNodelistSetType(nodelist, type);
    paNodelistSetSym(nodelist, sym);
    paNoderuleInsertNodelist(noderule, nodelist);
    return nodelist;
}

// Copy node expression from the source nodelist to the dest.
static void copyNodeExprsToNodelist(
    paNodelist source,
    paNodelist dest)
{
    paNodeExpr nodeExpr;

    paForeachNodelistNodeExpr(source, nodeExpr) {
        paNodelistAppendNodeExpr(dest, nodeExpr);
    } paEndNodelistNodeExpr;
}

// Add the sub-noderule's nodelists to this noderule's nodelists.
static void addNoderuleNodlistsToNodelists(
    paNoderule noderule,
    paNoderule subNoderule)
{
    paNodelist nodelist, subNodelist;
    paExprType type;
    utSym sym;

    paForeachNoderuleNodelist(subNoderule, subNodelist) {
        type = paNodelistGetType(subNodelist);
        sym = paNodelistGetSym(subNodelist);
        nodelist = paNoderuleFindNodelist(noderule, type, sym);
        if(nodelist == paNodelistNull) {
            nodelist = paNodelistCreate(noderule, type, sym);
        }
        copyNodeExprsToNodelist(subNodelist, nodelist);
    } paEndNoderuleNodelist;
}

// Build a hash table of node expressions that can match an expression based on
// expression type and symbol.  These are used to accelerate matching noderules to
// expressions.
static void buildNoderuleNodelists(
    paNoderule noderule)
{
    paSyntax syntax = paNoderuleGetSyntax(noderule);
    paNoderule subNoderule;
    paNodeExpr nodeExpr;
    paExprType type;
    paNodelist nodelist;
    utSym sym;

    paForeachNoderuleNodeExpr(noderule, nodeExpr) {
        if(paNodeExprGetType(nodeExpr) == PA_NODEEXPR_NODERULE) {
            subNoderule = paSyntaxFindNoderule(syntax, paNodeExprGetSym(nodeExpr));
            if(!paNoderuleBuiltNodelist(subNoderule)) {
                buildNoderuleNodelists(subNoderule);
            }
            addNoderuleNodlistsToNodelists(noderule, subNoderule);
        } else {
            type = findNodeExprTypeAndSym(nodeExpr, &sym);
            nodelist = paNoderuleFindNodelist(noderule, type, sym);
            if(nodelist == paNodelistNull) {
                nodelist = paNodelistCreate(noderule, type, sym);
            }
            paNodelistAppendNodeExpr(nodelist, nodeExpr);
        }
    } paEndNoderuleNodeExpr;
}

// Build a hash table of node expressions that can match an expression based on
// expression type and symbol.  These are used to accelerate matching noderules to
// expressions.
static void syntaxBuildNodelists(
    paSyntax syntax)
{
    paNoderule noderule;

    paForeachSyntaxNoderule(syntax, noderule) {
        if(!paNoderuleBuiltNodelist(noderule)) {
            buildNoderuleNodelists(noderule);
        }
    } paEndSyntaxNoderule;
}

// Set the precedence value on all the operators in the syntax.  This should be
// called whenever a new precedence group is added to the syntax.  Also set the
// concatenation operator, and check operator syntax.  Build a hash table of
// node expressions that can match an expression based on expression type and
// symbol.
void paSetOperatorPrecedence(
    paSyntax syntax)
{
    paPrecedenceGroup group;
    paOperator operator;
    paOperator concatenationOperator = paOperatorNull;
    uint32 precedence = 0;

    for(group = paSyntaxGetLastPrecedenceGroup(syntax); group != paPrecedenceGroupNull;
            group = paPrecedenceGroupGetPrevSyntaxPrecedenceGroup(group)) {
        paPrecedenceGroupSetPrecedence(group, precedence);
        precedence++;
    }
    paForeachSyntaxOperator(syntax, operator) {
        checkOperatorSyntax(operator);
        if(operatorConcatenatesExprs(operator)) {
            if(concatenationOperator != paOperatorNull) {
                utError("Operators %s and %s both concatenate exprs",
                    paOperatorGetName(concatenationOperator), paOperatorGetName(operator));
            }
            concatenationOperator = operator;
        }
    } paEndSyntaxOperator;
    paSyntaxSetConcatenationOperator(syntax, concatenationOperator);
    syntaxBuildNodelists(syntax);
}

// Determine if the expression is an operator expression with the operator name.
static inline bool exprMatches(
    paExpr expr,
    char *name)
{
    return paExprGetType(expr) == PA_EXPR_OPERATOR &&
        !strcmp(paOperatorGetName(paExprGetOperator(expr)), name);
}

// Get the name of the statment.
static inline char *getStatementName(
    paStatement statement)
{
    return utSymGetName(paStateruleGetSym(paStatementGetStaterule(statement)));
}

// Extract before expression symbols.
static void processBeforeExpr(
    paStaterule staterule,
    paExpr beforeExpr)
{
    paExpr identExpr;
    utSym sym;

    //identListExpr: ident | sequence[ident]
    if(paExprGetType(beforeExpr) == PA_EXPR_IDENT) {
        sym = paExprGetSym(beforeExpr);
        paStateruleAppendBeforeSym(staterule, sym);
    } else {
        paForeachExprExpr(beforeExpr, identExpr) {
            sym = paExprGetSym(identExpr);
            paStateruleAppendBeforeSym(staterule, sym);
        } paEndExprExpr;
    }
}

// Extract after expression symbols.
static void processAfterExpr(
    paStaterule staterule,
    paExpr afterExpr)
{
    paExpr identExpr;
    utSym sym;

    //identListExpr: ident | sequence[ident]
    if(paExprGetType(afterExpr) == PA_EXPR_IDENT) {
        sym = paExprGetSym(afterExpr);
        paStateruleAppendAfterSym(staterule, sym);
    } else {
        paForeachExprExpr(afterExpr, identExpr) {
            sym = paExprGetSym(identExpr);
            paStateruleAppendAfterSym(staterule, sym);
        } paEndExprExpr;
    }
}

// Create an element from the element expression.
paElement paElementCreate(
    paSyntax syntax,
    paPattern pattern,
    paExpr elemExpr)
{
    //elementExpr: STRING | ident
    paElement element = paElementAlloc();
    paExprType type = paExprGetType(elemExpr);
    paNodeExpr nodeExpr;
    paKeyword keyword;
    vaValue value;
    utSym sym;
    char *text;

    if(type == PA_EXPR_VALUE) {
        value = paExprGetValue(elemExpr);
        if(vaValueGetType(value) != VA_STRING) {
            utExit("Invalid element expression");
        }
        sym = utSymCreate((char *)vaStringGetValue(vaValueGetStringVal(value)));
        paElementSetIsKeyword(element, true);
        paElementSetSym(element, sym);
        keyword = paSyntaxFindKeyword(syntax, sym);
        if(keyword == paKeywordNull) {
            keyword = paKeywordAlloc();
            paKeywordSetSym(keyword, sym);
            paSyntaxAppendKeyword(syntax, keyword);
        }
        paKeywordAppendElement(keyword, element);
    } else if (type == PA_EXPR_IDENT) {
        text = utSymGetName(paExprGetSym(elemExpr));
        if(!strcmp(text, "expr")) {
            nodeExpr = paTokenNodeExprCreate(PA_NODEEXPR_EXPR);
        } else if(!strcmp(text, "ident")) {
            nodeExpr = paTokenNodeExprCreate(PA_NODEEXPR_IDENT);
        } else {
            nodeExpr = paSubruleNodeExprCreate(text);
        }
        paNodeExprSetLineNum(nodeExpr, paExprGetLineNum(elemExpr));
        paElementSetNodeExpr(element, nodeExpr);
        paElementSetSym(element, paExprGetSym(elemExpr));
    } else {
        utExit("Bad element expression");
    }
    paPatternAppendElement(pattern, element);
    return element;
}

// Create pattern object from the pattern expression.
paPattern paPatternCreate(
    paSyntax syntax,
    paExpr patternExpr)
{
    paPattern pattern = paPatternAlloc();
    paExpr elemExpr;

    //patternExpr: elementExpr | sequence[elementExpr]
    if(!exprMatches(patternExpr, "sequence")) {
        paElementCreate(syntax, pattern, patternExpr);
    } else {
        paForeachExprExpr(patternExpr, elemExpr) {
            paElementCreate(syntax, pattern, elemExpr);
        } paEndExprExpr;
    }
    return pattern;
}

// Create a staterule.
paStaterule paStateruleCreate(
    paSyntax syntax,
    paFuncptr handlerFuncptr,
    utSym name,
    bool hasBlock,
    paExpr patternExpr)
{
    paStaterule staterule = paStateruleAlloc();
    paPattern pattern = paPatternCreate(syntax, patternExpr);
    paElement element;

    paStateruleInsertPattern(staterule, pattern);
    paStateruleSetSym(staterule, name);
    paStateruleSetHandlerFuncptr(staterule, handlerFuncptr);
    paStateruleSetHasBlock(staterule, hasBlock);
    paForeachPatternElement(pattern, element) {
        if(paElementIsKeyword(element)) {
            paStateruleAppendSignature(staterule, paElementGetKeyword(element));
        } else {
            // Apend NULL keywords where exprs go.
            paStateruleAppendSignature(staterule, paKeywordNull);
        }
    } paEndPatternElement;
    paSyntaxAppendStaterule(syntax, staterule);
    return staterule;
}

// Check a handler expression, and return the staterule id and it's handler sym.
static void processHandlerExpr(
    paStatement statement,
    paExpr handlerExpr,
    utSym *identSym,
    paExpr *handlerDotExpr,
    paExpr *beforeExpr,
    paExpr *afterExpr)
{
    paExpr statementExpr = handlerExpr;
    paExpr identExpr;

    // handlerExpr: handler(statementExpr dotExpr) | statementExpr
    if(exprMatches(handlerExpr, "handler")) {
        statementExpr = paExprGetFirstExpr(handlerExpr);
        *handlerDotExpr = paExprGetNextExprExpr(statementExpr);
    }
	// statementExpr: ident | before(ident identListExpr) | after(ident identListExpr)
    *beforeExpr = paExprNull;
    *afterExpr = paExprNull;
    if(exprMatches(statementExpr, "before")) {
        identExpr = paExprGetFirstExpr(statementExpr);
        *beforeExpr = paExprGetNextExprExpr(identExpr);
    } else if(exprMatches(statementExpr, "after")) {
        identExpr = paExprGetFirstExpr(statementExpr);
        *afterExpr = paExprGetNextExprExpr(identExpr);
    } else {
        identExpr = statementExpr;
    }
    *identSym = paExprGetSym(identExpr);
}

// Handler for basicStatement: "statement" handlerExpr ":" patternExpr
static void basicStatementHandler(
    paStatement statement,
    bool preDecent)
{
    paStaterule staterule;
    paExpr handlerExpr = paStatementGetFirstExpr(statement);
    paExpr patternExpr = paExprGetNextStatementExpr(handlerExpr);
    paExpr beforeExpr, afterExpr, handlerDotExpr;
    paFuncptr handlerFuncptr = paFuncptrNull;
    utSym identSym;

    processHandlerExpr(statement, handlerExpr, &identSym, &handlerDotExpr,
        &beforeExpr, &afterExpr);
    if(handlerDotExpr != paExprNull) {
        identSym = paExprGetSym(handlerDotExpr);
        handlerFuncptr = paRootFindFuncptr(paTheRoot, identSym);
        if(handlerFuncptr == paFuncptrNull) {
            paExprError(handlerExpr, "Unable to find statement handler %s",
                utSymGetName(identSym));
        }
    }
    staterule = paStateruleCreate(paCurrentSyntax, handlerFuncptr, identSym, false,
        patternExpr);
    if(beforeExpr != paExprNull) {
        processBeforeExpr(staterule, beforeExpr);
    }
    if(afterExpr != paExprNull) {
        processAfterExpr(staterule, afterExpr);
    }
}

// Handler for blockStatement: "blockstatement" usesExpr ":" patternExpr
static void blockStatementHandler(
    paStatement statement,
    bool preDecent)
{
    paStaterule staterule;
    paExpr usesExpr = paStatementGetFirstExpr(statement);
    paExpr patternExpr = paExprGetNextStatementExpr(usesExpr);
    paExpr handlerExpr = usesExpr;
    paExpr syntaxExpr, beforeExpr, afterExpr, handlerDotExpr;
    paFuncptr handlerFuncptr = paFuncptrNull;
    utSym syntaxSym = utSymNull, identSym, handlerSym;

    // usesExpr: useSyntax(handlerExpr ident) | handlerExpr
    if(exprMatches(usesExpr, "useSyntax")) {
        handlerExpr = paExprGetFirstExpr(usesExpr);
        syntaxExpr = paExprGetNextExprExpr(handlerExpr);
        syntaxSym = paExprGetSym(syntaxExpr);
    }
    processHandlerExpr(statement, handlerExpr, &identSym, &handlerDotExpr,
        &beforeExpr, &afterExpr);
    if(handlerDotExpr != paExprNull) {
        handlerSym = paExprGetSym(handlerDotExpr);
        handlerFuncptr = paRootFindFuncptr(paTheRoot, handlerSym);
        if(handlerFuncptr == paFuncptrNull) {
            paExprError(handlerDotExpr, "Statement handler %s not found",
                utSymGetName(handlerSym));
        }
    }
    staterule = paStateruleCreate(paCurrentSyntax, handlerFuncptr, identSym, true,
        patternExpr);
    paStateruleSetSubSyntaxSym(staterule, syntaxSym);
    if(beforeExpr != paExprNull) {
        processBeforeExpr(staterule, beforeExpr);
    }
    if(afterExpr != paExprNull) {
        processAfterExpr(staterule, afterExpr);
    }
}

static paNodeExpr buildNodeExpr(paExpr expr);

// Create an operator node expression from a rule expression.
paNodeExpr paOperatorNodeExprCreate(
    paExpr opExpr)
{
    paNodeExpr opNodeExpr = paNodeExprAlloc();
    paExpr identExpr = paExprGetFirstExpr(opExpr);
    paExpr nodeListExpr = paExprGetNextExprExpr(identExpr);
    paExpr nodeExpr;

    // operator(ident nodeListExpr)
    paNodeExprSetLineNum(opNodeExpr, paExprGetLineNum(opExpr));
    paNodeExprSetType(opNodeExpr, PA_NODEEXPR_OPERATOR);
    paNodeExprSetSym(opNodeExpr, paExprGetSym(identExpr));
    // nodeListExpr: nodeExpr | sequence(nodeExpr)
    if(exprMatches(nodeListExpr, "sequence")) {
        paForeachExprExpr(nodeListExpr, nodeExpr) {
            paNodeExprAppendNodeExpr(opNodeExpr, buildNodeExpr(nodeExpr));
        } paEndExprExpr;
    } else {
        paNodeExprAppendNodeExpr(opNodeExpr, buildNodeExpr(nodeListExpr));
    }
    return opNodeExpr;
}

// Create a list node expression from a rule expression.
paNodeExpr paListNodeExprCreate(
    paExpr opExpr)
{
    paNodeExpr listNodeExpr = paNodeExprAlloc();
    paExpr identExpr = paExprGetFirstExpr(opExpr);
    paExpr nodeExpr = paExprGetNextExprExpr(identExpr);

    // list(ident nodeExpr)
    paNodeExprSetLineNum(listNodeExpr, paExprGetLineNum(opExpr));
    paNodeExprSetType(listNodeExpr, PA_NODEEXPR_LISTOPERATOR);
    paNodeExprSetSym(listNodeExpr, paExprGetSym(identExpr));
    paNodeExprAppendNodeExpr(listNodeExpr, buildNodeExpr(nodeExpr));
    return listNodeExpr;
}

// Build a node expression from the expression.
static paNodeExpr buildNodeExpr(
    paExpr expr)
{
    paNodeExpr nodeExpr;
    vaValue value;
    char *text;

    // nodeExpr: ident | operator(ident nodeListExpr) | list(ident nodeExpr)
    if(paExprGetType(expr) == PA_EXPR_VALUE) {
        value = paExprGetValue(expr);
        if(vaValueGetType(value) != VA_STRING) {
            utExit("Invalid element expression");
        }
        nodeExpr = paTokenNodeExprCreate(PA_NODEEXPR_CONSTIDENT);
        paNodeExprSetSym(nodeExpr,
            utSymCreate((char *)vaStringGetValue(vaValueGetStringVal(value))));
    } else if(paExprGetType(expr) == PA_EXPR_IDENT) {
        text = utSymGetName(paExprGetSym(expr));
        if(!strcmp(text, "expr")) {
            paExprError(expr, "Invalid used of 'expr' in noderule");
        }
        if(!strcmp(text, "ident")) {
            nodeExpr = paTokenNodeExprCreate(PA_NODEEXPR_IDENT);
        } else if(!strcmp(text, "string")) {
            nodeExpr = paTokenNodeExprCreate(PA_NODEEXPR_STRING);
        } else if(!strcmp(text, "float")) {
            nodeExpr = paTokenNodeExprCreate(PA_NODEEXPR_FLOAT);
        } else if(!strcmp(text, "integer")) {
            nodeExpr = paTokenNodeExprCreate(PA_NODEEXPR_INTEGER);
        } else if(!strcmp(text, "char")) {
            nodeExpr = paTokenNodeExprCreate(PA_NODEEXPR_CHAR);
        } else {
            return paSubruleNodeExprCreate(text);
        }
    } else if(exprMatches(expr, "operator")) {
        return paOperatorNodeExprCreate(expr);
    } else {
        utAssert(exprMatches(expr, "list"));
        return paListNodeExprCreate(expr);
    }
    paNodeExprSetLineNum(nodeExpr, paExprGetLineNum(expr));
    return nodeExpr;
}

// Create a Noderule object for matching node exprs.
paNoderule paNoderuleCreate(
    paSyntax syntax,
    utSym sym,
    paExpr ruleExpr)
{
    paNoderule noderule = paNoderuleAlloc();
    paNodeExpr nodeExpr;
    paExpr expr;

    paNoderuleSetSym(noderule, sym);
    paSyntaxAppendNoderule(syntax, noderule);
    if(exprMatches(ruleExpr, "or")) {
        paForeachExprExpr(ruleExpr, expr) {
            nodeExpr = buildNodeExpr(expr);
            paNoderuleAppendNodeExpr(noderule, nodeExpr);
        } paEndExprExpr;
    } else {
        nodeExpr = buildNodeExpr(ruleExpr);
        paNoderuleAppendNodeExpr(noderule, nodeExpr);
    }
    return noderule;
}

// Handler for ruleStatement: ident ":" ruleExpr
static void ruleStatementHandler(
    paStatement statement,
    bool preDecent)
{
    // statement ruleStatement: ident ":" ruleExpr
    paExpr identExpr = paStatementGetFirstExpr(statement);
    utSym sym = paExprGetSym(identExpr);
    paExpr ruleExpr = paExprGetNextStatementExpr(identExpr);

    paNoderuleCreate(paCurrentSyntax, sym, ruleExpr);
}

// Handler for groupStatement: "group"
static void groupStatementHandler(
    paStatement statement,
    bool preDecent)
{
    if(preDecent) {
        paCurrentPrecedenceGroup = paPrecedenceGroupCreate(paCurrentSyntax, paOperatorNull);
    } else {
        paCurrentPrecedenceGroup = paPrecedenceGroupNull;
    }
}

// Create a new operator object.
paOperator paOperatorCreate(
    paSyntax syntax, 
    paPrecedenceGroup precedenceGroup,
    paOperatorType type,
    utSym nameSym,
    paPattern pattern)
{
    paOperator operator = paOperatorAlloc();

    paOperatorSetType(operator, type);
    paOperatorSetSym(operator, nameSym);
    paOperatorInsertPattern(operator, pattern);
    paSyntaxAppendOperator(syntax, operator);
    paPrecedenceGroupAppendOperator(precedenceGroup, operator);
    return operator;
}

// Handler for operators.
static void operatorStatementHandler(
    paStatement statement,
    paOperatorType type)
{
    paPrecedenceGroup precedenceGroup = paCurrentPrecedenceGroup;
    paExpr identExpr = paStatementGetFirstExpr(statement);
    paExpr patternExpr = paExprGetNextStatementExpr(identExpr);
    paPattern pattern = paPatternCreate(paCurrentSyntax, patternExpr);
    utSym nameSym = paExprGetSym(identExpr);

    // statement leftStatement: "left" ident ":" patternExpr
    if(precedenceGroup == paPrecedenceGroupNull) {
        precedenceGroup = paPrecedenceGroupCreate(paCurrentSyntax, paOperatorNull);
    }
    paOperatorCreate(paCurrentSyntax, precedenceGroup, type, nameSym, pattern);
}

// Handler for leftStatement: "left" ident ":" patternExpr
static void leftStatementHandler(
    paStatement statement,
    bool preDecent)
{
    operatorStatementHandler(statement, PA_OP_LEFT);
}

// Handler for rightStatement: "right" ident ":" patternExpr
static void rightStatementHandler(
    paStatement statement,
    bool preDecent)
{
    operatorStatementHandler(statement, PA_OP_RIGHT);
}

// Handler for mergeStatement: "merge" ident ":" patternExpr
static void mergeStatementHandler(
    paStatement statement,
    bool preDecent)
{
    operatorStatementHandler(statement, PA_OP_MERGE);
}

// Handler for nonassociativeStatement: "nonassociative" ident ":" patternExpr
static void nonassociativeStatementHandler(
    paStatement statement,
    bool preDecent)
{
    operatorStatementHandler(statement, PA_OP_NONASSOCIATIVE);
}

// Check identifiers inthe node expression.
static void checkNodeExpr(
    paSyntax syntax,
    paNodeExpr nodeExpr)
{
    paNodeExpr subNodeExpr;
    paOperator operator;
    utSym sym = paNodeExprGetSym(nodeExpr);
    uint32 numSubExpr = 0;

    switch(paNodeExprGetType(nodeExpr)) {
    case PA_NODEEXPR_NODERULE:
        if(paSyntaxFindNoderule(syntax, sym) == paNoderuleNull) {
            utError("Noderule %s not defined", utSymGetName(sym));
        }
        break;
    case PA_NODEEXPR_OPERATOR: case PA_NODEEXPR_LISTOPERATOR:
        operator = paSyntaxFindOperator(syntax, sym);
        if(operator == paOperatorNull) {
            utError("Noderule %s not defined", utSymGetName(sym));
        }
        paForeachNodeExprNodeExpr(nodeExpr, subNodeExpr) {
            checkNodeExpr(syntax, subNodeExpr);
            numSubExpr++;
        } paEndNodeExprNodeExpr;
        if(paNodeExprGetType(nodeExpr) == PA_NODEEXPR_OPERATOR) {
            if(paOperatorGetType(operator) == PA_OP_MERGE) {
                paNodeExprError(nodeExpr, "Expected non-list node expresssion for operator %s",
                    paOperatorGetName(operator));
            }
        } else {
            if(paOperatorGetType(operator) != PA_OP_MERGE) {
                paNodeExprError(nodeExpr, "Expected list expression for operator %s",
                    paOperatorGetName(operator));
            }
            if(numSubExpr != 1) {
                paNodeExprError(nodeExpr, "List node expression has multiple sub-expressions");
            }
        }
        break;
    case PA_NODEEXPR_INTEGER: case PA_NODEEXPR_FLOAT: case PA_NODEEXPR_STRING:
    case PA_NODEEXPR_CHAR: case PA_NODEEXPR_EXPR: case PA_NODEEXPR_CONSTIDENT:
    case PA_NODEEXPR_IDENT:
        break;
    default:
        utExit("Unknown node expression type");
    }
}

// Check that identifiers in node expressions can be resolved.
static void checkNoderules(
    paSyntax syntax)
{
    paNoderule noderule;
    paNodeExpr nodeExpr;

    paForeachSyntaxNoderule(syntax, noderule) {
        paForeachNoderuleNodeExpr(noderule, nodeExpr) {
            checkNodeExpr(syntax, nodeExpr);
        } paEndNoderuleNodeExpr;
    } paEndSyntaxNoderule;
}

// Handle the syntax statement.
static void syntaxStatementHandler(
    paStatement statement,
    bool preDecent)
{
    static paSyntax prevSyntax;

    if(preDecent) {
        prevSyntax = paCurrentSyntax;
        paCurrentSyntax = paSyntaxCreate(paExprGetSym(paStatementGetFirstExpr(statement)));
    } else {
        paStatementDestroy(statement);
        checkNoderules(paCurrentSyntax);
        paSetOperatorPrecedence(paCurrentSyntax);
        paCurrentSyntax = prevSyntax;
    }
}

// Initialize global utSym values.
static void initGlobalSyms(void)
{
    paIdentSym = utSymCreate("ident");
    paIntegerSym = utSymCreate("integer");
    paFloatSym = utSymCreate("float");
    paStringSym = utSymCreate("string");
    paBoolSym = utSymCreate("bool");
    paExprSym = utSymCreate("expr");
    paCharSym = utSymCreate("char");
}

// Create objects representing built-in stuff, like statement patters and operators.
void paCreateBuiltins(void)
{
    paSyntax syntax = paSyntaxCreate(utSymCreate("syntaxStatement"));
    paStaterule staterule;

    initGlobalSyms();
    // statement basicStatement: "statement" handlerExpr ":" patternExpr
    stateruleCreate(syntax, basicStatementHandler, "basicStatement", false, "statement",
        "handlerExpr", ":", "patternExpr", NULL);
    // statement blockStatement: "blockstatement" usesExpr ":" patternExpr
    stateruleCreate(syntax, blockStatementHandler, "blockStatement", false,
        "blockstatement", "usesExpr", ":", "patternExpr", NULL);
    // statement ruleStatement: ident ":" ruleExpr
    stateruleCreate(syntax, ruleStatementHandler, "ruleStatement", false, "ident", ":",
        "ruleExpr", NULL);
    // blockstatement groupStatement: "group"
    stateruleCreate(syntax, groupStatementHandler, "groupStatement", true, "group", NULL);
    // statement leftStatement: "left" ident ":" patternExpr
    stateruleCreate(syntax, leftStatementHandler, "leftStatement", false, "left",
        "ident", ":", "patternExpr", NULL);
    // statement rightStatement: "right" ident ":" patternExpr
    stateruleCreate(syntax, rightStatementHandler, "rightStatement", false, "right",
        "ident", ":", "patternExpr", NULL);
    // statement mergeStatement: "merge" ident ":" patternExpr
    stateruleCreate(syntax, mergeStatementHandler, "mergeStatement", false, "merge",
        "ident", ":", "patternExpr", NULL);
    // statement nonassociativeStatement: "nonassociative" ident ":" patternExpr
    stateruleCreate(syntax, nonassociativeStatementHandler, "nonassociativeStatement",
        false, "nonassociative", "ident", ":", "patternExpr", NULL);

    // usesExpr: useSyntax(handlerExpr ident) | handlerExpr
    noderuleCreate(syntax, "usesExpr",
        opNodeExprCreate("useSyntax", paSubruleNodeExprCreate("handlerExpr"),
            paTokenNodeExprCreate(PA_NODEEXPR_IDENT), paNodeExprNull),
        paSubruleNodeExprCreate("handlerExpr"), paNodeExprNull);
    // handlerExpr: handler(statementExpr dotExpr) | statementExpr
    noderuleCreate(syntax, "handlerExpr",
        opNodeExprCreate("handler", paSubruleNodeExprCreate("statementExpr"),
            paSubruleNodeExprCreate("dotExpr"), paNodeExprNull),
        paSubruleNodeExprCreate("statementExpr"), paNodeExprNull);
    // dotExpr: ident | dot[ident]
    noderuleCreate(syntax, "dotExpr", paTokenNodeExprCreate(PA_NODEEXPR_IDENT),
        listNodeExprCreate("dot", paTokenNodeExprCreate(PA_NODEEXPR_IDENT)), paNodeExprNull);
    // statementExpr: IDENT | before(IDENT identListExpr) | after(IDENT identListExpr)
    noderuleCreate(syntax, "statementExpr", paTokenNodeExprCreate(PA_NODEEXPR_IDENT),
        opNodeExprCreate("before", paTokenNodeExprCreate(PA_NODEEXPR_IDENT),
            paSubruleNodeExprCreate("identListExpr"), paNodeExprNull),
        opNodeExprCreate("after", paTokenNodeExprCreate(PA_NODEEXPR_IDENT),
            paSubruleNodeExprCreate("identListExpr"), paNodeExprNull), paNodeExprNull);
    // identListExpr: IDENT | sequence[IDENT]
    noderuleCreate(syntax, "identListExpr", paTokenNodeExprCreate(PA_NODEEXPR_IDENT),
        listNodeExprCreate("sequence", paTokenNodeExprCreate(PA_NODEEXPR_IDENT)),
        paNodeExprNull);
    //ruleExpr: nodeExpr | or[nodeExpr]
    noderuleCreate(syntax, "ruleExpr", paSubruleNodeExprCreate("nodeExpr"),
        listNodeExprCreate("or", paSubruleNodeExprCreate("nodeExpr")), paNodeExprNull);
    //nodeExpr: ident | string | operator(ident nodeListExpr) | list(ident nodeExpr)
    noderuleCreate(syntax, "nodeExpr", paTokenNodeExprCreate(PA_NODEEXPR_IDENT),
        paTokenNodeExprCreate(PA_NODEEXPR_STRING),
        opNodeExprCreate("operator", paTokenNodeExprCreate(PA_NODEEXPR_IDENT),
            paSubruleNodeExprCreate("nodeListExpr"), paNodeExprNull),
        opNodeExprCreate("list", paTokenNodeExprCreate(PA_NODEEXPR_IDENT),
            paSubruleNodeExprCreate("nodeExpr"), paNodeExprNull), paNodeExprNull);
    //nodeListExpr: nodeExpr | sequence(nodeExpr)
    noderuleCreate(syntax, "nodeListExpr", paSubruleNodeExprCreate("nodeExpr"),
        listNodeExprCreate("sequence", paSubruleNodeExprCreate("nodeExpr")),
        paNodeExprNull);
    //patternExpr: elementExpr | sequence[elementExpr]
    noderuleCreate(syntax, "patternExpr", paSubruleNodeExprCreate("elementExpr"),
        listNodeExprCreate("sequence", paSubruleNodeExprCreate("elementExpr")),
        paNodeExprNull);
    //elementExpr: STRING | IDENT
    noderuleCreate(syntax, "elementExpr", paTokenNodeExprCreate(PA_NODEEXPR_STRING),
        paTokenNodeExprCreate(PA_NODEEXPR_IDENT), paNodeExprNull);

    //merge dot: expr '.' expr
    operatorCreate(syntax, PA_OP_LEFT, "dot", "expr", ".", "expr", NULL);
    //nonassociative node: expr "(" expr ")"
    operatorCreate(syntax, PA_OP_NONASSOCIATIVE, "operator", "expr", "(", "expr", ")", NULL);
    //nonassociative list: expr "[" expr "]"
    operatorCreate(syntax, PA_OP_NONASSOCIATIVE, "list", "expr", "[", "expr", "]", NULL);
    //merge sequence: expr expr # Operators without tokens are allowed
    operatorCreate(syntax, PA_OP_MERGE, "sequence", "expr", "expr", NULL);
    //nonassociative before: expr "before" expr
    operatorCreate(syntax, PA_OP_NONASSOCIATIVE, "before", "expr", "before", "expr", NULL);
    //nonassociative after: expr "after" expr
    operatorCreate(syntax, PA_OP_NONASSOCIATIVE, "after", "expr", "after", "expr", NULL);
    //merge or: expr "|" expr
    operatorCreate(syntax, PA_OP_MERGE, "or", "expr", "|", "expr", NULL);
    //nonassociative handler: expr "handler" expr
    operatorCreate(syntax, PA_OP_NONASSOCIATIVE, "handler", "expr", "handler", "expr", NULL);
    //nonassociative useSyntax: expr "uses" expr
    operatorCreate(syntax, PA_OP_NONASSOCIATIVE, "useSyntax", "expr", "uses", "expr", NULL);

    paL42Syntax = paSyntaxCreate(utSymCreate("l42"));
    staterule = stateruleCreate(paL42Syntax, syntaxStatementHandler, "syntaxStatement", true,
        "syntax", "ident", NULL);
    paStateruleSetSubSyntaxSym(staterule, paSyntaxGetSym(syntax));
    paSetOperatorPrecedence(syntax);

    paPrintSyntax(paL42Syntax);
    paPrintSyntax(syntax);
}

// Parse a source file or string.
static paStatement parseSource(void)
{
    paLineNum = 0;
    paCurrentPrecedenceGroup = paPrecedenceGroupNull;
    return paParse();
}

// Parse a command definition file.
paStatement paParseSourceFile(
    char *fileName)
{
    paStatement statement;

    paFile = fopen(fileName, "r");
    if(paFile == NULL) {
        fprintf(stderr, "Unable to open file %s\n", fileName);
        return paStatementNull;
    }
    statement = parseSource();
    fclose(paFile);
    paFile = NULL;
    return statement;
}
