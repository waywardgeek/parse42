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

static paStatement paOuterStatement, paPrevStatement;
static bool paDebug;

// Print out an error message and exit.
void paExprError(
    paExpr expr,
    char *message,
    ...)
{
    char *buff;
    va_list ap;

    va_start(ap, message);
    buff = utVsprintf((char *)message, ap);
    va_end(ap);
    utError("Line %d: %s", paExprGetLineNum(expr), buff);
}

// Read one line of tokens.  It is up to the caller to destroy these tokens.
static bool readOneLine(void)
{
    printf("Reading one line\n");
    paSyntax syntax;
    paStaterule staterule;
    paToken token = paLex();
    utSym subSyntaxSym;

    if(token == paTokenNull) {
        return false;
    }
    // Now deal with BEGIN/END tokens.
    if(paTokenGetType(token) == PA_TOK_BEGIN) {
        printf("Starting sub-statements\n");
        if(paPrevStatement == paStatementNull) {
            paError(token, "First line may not be intented");
        }
        paOuterStatement = paPrevStatement;
        subSyntaxSym = paStateruleGetSubSyntaxSym(paStatementGetStaterule(paOuterStatement));
        if(subSyntaxSym != utSymNull) {
            syntax = paRootFindSyntax(paTheRoot, subSyntaxSym);
            if(syntax != paSyntaxNull) {
                printf("Using syntax %s\n", paSyntaxGetName(syntax));
                paCurrentSyntax = syntax;
            } else {
                paError(token, "Syntax %s not found", utSymGetName(subSyntaxSym));
            }
        }
        paTokenDestroy(token);
        token = paLex();
    }
    while(token != paTokenNull && paTokenGetType(token) == PA_TOK_END) {
        printf("Finished sub-statements\n");
        paOuterStatement = paStatementGetStatement(paOuterStatement);
        staterule = paStatementGetStaterule(paOuterStatement);
        if(staterule == paStateruleNull) {
            syntax = paL42Syntax;
        } else {
            syntax = paRootFindSyntax(paTheRoot, paStateruleGetSubSyntaxSym(staterule));
            if(syntax == paSyntaxNull) {
                syntax = paStateruleGetSyntax(staterule);
            }
        }
        if(syntax != paCurrentSyntax) {
            printf("Using syntax %s\n", paSyntaxGetName(syntax));
            paCurrentSyntax = syntax;
        }
        paTokenDestroy(token);
        token = paLex();
    }
    while(token != paTokenNull && paTokenGetType(token) != PA_TOK_NEWLINE) {
        if(paTokenGetType(token) == PA_TOK_CHAR) {
            paError(token, "Illegal character in input");
        }
        paSyntaxAppendToken(paCurrentSyntax, token);
        paPrintToken(token);
        token = paLex();
    }
    if(token != paTokenNull) {
        paTokenDestroy(token); // We no longer need the NEWLINE token
    }
    return paSyntaxGetUsedToken(paCurrentSyntax) > 0;
}

// Destroy the tokens in the token list.
static void destroyLineTokens(void)
{
    paToken token;

    paForeachSyntaxToken(paCurrentSyntax, token) {
        paTokenDestroy(token);
    } paEndSyntaxToken;
    paSyntaxSetUsedToken(paCurrentSyntax, 0);
}

// Find the statement rule matching the current tokens.  We use paKeywordNull to
// hold he place of exprs.
static paStaterule lookupStaterule(void)
{
    paKeyword keywords[paSyntaxGetUsedToken(paCurrentSyntax)];
    paToken token;
    uint32 numKeywords = 0;
    bool lastWasExpr = false;

    paForeachSyntaxToken(paCurrentSyntax, token) {
        if(paTokenGetType(token) == PA_TOK_KEYWORD) {
            keywords[numKeywords++] = paTokenGetKeywordVal(token);
            lastWasExpr = false;
        } else {
            if(!lastWasExpr) {
                keywords[numKeywords++] = paKeywordNull;
            }
            lastWasExpr = true;
        }
    } paEndSyntaxToken;
    return paSyntaxFindStaterule(paCurrentSyntax, keywords, numKeywords);
}

// Determine if the operator matches the occurrence.  We match if the pattern
// has an expr or not matching hasLeftExpr, and we require the first set of
// contiguous keywords to match the tokens.
static bool patternMatches(
    paPattern pattern,
    paToken *tokens,
    uint32 numTokens,
    bool hasLeftExpr)
{
    paElement element = paPatternGetFirstElement(pattern);
    paToken token;
    uint32 tokenPos = 0;

    if(hasLeftExpr == (paElementGetKeyword(element) != paKeywordNull)) {
        return false;
    }
    if(hasLeftExpr) {
        element = paElementGetNextPatternElement(element);
    }
    while(element != paElementNull && paElementGetKeyword(element) != paKeywordNull) {
        if(tokenPos == numTokens) {
            return false;
        }
        token = tokens[tokenPos++];
        if(paTokenGetType(token) != PA_TOK_OPERATOR ||
                paElementGetKeyword(element) != paTokenGetKeywordVal(token)) {
            return false;
        }
        element = paElementGetNextPatternElement(element);
    }
    return true;
}

// Find the operator given the left hand expr, and the tokens.
static paOperator findOperator(
    paExpr leftExpr,
    paToken *tokens,
    uint32 numTokens,
    paKeyword endKeyword)
{
    paOperator operator;
    paPattern pattern;
    paElement element;
    paKeyword keyword;
    paToken token = tokens[0];
    bool hasLeftExpr = leftExpr != paExprNull;

    if(numTokens == 0) {
        return paOperatorNull;
    }
    if(leftExpr == paExprNull && paTokenGetType(token) != PA_TOK_OPERATOR) {
        if(numTokens == 1) {
            return paOperatorNull;
        }
        tokens++;
        numTokens--;
        token = tokens[0];
        hasLeftExpr = true;
    }
    keyword = paTokenGetKeywordVal(token);
    if(keyword != paKeywordNull && keyword == endKeyword) {
        return paOperatorNull;
    }
    if(paTokenGetType(token) != PA_TOK_OPERATOR) {
        if(!hasLeftExpr) {
            return paOperatorNull;
        }
        // This is the case where we have two exprs in series with no operator.
        return paSyntaxGetConcatenationOperator(paCurrentSyntax);
    }
    paForeachKeywordElement(keyword, element) {
        pattern = paElementGetPattern(element);
        operator = paPatternGetOperator(pattern);
        if(operator != paOperatorNull && patternMatches(pattern, tokens, numTokens,
                hasLeftExpr)) {
            return operator;
        }
    } paEndKeywordElement;
    paError(token, "Invalid operator");
    return paOperatorNull;
}

// Build an expr for a non-operator token.
static paExpr buildPrimaryExpr(
    paToken token)
{
    paExpr expr;

    switch(paTokenGetType(token)) {
    case PA_TOK_INTEGER:
        expr = paValueExprCreate(vaIntValueCreate(paTokenGetIntVal(token)));
        break;
    case PA_TOK_FLOAT:
        expr = paValueExprCreate(vaFloatValueCreate(paTokenGetFloatVal(token)));
        break;
    case PA_TOK_STRING:
        expr =  paValueExprCreate(vaStringValueCreate(
            vaStringCreate(paTokenGetText(token))));
        break;
    case PA_TOK_IDENT:
        expr = paIdentExprCreate(utSymCreate((char *)paTokenGetText(token)));
        break;
    default:
        utExit("Unknown token type");
    }
    paExprSetLineNum(expr, paTokenGetLineNum(token));
    return expr;
}

// Find the next keyword after this element in an operator pattern.  Operators
// with keywords must have alternating operators and exprs.
static paKeyword findNextKeyword(
    paElement element)
{
    element = paElementGetNextPatternElement(element);
    if(element == paElementNull || paElementGetKeyword(element) == paKeywordNull) {
        return paKeywordNull;
    }
    return paElementGetKeyword(element);
}

// Merge the source expr into the dest.
static void mergeExprs(
    paExpr source,
    paExpr dest)
{
    paExpr subExpr;

    printf("Merging %s operators\n", paOperatorGetName(paExprGetOperator(dest)));
    paSafeForeachExprExpr(source, subExpr) {
        paExprRemoveExpr(source, subExpr);
        paExprAppendExpr(dest, subExpr);
    } paEndSafeExprExpr;
    paExprDestroy(source);
}

// Just a forward declaration for double recursion.
static paExpr parseExpr(paToken *tokens, uint32 numTokens, paKeyword
    endKeyword, uint32 *tokensParsed);

// Parse one expr at the given precedence.  The left expr exists
// unless it's a prefix operator.
static paExpr parseSubExpr(
    paExpr leftExpr,
    paToken *tokens,
    uint32 numTokens,
    uint32 precedence,
    paKeyword endKeyword,
    uint32 *tokensParsed)
{
    paOperator operator;
    paElement element;
    paKeyword keyword, nextEndKeyword;
    paExpr expr = paExprNull, subExpr;
    uint32 tokenPos = 0;
    uint32 opPrecedence;
    bool firstTime;
    bool isConcatenation;

    utDo {
        operator = findOperator(leftExpr, tokens + tokenPos, numTokens - tokenPos, endKeyword);
        if(operator == paOperatorNull || precedence >
                paPrecedenceGroupGetPrecedence(paOperatorGetPrecedenceGroup(operator))) {
            if(leftExpr != paExprNull) {
                *tokensParsed = tokenPos;
                return leftExpr;
            }
            if(operator == paOperatorNull || paTokenGetType(tokens[0]) != PA_TOK_OPERATOR) {
                *tokensParsed = 1;
                return buildPrimaryExpr(tokens[0]);
            }
        }
        isConcatenation = operator == paSyntaxGetConcatenationOperator(paCurrentSyntax);
        opPrecedence = paPrecedenceGroupGetPrecedence(paOperatorGetPrecedenceGroup(operator));
    } utWhile(expr == paExprNull || opPrecedence >= precedence) {
        if(paOperatorGetType(operator) == PA_OP_MERGE && leftExpr != paExprNull &&
                operator == paExprGetOperator(leftExpr)) {
            expr = leftExpr;
            leftExpr = paExprNull;
            printf("Merging %s operators\n", paOperatorGetName(operator));
        } else {
            expr = paOperatorExprCreate(operator);
            paExprSetLineNum(expr, paTokenGetLineNum(tokens[tokenPos]));
        }
        firstTime = true;
        paForeachPatternElement(paOperatorGetPattern(operator), element) {
            keyword = paElementGetKeyword(element);
            if(keyword == paKeywordNull) {
                // Must be expr
                if(firstTime && leftExpr != paExprNull) {
                    subExpr = leftExpr;
                } else {
                    nextEndKeyword = findNextKeyword(element);
                    if(isConcatenation && firstTime) {
                        subExpr = parseExpr(tokens + tokenPos, 1,
                            nextEndKeyword, tokensParsed);
                    } else if(nextEndKeyword != paKeywordNull) {
                        subExpr = parseExpr(tokens + tokenPos, numTokens - tokenPos,
                            nextEndKeyword, tokensParsed);
                    } else {
                        subExpr = parseSubExpr(paExprNull, tokens + tokenPos,
                            numTokens - tokenPos, opPrecedence, endKeyword, tokensParsed);
                    }
                    tokenPos += *tokensParsed;
                }
                if(paOperatorGetType(operator) == PA_OP_MERGE &&
                        operator == paExprGetOperator(subExpr)) {
                    mergeExprs(subExpr, expr);
                } else {
                    paExprAppendExpr(expr, subExpr);
                }
            } else {
                if(paTokenGetKeywordVal(tokens[tokenPos]) != keyword) {
                    paError(tokens[tokenPos], "Expected operator %s",
                        paKeywordGetName(keyword));
                }
                tokenPos++;
            }
            firstTime = false;
        } paEndPatternElement;
        leftExpr = expr;
    } utRepeat;
    *tokensParsed = tokenPos;
    return expr;
}

// Recursively parse the expr.
static paExpr parseExpr(
    paToken *tokens,
    uint32 numTokens,
    paKeyword endKeyword,
    uint32 *tokensParsed)
{
    paExpr expr = parseSubExpr(paExprNull, tokens,
            numTokens, 0, endKeyword, tokensParsed);

    if(*tokensParsed != numTokens && (endKeyword == paKeywordNull ||
            paTokenGetKeywordVal(tokens[*tokensParsed]) != endKeyword)) {
        paError(tokens[*tokensParsed], "Unable to parse entire expr");
    }
    return expr;
}

// Parse the exprs for the statement.
static void parseExprs(
    paStatement statement)
{
    paExpr expr;
    paToken tokens[paSyntaxGetUsedToken(paCurrentSyntax)];
    paToken token;
    uint32 numTokens = 0;
    uint32 tokensParsed;

    paForeachSyntaxToken(paCurrentSyntax, token) {
        if(paTokenGetType(token) == PA_TOK_KEYWORD) {
            if(numTokens > 0) {
                expr = parseExpr(tokens, numTokens, paTokenGetKeywordVal(token),
                    &tokensParsed);
                paStatementAppendExpr(statement, expr);
            }
            numTokens = 0;
        } else {
            tokens[numTokens++] = token;
        }
    } paEndSyntaxToken;
    if(numTokens > 0) {
        expr = parseExpr(tokens, numTokens, paTokenGetKeywordVal(token),
            &tokensParsed);
        paStatementAppendExpr(statement, expr);
    }
}

static bool matchNoderule(paNoderule noderule, paExpr expr);

// Match the noderule to the expr.
static bool matchNodeExpr(
    paNodeExpr nodeExpr,
    paExpr expr)
{
    paNoderule noderule;
    paExpr subExpr;
    paNodeExpr subNode;
    paExprType type = paExprGetType(expr);
    utSym sym = paNodeExprGetSym(nodeExpr);

    if(paDebug) {
        printf("Matching nodeExpr ");
        paPrintNodeExpr(nodeExpr);
        printf(" to ");
        paPrintExpr(expr);
        printf("\n");
    }
    switch(paNodeExprGetType(nodeExpr)) {
    case PA_NODEEXPR_NODERULE:
        noderule = paSyntaxFindNoderule(paCurrentSyntax, sym);
        if(noderule == paNoderuleNull) {
            utExit("Noderule %s not found", utSymGetName(sym));
        }
        return matchNoderule(noderule, expr);
    case PA_NODEEXPR_OPERATOR:
        if(type != PA_EXPR_OPERATOR ||
                paOperatorGetSym(paExprGetOperator(expr)) != sym) {
            return false;
        }
        subExpr = paExprGetFirstExpr(expr);
        paForeachNodeExprNodeExpr(nodeExpr, subNode) {
            if(subExpr == paExprNull || !matchNodeExpr(subNode, subExpr)) {
                return false;
            }
            subExpr = paExprGetNextExprExpr(subExpr);
        } paEndNodeExprNodeExpr;
        return subExpr == paExprNull;
    case PA_NODEEXPR_LISTOPERATOR:
        if(type != PA_EXPR_OPERATOR ||
                paOperatorGetSym(paExprGetOperator(expr)) != sym) {
            return false;
        }
        subNode = paNodeExprGetFirstNodeExpr(nodeExpr);
        paForeachExprExpr(expr, subExpr) {
            if(!matchNodeExpr(subNode, subExpr)) {
                return false;
            }
        } paEndExprExpr;
        return true;
    case PA_NODEEXPR_INTEGER:
        return type == PA_EXPR_VALUE &&
            vaValueGetType(paExprGetValue(expr)) == VA_INT;
    case PA_NODEEXPR_FLOAT:
        return type == PA_EXPR_VALUE && vaValueGetType(paExprGetValue(expr)) == VA_FLOAT;
    case PA_NODEEXPR_STRING:
        return type == PA_EXPR_VALUE && vaValueGetType(paExprGetValue(expr)) == VA_STRING;
    case PA_NODEEXPR_IDENT:
        return type == PA_EXPR_IDENT;
    case PA_NODEEXPR_CONSTIDENT:
        if(type != PA_EXPR_VALUE || vaValueGetType(paExprGetValue(expr)) != VA_STRING) {
            return false;
        }
        return !strcmp((char *)vaStringGetValue(vaValueGetStringVal(paExprGetValue(expr))),
            utSymGetName(paNodeExprGetSym(nodeExpr)));
    case PA_NODEEXPR_EXPR:
        utError("Invalid use of 'expr' in a node rule");
        break;
    default:
        utExit("Unknown nodeExpr type");
    }
    return false; // Never gets here
}

// Find the node expression sym corresponding to this expressin.
static utSym findExprSym(
    paExpr expr)
{
    paExprType type = paExprGetType(expr);

    if(type == PA_EXPR_IDENT) {
        return paIdentSym;
    } else if(type == PA_EXPR_OPERATOR) {
        return paOperatorGetSym(paExprGetOperator(expr));
    } else if(type == PA_EXPR_VALUE) {
        switch(vaValueGetType(paExprGetValue(expr))) {
        case VA_INT:
            return paIntegerSym;
        case VA_FLOAT:
            return paFloatSym;
        case VA_STRING:
            return paStringSym;
        case VA_BOOL:
            return paBoolSym;
        default:
            utExit("Unknown value type");
        }
    }
    return utSymNull;
}

// Match the noderule to the expr.
static bool matchNoderule(
    paNoderule noderule,
    paExpr expr)
{
    paNodeExpr nodeExpr;
    paNodelist nodelist;
    paExprType type = paExprGetType(expr);
    utSym sym = utSymNull;

    if(paDebug) {
        printf("Trying noderule ");
        paPrintNoderule(noderule);
    }
    sym = findExprSym(expr);
    nodelist = paNoderuleFindNodelist(noderule, type, sym);
    if(nodelist == paNodelistNull) {
        if(paDebug) {
            printf("No noderule found!\n");
        }
        return false;
    }
    paForeachNodelistNodeExpr(nodelist, nodeExpr) {
        if(matchNodeExpr(nodeExpr, expr)) {
            return true;
        }
    } paEndNodelistNodeExpr;
    if(paDebug) {
        printf("Failed to find a match.\n");
    }
    return false;
}

// See if the exprs match the node rules.
static void matchNoderules(
    paStatement statement)
{
    paExpr expr = paStatementGetFirstExpr(statement);
    paPattern pattern = paStateruleGetPattern(paStatementGetStaterule(statement));
    paElement element;
    paNoderule noderule;
    utSym sym;

    paForeachPatternElement(pattern, element) {
        if(paElementGetKeyword(element) == paKeywordNull) {
            sym = paElementGetSym(element);
            if(sym != paIdentSym) {
                noderule = paSyntaxFindNoderule(paCurrentSyntax, sym);
                if(noderule == paNoderuleNull) {
                    utError("Noderule %s not defined", utSymGetName(sym));
                } else {
                    if(!matchNoderule(noderule, expr)) {
                        paDebug = true;
                        matchNoderule(noderule, expr);
                        paExprError(expr, "Line %u: invalid %s expr", paLineNum,
                            paNoderuleGetName(noderule));
                    }
                }
            } else {
                if(paExprGetType(expr) != PA_EXPR_IDENT) {
                    paExprError(expr, "Line %u: Expected identifier", paLineNum);
                }
            }
            expr = paExprGetNextStatementExpr(expr);
        }
    } paEndPatternElement;
}

// Print out the exprs on the statement.
static void printStatementExprs(
    paStatement statement)
{
    paExpr expr;

    paForeachStatementExpr(statement, expr) {
        paPrintExpr(expr);
        printf(" ");
    } paEndStatementExpr;
    printf("\n");
}

// Remove any trailing comment token from the current input line and return it's string.
static vaString getLineComment(void)
{
    uint32 numTokens = paSyntaxGetUsedToken(paCurrentSyntax);
    paToken token = paSyntaxGetiToken(paCurrentSyntax, numTokens - 1);
    vaString string;

    if(paTokenGetType(token) != PA_TOK_COMMENT) {
        return vaStringNull;
    }
    paSyntaxSetUsedToken(paCurrentSyntax, numTokens - 1);
    string = vaStringCreate(paTokenGetText(token));
    paTokenDestroy(token);
    return string;
}

// Build a new comment statement.
static paStatement buildCommentStatement(
    vaString comment)
{
    paStatement statement = paStatementAlloc();

    paStatementSetType(statement, PA_STATE_COMMENT);
    paStatementAppendStatement(paOuterStatement, statement);
    printf("Comment statement: %s\n", vaStringGetValue(comment));
    paStatementSetComment(statement, comment);
    return statement;
}

// Parse the tokens in the root list and create a statement from them.
static paStatement parseStatement(void)
{
    paStaterule staterule;
    paStatement statement;
    vaString comment = getLineComment();

    if(paSyntaxGetUsedToken(paCurrentSyntax) == 0) {
        // Just a comment statement
        return buildCommentStatement(comment);
    }
    staterule = lookupStaterule();
    if(staterule == paStateruleNull) {
        paError(paSyntaxGetiToken(paCurrentSyntax, 0),
            "Syntax error: statement not recognized");
    }
    printf("Found staterule: ");
    paPrintStaterule(staterule);
    statement = paStatementAlloc();
    paStatementSetType(statement, PA_STATE_USER);
    paStatementAppendStatement(paOuterStatement, statement);
    paStateruleAppendStatement(staterule, statement);
    parseExprs(statement);
    printStatementExprs(statement);
    matchNoderules(statement);
    paStatementSetComment(statement, comment);
    return statement;
}

// Run statement handlers on the module's statements, bottom up, so that
// statements indented more are handled first.
static void handleStatement(
    paStatement statement)
{
    paStaterule staterule = paStatementGetStaterule(statement);
    paStatement subStatement;
    paFuncptr handlerFuncptr;
    paStatementHandler handler = NULL;

    if(staterule == paStateruleNull) {
        return; // Only handle user statements
    }
    handlerFuncptr = paStateruleGetHandlerFuncptr(staterule);
    if(handlerFuncptr != paFuncptrNull) {
        handler = paFuncptrGetValue(handlerFuncptr);
    }
    if(handler != NULL && paStateruleHasBlock(staterule)) {
        handler(statement, true);
    }
    paSafeForeachStatementStatement(statement, subStatement) {
        handleStatement(subStatement);
    } paEndSafeStatementStatement;
    if(handler != NULL) {
        handler(statement, false);
    }
}

// Parse an L42 file.  This is done one statement at a time.  Statements are
// NEWLINE terminated.  Sub-statements are between BEGIN and END tokens.
paModule paParse(
    paModule outerModule,
    utSym moduleName)
{
    paModule module = paModuleCreate(outerModule, moduleName);
    paStatement moduleStatement, statement;

    paCurrentSyntax = paL42Syntax;
    statement = paModuleGetStatement(outerModule);
    moduleStatement = paModuleStatementCreate(statement, module);
    paOuterStatement = moduleStatement;
    paLexerStart();
    paPrevStatement = paStatementNull;
    //paDebug = false;
    paDebug = true;
    paIdentSym = utSymCreate("ident");
    while(readOneLine()) {
        paPrevStatement = parseStatement();
        destroyLineTokens();
    }
    paForeachStatementStatement(moduleStatement, statement) {
        handleStatement(statement);
    } paEndStatementStatement;
    paLexerStop();
    paPrintSyntax(paL42Syntax);
    return module;
}
