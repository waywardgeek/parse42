#include "pa.h"

paRoot paTheRoot;
FILE *paFile;
// Must be set before parsing so that the parser knows where to add stuff.
uint32 paFileSize, paLineNum;
paSyntax paParseSyntax, paCurrentSyntax;
utSym paIdentSym, paIntegerSym, paFloatSym, paStringSym, paBoolSym, paCharSym, paExprSym;

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

// Create a new statement parsing rule.
static paStaterule stateruleCreate(
    paSyntax syntax,
    char *name,
    bool hasBlock,
    ...)
{
    paStaterule staterule = paStateruleAlloc();
    paPattern pattern = paPatternAlloc();
    paElement element;
    va_list ap;
    char *arg;

    paStateruleInsertPattern(staterule, pattern);
    paStateruleSetSym(staterule, utSymCreate(name));
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

// Create objects representing built-in stuff, like statement patters and operators.
void paCreateBuiltins(void)
{
    paSyntax syntax = paSyntaxCreate(utSymCreate("syntaxStatement"));

    initGlobalSyms();
    // statement simpleStatement: "statement" statementExpr ":" patternExpr
    stateruleCreate(syntax, "simpleStatement", false, "statement",
        "statementExpr", ":", "patternExpr", NULL);
    // statement blockStatement: "blockstatement" usesExpr ":" patternExpr
    stateruleCreate(syntax, "blockStatement", false,
        "blockstatement", "usesExpr", ":", "patternExpr", NULL);
    // statement ruleStatement: ident ":" ruleExpr
    stateruleCreate(syntax, "ruleStatement", false, "ident", ":",
        "ruleExpr", NULL);
    // blockstatement groupStatement: "group"
    stateruleCreate(syntax, "groupStatement", true, "group", NULL);
    // statement leftStatement: "left" ident ":" patternExpr
    stateruleCreate(syntax, "leftStatement", false, "left",
        "ident", ":", "patternExpr", NULL);
    // statement rightStatement: "right" ident ":" patternExpr
    stateruleCreate(syntax, "rightStatement", false, "right",
        "ident", ":", "patternExpr", NULL);
    // statement mergeStatement: "merge" ident ":" patternExpr
    stateruleCreate(syntax, "mergeStatement", false, "merge",
        "ident", ":", "patternExpr", NULL);
    // statement noneStatement: "none" ident ":" patternExpr
    stateruleCreate(syntax, "noneStatement",
        false, "none", "ident", ":", "patternExpr", NULL);

    // usesExpr: useSyntax(statementExpr ident) | statementExpr
    noderuleCreate(syntax, "usesExpr",
        opNodeExprCreate("useSyntax", paSubruleNodeExprCreate("statementExpr"),
            paTokenNodeExprCreate(PA_NODEEXPR_IDENT), paNodeExprNull),
        paSubruleNodeExprCreate("statementExpr"), paNodeExprNull);
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
    //none node: expr "(" expr ")"
    operatorCreate(syntax, PA_OP_NONE, "operator", "expr", "(", "expr", ")", NULL);
    //none list: expr "[" expr "]"
    operatorCreate(syntax, PA_OP_NONE, "list", "expr", "[", "expr", "]", NULL);
    //merge sequence: expr expr # Operators without tokens are allowed
    operatorCreate(syntax, PA_OP_MERGE, "sequence", "expr", "expr", NULL);
    //none before: expr "before" expr
    operatorCreate(syntax, PA_OP_NONE, "before", "expr", "before", "expr", NULL);
    //none after: expr "after" expr
    operatorCreate(syntax, PA_OP_NONE, "after", "expr", "after", "expr", NULL);
    //merge or: expr "|" expr
    operatorCreate(syntax, PA_OP_MERGE, "or", "expr", "|", "expr", NULL);
    //none useSyntax: expr "uses" expr
    operatorCreate(syntax, PA_OP_NONE, "useSyntax", "expr", "uses", "expr", NULL);
    paCheckNoderules(syntax);
    paSetOperatorPrecedence(syntax);
    paParseSyntax = syntax;
    paPrintSyntax(syntax);
}

// Parse a command definition file.
paStatement paParseSourceFile(
    paSyntax syntax,
    char *fileName)
{
    paStatement statement;

    paFile = fopen(fileName, "r");
    if(paFile == NULL) {
        fprintf(stderr, "Unable to open file %s\n", fileName);
        return paStatementNull;
    }
    paLineNum = 0;
    statement = paParse(syntax);
    fclose(paFile);
    paFile = NULL;
    return statement;
}
