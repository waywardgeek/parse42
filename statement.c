// Statement methods.
#include "pa.h"

// Create a new statement.
paStatement paStatementCreate(
    paStatement outerStatement,
    paStaterule staterule)
{
    paStatement statement = paStatementAlloc();

    if(outerStatement != paStatementNull) {
        paStatementAppendStatement(outerStatement, statement);
    }
    if(staterule != paStateruleNull) {
        paStateruleAppendStatement(staterule, statement);
    }
    return statement;
}

// Create a comment statement.
paStatement paCommentStatementCreate(
    paStatement outerStatement,
    vaString comment)
{
    paStatement statement = paStatementCreate(outerStatement, paStateruleNull);

    paStatementSetIsComment(statement, true);
    paStatementSetComment(statement, comment);
    return statement;
}
