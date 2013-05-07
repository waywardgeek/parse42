#include "co.h"

coRoot coTheRoot;
coNamespace coTopNamespace;
coModule coTopModule;
coStatement coTopStatement;
coSyntax coL42Syntax;

// Create the root module statement.
void start(
    char *arg0)
{
    char *exeName;

    utStart();
    exeName = utReplaceSuffix(utBaseName(arg0), "");
    utInitLogFile(utSprintf("%s.log", exeName));
    coDatabaseStart();
    vaValueStart();
    coTheRoot = coRootAlloc();
    coTopModule = coModuleCreate(coModuleNull, utSymCreate("top"));
    coTopNamespace = coIdentGetSubNamespace(coModuleGetIdent(coTopModule));
    coTopStatement = coModuleStatementCreate(coStatementNull, coTopModule);
    coCreateBuiltins();
}

static void stop(void)
{
    vaValueStop();
    coDatabaseStop();
    utStop(false);
}

// Main entry point.  Process parameters and call the generators.
int main(
    int argc,
    char *argv[])
{
    coModule module;
    int xArg = 1;

    start(argv[0]);
    if(argc >= 2) {
        for(xArg = 1; xArg < argc; xArg++) {
            module = coParseSourceFile(coTopModule, argv[xArg]);
            coPreprocessDataTypes(module);
            //coGenerateCCode(module, utReplaceSuffix(utBaseName(argv[1]), ".c"));
        }
    } else {
        coInterpreter();
    }
    stop();
    return 0;
}
