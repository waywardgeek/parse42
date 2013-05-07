// Expresion methods.
#include "co.h"

// Print the operator expr.
static void printOperatorExpr(
    coExpr expr)
{
    coOperator operator = coExprGetOperator(expr);
    coExpr subExpr = coExprGetFirstExpr(expr);
    coElement element;
    coKeyword keyword;
    bool firstTime = true;

    if(coOperatorGetType(operator) == CO_OP_MERGE) {
        printf("%s[", coOperatorGetName(operator));
        coForeachExprExpr(expr, subExpr) {
            if(!firstTime) {
                printf(" ");
            }
            firstTime = false;
            coPrintExpr(subExpr);
        } coEndExprExpr;
        printf("]");
    } else {
        printf("%s(", coOperatorGetName(operator));
        coForeachPatternElement(coOperatorGetPattern(operator), element) {
            keyword = coElementGetKeyword(element);
            if(keyword == coKeywordNull) {
                if(!firstTime) {
                    printf(" ");
                }
                firstTime = false;
                coPrintExpr(subExpr);
                subExpr = coExprGetNextExprExpr(subExpr);
            }
        } coEndPatternElement;
        if(subExpr != coExprNull) {
            utError("Malformed %s expr", coOperatorGetName(operator));
        }
        printf(")");
    }
}

// Print the expr.
void coPrintExpr(
    coExpr expr)
{
    switch(coExprGetType(expr)) {
    case CO_EXPR_TUPLE:
    case CO_EXPR_LIST:
    case CO_EXPR_CONDITIONAL:
    case CO_EXPR_OR:
    case CO_EXPR_AND:
    case CO_EXPR_XOR:
    case CO_EXPR_NOT:
    case CO_EXPR_LESS_THAN:
    case CO_EXPR_GREATER_THAN:
    case CO_EXPR_EQUAL:
    case CO_EXPR_GREATER_OR_EQUAL:
    case CO_EXPR_LESS_OR_EQUAL:
    case CO_EXPR_NOT_EQUAL:
    case CO_EXPR_BITOR:
    case CO_EXPR_BITXOR:
    case CO_EXPR_BITAND:
    case CO_EXPR_SHIFT_LEFT:
    case CO_EXPR_SHIFT_RIGHT:
    case CO_EXPR_ADD:
    case CO_EXPR_SUBTRACT:
    case CO_EXPR_COMPLIMENT:
    case CO_EXPR_MULTIPLY:
    case CO_EXPR_DIVIDE:
    case CO_EXPR_MODULUS:
    case CO_EXPR_NEGATE:
    case CO_EXPR_POWER:
    case CO_EXPR_INDEX:
    case CO_EXPR_DOT:
    case CO_EXPR_CALL:
    case CO_EXPR_ASSIGN:
    case CO_EXPR_PRE_PLUSPLUS:
    case CO_EXPR_POST_PLUSPLUS:
    case CO_EXPR_PRE_MINUSMINUS:
    case CO_EXPR_POST_MINUSMINUS:
        printf("Bill: finisih print expr\n");
        break;
    case CO_EXPR_VALUE:
        vaPrintValue(coExprGetValue(expr));
        break;
    case CO_EXPR_IDENT:
        printf("%s", utSymGetName(coExprGetSym(expr)));
        break;
    case CO_EXPR_OPERATOR:
        printOperatorExpr(expr);
        break;
    default:
        utExit("Unknown expr type");
    }
}

// Create an expr.
coExpr coExprCreate(
    coExprType type)
{
    coExpr expr = coExprAlloc();

    coExprSetType(expr, type);
    return expr;
}

// Create a unary expresion.
coExpr coUnaryExprCreate(
    coExprType type,
    coExpr a)
{
    coExpr expr = coExprCreate(type);

    coExprAppendExpr(expr, a);
    return expr;
}

// Create a binary expr.
coExpr coBinaryExprCreate(
    coExprType type,
    coExpr a,
    coExpr b)
{
    coExpr expr = coExprCreate(type);

    coExprAppendExpr(expr, a);
    coExprAppendExpr(expr, b);
    return expr;
}

// Create an expr with three children.
coExpr coTrinaryExprCreate(
    coExprType type,
    coExpr a,
    coExpr b,
    coExpr c)
{
    coExpr expr = coExprCreate(type);

    coExprAppendExpr(expr, a);
    coExprAppendExpr(expr, b);
    coExprAppendExpr(expr, c);
    return expr;
}

// Create a constant expr.
coExpr coValueExprCreate(
    vaValue value)
{
    coExpr expr = coExprCreate(CO_EXPR_VALUE);

    coExprSetValue(expr, value);
    return expr;
}

// Create an identifier expr.
coExpr coIdentExprCreate(
    utSym sym)
{
    coExpr expr = coExprCreate(CO_EXPR_IDENT);

    coExprSetSym(expr, sym);
    return expr;
}

// Create an expr using an operator object.
coExpr coOperatorExprCreate(
    coOperator operator)
{
    coExpr expr = coExprCreate(CO_EXPR_OPERATOR);

    coOperatorAppendExpr(operator, expr);
    return expr;
}
