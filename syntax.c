#include "pa.h"

static paSyntax paTargetSyntax;
static paPrecedenceGroup paCurrentPrecedenceGroup;

static utSym paSimpleStatmentSym, paBlockStatementSym, paRuleStatementSym,
    paGroupStatementSym, paLeftStatementSym, paRightStatementSym, paMergeStatementSym,
    paNoneStatementSym;

// Initialize some globals for handling syntax statements.
void paSyntaxStart(void)
{
    paSimpleStatmentSym = utSymCreate("simpleStatement");
    paBlockStatementSym = utSymCreate("blockStatement");
    paRuleStatementSym = utSymCreate("ruleStatement");
    paGroupStatementSym = utSymCreate("groupStatement");
    paLeftStatementSym = utSymCreate("leftStatement");
    paRightStatementSym = utSymCreate("rightStatement");
    paMergeStatementSym = utSymCreate("mergeStatement");
    paNoneStatementSym = utSymCreate("noneStatement");
}

// Nothing for now.
void paSyntaxStop(void)
{
}

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

// Determine if the expression is an operator expression with the operator name.
static inline bool exprMatches(
    paExpr expr,
    char *name)
{
    return paExprGetType(expr) == PA_EXPR_OPERATOR &&
        !strcmp(paOperatorGetName(paExprGetOperator(expr)), name);
}

// Create a new Syntax object.
paSyntax paSyntaxCreate(
    utSym name)
{
    paSyntax syntax = paRootFindSyntax(paTheRoot, name);

    if(syntax == paSyntaxNull) {
        syntax = paSyntaxAlloc();
        paSyntaxSetSym(syntax, name);
        paRootAppendSyntax(paTheRoot, syntax);
    }
    return syntax;
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

// Check a handler expression, and return the staterule id and it's handler sym.
static void processStatementExpr(
    paStatement statement,
    paExpr statementExpr,
    utSym *identSym,
    paExpr *beforeExpr,
    paExpr *afterExpr)
{
    paExpr identExpr;

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

// Handler for simpleStatement: "statement" statementExpr ":" patternExpr
static void handleSimpleStatement(
    paStatement statement)
{
    paStaterule staterule;
    paExpr statementExpr = paStatementGetFirstExpr(statement);
    paExpr patternExpr = paExprGetNextStatementExpr(statementExpr);
    paExpr beforeExpr, afterExpr;
    utSym identSym;

    processStatementExpr(statement, statementExpr, &identSym, &beforeExpr, &afterExpr);
    staterule = paStateruleCreate(paTargetSyntax, identSym, false, patternExpr);
    if(beforeExpr != paExprNull) {
        processBeforeExpr(staterule, beforeExpr);
    }
    if(afterExpr != paExprNull) {
        processAfterExpr(staterule, afterExpr);
    }
}

// Handler for blockStatement: "blockstatement" usesExpr ":" patternExpr
static void handleBlockStatement(
    paStatement statement)
{
    paStaterule staterule;
    paExpr usesExpr = paStatementGetFirstExpr(statement);
    paExpr patternExpr = paExprGetNextStatementExpr(usesExpr);
    paExpr statementExpr = usesExpr;
    paExpr syntaxExpr, beforeExpr, afterExpr;
    utSym syntaxSym = utSymNull, identSym;

    // usesExpr: useSyntax(statementExpr ident) | statementExpr
    if(exprMatches(usesExpr, "useSyntax")) {
        statementExpr = paExprGetFirstExpr(usesExpr);
        syntaxExpr = paExprGetNextExprExpr(statementExpr);
        syntaxSym = paExprGetSym(syntaxExpr);
    }
    processStatementExpr(statement, statementExpr, &identSym, &beforeExpr, &afterExpr);
    staterule = paStateruleCreate(paTargetSyntax, identSym, true, patternExpr);
    paStateruleSetSubSyntaxSym(staterule, syntaxSym);
    if(beforeExpr != paExprNull) {
        processBeforeExpr(staterule, beforeExpr);
    }
    if(afterExpr != paExprNull) {
        processAfterExpr(staterule, afterExpr);
    }
}

// Handler for ruleStatement: ident ":" ruleExpr
static void handleRuleStatement(
    paStatement statement)
{
    // statement ruleStatement: ident ":" ruleExpr
    paExpr identExpr = paStatementGetFirstExpr(statement);
    utSym sym = paExprGetSym(identExpr);
    paExpr ruleExpr = paExprGetNextStatementExpr(identExpr);

    paNoderuleCreate(paTargetSyntax, sym, ruleExpr);
}

static void handleStatement(paStatement statement);

// Handler for groupStatement: "group"
static void handleGroupStatement(
    paStatement statement)
{
    paStatement subStatement;

    paCurrentPrecedenceGroup = paPrecedenceGroupCreate(paTargetSyntax, paOperatorNull);
    paSafeForeachStatementStatement(statement, subStatement) {
        handleStatement(subStatement);
    } paEndSafeStatementStatement;
    paCurrentPrecedenceGroup = paPrecedenceGroupNull;
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
static void handleOperatorStatement(
    paStatement statement,
    paOperatorType type)
{
    paPrecedenceGroup precedenceGroup = paCurrentPrecedenceGroup;
    paExpr identExpr = paStatementGetFirstExpr(statement);
    paExpr patternExpr = paExprGetNextStatementExpr(identExpr);
    paPattern pattern = paPatternCreate(paTargetSyntax, patternExpr);
    utSym nameSym = paExprGetSym(identExpr);

    // statement leftStatement: "left" ident ":" patternExpr
    if(precedenceGroup == paPrecedenceGroupNull) {
        precedenceGroup = paPrecedenceGroupCreate(paTargetSyntax, paOperatorNull);
    }
    paOperatorCreate(paTargetSyntax, precedenceGroup, type, nameSym, pattern);
}

// Update syntax rules from the syntax statement.
static void handleStatement(
    paStatement statement)
{
    paStaterule staterule = paStatementGetStaterule(statement);
    utSym name;
  
    if(paStatementIsComment(statement)) {
        return;
    }
    name = paStateruleGetSym(staterule);
    if(name == paSimpleStatmentSym) {
        handleSimpleStatement(statement);
    } else if(name == paBlockStatementSym) {
        handleBlockStatement(statement);
    } else if(name == paRuleStatementSym) {
        handleRuleStatement(statement);
    } else if(name == paGroupStatementSym) {
        handleGroupStatement(statement);
    } else if(name == paLeftStatementSym) {
        handleOperatorStatement(statement, PA_OP_LEFT);
    } else if(name == paRightStatementSym) {
        handleOperatorStatement(statement, PA_OP_RIGHT);
    } else if(name == paMergeStatementSym) {
        handleOperatorStatement(statement, PA_OP_MERGE);
    } else if(name == paNoneStatementSym) {
        handleOperatorStatement(statement, PA_OP_NONE);
    } else {
        utExit("Unknown statement type %s", utSymGetName(name));
    }
}

static paNodeExpr buildNodeExpr(paExpr expr);

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

// Check identifiers in the node expression.
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
void paCheckNoderules(
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
    utSym name,
    bool hasBlock,
    paExpr patternExpr)
{
    paStaterule staterule = paStateruleAlloc();
    paPattern pattern = paPatternCreate(syntax, patternExpr);
    paElement element;

    paStateruleInsertPattern(staterule, pattern);
    paStateruleSetSym(staterule, name);
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

// Update syntax rules from the syntax statement.
void paProcessSyntaxStatement(
    paSyntax targetSyntax,
    paStatement statement)
{
    paStatement subStatement;

    paCurrentPrecedenceGroup = paPrecedenceGroupNull;
    paTargetSyntax = targetSyntax;
    paForeachStatementStatement(statement, subStatement) {
        handleStatement(subStatement);
    } paEndStatementStatement;
    paCheckNoderules(paTargetSyntax);
    paSetOperatorPrecedence(paTargetSyntax);
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
    case PA_OP_NONE: printf("none"); break;
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
        printf("group {\n");
        paForeachPrecedenceGroupOperator(precedenceGroup, operator) {
            printf("\t\t");
            paPrintOperator(operator);
        } paEndPrecedenceGroupOperator;
        printf("}\n");
    }
}

// Print out the syntax.
void paPrintSyntax(
    paSyntax syntax)
{
    paStaterule staterule;
    paNoderule noderule;
    paPrecedenceGroup precedenceGroup;

    printf("syntax %s {\n", paSyntaxGetName(syntax));
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
    printf("}\n");
}

// Create an identifier token node expr.
paNodeExpr paTokenNodeExprCreate(
    paNodeExprType type)
{
    paNodeExpr nodeExpr = paNodeExprAlloc();

    paNodeExprSetType(nodeExpr, type);
    return nodeExpr;
}

// Create a sub-node rule node expr.
paNodeExpr paSubruleNodeExprCreate(
    char *ruleName)
{
    paNodeExpr nodeExpr = paNodeExprAlloc();

    paNodeExprSetType(nodeExpr, PA_NODEEXPR_NODERULE);
    paNodeExprSetSym(nodeExpr, utSymCreate(ruleName));
    return nodeExpr;
}

// Create a new nodelist object.
paNodelist paNodelistCreate(
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

