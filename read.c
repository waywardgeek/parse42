#include "co.h"

FILE *coFile;
// Must be set before parsing so that the parser knows where to add stuff.
uint32 coFileSize, coLineNum;
coSyntax coCurrentSyntax;
utSym coIdentSym, coIntegerSym, coFloatSym, coStringSym, coBoolSym, coCharSym, coExprSym;
coModule coBuiltinModule;

static coPrecedenceGroup coCurrentPrecedenceGroup;

// Print out an error message and exit.
void coNodeExprError(
    coNodeExpr nodeExpr,
    char *message,
    ...)
{
    char *buff;
    va_list ap;

    va_start(ap, message);
    buff = utVsprintf((char *)message, ap);
    va_end(ap);
    utError("Line %d: %s", coNodeExprGetLineNum(nodeExpr), buff);
}

// Print a node expr.
void coPrintNodeExpr(
    coNodeExpr nodeExpr)
{
    coNodeExpr subNodeExpr;
    bool firstTime;

    switch(coNodeExprGetType(nodeExpr)) {
    case CO_NODEEXPR_IDENT: printf("ident"); break;
    case CO_NODEEXPR_EXPR:printf("expr"); break;
    case CO_NODEEXPR_INTEGER: printf("integer"); break;
    case CO_NODEEXPR_FLOAT: printf("float"); break;
    case CO_NODEEXPR_STRING: printf("string"); break;
    case CO_NODEEXPR_CHAR:printf("char"); break;
    case CO_NODEEXPR_CONSTIDENT:
        printf("\"%s\"", utSymGetName(coNodeExprGetSym(nodeExpr)));
        break;
    case CO_NODEEXPR_NODERULE:
        printf("%s", utSymGetName(coNodeExprGetSym(nodeExpr)));
        break;
    case CO_NODEEXPR_OPERATOR:
        printf("%s(", utSymGetName(coNodeExprGetSym(nodeExpr)));
        firstTime = true;
        coForeachNodeExprNodeExpr(nodeExpr, subNodeExpr) {
            if(!firstTime) {
                printf(" ");
            }
            coPrintNodeExpr(subNodeExpr);
            firstTime = false;
        } coEndNodeExprNodeExpr;
        printf(")");
        break;
    case CO_NODEEXPR_LISTOPERATOR:
        printf("%s[", utSymGetName(coNodeExprGetSym(nodeExpr)));
        coPrintNodeExpr(coNodeExprGetFirstNodeExpr(nodeExpr));
        printf("]");
        break;
    default:
        utExit("Unknown node expr type");
    }
}

// Print an element.
void coPrintElement(
    coElement element)
{
    if(coElementIsKeyword(element)) {
        printf("\"%s\"", utSymGetName(coElementGetSym(element)));
    } else {
        coPrintNodeExpr(coElementGetNodeExpr(element));
    }
}

// Print a pattern.
void coPrintPattern(
    coPattern pattern)
{
    coElement element;
    bool isFirst = true;

    coForeachPatternElement(pattern, element) {
        if(!isFirst) {
            printf(" ");
        }
        coPrintElement(element);
        isFirst = false;
    } coEndPatternElement;
    printf("\n");
}

// Print the staterule.
void coPrintStaterule(
    coStaterule staterule)
{
    utSym subSyntaxSym = coStateruleGetSubSyntaxSym(staterule);
    coFuncptr handlerFuncptr = coStateruleGetHandlerFuncptr(staterule);
    utSym sym;

    // TODO: print full use expression
    if(coStateruleHasBlock(staterule)) {
        printf("blockstatement ");
    } else {
        printf("statement ");
    }
    printf("%s", utSymGetName(coStateruleGetSym(staterule)));
    if(coStateruleGetNumBeforeSym(staterule) > 0) {
        printf(" before");
        coForeachStateruleBeforeSym(staterule, sym) {
            printf(" %s", utSymGetName(sym));
        } coEndStateruleBeforeSym;
    }
    if(coStateruleGetNumAfterSym(staterule) > 0) {
        printf(" after");
        coForeachStateruleAfterSym(staterule, sym) {
            printf(" %s", utSymGetName(sym));
        } coEndStateruleAfterSym;
    }
    if(handlerFuncptr != coFuncptrNull) {
        printf(" handler %s", coFuncptrGetName(handlerFuncptr));
    }
    if(subSyntaxSym != utSymNull) {
        printf(" uses %s", utSymGetName(subSyntaxSym));
    } 
    printf(": ");
    coPrintPattern(coStateruleGetPattern(staterule));
}

// Print a node rule.
void coPrintNoderule(
    coNoderule noderule)
{
    coNodeExpr nodeExpr;
    bool firstTime = true;

    printf("%s: ", coNoderuleGetName(noderule));
    coForeachNoderuleNodeExpr(noderule, nodeExpr) {
        if(!firstTime) {
            printf(" | ");
        }
        coPrintNodeExpr(nodeExpr);
        firstTime = false;
    } coEndNoderuleNodeExpr;
    printf("\n");
}

// Print the operator.
void coPrintOperator(
    coOperator operator)
{
    switch(coOperatorGetType(operator)) {
    case CO_OP_LEFT: printf("left"); break;
    case CO_OP_RIGHT: printf("right"); break;
    case CO_OP_NONASSOCIATIVE: printf("nonassociative"); break;
    case CO_OP_MERGE: printf("merge"); break;
    default:
        utExit("Unknown operator type");
    }
    printf(" %s: ", coOperatorGetName(operator));
    coPrintPattern(coOperatorGetPattern(operator));
}

// Return the number of operators in the precedence group.
static uint32 countPrecedenceGroupOperators(
    coPrecedenceGroup precedenceGroup)
{
    coOperator operator;
    uint32 numOperators = 0;

    coForeachPrecedenceGroupOperator(precedenceGroup, operator) {
        numOperators++;
    } coEndPrecedenceGroupOperator;
    return numOperators;
}

// Print a precedence group.
void coPrintPrecedenceGroup(
    coPrecedenceGroup precedenceGroup)
{
    coOperator operator;

    if(countPrecedenceGroupOperators(precedenceGroup) == 1) {
        coPrintOperator(coPrecedenceGroupGetFirstOperator(precedenceGroup));
    } else {
        printf("group\n");
        coForeachPrecedenceGroupOperator(precedenceGroup, operator) {
            printf("\t\t");
            coPrintOperator(operator);
        } coEndPrecedenceGroupOperator;
    }
}

// Print out the syntax.
void coPrintSyntax(
    coSyntax syntax)
{
    coStaterule staterule;
    coNoderule noderule;
    coPrecedenceGroup precedenceGroup;

    printf("syntax %s\n", coSyntaxGetName(syntax));
    coForeachSyntaxStaterule(syntax, staterule) {
        printf("\t");
        coPrintStaterule(staterule);
    } coEndSyntaxStaterule;
    coForeachSyntaxNoderule(syntax, noderule) {
        printf("\t");
        coPrintNoderule(noderule);
    } coEndSyntaxNoderule;
    coForeachSyntaxPrecedenceGroup(syntax, precedenceGroup) {
        printf("\t");
        coPrintPrecedenceGroup(precedenceGroup);
    } coEndSyntaxPrecedenceGroup;
}

// Create a new Syntax object.
coSyntax coSyntaxCreate(
    utSym sym)
{
    coSyntax syntax = coRootFindSyntax(coTheRoot, sym);

    if(syntax == coSyntaxNull) {
        syntax = coSyntaxAlloc();
        coSyntaxSetSym(syntax, sym);
        coRootAppendSyntax(coTheRoot, syntax);
    }
    return syntax;
}

// Create a Noderule object for matching node exprs.
static coNoderule noderuleCreate(
    coSyntax syntax,
    char *name,
    ...)
{
    coNoderule noderule = coNoderuleAlloc();
    va_list ap;
    coNodeExpr arg;

    coNoderuleSetSym(noderule, utSymCreate(name));
    coSyntaxAppendNoderule(syntax, noderule);
    va_start(ap, name); 
    arg = va_arg(ap, coNodeExpr);
    while(arg != coNodeExprNull) {
        coNoderuleAppendNodeExpr(noderule, arg);
        arg = va_arg(ap, coNodeExpr);
    }
    va_end(ap);
    return noderule;
}

// Create an operator node expr, with a list of sub-node exprs.
static coNodeExpr opNodeExprCreate(
    char *operatorName,
    ...)
{
    coNodeExpr nodeExpr = coNodeExprAlloc();
    va_list ap;
    coNodeExpr arg;

    coNodeExprSetType(nodeExpr, CO_NODEEXPR_OPERATOR);
    coNodeExprSetSym(nodeExpr, utSymCreate(operatorName));
    va_start(ap, operatorName); 
    arg = va_arg(ap, coNodeExpr);
    while(arg != coNodeExprNull) {
        coNodeExprAppendNodeExpr(nodeExpr, arg);
        arg = va_arg(ap, coNodeExpr);
    }
    va_end(ap);
    return nodeExpr;
}

// Create an identifier token node expr.
static coNodeExpr coTokenNodeExprCreate(
    coNodeExprType type)
{
    coNodeExpr nodeExpr = coNodeExprAlloc();

    coNodeExprSetType(nodeExpr, type);
    return nodeExpr;
}

// Create a sub-node rule node expr.
static coNodeExpr coSubruleNodeExprCreate(
    char *ruleName)
{
    coNodeExpr nodeExpr = coNodeExprAlloc();

    coNodeExprSetType(nodeExpr, CO_NODEEXPR_NODERULE);
    coNodeExprSetSym(nodeExpr, utSymCreate(ruleName));
    return nodeExpr;
}

// Create a list operator node expr.
static coNodeExpr listNodeExprCreate(
    char *operatorName,
    coNodeExpr subNodeExpr)
{
    coNodeExpr nodeExpr = coNodeExprAlloc();

    coNodeExprSetType(nodeExpr, CO_NODEEXPR_LISTOPERATOR);
    coNodeExprSetSym(nodeExpr, utSymCreate(operatorName));
    coNodeExprAppendNodeExpr(nodeExpr, subNodeExpr);
    return nodeExpr;
}

// Create a new element.
static coElement elementCreate(
    coSyntax syntax,
    coPattern pattern,
    char *text)
{
    coElement element = coElementAlloc();
    coKeyword keyword;
    coNodeExpr nodeExpr = coNodeExprNull;
    utSym sym = utSymCreate(text);

    coElementSetSym(element, sym);
    if(!strcmp(text, "expr")) {
        nodeExpr = coTokenNodeExprCreate(CO_NODEEXPR_EXPR);
    } else if(!strcmp(text + strlen(text) - 4, "Expr")) {
        nodeExpr = coSubruleNodeExprCreate(text);
    } else if(!strcmp(text, "ident")) {
        nodeExpr = coTokenNodeExprCreate(CO_NODEEXPR_IDENT);
    } else {
        coElementSetIsKeyword(element, true);
        keyword = coSyntaxFindKeyword(syntax, sym);
        if(keyword == coKeywordNull) {
            keyword = coKeywordAlloc();
            coKeywordSetSym(keyword, sym);
            coSyntaxAppendKeyword(syntax, keyword);
        }
        coKeywordAppendElement(keyword, element);
    }
    coElementSetNodeExpr(element, nodeExpr);
    coPatternAppendElement(pattern, element);
    return element;
}

// Create a new statement parsing rule.
static coStaterule stateruleCreate(
    coSyntax syntax,
    coStatementHandler handler,
    char *name,
    bool hasBlock,
    ...)
{
    coStaterule staterule = coStateruleAlloc();
    coPattern pattern = coPatternAlloc();
    coElement element;
    coFuncptr handlerFuncptr = coFuncptrNull;
    va_list ap;
    char *arg;

    coStateruleInsertPattern(staterule, pattern);
    coStateruleSetSym(staterule, utSymCreate(name));
    if(handler != NULL) {
        handlerFuncptr = coFuncptrCreate(coBuiltinModule,
            utSymCreateFormatted("%sHandler", name), handler);
        coStateruleSetHandlerFuncptr(staterule, handlerFuncptr);
    }
    coStateruleSetHasBlock(staterule, hasBlock);
    va_start(ap, hasBlock); 
    arg = va_arg(ap, char *);
    while(arg != NULL) {
        element = elementCreate(syntax, pattern, arg);
        if(coElementIsKeyword(element)) {
            coStateruleAppendSignature(staterule, coElementGetKeyword(element));
        } else {
            // Apend NULL keywords where exprs go.
            coStateruleAppendSignature(staterule, coKeywordNull);
        }
        arg = va_arg(ap, char *);
    }
    va_end(ap);
    coSyntaxAppendStaterule(syntax, staterule);
    return staterule;
}

// Create a new precedence group.
coPrecedenceGroup coPrecedenceGroupCreate(
    coSyntax syntax,
    coOperator operator)
{
    coPrecedenceGroup precedenceGroup = coPrecedenceGroupAlloc();
    coPrecedenceGroup prevGroup = coPrecedenceGroupNull, nextGroup;

    if(operator != coOperatorNull) {
        nextGroup = coOperatorGetPrecedenceGroup(operator);
        prevGroup = coPrecedenceGroupGetPrevSyntaxPrecedenceGroup(nextGroup);
    }
    if(prevGroup != coPrecedenceGroupNull) {
        coSyntaxInsertAfterPrecedenceGroup(syntax, prevGroup, precedenceGroup);
    } else {
        coSyntaxAppendPrecedenceGroup(syntax, precedenceGroup);
    }
    return precedenceGroup;
}

// Create a new operator.
static coOperator operatorCreate(
    coSyntax syntax,
    coOperatorType type,
    char *name,
    ...)
{
    coPrecedenceGroup precedenceGroup = coPrecedenceGroupCreate(syntax, coOperatorNull);
    coOperator operator = coOperatorAlloc();
    coPattern pattern = coPatternAlloc();
    coElement element;
    va_list ap;
    char *arg;

    coOperatorInsertPattern(operator, pattern);
    coOperatorSetType(operator, type);
    coOperatorSetSym(operator, utSymCreate(name));
    coSyntaxAppendOperator(syntax, operator);
    coPrecedenceGroupAppendOperator(precedenceGroup, operator);
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
    coOperator operator)
{
    coElement element;
    utSym exprSym = utSymCreate("expr");

    coForeachPatternElement(coOperatorGetPattern(operator), element) {
        if(coElementGetKeyword(element) == coKeywordNull &&
                coElementGetSym(element) != exprSym) {
            utError("Invalid element '%s' in operator %s.  Only strings and expr are allowed.",
                utSymGetName(coElementGetSym(element)), coOperatorGetName(operator));
        }
    } coEndPatternElement;
}

// Determine if the operator has two exprs side by side, with no keyword
// between.  Also check that all non-keywords are the identifier "expr".
static bool operatorConcatenatesExprs(
    coOperator operator)
{
    coElement element;
    bool lastWasExpr = false;
    bool concatenatesExprs = false;
    bool hasKeywords = false;

    coForeachPatternElement(coOperatorGetPattern(operator), element) {
        if(coElementGetKeyword(element) == coKeywordNull) {
            if(lastWasExpr) {
                concatenatesExprs = true;
            }
            lastWasExpr = true;
        } else {
            hasKeywords = true;
        }
    } coEndPatternElement;
    return !hasKeywords && concatenatesExprs;
}

// Determine the expression type and expected symbol of expressions matching the node
// expression.
static coExprType findNodeExprTypeAndSym(
    coNodeExpr nodeExpr,
    utSym *symPtr)
{
    coExprType type;
    utSym sym = utSymNull;

    switch(coNodeExprGetType(nodeExpr)) {
    case CO_NODEEXPR_IDENT:
        type = CO_EXPR_IDENT;
        sym = coIdentSym;
        break;
    case CO_NODEEXPR_EXPR:
        type = CO_EXPR_IDENT;
        sym = coExprSym;
        break;
    case CO_NODEEXPR_INTEGER:
        type = CO_EXPR_VALUE;
        sym = coIntegerSym;
        break;
    case CO_NODEEXPR_FLOAT:
        type = CO_EXPR_VALUE;
        sym = coFloatSym;
        break;
    case CO_NODEEXPR_STRING:
        type = CO_EXPR_VALUE;
        sym = coStringSym;
        break;
    case CO_NODEEXPR_CHAR:
        type = CO_EXPR_VALUE;
        sym = coCharSym;
        break;
    case CO_NODEEXPR_CONSTIDENT:
        type = CO_EXPR_IDENT;
        sym = coNodeExprGetSym(nodeExpr);
        break;
    case CO_NODEEXPR_OPERATOR:
    case CO_NODEEXPR_LISTOPERATOR:
        type = CO_EXPR_OPERATOR;
        sym = coNodeExprGetSym(nodeExpr);
        break;
    default:
        utExit("Unknown node expr type");
    }
    *symPtr = sym;
    return type;
}

// Create a new nodelist object.
static coNodelist coNodelistCreate(
    coNoderule noderule,
    coExprType type,
    utSym sym)
{
    coNodelist nodelist = coNodelistAlloc();

    coNodelistSetType(nodelist, type);
    coNodelistSetSym(nodelist, sym);
    coNoderuleInsertNodelist(noderule, nodelist);
    return nodelist;
}

// Copy node expression from the source nodelist to the dest.
static void copyNodeExprsToNodelist(
    coNodelist source,
    coNodelist dest)
{
    coNodeExpr nodeExpr;

    coForeachNodelistNodeExpr(source, nodeExpr) {
        coNodelistAppendNodeExpr(dest, nodeExpr);
    } coEndNodelistNodeExpr;
}

// Add the sub-noderule's nodelists to this noderule's nodelists.
static void addNoderuleNodlistsToNodelists(
    coNoderule noderule,
    coNoderule subNoderule)
{
    coNodelist nodelist, subNodelist;
    coExprType type;
    utSym sym;

    coForeachNoderuleNodelist(subNoderule, subNodelist) {
        type = coNodelistGetType(subNodelist);
        sym = coNodelistGetSym(subNodelist);
        nodelist = coNoderuleFindNodelist(noderule, type, sym);
        if(nodelist == coNodelistNull) {
            nodelist = coNodelistCreate(noderule, type, sym);
        }
        copyNodeExprsToNodelist(subNodelist, nodelist);
    } coEndNoderuleNodelist;
}

// Build a hash table of node expressions that can match an expression based on
// expression type and symbol.  These are used to accelerate matching noderules to
// expressions.
static void buildNoderuleNodelists(
    coNoderule noderule)
{
    coSyntax syntax = coNoderuleGetSyntax(noderule);
    coNoderule subNoderule;
    coNodeExpr nodeExpr;
    coExprType type;
    coNodelist nodelist;
    utSym sym;

    coForeachNoderuleNodeExpr(noderule, nodeExpr) {
        if(coNodeExprGetType(nodeExpr) == CO_NODEEXPR_NODERULE) {
            subNoderule = coSyntaxFindNoderule(syntax, coNodeExprGetSym(nodeExpr));
            if(!coNoderuleBuiltNodelist(subNoderule)) {
                buildNoderuleNodelists(subNoderule);
            }
            addNoderuleNodlistsToNodelists(noderule, subNoderule);
        } else {
            type = findNodeExprTypeAndSym(nodeExpr, &sym);
            nodelist = coNoderuleFindNodelist(noderule, type, sym);
            if(nodelist == coNodelistNull) {
                nodelist = coNodelistCreate(noderule, type, sym);
            }
            coNodelistAppendNodeExpr(nodelist, nodeExpr);
        }
    } coEndNoderuleNodeExpr;
}

// Build a hash table of node expressions that can match an expression based on
// expression type and symbol.  These are used to accelerate matching noderules to
// expressions.
static void syntaxBuildNodelists(
    coSyntax syntax)
{
    coNoderule noderule;

    coForeachSyntaxNoderule(syntax, noderule) {
        if(!coNoderuleBuiltNodelist(noderule)) {
            buildNoderuleNodelists(noderule);
        }
    } coEndSyntaxNoderule;
}

// Set the precedence value on all the operators in the syntax.  This should be
// called whenever a new precedence group is added to the syntax.  Also set the
// concatenation operator, and check operator syntax.  Build a hash table of
// node expressions that can match an expression based on expression type and
// symbol.
void coSetOperatorPrecedence(
    coSyntax syntax)
{
    coPrecedenceGroup group;
    coOperator operator;
    coOperator concatenationOperator = coOperatorNull;
    uint32 precedence = 0;

    for(group = coSyntaxGetLastPrecedenceGroup(syntax); group != coPrecedenceGroupNull;
            group = coPrecedenceGroupGetPrevSyntaxPrecedenceGroup(group)) {
        coPrecedenceGroupSetPrecedence(group, precedence);
        precedence++;
    }
    coForeachSyntaxOperator(syntax, operator) {
        checkOperatorSyntax(operator);
        if(operatorConcatenatesExprs(operator)) {
            if(concatenationOperator != coOperatorNull) {
                utError("Operators %s and %s both concatenate exprs",
                    coOperatorGetName(concatenationOperator), coOperatorGetName(operator));
            }
            concatenationOperator = operator;
        }
    } coEndSyntaxOperator;
    coSyntaxSetConcatenationOperator(syntax, concatenationOperator);
    syntaxBuildNodelists(syntax);
}

// Determine if the expression is an operator expression with the operator name.
static inline bool exprMatches(
    coExpr expr,
    char *name)
{
    return coExprGetType(expr) == CO_EXPR_OPERATOR &&
        !strcmp(coOperatorGetName(coExprGetOperator(expr)), name);
}

// Get the name of the statment.
static inline char *getStatementName(
    coStatement statement)
{
    return utSymGetName(coStateruleGetSym(coStatementGetStaterule(statement)));
}

// Extract before expression symbols.
static void processBeforeExpr(
    coStaterule staterule,
    coExpr beforeExpr)
{
    coExpr identExpr;
    utSym sym;

    //identListExpr: ident | sequence[ident]
    if(coExprGetType(beforeExpr) == CO_EXPR_IDENT) {
        sym = coExprGetSym(beforeExpr);
        coStateruleAppendBeforeSym(staterule, sym);
    } else {
        coForeachExprExpr(beforeExpr, identExpr) {
            sym = coExprGetSym(identExpr);
            coStateruleAppendBeforeSym(staterule, sym);
        } coEndExprExpr;
    }
}

// Extract after expression symbols.
static void processAfterExpr(
    coStaterule staterule,
    coExpr afterExpr)
{
    coExpr identExpr;
    utSym sym;

    //identListExpr: ident | sequence[ident]
    if(coExprGetType(afterExpr) == CO_EXPR_IDENT) {
        sym = coExprGetSym(afterExpr);
        coStateruleAppendAfterSym(staterule, sym);
    } else {
        coForeachExprExpr(afterExpr, identExpr) {
            sym = coExprGetSym(identExpr);
            coStateruleAppendAfterSym(staterule, sym);
        } coEndExprExpr;
    }
}

// Create an element from the element expression.
coElement coElementCreate(
    coSyntax syntax,
    coPattern pattern,
    coExpr elemExpr)
{
    //elementExpr: STRING | ident
    coElement element = coElementAlloc();
    coExprType type = coExprGetType(elemExpr);
    coNodeExpr nodeExpr;
    coKeyword keyword;
    vaValue value;
    utSym sym;
    char *text;

    if(type == CO_EXPR_VALUE) {
        value = coExprGetValue(elemExpr);
        if(vaValueGetType(value) != VA_STRING) {
            utExit("Invalid element expression");
        }
        sym = utSymCreate((char *)vaStringGetValue(vaValueGetStringVal(value)));
        coElementSetIsKeyword(element, true);
        coElementSetSym(element, sym);
        keyword = coSyntaxFindKeyword(syntax, sym);
        if(keyword == coKeywordNull) {
            keyword = coKeywordAlloc();
            coKeywordSetSym(keyword, sym);
            coSyntaxAppendKeyword(syntax, keyword);
        }
        coKeywordAppendElement(keyword, element);
    } else if (type == CO_EXPR_IDENT) {
        text = utSymGetName(coExprGetSym(elemExpr));
        if(!strcmp(text, "expr")) {
            nodeExpr = coTokenNodeExprCreate(CO_NODEEXPR_EXPR);
        } else if(!strcmp(text, "ident")) {
            nodeExpr = coTokenNodeExprCreate(CO_NODEEXPR_IDENT);
        } else {
            nodeExpr = coSubruleNodeExprCreate(text);
        }
        coNodeExprSetLineNum(nodeExpr, coExprGetLineNum(elemExpr));
        coElementSetNodeExpr(element, nodeExpr);
        coElementSetSym(element, coExprGetSym(elemExpr));
    } else {
        utExit("Bad element expression");
    }
    coPatternAppendElement(pattern, element);
    return element;
}

// Create pattern object from the pattern expression.
coPattern coPatternCreate(
    coSyntax syntax,
    coExpr patternExpr)
{
    coPattern pattern = coPatternAlloc();
    coExpr elemExpr;

    //patternExpr: elementExpr | sequence[elementExpr]
    if(!exprMatches(patternExpr, "sequence")) {
        coElementCreate(syntax, pattern, patternExpr);
    } else {
        coForeachExprExpr(patternExpr, elemExpr) {
            coElementCreate(syntax, pattern, elemExpr);
        } coEndExprExpr;
    }
    return pattern;
}

// Create a staterule.
coStaterule coStateruleCreate(
    coSyntax syntax,
    coFuncptr handlerFuncptr,
    utSym name,
    bool hasBlock,
    coExpr patternExpr)
{
    coStaterule staterule = coStateruleAlloc();
    coPattern pattern = coPatternCreate(syntax, patternExpr);
    coElement element;

    coStateruleInsertPattern(staterule, pattern);
    coStateruleSetSym(staterule, name);
    coStateruleSetHandlerFuncptr(staterule, handlerFuncptr);
    coStateruleSetHasBlock(staterule, hasBlock);
    coForeachPatternElement(pattern, element) {
        if(coElementIsKeyword(element)) {
            coStateruleAppendSignature(staterule, coElementGetKeyword(element));
        } else {
            // Apend NULL keywords where exprs go.
            coStateruleAppendSignature(staterule, coKeywordNull);
        }
    } coEndPatternElement;
    coSyntaxAppendStaterule(syntax, staterule);
    return staterule;
}

// Check a handler expression, and return the staterule id and it's handler sym.
static void processHandlerExpr(
    coStatement statement,
    coExpr handlerExpr,
    utSym *identSym,
    coExpr *handlerDotExpr,
    coExpr *beforeExpr,
    coExpr *afterExpr)
{
    coExpr statementExpr = handlerExpr;
    coExpr identExpr;

    // handlerExpr: handler(statementExpr dotExpr) | statementExpr
    if(exprMatches(handlerExpr, "handler")) {
        statementExpr = coExprGetFirstExpr(handlerExpr);
        *handlerDotExpr = coExprGetNextExprExpr(statementExpr);
    }
	// statementExpr: ident | before(ident identListExpr) | after(ident identListExpr)
    *beforeExpr = coExprNull;
    *afterExpr = coExprNull;
    if(exprMatches(statementExpr, "before")) {
        identExpr = coExprGetFirstExpr(statementExpr);
        *beforeExpr = coExprGetNextExprExpr(identExpr);
    } else if(exprMatches(statementExpr, "after")) {
        identExpr = coExprGetFirstExpr(statementExpr);
        *afterExpr = coExprGetNextExprExpr(identExpr);
    } else {
        identExpr = statementExpr;
    }
    *identSym = coExprGetSym(identExpr);
}

// Handler for basicStatement: "statement" handlerExpr ":" patternExpr
static void basicStatementHandler(
    coStatement statement,
    bool preDecent)
{
    coStaterule staterule;
    coExpr handlerExpr = coStatementGetFirstExpr(statement);
    coExpr patternExpr = coExprGetNextStatementExpr(handlerExpr);
    coExpr beforeExpr, afterExpr, handlerDotExpr;
    coFuncptr handlerFuncptr = coFuncptrNull;
    coIdent ident;
    utSym identSym;

    processHandlerExpr(statement, handlerExpr, &identSym, &handlerDotExpr,
        &beforeExpr, &afterExpr);
    if(handlerDotExpr != coExprNull) {
        ident = coFindIdent(handlerDotExpr);
        if(ident == coIdentNull) {
            coExprError(handlerExpr, "Unable to find statement handler");
        }
        if(coIdentGetType(ident) != CO_IDENT_FUNCPTR) {
            coExprError(handlerExpr, "Not a statement handler");
        }
        // TODO: type checking?
        handlerFuncptr = coIdentGetFuncptr(ident);
    }
    staterule = coStateruleCreate(coCurrentSyntax, handlerFuncptr, identSym, false,
        patternExpr);
    if(beforeExpr != coExprNull) {
        processBeforeExpr(staterule, beforeExpr);
    }
    if(afterExpr != coExprNull) {
        processAfterExpr(staterule, afterExpr);
    }
}

// Handler for blockStatement: "blockstatement" usesExpr ":" patternExpr
static void blockStatementHandler(
    coStatement statement,
    bool preDecent)
{
    coStaterule staterule;
    coExpr usesExpr = coStatementGetFirstExpr(statement);
    coExpr patternExpr = coExprGetNextStatementExpr(usesExpr);
    coExpr handlerExpr = usesExpr;
    coExpr syntaxExpr, beforeExpr, afterExpr, handlerDotExpr;
    coFuncptr handlerFuncptr = coFuncptrNull;
    coIdent ident;
    utSym syntaxSym = utSymNull, identSym;

    // usesExpr: useSyntax(handlerExpr ident) | handlerExpr
    if(exprMatches(usesExpr, "useSyntax")) {
        handlerExpr = coExprGetFirstExpr(usesExpr);
        syntaxExpr = coExprGetNextExprExpr(handlerExpr);
        syntaxSym = coExprGetSym(syntaxExpr);
    }
    processHandlerExpr(statement, handlerExpr, &identSym, &handlerDotExpr,
        &beforeExpr, &afterExpr);
    if(handlerDotExpr != coExprNull) {
        ident = coFindIdent(handlerDotExpr);
        if(ident == coIdentNull) {
            coExprError(handlerDotExpr, "Statement handler not found");
        }
        handlerFuncptr = coIdentGetFuncptr(ident);
    }
    staterule = coStateruleCreate(coCurrentSyntax, handlerFuncptr, identSym, true,
        patternExpr);
    coStateruleSetSubSyntaxSym(staterule, syntaxSym);
    if(beforeExpr != coExprNull) {
        processBeforeExpr(staterule, beforeExpr);
    }
    if(afterExpr != coExprNull) {
        processAfterExpr(staterule, afterExpr);
    }
}

static coNodeExpr buildNodeExpr(coExpr expr);

// Create an operator node expression from a rule expression.
coNodeExpr coOperatorNodeExprCreate(
    coExpr opExpr)
{
    coNodeExpr opNodeExpr = coNodeExprAlloc();
    coExpr identExpr = coExprGetFirstExpr(opExpr);
    coExpr nodeListExpr = coExprGetNextExprExpr(identExpr);
    coExpr nodeExpr;

    // operator(ident nodeListExpr)
    coNodeExprSetLineNum(opNodeExpr, coExprGetLineNum(opExpr));
    coNodeExprSetType(opNodeExpr, CO_NODEEXPR_OPERATOR);
    coNodeExprSetSym(opNodeExpr, coExprGetSym(identExpr));
    // nodeListExpr: nodeExpr | sequence(nodeExpr)
    if(exprMatches(nodeListExpr, "sequence")) {
        coForeachExprExpr(nodeListExpr, nodeExpr) {
            coNodeExprAppendNodeExpr(opNodeExpr, buildNodeExpr(nodeExpr));
        } coEndExprExpr;
    } else {
        coNodeExprAppendNodeExpr(opNodeExpr, buildNodeExpr(nodeListExpr));
    }
    return opNodeExpr;
}

// Create a list node expression from a rule expression.
coNodeExpr coListNodeExprCreate(
    coExpr opExpr)
{
    coNodeExpr listNodeExpr = coNodeExprAlloc();
    coExpr identExpr = coExprGetFirstExpr(opExpr);
    coExpr nodeExpr = coExprGetNextExprExpr(identExpr);

    // list(ident nodeExpr)
    coNodeExprSetLineNum(listNodeExpr, coExprGetLineNum(opExpr));
    coNodeExprSetType(listNodeExpr, CO_NODEEXPR_LISTOPERATOR);
    coNodeExprSetSym(listNodeExpr, coExprGetSym(identExpr));
    coNodeExprAppendNodeExpr(listNodeExpr, buildNodeExpr(nodeExpr));
    return listNodeExpr;
}

// Build a node expression from the expression.
static coNodeExpr buildNodeExpr(
    coExpr expr)
{
    coNodeExpr nodeExpr;
    vaValue value;
    char *text;

    // nodeExpr: ident | operator(ident nodeListExpr) | list(ident nodeExpr)
    if(coExprGetType(expr) == CO_EXPR_VALUE) {
        value = coExprGetValue(expr);
        if(vaValueGetType(value) != VA_STRING) {
            utExit("Invalid element expression");
        }
        nodeExpr = coTokenNodeExprCreate(CO_NODEEXPR_CONSTIDENT);
        coNodeExprSetSym(nodeExpr,
            utSymCreate((char *)vaStringGetValue(vaValueGetStringVal(value))));
    } else if(coExprGetType(expr) == CO_EXPR_IDENT) {
        text = utSymGetName(coExprGetSym(expr));
        if(!strcmp(text, "expr")) {
            coExprError(expr, "Invalid used of 'expr' in noderule");
        }
        if(!strcmp(text, "ident")) {
            nodeExpr = coTokenNodeExprCreate(CO_NODEEXPR_IDENT);
        } else if(!strcmp(text, "string")) {
            nodeExpr = coTokenNodeExprCreate(CO_NODEEXPR_STRING);
        } else if(!strcmp(text, "float")) {
            nodeExpr = coTokenNodeExprCreate(CO_NODEEXPR_FLOAT);
        } else if(!strcmp(text, "integer")) {
            nodeExpr = coTokenNodeExprCreate(CO_NODEEXPR_INTEGER);
        } else if(!strcmp(text, "char")) {
            nodeExpr = coTokenNodeExprCreate(CO_NODEEXPR_CHAR);
        } else {
            return coSubruleNodeExprCreate(text);
        }
    } else if(exprMatches(expr, "operator")) {
        return coOperatorNodeExprCreate(expr);
    } else {
        utAssert(exprMatches(expr, "list"));
        return coListNodeExprCreate(expr);
    }
    coNodeExprSetLineNum(nodeExpr, coExprGetLineNum(expr));
    return nodeExpr;
}

// Create a Noderule object for matching node exprs.
coNoderule coNoderuleCreate(
    coSyntax syntax,
    utSym sym,
    coExpr ruleExpr)
{
    coNoderule noderule = coNoderuleAlloc();
    coNodeExpr nodeExpr;
    coExpr expr;

    coNoderuleSetSym(noderule, sym);
    coSyntaxAppendNoderule(syntax, noderule);
    if(exprMatches(ruleExpr, "or")) {
        coForeachExprExpr(ruleExpr, expr) {
            nodeExpr = buildNodeExpr(expr);
            coNoderuleAppendNodeExpr(noderule, nodeExpr);
        } coEndExprExpr;
    } else {
        nodeExpr = buildNodeExpr(ruleExpr);
        coNoderuleAppendNodeExpr(noderule, nodeExpr);
    }
    return noderule;
}

// Handler for ruleStatement: ident ":" ruleExpr
static void ruleStatementHandler(
    coStatement statement,
    bool preDecent)
{
    // statement ruleStatement: ident ":" ruleExpr
    coExpr identExpr = coStatementGetFirstExpr(statement);
    utSym sym = coExprGetSym(identExpr);
    coExpr ruleExpr = coExprGetNextStatementExpr(identExpr);

    coNoderuleCreate(coCurrentSyntax, sym, ruleExpr);
}

// Handler for groupStatement: "group"
static void groupStatementHandler(
    coStatement statement,
    bool preDecent)
{
    if(preDecent) {
        coCurrentPrecedenceGroup = coPrecedenceGroupCreate(coCurrentSyntax, coOperatorNull);
    } else {
        coCurrentPrecedenceGroup = coPrecedenceGroupNull;
    }
}

// Create a new operator object.
coOperator coOperatorCreate(
    coSyntax syntax, 
    coPrecedenceGroup precedenceGroup,
    coOperatorType type,
    utSym nameSym,
    coPattern pattern)
{
    coOperator operator = coOperatorAlloc();

    coOperatorSetType(operator, type);
    coOperatorSetSym(operator, nameSym);
    coOperatorInsertPattern(operator, pattern);
    coSyntaxAppendOperator(syntax, operator);
    coPrecedenceGroupAppendOperator(precedenceGroup, operator);
    return operator;
}

// Handler for operators.
static void operatorStatementHandler(
    coStatement statement,
    coOperatorType type)
{
    coPrecedenceGroup precedenceGroup = coCurrentPrecedenceGroup;
    coExpr identExpr = coStatementGetFirstExpr(statement);
    coExpr patternExpr = coExprGetNextStatementExpr(identExpr);
    coPattern pattern = coPatternCreate(coCurrentSyntax, patternExpr);
    utSym nameSym = coExprGetSym(identExpr);

    // statement leftStatement: "left" ident ":" patternExpr
    if(precedenceGroup == coPrecedenceGroupNull) {
        precedenceGroup = coPrecedenceGroupCreate(coCurrentSyntax, coOperatorNull);
    }
    coOperatorCreate(coCurrentSyntax, precedenceGroup, type, nameSym, pattern);
}

// Handler for leftStatement: "left" ident ":" patternExpr
static void leftStatementHandler(
    coStatement statement,
    bool preDecent)
{
    operatorStatementHandler(statement, CO_OP_LEFT);
}

// Handler for rightStatement: "right" ident ":" patternExpr
static void rightStatementHandler(
    coStatement statement,
    bool preDecent)
{
    operatorStatementHandler(statement, CO_OP_RIGHT);
}

// Handler for mergeStatement: "merge" ident ":" patternExpr
static void mergeStatementHandler(
    coStatement statement,
    bool preDecent)
{
    operatorStatementHandler(statement, CO_OP_MERGE);
}

// Handler for nonassociativeStatement: "nonassociative" ident ":" patternExpr
static void nonassociativeStatementHandler(
    coStatement statement,
    bool preDecent)
{
    operatorStatementHandler(statement, CO_OP_NONASSOCIATIVE);
}

// Check identifiers inthe node expression.
static void checkNodeExpr(
    coSyntax syntax,
    coNodeExpr nodeExpr)
{
    coNodeExpr subNodeExpr;
    coOperator operator;
    utSym sym = coNodeExprGetSym(nodeExpr);
    uint32 numSubExpr = 0;

    switch(coNodeExprGetType(nodeExpr)) {
    case CO_NODEEXPR_NODERULE:
        if(coSyntaxFindNoderule(syntax, sym) == coNoderuleNull) {
            utError("Noderule %s not defined", utSymGetName(sym));
        }
        break;
    case CO_NODEEXPR_OPERATOR: case CO_NODEEXPR_LISTOPERATOR:
        operator = coSyntaxFindOperator(syntax, sym);
        if(operator == coOperatorNull) {
            utError("Noderule %s not defined", utSymGetName(sym));
        }
        coForeachNodeExprNodeExpr(nodeExpr, subNodeExpr) {
            checkNodeExpr(syntax, subNodeExpr);
            numSubExpr++;
        } coEndNodeExprNodeExpr;
        if(coNodeExprGetType(nodeExpr) == CO_NODEEXPR_OPERATOR) {
            if(coOperatorGetType(operator) == CO_OP_MERGE) {
                coNodeExprError(nodeExpr, "Expected non-list node expresssion for operator %s",
                    coOperatorGetName(operator));
            }
        } else {
            if(coOperatorGetType(operator) != CO_OP_MERGE) {
                coNodeExprError(nodeExpr, "Expected list expression for operator %s",
                    coOperatorGetName(operator));
            }
            if(numSubExpr != 1) {
                coNodeExprError(nodeExpr, "List node expression has multiple sub-expressions");
            }
        }
        break;
    case CO_NODEEXPR_INTEGER: case CO_NODEEXPR_FLOAT: case CO_NODEEXPR_STRING:
    case CO_NODEEXPR_CHAR: case CO_NODEEXPR_EXPR: case CO_NODEEXPR_CONSTIDENT:
    case CO_NODEEXPR_IDENT:
        break;
    default:
        utExit("Unknown node expression type");
    }
}

// Check that identifiers in node expressions can be resolved.
static void checkNoderules(
    coSyntax syntax)
{
    coNoderule noderule;
    coNodeExpr nodeExpr;

    coForeachSyntaxNoderule(syntax, noderule) {
        coForeachNoderuleNodeExpr(noderule, nodeExpr) {
            checkNodeExpr(syntax, nodeExpr);
        } coEndNoderuleNodeExpr;
    } coEndSyntaxNoderule;
}

// Handle the syntax statement.
static void syntaxStatementHandler(
    coStatement statement,
    bool preDecent)
{
    static coSyntax prevSyntax;

    if(preDecent) {
        prevSyntax = coCurrentSyntax;
        coCurrentSyntax = coSyntaxCreate(coExprGetSym(coStatementGetFirstExpr(statement)));
    } else {
        coStatementDestroy(statement);
        checkNoderules(coCurrentSyntax);
        coSetOperatorPrecedence(coCurrentSyntax);
        coCurrentSyntax = prevSyntax;
    }
}

// Initialize global utSym values.
static void initGlobalSyms(void)
{
    coIdentSym = utSymCreate("ident");
    coIntegerSym = utSymCreate("integer");
    coFloatSym = utSymCreate("float");
    coStringSym = utSymCreate("string");
    coBoolSym = utSymCreate("bool");
    coExprSym = utSymCreate("expr");
    coCharSym = utSymCreate("char");
}

// Register functions to handle L42 core statements.
static void registerL42Handlers(void)
{
    coBuiltinModule = coModuleCreate(coTopModule, utSymCreate("builtin"));
    coFuncptrCreate(coBuiltinModule, utSymCreate("classStatementHandler"),
        coClassStatementHandler);
    coFuncptrCreate(coBuiltinModule, utSymCreate("defStatementHandler"),
        coDefStatementHandler);
    coFuncptrCreate(coBuiltinModule, utSymCreate("doStatementHandler"),
        coDoStatementHandler);
    coFuncptrCreate(coBuiltinModule, utSymCreate("whileStatementHandler"),
        coWhileStatementHandler);
    coFuncptrCreate(coBuiltinModule, utSymCreate("ifStatementHandler"),
        coIfStatementHandler);
    coFuncptrCreate(coBuiltinModule, utSymCreate("elseIfStatementHandler"),
        coElseIfStatementHandler);
    coFuncptrCreate(coBuiltinModule, utSymCreate("elseStatementHandler"),
        coElseStatementHandler);
    coFuncptrCreate(coBuiltinModule, utSymCreate("returnStatementHandler"),
        coReturnStatementHandler);
    coFuncptrCreate(coBuiltinModule, utSymCreate("enumStatementHandler"),
        coEnumStatementHandler);
    coFuncptrCreate(coBuiltinModule, utSymCreate("exprStatementHandler"),
        coExprStatementHandler);
}

// Create objects representing built-in stuff, like statement patters and operators.
void coCreateBuiltins(void)
{
    coSyntax syntax = coSyntaxCreate(utSymCreate("syntaxStatement"));
    coStaterule staterule;

    initGlobalSyms();
    registerL42Handlers();
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
        opNodeExprCreate("useSyntax", coSubruleNodeExprCreate("handlerExpr"),
            coTokenNodeExprCreate(CO_NODEEXPR_IDENT), coNodeExprNull),
        coSubruleNodeExprCreate("handlerExpr"), coNodeExprNull);
    // handlerExpr: handler(statementExpr dotExpr) | statementExpr
    noderuleCreate(syntax, "handlerExpr",
        opNodeExprCreate("handler", coSubruleNodeExprCreate("statementExpr"),
            coSubruleNodeExprCreate("dotExpr"), coNodeExprNull),
        coSubruleNodeExprCreate("statementExpr"), coNodeExprNull);
    // dotExpr: ident | dot[ident]
    noderuleCreate(syntax, "dotExpr", coTokenNodeExprCreate(CO_NODEEXPR_IDENT),
        listNodeExprCreate("dot", coTokenNodeExprCreate(CO_NODEEXPR_IDENT)), coNodeExprNull);
    // statementExpr: IDENT | before(IDENT identListExpr) | after(IDENT identListExpr)
    noderuleCreate(syntax, "statementExpr", coTokenNodeExprCreate(CO_NODEEXPR_IDENT),
        opNodeExprCreate("before", coTokenNodeExprCreate(CO_NODEEXPR_IDENT),
            coSubruleNodeExprCreate("identListExpr"), coNodeExprNull),
        opNodeExprCreate("after", coTokenNodeExprCreate(CO_NODEEXPR_IDENT),
            coSubruleNodeExprCreate("identListExpr"), coNodeExprNull), coNodeExprNull);
    // identListExpr: IDENT | sequence[IDENT]
    noderuleCreate(syntax, "identListExpr", coTokenNodeExprCreate(CO_NODEEXPR_IDENT),
        listNodeExprCreate("sequence", coTokenNodeExprCreate(CO_NODEEXPR_IDENT)),
        coNodeExprNull);
    //ruleExpr: nodeExpr | or[nodeExpr]
    noderuleCreate(syntax, "ruleExpr", coSubruleNodeExprCreate("nodeExpr"),
        listNodeExprCreate("or", coSubruleNodeExprCreate("nodeExpr")), coNodeExprNull);
    //nodeExpr: ident | string | operator(ident nodeListExpr) | list(ident nodeExpr)
    noderuleCreate(syntax, "nodeExpr", coTokenNodeExprCreate(CO_NODEEXPR_IDENT),
        coTokenNodeExprCreate(CO_NODEEXPR_STRING),
        opNodeExprCreate("operator", coTokenNodeExprCreate(CO_NODEEXPR_IDENT),
            coSubruleNodeExprCreate("nodeListExpr"), coNodeExprNull),
        opNodeExprCreate("list", coTokenNodeExprCreate(CO_NODEEXPR_IDENT),
            coSubruleNodeExprCreate("nodeExpr"), coNodeExprNull), coNodeExprNull);
    //nodeListExpr: nodeExpr | sequence(nodeExpr)
    noderuleCreate(syntax, "nodeListExpr", coSubruleNodeExprCreate("nodeExpr"),
        listNodeExprCreate("sequence", coSubruleNodeExprCreate("nodeExpr")),
        coNodeExprNull);
    //patternExpr: elementExpr | sequence[elementExpr]
    noderuleCreate(syntax, "patternExpr", coSubruleNodeExprCreate("elementExpr"),
        listNodeExprCreate("sequence", coSubruleNodeExprCreate("elementExpr")),
        coNodeExprNull);
    //elementExpr: STRING | IDENT
    noderuleCreate(syntax, "elementExpr", coTokenNodeExprCreate(CO_NODEEXPR_STRING),
        coTokenNodeExprCreate(CO_NODEEXPR_IDENT), coNodeExprNull);

    //merge dot: expr '.' expr
    operatorCreate(syntax, CO_OP_LEFT, "dot", "expr", ".", "expr", NULL);
    //nonassociative node: expr "(" expr ")"
    operatorCreate(syntax, CO_OP_NONASSOCIATIVE, "operator", "expr", "(", "expr", ")", NULL);
    //nonassociative list: expr "[" expr "]"
    operatorCreate(syntax, CO_OP_NONASSOCIATIVE, "list", "expr", "[", "expr", "]", NULL);
    //merge sequence: expr expr # Operators without tokens are allowed
    operatorCreate(syntax, CO_OP_MERGE, "sequence", "expr", "expr", NULL);
    //nonassociative before: expr "before" expr
    operatorCreate(syntax, CO_OP_NONASSOCIATIVE, "before", "expr", "before", "expr", NULL);
    //nonassociative after: expr "after" expr
    operatorCreate(syntax, CO_OP_NONASSOCIATIVE, "after", "expr", "after", "expr", NULL);
    //merge or: expr "|" expr
    operatorCreate(syntax, CO_OP_MERGE, "or", "expr", "|", "expr", NULL);
    //nonassociative handler: expr "handler" expr
    operatorCreate(syntax, CO_OP_NONASSOCIATIVE, "handler", "expr", "handler", "expr", NULL);
    //nonassociative useSyntax: expr "uses" expr
    operatorCreate(syntax, CO_OP_NONASSOCIATIVE, "useSyntax", "expr", "uses", "expr", NULL);

    coL42Syntax = coSyntaxCreate(utSymCreate("l42"));
    staterule = stateruleCreate(coL42Syntax, syntaxStatementHandler, "syntaxStatement", true,
        "syntax", "ident", NULL);
    coStateruleSetSubSyntaxSym(staterule, coSyntaxGetSym(syntax));
    coSetOperatorPrecedence(syntax);

    coPrintSyntax(coL42Syntax);
    coPrintSyntax(syntax);
}

// Parse a source file or string.
static coModule parseSource(
    coModule outerModule,
    utSym moduleName)
{
    coLineNum = 0;
    coCurrentPrecedenceGroup = coPrecedenceGroupNull;
    return coParse(outerModule, moduleName);
}

// Parse a command definition file.
coModule coParseSourceFile(
    coModule outerModule,
    char *fileName)
{
    coModule module;
    utSym sym;
    coFile = fopen(fileName, "r");

    if(coFile == NULL) {
        fprintf(stderr, "Unable to open file %s\n", fileName);
        return coModuleNull;
    }
    sym = utSymCreate(utReplaceSuffix(fileName, ""));
    module = parseSource(outerModule, sym);
    if(module == coModuleNull) {
        fclose(coFile);
        coFile = NULL;
        return coModuleNull;
    }
    fclose(coFile);
    coFile = NULL;
    return module;
}

// Parse a string as a source file.  The returned module has no identifier,
// which means it should be in the top level name space.
coModule coParseCommand(void)
{
    coModule module;

    coFile = NULL;
    module = parseSource(coTopModule, utSymNull);
    if(module == coModuleNull) {
        return coModuleNull;
    }
    return module;
}

