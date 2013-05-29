#include "co.h"

paRoot paTheRoot;
paNamespace paTopNamespace;
paModule paTopModule;
paStatement paTopStatement;
paSyntax paL42Syntax;

// Create the root module statement.
void start(
    char *arg0)
{
    char *exeName;

    utStart();
    exeName = utReplaceSuffix(utBaseName(arg0), "");
    utInitLogFile(utSprintf("%s.log", exeName));
    paDatabaseStart();
    vaValueStart();
    paTheRoot = paRootAlloc();
    paTopModule = paModuleCreate(paModuleNull, utSymCreate("top"));
    paTopNamespace = paIdentGetSubNamespace(paModuleGetIdent(paTopModule));
    paTopStatement = paModuleStatementCreate(paStatementNull, paTopModule);
    paCreateBuiltins();
}

static void stop(void)
{
    vaValueStop();
    paDatabaseStop();
    utStop(false);
}

// Main entry point.  Process parameters and call the generators.
int main(
    int argc,
    char *argv[])
{
    paModule module;
    int xArg = 1;

    start(argv[0]);
    if(argc >= 2) {
        for(xArg = 1; xArg < argc; xArg++) {
            module = paParseSourceFile(paTopModule, argv[xArg]);
            paPreprocessDataTypes(module);
            //paGenerateCCode(module, utReplaceSuffix(utBaseName(argv[1]), ".c"));
        }
    } else {
        paInterpreter();
    }
    stop();
    return 0;
}
