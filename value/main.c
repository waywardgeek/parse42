#include "value.h"

// Main entry point.  Process parameters and call the generators.
int main(
    int argc,
    char *argv[])
{
    utStart();
    vaValueStart();
    vaBencodeSelfTest();
    vaValueStop();
    utStop(false);
    return 0;
}
