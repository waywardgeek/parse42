/* Encode/decode a value to a binary sequence of bytes.

This file implements the binary encoding of L42 values. */

#include <ctype.h>
#include "value.h"

// These structures help convert byte order to network byte order, which is most significant
// byte first.
union vaOrder32Union {
    uint32 intVal;
    uint8 byteArray[4];
};

static union vaOrder32Union vaOrder32;
static union vaOrder64Union vaOrder64;

union vaOrder64Union {
    uint64 intVal;
    uint8 byteArray[8];
};

static uchar *vaEncodedValue;
static uint64 vaValueSize, vaValuePos;
static uchar *vaStringBuffer;
static uint32 vaStringBufferSize, vaStringBufferPos;

// Initialize the bencode module.
void vaBencodeStart(void)
{
    vaValueSize = 42;
    vaEncodedValue = utNewA(uint8, vaValueSize);
    vaOrder32.intVal = 0x00010203u;
    vaOrder64.intVal = 0x0001020304050607llu;
    vaStringBufferSize = 42;
    vaStringBuffer = utNewA(uchar, vaStringBufferSize);
}

// Free memory used by the bencode module.
void vaBencodeStop(void)
{
    utFree(vaEncodedValue);
    utFree(vaStringBuffer);
}

// Decode an encoded unsigned integer value.
uint64 vaDecodeUint(
    uint8 *bytes)
{
    uint8 numBytes = ((*bytes++) & 0x7) + 1;
    uint64 value = 0;

    while(numBytes--) {
        // We use network byte order - most significant byte first.
        value = (value << 8) | *bytes++;
    }
    return value;
}

// Decode a float value.
float vaDecodeFloat(
    uint8 *bytes)
{
    float value;
    uint8 *p = (uint8 *)(void *)&value;
    int i;

    for(i = 0; i < 4; i++) {
        p[vaOrder32.byteArray[i]] = *bytes++;
    }
    return value;
}

// Decode a double value.
double vaDecodeDouble(
    uint8 *bytes)
{
    double value;
    uint8 *p = (uint8 *)(void *)&value;
    int i;

    for(i = 0; i < 8; i++) {
        p[vaOrder64.byteArray[i]] = *bytes++;
    }
    return value;
}

// Add a character to the string buffer.
static inline void addToStringBuffer(
    uchar c)
{
    if(vaStringBufferPos == vaStringBufferSize) {
        vaStringBufferSize <<= 1;
        utResizeArray(vaStringBuffer, vaStringBufferSize);
    }
    vaStringBuffer[vaStringBufferPos++] = c;
}

// Insert backslashes before quotes, and covert tabs to \t, newlines to \n, and
// returns to \r.  Other control characters are encoded as \%d.  This returns a pointer to a
// static buffer, which is overwritten on the next call.
uchar *vaMungeString(
    uint8 *bytes)
{
    uchar c = *bytes++;
    uint8 extra;

    vaStringBufferPos = 0;
    addToStringBuffer('"');
    while(c != '\0') {
        if(c < ' ' || c == '"' || c == '\\') {
            addToStringBuffer('\\');
            if(c == '"') {
                addToStringBuffer('"');
            } else if(c == '\\') {
                addToStringBuffer('\\');
            } else if(c == '\t') {
                addToStringBuffer('t');
            } else if(c == '\n') {
                addToStringBuffer('n');
            } else if(c == '\r') {
                addToStringBuffer('r');
            } else {
                if(c >= 10) {
                    addToStringBuffer(c/10 + '0');
                    c -= (c/10)*10;
                }
                addToStringBuffer(c + '0');
            }
        } else {
            addToStringBuffer(c);
            extra = utf8FindLength(c) - 1;
            while(extra--) {
                c = *bytes++;
                addToStringBuffer(c);
            }
        }
        c = *bytes++;
    }
    addToStringBuffer('"');
    addToStringBuffer('\0');
    return vaStringBuffer;
}

// Escape anything that's not alnum or _.  Allow non-ASCII characers without escapes.
uchar *vaEscapeIdent(
    uint8 *bytes)
{
    uchar c;
    uint8 extra;

    if(!strcmp((char *)bytes, "false")) {
        return (uchar *)"\\false";
    } else if(!strcmp((char *)bytes, "true")) {
        return (uchar *)"\\true";
    } else if(!strcmp((char *)bytes, "null")) {
        return (uchar *)"\\null";
    }
    c = *bytes++;
    vaStringBufferPos = 0;
    while(c != '\0') {
        if(c < ' ') {
            addToStringBuffer('\\');
            if(c == '\t') {
                addToStringBuffer('t');
            } else if(c == '\n') {
                addToStringBuffer('n');
            } else if(c == '\r') {
                addToStringBuffer('r');
            } else {
                if(c >= 10) {
                    addToStringBuffer(c/10 + '0');
                    c -= (c/10)*10;
                }
                addToStringBuffer(c + '0');
            }
        } else {
            if(!isalnum(c) && !(c & 0x80)) {
                addToStringBuffer('\\');
            }
            addToStringBuffer(c);
            extra = utf8FindLength(c) - 1;
            while(extra--) {
                c = *bytes++;
                addToStringBuffer(c);
            }
        }
        c = *bytes++;
    }
    addToStringBuffer('\0');
    return vaStringBuffer;
}

// Find the length of an encoded value, including the type byte.
uint64 vaFindEncodedValueLength(
    uint8 *bytes)
{
    uint8 type = *bytes;
    uint64 length, totalLength;

    if(type & 0x80) {
        // Must be a length coded value.
        if((type & 0xf8) == 0x98) {
            // Blobs have extra data
            return (type & 0x7) + 2 + vaDecodeUint(bytes);
        }
        return (type & 0x7) + 2;
    }
    switch(type) {
    case 'T': case 'F': case '0': return 1;
    case 'f': return 5;
    case 'g': return 9;
    case 's': case 'i': return 2 + strlen((char *)(bytes + 1));
    case 't': case 'l': case 'd':
        totalLength = 2;
        type = *++bytes;
        while(type != 'E') {
            length = vaFindEncodedValueLength(bytes);
            bytes += length;
            totalLength += length;
            type = *bytes;
        }
        return totalLength;
    default:
        utExit("Invalid encoded value");
    }
    return 0; // Dummy return
}

// Just print a tupple.
static void printTupple(
    uint8 *bytes)
{
    uint8 type = *++bytes;
    bool firstTime = true;

    printf("(");
    while(type != 'E') {
        if(!firstTime) {
            printf(", ");
        }
        firstTime = false;
        vaPrintEncodedValue(bytes);
        bytes += vaFindEncodedValueLength(bytes);
    }
    printf(")");
}

// Just print a list.
static void printList(
    uint8 *bytes)
{
    uint8 type = *++bytes;
    bool firstTime = true;

    printf("[");
    while(type != 'E') {
        if(!firstTime) {
            printf(", ");
        }
        firstTime = false;
        vaPrintEncodedValue(bytes);
        bytes += vaFindEncodedValueLength(bytes);
    }
    printf("]");
}

// Just print a dictionary.
static void printDictionary(
    uint8 *bytes)
{
    uint8 type = *++bytes;
    bool firstTime = true;

    printf("<|");
    while(type != 'E') {
        if(!firstTime) {
            printf(", ");
        }
        firstTime = false;
        vaPrintEncodedValue(bytes);
        bytes += vaFindEncodedValueLength(bytes);
        printf(":");
        vaPrintEncodedValue(bytes);
        bytes += vaFindEncodedValueLength(bytes);
    }
    printf("|>");
}

// Print raw bytes in hex.
static void printBlob(
    uint8 *bytes)
{
    uint64 length = vaDecodeUint(bytes);
    uint8 value;

    bytes += (*bytes & 0x7) + 2;
    printf("0b");
    while(length--) {
        value = *bytes++;
        printf("%c%c", toHex(value >> 4), toHex(value & 0xf));
    }
}

// Just print the encoded value as ASCII
void vaPrintEncodedValue(
    uint8 *bytes)
{
    uint8 type = *bytes;

    if(type & 0x80) {
        // Must be an integer coded value.
        switch(type >> 3) {
        case 0: printf("%llu", vaDecodeUint(bytes)); break;
        case 1: printf("-%llu", vaDecodeUint(bytes)); break;
        case 2: printf("0p%llx", vaDecodeUint(bytes)); break;
        case 3: printBlob(bytes);
        default:
            utExit("Invalide encoded value");
        }
        return;
    }
    switch(type) {
    case 'T': printf("true"); break;
    case 'F': printf("false"); break;
    case '0': printf("null"); break;
    case 'f': printf("%gf", vaDecodeFloat(bytes + 1)); break;
    case 'g': printf("%g", vaDecodeDouble(bytes + 1)); break;
    case 's': printf("\"%s\"", vaMungeString(bytes + 1)); break;
    case 'i': printf("%s", vaEscapeIdent(bytes + 1)); break;
    case 't': printTupple(bytes); break;
    case 'l': printList(bytes); break;
    case 'd': printDictionary(bytes); break;
    default:
        utExit("Invalid encoded value");
    }
}

// Decode a list of values as a vaList
vaList decodeList(
    uint8 *bytes)
{
    vaList list = vaListAlloc();
    vaValue value;

    while(*bytes != 'E') {
        value = vaBdecode(bytes);
        vaListAppendValue(list, value);
        bytes += vaFindEncodedValueLength(bytes);
    }
    return list;
}

// Create a dictionary from a list of alternating keys and values.
static vaDictionary createDictionaryFromList(
    vaList list)
{
    vaDictionary dictionary = vaDictionaryCreate();
    vaValue key, value;
    uint32 numValues = vaListGetUsedValue(list);
    uint32 xValue;

    if(numValues & 1) {
        utExit("Dictionary has odd number of values");
    }
    for(xValue = 0; xValue < numValues; xValue += 2) {
        key = vaListGetiValue(list, xValue);
        value = vaListGetiValue(list, xValue + 1);
        vaDictionaryInsertValue(dictionary, key, value);
    }
    return dictionary;
}

// Decode a blob.
static vaBlob decodeBlob(
    uchar *bytes)
{
    uint64 length = vaDecodeUint(bytes);

    // Skip the length integer
    bytes += (*bytes & 0x7) + 2;
    return vaBlobCreate(length, bytes);
}

// Just print the hex vesion of the message recieved.
static void printMessage(
    uint8 *bytes)
{
    uint64 length = vaFindEncodedValueLength(bytes);
    uint8 value;
    
    printf("Decoding %llu byte message: ", length);
    while(length--) {
        value = *bytes++;
        printf("%c%c", toHex(value >> 4), toHex(value & 0xf));
    }
    printf("\n");
}

// Convert a binary encoded value to a vaValue
vaValue vaBdecode(
    uchar *bytes)
{
    uint8 type = *bytes;

    printMessage(bytes);
    if(type & 0x80) {
        // Must be an integer coded value.
        switch((type >> 3) & 0xf) {
        case 0: return vaPosIntValueCreate(vaDecodeUint(bytes));
        case 1: return vaNegIntValueCreate(vaDecodeUint(bytes));
        case 2: return vaObjectValueCreate(vaDecodeUint(bytes));
        case 3: return vaBlobValueCreate(decodeBlob(bytes));
        default:
            utExit("Invalide encoded value");
        }
    }
    switch(type) {
    case 'T': return vaBoolValueCreate(true);
    case 'F': return vaBoolValueCreate(false);
    case '0': return vaNullValueCreate();
    case 'f': return vaFloatValueCreate(vaDecodeFloat(bytes + 1));
    case 'g': return vaDoubleValueCreate(vaDecodeDouble(bytes + 1));
    case 's': return vaStringValueCreate(vaStringCreate(bytes + 1));
    case 'i': return vaIdentValueCreate(utSymCreate((char *)(bytes + 1)));
    case 't': return vaTupleValueCreate(decodeList(bytes + 1));
    case 'l': return vaListValueCreate(decodeList(bytes + 1));
    case 'd': return vaDictionaryValueCreate(createDictionaryFromList(decodeList(bytes + 1)));
    default:
        utExit("Invalid encoded value");
    }
    return vaValueNull; // Dummy return
}

// Add a byte to the encoded value buffer.
static inline void addByte(
    uint8 value)
{
    if(vaValuePos == vaValueSize) {
        vaValueSize <<= 1;
        utResizeArray(vaEncodedValue, vaValueSize);
    }
    vaEncodedValue[vaValuePos++] = value;
    printf("Adding %x\n", value);
}

// Forward declaration for recursion.
static void bencode(vaValue value);

// Count the number of bytes needed to represent the unsigned integer.
static inline uint8 countUintBytes(
    uint64 value)
{
    uint8 numBytes = 0;

    if(value == 0) {
        return 1;
    }
    while(value) {
        numBytes++;
        value >>= 8;
    }
    return numBytes;
}

// Count the number of bytes needed to represent the integer.  It's more complex than counting
// for unsigned because we need an extra bit to indicate the sign.
static inline uint8 countIntBytes(
    int64 value)
{
    if((value >> 63) & 1) {
        // Negaive value
        value = ~value;
    }
    return countUintBytes(value << 1);
}

// Encode an integer.
static void encodeUint(
    uint8 prefix,
    uint64 value)
{
    uint8 numBytes = countUintBytes(value);
    uint8 highByte;

    addByte(0x80 | (prefix << 3) | (numBytes - 1));
    while(numBytes--) {
        highByte = value >> (numBytes << 3);
        addByte(highByte);
    }
}

// Encode a floating point number.
static void encodeFloat(
    float value)
{
    uint8 *p = (uint8 *)(void *)&value;
    int i;

    addByte('f');
    for(i = 0; i < 4; i++) {
        addByte(p[vaOrder32.byteArray[i]]);
    }
}

// Encode a double precision floating point number.
static void encodeDouble(
    double value)
{
    uint8 *p = (uint8 *)(void *)&value;
    int i;

    addByte('g');
    for(i = 0; i < 8; i++) {
        addByte(p[vaOrder64.byteArray[i]]);
    }
}

// Encode a string value.
static void encodeString(
    vaString string)
{
    uchar *p = vaStringGetValue(string);
    uchar c;

    addByte('s');
    do {
        c = *p++;
        addByte(c);
    } while(c != '\0');
}

// Encode an ident value.
static void encodeIdent(
    utSym sym)
{
    uchar *p = (uchar *)utSymGetName(sym);
    uchar c;

    addByte('i');
    do {
        c = *p++;
        addByte(c);
    } while(c != '\0');
}

// Encode a list of values.
static void encodeList(
    vaList list)
{
    vaValue value;

    vaForeachListValue(list, value) {
        bencode(value);
    } vaEndListValue;
    addByte('E');
}

// Encode a dictionary.
static void encodeDictionary(
    vaDictionary dictionary)
{
    vaDictEntry dictEntry;

    addByte('d');
    vaForeachDictionaryDictEntry(dictionary, dictEntry) {
        bencode(vaDictEntryGetKey(dictEntry));
        bencode(vaDictEntryGetValue(dictEntry));
    } vaEndDictionaryDictEntry;
    addByte('E');
}

// Encode a blob object.
static void encodeBlob(
    vaBlob blob)
{
    uint64 length = vaBlobGetLength(blob);
    uint8 *p = vaBlobGetValue(blob);

    encodeUint(3, length);
    while(length--) {
        addByte(*p++);
    }
}

// Encode a value to a binary representation, and add it to the vaEncodedValue array.
static void bencode(
    vaValue value)
{
    vaType type = vaValueGetType(value);

    switch(type) {
    case VA_POSINT: encodeUint(0, vaValueGetUintVal(value)); break;
    case VA_NEGINT: encodeUint(1, vaValueGetUintVal(value)); break;
    case VA_OBJECT: encodeUint(2, vaValueGetObjectVal(value)); break;
    case VA_BOOL:
        if(vaValueBoolVal(value)) {
            addByte('T');
        } else {
            addByte('F');
        }
        break;
    case VA_NULL: addByte('0'); break;
    case VA_FLOAT: encodeFloat(vaValueGetFloatVal(value)); break;
    case VA_DOUBLE: encodeDouble(vaValueGetDoubleVal(value)); break;
    case VA_STRING: encodeString(vaValueGetStringVal(value)); break;
    case VA_IDENT: encodeIdent(vaValueGetNameVal(value)); break;
    case VA_TUPLE: addByte('t'); encodeList(vaValueGetTupleVal(value)); break;
    case VA_LIST: addByte('l'); encodeList(vaValueGetListVal(value)); break;
    case VA_DICTIONARY: encodeDictionary(vaValueGetDictionaryVal(value)); break;
    case VA_BLOB: encodeBlob(vaValueGetBlobVal(value)); break;
    default:
        utExit("Invalid encoded value");
    }
}

// Encode a value to a binary representation.  This returns a utBuffer, so use it soon.
uchar *vaBencode(
    vaValue value,
    uint64 *length)
{
    uchar *bytes;

    vaValuePos = 0;
    bencode(value);
    *length = vaValuePos;
    bytes = utNewBufA(uchar, vaValuePos);
    memcpy(bytes, vaEncodedValue, vaValuePos);
    return bytes;
}

// Check that everything works for the string.  Encode and decode it, and verify the values are
// equal.
static void selfTest(
    uchar *string,
    bool shouldPass,
    bool preciseFloats)
{
    uint64 length;
    vaValue value1, value2;
    uint8 *bytes;
    uchar *finalValue;

    printf("Starting %s\n", string);
    value1 = vaParseValue(string);
    bytes = vaBencode(value1, &length);
    value2 = vaBdecode(bytes);
    if(!vaValuesEqual(value1, value2)) {
        utExit("Failed encode/decode test: %s != \n", vaValue2String(value1),
            vaValue2String(value2));
    }
    finalValue = preciseFloats? vaValue2PreciseString(value2) : vaValue2String(value2);
    if(shouldPass) {
        if(strcmp((char *)string, (char *)finalValue)) {
            utExit("After test, %s != %s\n", string, finalValue);
        }
    }
    printf("Passed %s -> %s\n", string, finalValue);
}

typedef struct {
    char *value;
    bool shouldPass;
    bool preciseFloats;
} vaTestType;

// Benode unit test.
void vaBencodeSelfTest(void)
{
    uint32 xTest;
    vaTestType tests[] = {
        {"true", true, false},
        {"false", true, false},
        {"null", true, false},
        {"12345678", true, false},
        {"-12345", true, false},
        {"0xFEDCBA9876543210", false, false},
        {"0xABCp-42", false, true},
        {"0x123456789.AB", false, true},
        {"0x1.23456p42", true, true},
        {"0x1.23456789ABCp-123", true, true},
        {"0p1234", true, false},
        {"[1, 2, 3]", true, false},
        {"(a, 1, \"test\")", true, false},
        {"0b0123456789ABCDEF", true, false},
        {"<|a:1, b:2, 1.23:3.333|>", true, false},
        {"\\false", true, false},
        {"<|[(0, 1), (2, 3)]:(a, b), <|[4, 5, 6]:false|>:\\true|>", true, false},
        {NULL, false, false},
    };
    vaTestType test;

    printf("\n");
    for(xTest = 0; tests[xTest].value != NULL; xTest++) {
        test = tests[xTest];
        selfTest((uchar *)test.value, test.shouldPass, test.preciseFloats);
    }
    printf("\n");
}

