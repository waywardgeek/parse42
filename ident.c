// Methods for class Ident.
#include "co.h"

// Create a unique identifier in a namespace.  If it already exists, report an
// error.  The top namespace has an ident, which is not in a namespeace.
coIdent coIdentCreate(
    coNamespace namespace,
    coIdentType type,
    utSym name)
{
    coIdent ident;

    if(namespace != coNamespaceNull) {
        ident = coNamespaceFindIdent(namespace, name);
        if(ident != coIdentNull) {
            utError("Identifier %s already exists in namespace %s",
                utSymGetName(name), coNamespaceGetName(namespace));

        }
    }
    ident = coIdentAlloc();
    coIdentSetType(ident, type);
    coIdentSetSym(ident, name);
    if(namespace != coNamespaceNull) {
       coNamespaceInsertIdent(namespace, ident);
    }
    return ident;
}

// Find the identifier given a dot expression.
coIdent coFindIdent(
    coExpr dotExpr)
{
    coNamespace namespace = coTopNamespace;
    coExpr identExpr;
    coIdent ident;

    if(coExprGetType(dotExpr) == CO_EXPR_IDENT) {
        return coNamespaceFindIdent(namespace, coExprGetSym(dotExpr));
    }
    coForeachExprExpr(dotExpr, identExpr) {
        if(namespace == coNamespaceNull) {
            coExprError(dotExpr, "Undefined identifier");
        }
        if(coExprGetType(identExpr) != CO_EXPR_IDENT) {
            coExprError(dotExpr, "Non-identifier in dot expression");
        }
        ident = coNamespaceFindIdent(namespace, coExprGetSym(identExpr));
        if(ident == coIdentNull) {
            coExprError(dotExpr, "Identifier %s not found",
                utSymGetName(coExprGetSym(identExpr)));
        }
        namespace = coIdentGetSubNamespace(ident);
    } coEndExprExpr;
    return ident;
}
