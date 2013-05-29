// List methods.
#include "value.h"

// Determine if two lists are equal.
bool vaListsEqual(
    vaList list1,
    vaList list2)
{
    vaValue value1, value2;
    uint32 numValues = vaListGetUsedValue(list1);
    uint32 xValue;

    if(vaListGetUsedValue(list2) != numValues) {
        return false;
    }
    for(xValue = 0; xValue < numValues; xValue++) {
        value1 = vaListGetiValue(list1, xValue);
        value2 = vaListGetiValue(list2, xValue);
        if(!vaValuesEqual(value1, value2)) {
            return false;
        }
    }
    return true;
}
