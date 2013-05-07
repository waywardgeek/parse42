/*
    This parsing algorithm should be very fast, comparable to bison/flex in
    real-world applications, yet it allows on-the-fly definition of new
    statements and operators.

    Parser speed is critical, so some flexibilty is lost.  In particular,
    statements are grouped like python, one per line.  One syntax is active at
    any time.  Statements are parsed one at a time.  The lexer is fixed, and not
    upgradable, but new keywords and operators can be defined.

    Parsing a statement begins with finding it's keyword signature.  Everything
    between, or before or after a keyword must be reducable to an expr.  A
    statement matching this signature is looked up in a hash table, and it is a
    syntax error if none is found.

    Next, a "precedence parser" parses each set of tokens between keywords
    as an expr, and creates an expr tree from it.  Then syntactic
    checks are made on the expr tree as a post-process.  This process is
    very fast because all the expr types have been established.

    A limitation is that operators can't be keywords and vise-versa, within any
    given syntax.  This enables the fast statement lookup.

    The "core" L42 syntax is complex enough that it was written in the "syntax"
    grammar, and parsed just like any other.
*/

#include "co.h"

static coStatement coOuterStatement, coPrevStatement;
static bool coDebug;

// Print out an error message and exit.
void coExprError(
    coExpr expr,
    char *message,
    ...)
{
    char *buff;
    va_list ap;

    va_start(ap, message);
    buff = utVsprintf((char *)message, ap);
    va_end(ap);
    utError("Line %d: %s", coExprGetLineNum(expr), buff);
}

// Read one line of tokens.  It is up to the caller to destroy these tokens.
static bool readOneLine(void)
{
    printf("Reading one line\n");
    coSyntax syntax;
    coStaterule staterule;
    coToken token = coLex();
    utSym subSyntaxSym;

    if(token == coTokenNull) {
        return false;
    }
    // Now deal with BEGIN/END tokens.
    if(coTokenGetType(token) == CO_TOK_BEGIN) {
        printf("Starting sub-statements\n");
        if(coPrevStatement == coStatementNull) {
            coError(token, "First line may not be intented");
        }
        coOuterStatement = coPrevStatement;
        subSyntaxSym = coStateruleGetSubSyntaxSym(coStatementGetStaterule(coOuterStatement));
        if(subSyntaxSym != utSymNull) {
            syntax = coRootFindSyntax(coTheRoot, subSyntaxSym);
            if(syntax != coSyntaxNull) {
                printf("Using syntax %s\n", coSyntaxGetName(syntax));
                coCurrentSyntax = syntax;
            } else {
                coError(token, "Syntax %s not found", utSymGetName(subSyntaxSym));
            }
        }
        coTokenDestroy(token);
        token = coLex();
    }
    while(token != coTokenNull && coTokenGetType(token) == CO_TOK_END) {
        printf("Finished sub-statements\n");
        coOuterStatement = coStatementGetStatement(coOuterStatement);
        staterule = coStatementGetStaterule(coOuterStatement);
        if(staterule == coStateruleNull) {
            syntax = coL42Syntax;
        } else {
            syntax = coRootFindSyntax(coTheRoot, coStateruleGetSubSyntaxSym(staterule));
            if(syntax == coSyntaxNull) {
                syntax = coStateruleGetSyntax(staterule);
            }
        }
        if(syntax != coCurrentSyntax) {
            printf("Using syntax %s\n", coSyntaxGetName(syntax));
            coCurrentSyntax = syntax;
        }
        coTokenDestroy(token);
        token = coLex();
    }
    while(token != coTokenNull && coTokenGetType(token) != CO_TOK_NEWLINE) {
        if(coTokenGetType(token) == CO_TOK_CHAR) {
            coError(token, "Illegal character in input");
        }
        coSyntaxAppendToken(coCurrentSyntax, token);
        coPrintToken(token);
        token = coLex();
    }
    if(token != coTokenNull) {
        coTokenDestroy(token); // We no longer need the NEWLINE token
    }
    return coSyntaxGetUsedToken(coCurrentSyntax) > 0;
}

// Destroy the tokens in the token list.
static void destroyLineTokens(void)
{
    coToken token;

    coForeachSyntaxToken(coCurrentSyntax, token) {
        coTokenDestroy(token);
    } coEndSyntaxToken;
    coSyntaxSetUsedToken(coCurrentSyntax, 0);
}

// Find the statement rule matching the current tokens.  We use coKeywordNull to
// hold he place of exprs.
static coStaterule lookupStaterule(void)
{
    coKeyword keywords[coSyntaxGetUsedToken(coCurrentSyntax)];
    coToken token;
    uint32 numKeywords = 0;
    bool lastWasExpr = false;

    coForeachSyntaxToken(coCurrentSyntax, token) {
        if(coTokenGetType(token) == CO_TOK_KEYWORD) {
            keywords[numKeywords++] = coTokenGetKeywordVal(token);
            lastWasExpr = false;
        } else {
            if(!lastWasExpr) {
                keywords[numKeywords++] = coKeywordNull;
            }
            lastWasExpr = true;
        }
    } coEndSyntaxToken;
    return coSyntaxFindStaterule(coCurrentSyntax, keywords, numKeywords);
}

// Determine if the operator matches the occurrence.  We match if the pattern
// has an expr or not matching hasLeftExpr, and we require the first set of
// contiguous keywords to match the tokens.
static bool patternMatches(
    coPattern pattern,
    coToken *tokens,
    uint32 numTokens,
    bool hasLeftExpr)
{
    coElement element = coPatternGetFirstElement(pattern);
    coToken token;
    uint32 tokenPos = 0;

    if(hasLeftExpr == (coElementGetKeyword(element) != coKeywordNull)) {
        return false;
    }
    if(hasLeftExpr) {
        element = coElementGetNextPatternElement(element);
    }
    while(element != coElementNull && coElementGetKeyword(element) != coKeywordNull) {
        if(tokenPos == numTokens) {
            return false;
        }
        token = tokens[tokenPos++];
        if(coTokenGetType(token) != CO_TOK_OPERATOR ||
                coElementGetKeyword(element) != coTokenGetKeywordVal(token)) {
            return false;
        }
        element = coElementGetNextPatternElement(element);
    }
    return true;
}

// Find the operator given the left hand expr, and the tokens.
static coOperator findOperator(
    coExpr leftExpr,
    coToken *tokens,
    uint32 numTokens,
    coKeyword endKeyword)
{
    coOperator operator;
    coPattern pattern;
    coElement element;
    coKeyword keyword;
    coToken token = tokens[0];
    bool hasLeftExpr = leftExpr != coExprNull;

    if(numTokens == 0) {
        return coOperatorNull;
    }
    if(leftExpr == coExprNull && coTokenGetType(token) != CO_TOK_OPERATOR) {
        if(numTokens == 1) {
            return coOperatorNull;
        }
        tokens++;
        numTokens--;
        token = tokens[0];
        hasLeftExpr = true;
    }
    keyword = coTokenGetKeywordVal(token);
    if(keyword != coKeywordNull && keyword == endKeyword) {
        return coOperatorNull;
    }
    if(coTokenGetType(token) != CO_TOK_OPERATOR) {
        if(!hasLeftExpr) {
            return coOperatorNull;
        }
        // This is the case where we have two exprs in series with no operator.
        return coSyntaxGetConcatenationOperator(coCurrentSyntax);
    }
    coForeachKeywordElement(keyword, element) {
        pattern = coElementGetPattern(element);
        operator = coPatternGetOperator(pattern);
        if(operator != coOperatorNull && patternMatches(pattern, tokens, numTokens,
                hasLeftExpr)) {
            return operator;
        }
    } coEndKeywordElement;
    coError(token, "Invalid operator");
    return coOperatorNull;
}

// Build an expr for a non-operator token.
static coExpr buildPrimaryExpr(
    coToken token)
{
    coExpr expr;

    switch(coTokenGetType(token)) {
    case CO_TOK_INTEGER:
        expr = coValueExprCreate(vaIntValueCreate(coTokenGetIntVal(token)));
        break;
    case CO_TOK_FLOAT:
        expr = coValueExprCreate(vaFloatValueCreate(coTokenGetFloatVal(token)));
        break;
    case CO_TOK_STRING:
        expr =  coValueExprCreate(vaStringValueCreate(
            vaStringCreate(coTokenGetText(token))));
        break;
    case CO_TOK_IDENT:
        expr = coIdentExprCreate(utSymCreate((char *)coTokenGetText(token)));
        break;
    default:
        utExit("Unknown token type");
    }
    coExprSetLineNum(expr, coTokenGetLineNum(token));
    return expr;
}

// Find the next keyword after this element in an operator pattern.  Operators
// with keywords must have alternating operators and exprs.
static coKeyword findNextKeyword(
    coElement element)
{
    element = coElementGetNextPatternElement(element);
    if(element == coElementNull || coElementGetKeyword(element) == coKeywordNull) {
        return coKeywordNull;
    }
    return coElementGetKeyword(element);
}

// Merge the source expr into the dest.
static void mergeExprs(
    coExpr source,
    coExpr dest)
{
    coExpr subExpr;

    printf("Merging %s operators\n", coOperatorGetName(coExprGetOperator(dest)));
    coSafeForeachExprExpr(source, subExpr) {
        coExprRemoveExpr(source, subExpr);
        coExprAppendExpr(dest, subExpr);
    } coEndSafeExprExpr;
    coExprDestroy(source);
}

// Just a forward declaration for double recursion.
static coExpr parseExpr(coToken *tokens, uint32 numTokens, coKeyword
    endKeyword, uint32 *tokensParsed);

// Parse one expr at the given precedence.  The left expr exists
// unless it's a prefix operator.
static coExpr parseSubExpr(
    coExpr leftExpr,
    coToken *tokens,
    uint32 numTokens,
    uint32 precedence,
    coKeyword endKeyword,
    uint32 *tokensParsed)
{
    coOperator operator;
    coElement element;
    coKeyword keyword, nextEndKeyword;
    coExpr expr = coExprNull, subExpr;
    uint32 tokenPos = 0;
    uint32 opPrecedence;
    bool firstTime;
    bool isConcatenation;

    utDo {
        operator = findOperator(leftExpr, tokens + tokenPos, numTokens - tokenPos, endKeyword);
        if(operator == coOperatorNull || precedence >
                coPrecedenceGroupGetPrecedence(coOperatorGetPrecedenceGroup(operator))) {
            if(leftExpr != coExprNull) {
                *tokensParsed = tokenPos;
                return leftExpr;
            }
            if(operator == coOperatorNull || coTokenGetType(tokens[0]) != CO_TOK_OPERATOR) {
                *tokensParsed = 1;
                return buildPrimaryExpr(tokens[0]);
            }
        }
        isConcatenation = operator == coSyntaxGetConcatenationOperator(coCurrentSyntax);
        opPrecedence = coPrecedenceGroupGetPrecedence(coOperatorGetPrecedenceGroup(operator));
    } utWhile(expr == coExprNull || opPrecedence >= precedence) {
        if(coOperatorGetType(operator) == CO_OP_MERGE && leftExpr != coExprNull &&
                operator == coExprGetOperator(leftExpr)) {
            expr = leftExpr;
            leftExpr = coExprNull;
            printf("Merging %s operators\n", coOperatorGetName(operator));
        } else {
            expr = coOperatorExprCreate(operator);
            coExprSetLineNum(expr, coTokenGetLineNum(tokens[tokenPos]));
        }
        firstTime = true;
        coForeachPatternElement(coOperatorGetPattern(operator), element) {
            keyword = coElementGetKeyword(element);
            if(keyword == coKeywordNull) {
                // Must be expr
                if(firstTime && leftExpr != coExprNull) {
                    subExpr = leftExpr;
                } else {
                    nextEndKeyword = findNextKeyword(element);
                    if(isConcatenation && firstTime) {
                        subExpr = parseExpr(tokens + tokenPos, 1,
                            nextEndKeyword, tokensParsed);
                    } else if(nextEndKeyword != coKeywordNull) {
                        subExpr = parseExpr(tokens + tokenPos, numTokens - tokenPos,
                            nextEndKeyword, tokensParsed);
                    } else {
                        subExpr = parseSubExpr(coExprNull, tokens + tokenPos,
                            numTokens - tokenPos, opPrecedence, endKeyword, tokensParsed);
                    }
                    tokenPos += *tokensParsed;
                }
                if(coOperatorGetType(operator) == CO_OP_MERGE &&
                        operator == coExprGetOperator(subExpr)) {
                    mergeExprs(subExpr, expr);
                } else {
                    coExprAppendExpr(expr, subExpr);
                }
            } else {
                if(coTokenGetKeywordVal(tokens[tokenPos]) != keyword) {
                    coError(tokens[tokenPos], "Expected operator %s",
                        coKeywordGetName(keyword));
                }
                tokenPos++;
            }
            firstTime = false;
        } coEndPatternElement;
        leftExpr = expr;
    } utRepeat;
    *tokensParsed = tokenPos;
    return expr;
}

// Recursively parse the expr.
static coExpr parseExpr(
    coToken *tokens,
    uint32 numTokens,
    coKeyword endKeyword,
    uint32 *tokensParsed)
{
    coExpr expr = parseSubExpr(coExprNull, tokens,
            numTokens, 0, endKeyword, tokensParsed);

    if(*tokensParsed != numTokens && (endKeyword == coKeywordNull ||
            coTokenGetKeywordVal(tokens[*tokensParsed]) != endKeyword)) {
        coError(tokens[*tokensParsed], "Unable to parse entire expr");
    }
    return expr;
}

// Parse the exprs for the statement.
static void parseExprs(
    coStatement statement)
{
    coExpr expr;
    coToken tokens[coSyntaxGetUsedToken(coCurrentSyntax)];
    coToken token;
    uint32 numTokens = 0;
    uint32 tokensParsed;

    coForeachSyntaxToken(coCurrentSyntax, token) {
        if(coTokenGetType(token) == CO_TOK_KEYWORD) {
            if(numTokens > 0) {
                expr = parseExpr(tokens, numTokens, coTokenGetKeywordVal(token),
                    &tokensParsed);
                coStatementAppendExpr(statement, expr);
            }
            numTokens = 0;
        } else {
            tokens[numTokens++] = token;
        }
    } coEndSyntaxToken;
    if(numTokens > 0) {
        expr = parseExpr(tokens, numTokens, coTokenGetKeywordVal(token),
            &tokensParsed);
        coStatementAppendExpr(statement, expr);
    }
}

static bool matchNoderule(coNoderule noderule, coExpr expr);

// Match the noderule to the expr.
static bool matchNodeExpr(
    coNodeExpr nodeExpr,
    coExpr expr)
{
    coNoderule noderule;
    coExpr subExpr;
    coNodeExpr subNode;
    coExprType type = coExprGetType(expr);
    utSym sym = coNodeExprGetSym(nodeExpr);

    if(coDebug) {
        printf("Matching nodeExpr ");
        coPrintNodeExpr(nodeExpr);
        printf(" to ");
        coPrintExpr(expr);
        printf("\n");
    }
    switch(coNodeExprGetType(nodeExpr)) {
    case CO_NODEEXPR_NODERULE:
        noderule = coSyntaxFindNoderule(coCurrentSyntax, sym);
        if(noderule == coNoderuleNull) {
            utExit("Noderule %s not found", utSymGetName(sym));
        }
        return matchNoderule(noderule, expr);
    case CO_NODEEXPR_OPERATOR:
        if(type != CO_EXPR_OPERATOR ||
                coOperatorGetSym(coExprGetOperator(expr)) != sym) {
            return false;
        }
        subExpr = coExprGetFirstExpr(expr);
        coForeachNodeExprNodeExpr(nodeExpr, subNode) {
            if(subExpr == coExprNull || !matchNodeExpr(subNode, subExpr)) {
                return false;
            }
            subExpr = coExprGetNextExprExpr(subExpr);
        } coEndNodeExprNodeExpr;
        return subExpr == coExprNull;
    case CO_NODEEXPR_LISTOPERATOR:
        if(type != CO_EXPR_OPERATOR ||
                coOperatorGetSym(coExprGetOperator(expr)) != sym) {
            return false;
        }
        subNode = coNodeExprGetFirstNodeExpr(nodeExpr);
        coForeachExprExpr(expr, subExpr) {
            if(!matchNodeExpr(subNode, subExpr)) {
                return false;
            }
        } coEndExprExpr;
        return true;
    case CO_NODEEXPR_INTEGER:
        return type == CO_EXPR_VALUE &&
            vaValueGetType(coExprGetValue(expr)) == VA_INT;
    case CO_NODEEXPR_FLOAT:
        return type == CO_EXPR_VALUE && vaValueGetType(coExprGetValue(expr)) == VA_FLOAT;
    case CO_NODEEXPR_STRING:
        return type == CO_EXPR_VALUE && vaValueGetType(coExprGetValue(expr)) == VA_STRING;
    case CO_NODEEXPR_IDENT:
        return type == CO_EXPR_IDENT;
    case CO_NODEEXPR_CONSTIDENT:
        if(type != CO_EXPR_VALUE || vaValueGetType(coExprGetValue(expr)) != VA_STRING) {
            return false;
        }
        return !strcmp((char *)vaStringGetValue(vaValueGetStringVal(coExprGetValue(expr))),
            utSymGetName(coNodeExprGetSym(nodeExpr)));
    case CO_NODEEXPR_EXPR:
        utError("Invalid use of 'expr' in a node rule");
        break;
    default:
        utExit("Unknown nodeExpr type");
    }
    return false; // Never gets here
}

// Find the node expression sym corresponding to this expressin.
static utSym findExprSym(
    coExpr expr)
{
    coExprType type = coExprGetType(expr);

    if(type == CO_EXPR_IDENT) {
        return coIdentSym;
    } else if(type == CO_EXPR_OPERATOR) {
        return coOperatorGetSym(coExprGetOperator(expr));
    } else if(type == CO_EXPR_VALUE) {
        switch(vaValueGetType(coExprGetValue(expr))) {
        case VA_INT:
            return coIntegerSym;
        case VA_FLOAT:
            return coFloatSym;
        case VA_STRING:
            return coStringSym;
        case VA_BOOL:
            return coBoolSym;
        default:
            utExit("Unknown value type");
        }
    }
    return utSymNull;
}

// Match the noderule to the expr.
static bool matchNoderule(
    coNoderule noderule,
    coExpr expr)
{
    coNodeExpr nodeExpr;
    coNodelist nodelist;
    coExprType type = coExprGetType(expr);
    utSym sym = utSymNull;

    if(coDebug) {
        printf("Trying noderule ");
        coPrintNoderule(noderule);
    }
    sym = findExprSym(expr);
    nodelist = coNoderuleFindNodelist(noderule, type, sym);
    if(nodelist == coNodelistNull) {
        if(coDebug) {
            printf("No noderule found!\n");
        }
        return false;
    }
    coForeachNodelistNodeExpr(nodelist, nodeExpr) {
        if(matchNodeExpr(nodeExpr, expr)) {
            return true;
        }
    } coEndNodelistNodeExpr;
    if(coDebug) {
        printf("Failed to find a match.\n");
    }
    return false;
}

// See if the exprs match the node rules.
static void matchNoderules(
    coStatement statement)
{
    coExpr expr = coStatementGetFirstExpr(statement);
    coPattern pattern = coStateruleGetPattern(coStatementGetStaterule(statement));
    coElement element;
    coNoderule noderule;
    utSym sym;

    coForeachPatternElement(pattern, element) {
        if(coElementGetKeyword(element) == coKeywordNull) {
            sym = coElementGetSym(element);
            if(sym != coIdentSym) {
                noderule = coSyntaxFindNoderule(coCurrentSyntax, sym);
                if(noderule == coNoderuleNull) {
                    utError("Noderule %s not defined", utSymGetName(sym));
                } else {
                    if(!matchNoderule(noderule, expr)) {
                        coDebug = true;
                        matchNoderule(noderule, expr);
                        coExprError(expr, "Line %u: invalid %s expr", coLineNum,
                            coNoderuleGetName(noderule));
                    }
                }
            } else {
                if(coExprGetType(expr) != CO_EXPR_IDENT) {
                    coExprError(expr, "Line %u: Expected identifier", coLineNum);
                }
            }
            expr = coExprGetNextStatementExpr(expr);
        }
    } coEndPatternElement;
}

// Print out the exprs on the statement.
static void printStatementExprs(
    coStatement statement)
{
    coExpr expr;

    coForeachStatementExpr(statement, expr) {
        coPrintExpr(expr);
        printf(" ");
    } coEndStatementExpr;
    printf("\n");
}

// Remove any trailing comment token from the current input line and return it's string.
static vaString getLineComment(void)
{
    uint32 numTokens = coSyntaxGetUsedToken(coCurrentSyntax);
    coToken token = coSyntaxGetiToken(coCurrentSyntax, numTokens - 1);
    vaString string;

    if(coTokenGetType(token) != CO_TOK_COMMENT) {
        return vaStringNull;
    }
    coSyntaxSetUsedToken(coCurrentSyntax, numTokens - 1);
    string = vaStringCreate(coTokenGetText(token));
    coTokenDestroy(token);
    return string;
}

// Build a new comment statement.
static coStatement buildCommentStatement(
    vaString comment)
{
    coStatement statement = coStatementAlloc();

    coStatementSetType(statement, CO_STATE_COMMENT);
    coStatementAppendStatement(coOuterStatement, statement);
    printf("Comment statement: %s\n", vaStringGetValue(comment));
    coStatementSetComment(statement, comment);
    return statement;
}

// Parse the tokens in the root list and create a statement from them.
static coStatement parseStatement(void)
{
    coStaterule staterule;
    coStatement statement;
    vaString comment = getLineComment();

    if(coSyntaxGetUsedToken(coCurrentSyntax) == 0) {
        // Just a comment statement
        return buildCommentStatement(comment);
    }
    staterule = lookupStaterule();
    if(staterule == coStateruleNull) {
        coError(coSyntaxGetiToken(coCurrentSyntax, 0),
            "Syntax error: statement not recognized");
    }
    printf("Found staterule: ");
    coPrintStaterule(staterule);
    statement = coStatementAlloc();
    coStatementSetType(statement, CO_STATE_USER);
    coStatementAppendStatement(coOuterStatement, statement);
    coStateruleAppendStatement(staterule, statement);
    parseExprs(statement);
    printStatementExprs(statement);
    matchNoderules(statement);
    coStatementSetComment(statement, comment);
    return statement;
}

// Run statement handlers on the module's statements, bottom up, so that
// statements indented more are handled first.
static void handleStatement(
    coStatement statement)
{
    coStaterule staterule = coStatementGetStaterule(statement);
    coStatement subStatement;
    coFuncptr handlerFuncptr;
    coStatementHandler handler = NULL;

    if(staterule == coStateruleNull) {
        return; // Only handle user statements
    }
    handlerFuncptr = coStateruleGetHandlerFuncptr(staterule);
    if(handlerFuncptr != coFuncptrNull) {
        handler = coFuncptrGetValue(handlerFuncptr);
    }
    if(handler != NULL && coStateruleHasBlock(staterule)) {
        handler(statement, true);
    }
    coSafeForeachStatementStatement(statement, subStatement) {
        handleStatement(subStatement);
    } coEndSafeStatementStatement;
    if(handler != NULL) {
        handler(statement, false);
    }
}

// Parse an L42 file.  This is done one statement at a time.  Statements are
// NEWLINE terminated.  Sub-statements are between BEGIN and END tokens.
coModule coParse(
    coModule outerModule,
    utSym moduleName)
{
    coModule module = coModuleCreate(outerModule, moduleName);
    coStatement moduleStatement, statement;

    coCurrentSyntax = coL42Syntax;
    statement = coModuleGetStatement(outerModule);
    moduleStatement = coModuleStatementCreate(statement, module);
    coOuterStatement = moduleStatement;
    coLexerStart();
    coPrevStatement = coStatementNull;
    //coDebug = false;
    coDebug = true;
    coIdentSym = utSymCreate("ident");
    while(readOneLine()) {
        coPrevStatement = parseStatement();
        destroyLineTokens();
    }
    coForeachStatementStatement(moduleStatement, statement) {
        handleStatement(statement);
    } coEndStatementStatement;
    coLexerStop();
    coPrintSyntax(coL42Syntax);
    return module;
}
