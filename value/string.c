#include "value.h"

// Create a new string object.  If one with the same value already exists, just
// return it.
vaString vaStringCreate(
    uchar *value)
{
    uint32 length = strlen((char *)value) + 1;
    vaString string = vaRootFindString(vaTheRoot, value, length);

    if(string != vaStringNull) {
        return string;
    }
    string = vaStringAlloc();
    vaStringSetValue(string, value, length);
    vaRootInsertString(vaTheRoot, string);
    return string;
}

// Concatenate two strings.  Null strings are allowed.
vaString vaStrcat(
    vaString string1,
    vaString string2)
{
    if(string1 == vaStringNull) {
        return string2;
    } else if(string2 == vaStringNull) {
        return string1;
    }
    return vaStringCreate((uchar *)utCatStrings((char *)vaStringGetValue(string1),
        (char *)vaStringGetValue(string2)));
}
