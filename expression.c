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
    case PA_EXPR_TUPLE:
    case PA_EXPR_LIST:
    case PA_EXPR_CONDITIONAL:
    case PA_EXPR_OR:
    case PA_EXPR_AND:
    case PA_EXPR_XOR:
    case PA_EXPR_NOT:
    case PA_EXPR_LESS_THAN:
    case PA_EXPR_GREATER_THAN:
    case PA_EXPR_EQUAL:
    case PA_EXPR_GREATER_OR_EQUAL:
    case PA_EXPR_LESS_OR_EQUAL:
    case PA_EXPR_NOT_EQUAL:
    case PA_EXPR_BITOR:
    case PA_EXPR_BITXOR:
    case PA_EXPR_BITAND:
    case PA_EXPR_SHIFT_LEFT:
    case PA_EXPR_SHIFT_RIGHT:
    case PA_EXPR_ADD:
    case PA_EXPR_SUBTRACT:
    case PA_EXPR_COMPLIMENT:
    case PA_EXPR_MULTIPLY:
    case PA_EXPR_DIVIDE:
    case PA_EXPR_MODULUS:
    case PA_EXPR_NEGATE:
    case PA_EXPR_POWER:
    case PA_EXPR_INDEX:
    case PA_EXPR_DOT:
    case PA_EXPR_CALL:
    case PA_EXPR_ASSIGN:
    case PA_EXPR_PRE_PLUSPLUS:
    case PA_EXPR_POST_PLUSPLUS:
    case PA_EXPR_PRE_MINUSMINUS:
    case PA_EXPR_POST_MINUSMINUS:
        printf("Bill: finisih print expr\n");
        break;
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

// Create a unary expresion.
paExpr paUnaryExprCreate(
    paExprType type,
    paExpr a)
{
    paExpr expr = paExprCreate(type);

    paExprAppendExpr(expr, a);
    return expr;
}

// Create a binary expr.
paExpr paBinaryExprCreate(
    paExprType type,
    paExpr a,
    paExpr b)
{
    paExpr expr = paExprCreate(type);

    paExprAppendExpr(expr, a);
    paExprAppendExpr(expr, b);
    return expr;
}

// Create an expr with three children.
paExpr paTrinaryExprCreate(
    paExprType type,
    paExpr a,
    paExpr b,
    paExpr c)
{
    paExpr expr = paExprCreate(type);

    paExprAppendExpr(expr, a);
    paExprAppendExpr(expr, b);
    paExprAppendExpr(expr, c);
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
