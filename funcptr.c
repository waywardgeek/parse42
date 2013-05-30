// Methods for function pointers.
#include "pa.h"

// Create a function pointer object.
paFuncptr paFuncptrCreate(
    utSym sym,
    paStatementHandler handler)
{
    paFuncptr funcptr = paFuncptrAlloc();

    if(handler == NULL) {
        utExit("NULL not valid funcptr");
    }
    paFuncptrSetValue(funcptr, handler);
    paFuncptrSetSym(funcptr, sym);
    paRootAppendFuncptr(paTheRoot, funcptr);
    return funcptr;
}

