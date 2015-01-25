// Expresion methods.
#include "pa.h"

// Print the operator expr.
static void printOperatorExpr(
    paExpr expr)
{
    paOperator operator = paExprGetOperator(expr);
    paExpr subExpr = paExprGetFirstExpr(expr);
    paElement element;
    paKeyword keyword;
    bool firstTime = true;

    if(paOperatorGetType(operator) == PA_OP_MERGE) {
        printf("%s[", paOperatorGetName(operator));
        paForeachExprExpr(expr, subExpr) {
            if(!firstTime) {
                printf(" ");
            }
            firstTime = false;
            paPrintExpr(subExpr);
        } paEndExprExpr;
        printf("]");
    } else {
        printf("%s(", paOperatorGetName(operator));
        paForeachPatternElement(paOperatorGetPattern(operator), element) {
            keyword = paElementGetKeyword(element);
            if(keyword == paKeywordNull) {
                if(!firstTime) {
                    printf(" ");
                }
                firstTime = false;
                paPrintExpr(subExpr);
                subExpr = paExprGetNextExprExpr(subExpr);
            }
        } paEndPatternElement;
        if(subExpr != paExprNull) {
            utError("Malformed %s expr", paOperatorGetName(operator));
        }
        printf(")");
    }
}

// Print the expr.
void paPrintExpr(
    paExpr expr)
{
    switch(paExprGetType(expr)) {
    case PA_EXPR_VALUE:
        vaPrintValue(paExprGetValue(expr));
        break;
    case PA_EXPR_IDENT:
        printf("%s", utSymGetName(paExprGetSym(expr)));
        break;
    case PA_EXPR_OPERATOR:
        printOperatorExpr(expr);
        break;
    default:
        utExit("Unknown expr type");
    }
}

// Create an expr.
paExpr paExprCreate(
    paExprType type)
{
    paExpr expr = paExprAlloc();

    paExprSetType(expr, type);
    return expr;
}

// Create a constant expr.
paExpr paValueExprCreate(
    vaValue value)
{
    paExpr expr = paExprCreate(PA_EXPR_VALUE);

    paExprSetValue(expr, value);
    return expr;
}

// Create an identifier expr.
paExpr paIdentExprCreate(
    utSym sym)
{
    paExpr expr = paExprCreate(PA_EXPR_IDENT);

    paExprSetSym(expr, sym);
    return expr;
}

// Create an expr using an operator object.
paExpr paOperatorExprCreate(
    paOperator operator)
{
    paExpr expr = paExprCreate(PA_EXPR_OPERATOR);

    paOperatorAppendExpr(operator, expr);
    return expr;
}
