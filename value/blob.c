// Blob methods
#include <stdlib.h>
#include "value.h"

// Create a new binary blob object.
vaBlob vaBlobCreate(
    uint64 length,
    uint8 *bytes)
{
    vaBlob blob = vaBlobAlloc();
    uint8 *allocatedBytes = (uint8 *)malloc(length);

    memcpy(allocatedBytes, bytes, length);
    vaBlobSetLength(blob, length);
    vaBlobSetValue(blob, allocatedBytes);
    return blob;
}

// Destructor hook for freeing blob data.
void vaFreeBlobData(
    vaBlob blob)
{
    free(vaBlobGetValue(blob));
}

// Compare two blobs.
bool vaBlobsEqual(
    vaBlob blob1,
    vaBlob blob2)
{
    uint64 length = vaBlobGetLength(blob1);

    if(vaBlobGetLength(blob2) != length) {
        return false;
    }
    return !memcmp(vaBlobGetValue(blob1), vaBlobGetValue(blob2), length);
}
