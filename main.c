#include "pa.h"

// Create the root module statement.
void start(
    char *arg0)
{
    char *exeName;

    utStart();
    exeName = utReplaceSuffix(utBaseName(arg0), "");
    utInitLogFile(utSprintf("%s.log", exeName));
    paDatabaseStart();
    paTheRoot = paRootAlloc();
    vaValueStart();
    paSyntaxStart();
    paCreateBuiltins();
}

static void stop(void)
{
    paSyntaxStop();
    vaValueStop();
    paDatabaseStop();
    utStop(false);
}

// Main entry point.  Process parameters and call the generators.
int main(
    int argc,
    char *argv[])
{
    paSyntax syntax;
    paStatement statement;
    int xArg = 1;

    start(argv[0]);
    if(utSetjmp()) {
        printf("Error occured.\n");
        stop();
        return 1;
    }
    if(argc < 2) {
        printf("Usage: parse42 rulesFile [dataFile...]\n");
        return 1;
    }
    statement = paParseSourceFile(paParseSyntax, argv[1]);
    syntax = paSyntaxCreate(utSymCreate(utReplaceSuffix(argv[xArg], "")));
    paProcessSyntaxStatement(syntax, statement);
    for(xArg = 2; xArg < argc; xArg++) {
        statement = paParseSourceFile(syntax, argv[xArg]);
    }
    utUnsetjmp();
    stop();
    return 0;
}
