#include "pa.h"

paRoot paTheRoot;

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
    int xArg = 1;

    start(argv[0]);
    if(argc < 2) {
        printf("Usage: parse42 file...\n");
        return 1;
    }
    for(xArg = 1; xArg < argc; xArg++) {
        paParseSourceFile(argv[xArg]);
    }
    stop();
    return 0;
}
