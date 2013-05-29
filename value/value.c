// Value methods.
#include <stdlib.h>
#include <ctype.h>
#include "value.h"

vaRoot vaTheRoot;
utSym vaTrueSym, vaFalseSym, vaNullSym;

static uchar *vaStringBuffer;
static uint64 vaStringSize, vaStringPos;
static bool vaUseHexFloats;

// Initialize the value module.
void vaValueStart(void)
{
    vaDatabaseStart();
    vaBlobSetDestructorCallback(vaFreeBlobData);
    utf8Start();
    vaBencodeStart();
    vaTheRoot = vaRootAlloc();
    vaTrueSym = utSymCreate("true");
    vaFalseSym = utSymCreate("false");
    vaNullSym = utSymCreate("null");
    vaStringSize = 42;
    vaStringBuffer = utNewA(uchar, vaStringSize);
}

// Free memory used by the value module.
void vaValueStop(void)
{
    utFree(vaStringBuffer);
    vaBencodeStop();
    utf8Stop();
    vaDatabaseStop();
}

// Add the single byte character to the string buffer.
static inline void addChar(
    uchar c)
{
    if(vaStringPos == vaStringSize) {
        vaStringSize += vaStringSize >> 1;
        utResizeArray(vaStringBuffer, vaStringSize);
    }
    vaStringBuffer[vaStringPos++] = c;
}

// Add the string to the string buffer.
static inline void addString(
    char *string)
{
    uint64 length = strlen(string);

    if(vaStringPos + length >= vaStringSize) {
        vaStringSize += (vaStringSize >> 1) + length;
        utResizeArray(vaStringBuffer, vaStringSize);
    }
    strcpy((char *)(vaStringBuffer + vaStringPos), string);
    vaStringPos += length;
}

// Add a utf8 character and return a pointer to the next character.
static inline void addUtf8Char(
    uchar **bytesPtr)
{
    uchar *bytes = *bytesPtr;
    int length = utf8FindLength(*bytes);

    if(vaStringPos + length >= vaStringSize) {
        vaStringSize += (vaStringSize >> 1) + length;
        utResizeArray(vaStringBuffer, vaStringSize);
    }
    while(length--) {
        vaStringBuffer[vaStringPos++] = *bytes++;
    }
    *bytesPtr = bytes;
}


// Print a formatted string to the end of the string buffer.
static void addFormatted(
    char *format,
    ...)
{
    va_list ap;
    char *buffer;

    va_start(ap, format);
    buffer = utVsprintf(format, ap);
    va_end(ap);
    addString(buffer);
}

// Print out the value.
void vaPrintValue(
    vaValue value)
{
    uchar *string = vaValue2String(value);

    printf("%s", string);
}

// Forward declaration for recursion.
static void addValue(vaValue value);

// Add a tuple to the string buffer.
static void addTupple(
    vaList list)
{
    vaValue value;
    bool firstTime = true;

    addChar('(');
    vaForeachListValue(list, value) {
        if(!firstTime) {
            addString(", ");
        }
        firstTime = false;
        addValue(value);
    } vaEndListValue;
    addChar(')');
}

// Add a list of values to the string buffer.
static void addList(
    vaList list)
{
    vaValue value;
    bool firstTime = true;

    addChar('[');
    vaForeachListValue(list, value) {
        if(!firstTime) {
            addString(", ");
        }
        firstTime = false;
        addValue(value);
    } vaEndListValue;
    addChar(']');
}

// Add a dictionary to the string buffer.
static void addDictionary(
    vaDictionary dictionary)
{
    vaDictEntry dictEntry;
    bool firstTime = true;

    addString("[|");
    vaForeachDictionaryDictEntry(dictionary, dictEntry) {
        if(!firstTime) {
            addString(", ");
        }
        firstTime = false;
        addValue(vaDictEntryGetKey(dictEntry));
        addChar(':');
        addValue(vaDictEntryGetValue(dictEntry));
    } vaEndDictionaryDictEntry;
    addString("|]");
}

// Add the blob to the string buffer.
static void addBlob(
    vaBlob blob)
{
    uint64 length = vaBlobGetLength(blob);
    uint8 *bytes = vaBlobGetValue(blob);
    uint8 value;

    addString("0b");
    while(length--) {
        value = *bytes++;
        addChar(toHex(value >> 4));
        addChar(toHex(value & 0xf));
    }
}

union doubleToUint64Union {
    double doubleVal;
    uint64 uint64Val; 
};

// Add a double to the string buffer.  We assume standard IEEE format.
static void addDouble(
    double value)
{
    union doubleToUint64Union u;
    int32 exponent;
    uint64 intVal, fraction;
    uint8 sign;

    if(vaUseHexFloats) {
        u.doubleVal = value;
        intVal = u.uint64Val;
        fraction = intVal & 0xfffffffffffffllu;
        while(fraction && ((fraction & 0xf) == 0)) {
            // Remove trailing 0's
            fraction >>= 4;
        }
        exponent = ((intVal >> 52) & 0x7ff) - 1023;
        sign = intVal >> 63;
        if(sign == 1) {
            addChar('-');
        }
        addFormatted("0x1.%llXp%d", fraction, exponent);
    } else {
        addFormatted("%g", value);
    }
}

union floatToUint32Union {
    float floatVal;
    uint32 uint32Val; 
};

// Add a float to the string buffer.  We assume standard IEEE format.
static void addFloat(
    float value)
{
    union floatToUint32Union u;
    int32 exponent;
    uint32 intVal, fraction;
    uint8 sign;

    if(vaUseHexFloats) {
        u.floatVal = value;
        intVal = u.uint32Val;
        fraction = (intVal & 0x7fffffu) << 1;
        while(fraction && ((fraction & 0xf) == 0)) {
            // Remove trailing 0's
            fraction >>= 4;
        }
        exponent = ((intVal >> 23) & 0xff) - 127;
        sign = intVal >> 31;
        if(sign == 1) {
            addChar('-');
        }
        addFormatted("0x1.%Xp%d", fraction, exponent);
    } else {
        addFormatted("%g", value);
    }
}

// Convert the value to a string.
static void addValue(
    vaValue value)
{
    switch(vaValueGetType(value)) {
    case VA_POSINT:
        addFormatted("%llu", vaValueGetUintVal(value));
        break;
    case VA_NEGINT:
        addFormatted("-%llu", vaValueGetUintVal(value));
        break;
    case VA_OBJECT:
        addFormatted("0p%llx", vaValueGetObjectVal(value));
        break;
    case VA_FLOAT:
        addFloat(vaValueGetFloatVal(value));
        break;
    case VA_DOUBLE:
        addDouble(vaValueGetDoubleVal(value));
        break;
    case VA_STRING:
        addString((char *)vaMungeString(vaStringGetValue(vaValueGetStringVal(value))));
        break;
    case VA_BOOL:
        if(vaValueBoolVal(value)) {
            addString("true");
        } else {
            addString("false");
        }
        break;
    case VA_IDENT:
        addString((char *)vaEscapeIdent((uchar *)utSymGetName(vaValueGetNameVal(value))));
        break;
    case VA_TUPLE:
        addTupple(vaValueGetTupleVal(value));
        break;
    case VA_LIST:
        addList(vaValueGetListVal(value));
        break;
    case VA_NULL:
        addString("null");
        break;
    case VA_DICTIONARY:
        addDictionary(vaValueGetDictionaryVal(value));
        break;
    case VA_BLOB:
        addBlob(vaValueGetBlobVal(value));
        break;
    default:
        utExit("Unknown value type");
    }
}

// Convert the value to a string.
uchar *vaValue2String(
    vaValue value)
{
    vaUseHexFloats = false;
    vaStringPos = 0;
    addValue(value);
    addChar('\0');
    return vaStringBuffer;
}

// Convert the value to a string, but use hex format for floats.  This allows values to be
// parsed without rouding errors.
uchar *vaValue2PreciseString(
    vaValue value)
{
    vaUseHexFloats = true;
    vaStringPos = 0;
    addValue(value);
    addChar('\0');
    return vaStringBuffer;
}

// Create a raw value object.
static inline vaValue valueCreate(
   vaType type)
{
    vaValue value = vaValueAlloc();

    vaValueSetType(value, type);
    return value;
}

// Create a positive integer value.
vaValue vaPosIntValueCreate(
    uint64 val)
{
    vaValue value = valueCreate(VA_POSINT);

    vaValueSetUintVal(value, val);
    return value;
}

// Create a negative integer value.
vaValue vaNegIntValueCreate(
    uint64 val)
{
    vaValue value = valueCreate(VA_NEGINT);

    vaValueSetUintVal(value, val);
    return value;
}


// Create an integer value from a signed 64 bit integer.
vaValue vaIntValueCreate(
    int64 val)
{
    vaValue value;

    if(val >= 0) {
        value = valueCreate(VA_POSINT);
        vaValueSetUintVal(value, val);
    } else {
        value = valueCreate(VA_NEGINT);
        vaValueSetUintVal(value, -val);
    }
    return value;
}

// Create an unsigned integer value.
vaValue vaUintValueCreate(
    int64 val)
{
    vaValue value = valueCreate(VA_POSINT);

    vaValueSetUintVal(value, val);
    return value;
}

// Create an object reference value.
vaValue vaObjectValueCreate(
    int64 val)
{
    vaValue value = valueCreate(VA_OBJECT);

    vaValueSetObjectVal(value, val);
    return value;
}

// Create a string value.
vaValue vaStringValueCreate(
    vaString string)
{
    vaValue value = valueCreate(VA_STRING);

    vaValueSetStringVal(value, string);
    return value;
}

// Create a floating point value.
vaValue vaFloatValueCreate(
    float val)
{
    vaValue value = valueCreate(VA_FLOAT);

    vaValueSetFloatVal(value, val);
    return value;
}

// Create a double precision floating point value.
vaValue vaDoubleValueCreate(
    double val)
{
    vaValue value = valueCreate(VA_DOUBLE);

    vaValueSetDoubleVal(value, val);
    return value;
}

// Create a Boolean point value.
vaValue vaBoolValueCreate(
    bool val)
{
    vaValue value = valueCreate(VA_BOOL);

    vaValueSetBoolVal(value, val);
    return value;
}

// Create an entry value.
vaValue vaIdentValueCreate(
    utSym val)
{
    vaValue value = valueCreate(VA_IDENT);

    vaValueSetNameVal(value, val);
    return value;
}

// Create a tuple value from a list of values.
vaValue vaTupleValueCreate(
    vaList vals)
{
    vaValue value = valueCreate(VA_TUPLE);

    vaValueSetTupleVal(value, vals);
    return value;
}

// Create a list value from a list of values.
vaValue vaListValueCreate(
    vaList vals)
{
    vaValue value = valueCreate(VA_LIST);

    vaValueSetListVal(value, vals);
    return value;
}

// Create a dictionary value from a list of values.
vaValue vaDictionaryValueCreate(
    vaDictionary dictionary)
{
    vaValue value = valueCreate(VA_DICTIONARY);

    vaValueSetDictionaryVal(value, dictionary);
    return value;
}

// Create a null value.
vaValue vaNullValueCreate(void)
{
    return valueCreate(VA_NULL);
}

// Create a binary blob value.
vaValue vaBlobValueCreate(
    vaBlob blob)
{
    vaValue value = valueCreate(VA_BLOB);

    vaValueSetBlobVal(value, blob);
    return value;
}

// Determine if two values are equal.  For lists, tupples, and dictionaries, we have to compare
// sub-values.
bool vaValuesEqual(
    vaValue value1,
    vaValue value2)
{
    if(vaValueGetType(value1) != vaValueGetType(value2)) {
        return false;
    }
    switch(vaValueGetType(value1)) {
    case VA_POSINT: case VA_NEGINT:
        return vaValueGetUintVal(value1) == vaValueGetUintVal(value2);
    case VA_FLOAT: return vaValueGetFloatVal(value1) == vaValueGetFloatVal(value2);
    case VA_DOUBLE: return vaValueGetDoubleVal(value1) == vaValueGetDoubleVal(value2);
    case VA_STRING: return vaValueGetStringVal(value1) == vaValueGetStringVal(value2);
    case VA_BOOL: return vaValueBoolVal(value1) == vaValueBoolVal(value2);
    case VA_IDENT: return vaValueGetNameVal(value1) == vaValueGetNameVal(value2);
    case VA_TUPLE: return vaListsEqual(vaValueGetTupleVal(value1),
            vaValueGetTupleVal(value2));
    case VA_LIST: return vaListsEqual(vaValueGetListVal(value1),
            vaValueGetListVal(value2));
    case VA_OBJECT: return vaValueGetObjectVal(value1) == vaValueGetObjectVal(value2);
    case VA_NULL: return true;
    case VA_DICTIONARY: return vaDictionariesEqual(vaValueGetDictionaryVal(value1),
            vaValueGetDictionaryVal(value2));
    case VA_BLOB: return vaBlobsEqual(vaValueGetBlobVal(value1), vaValueGetBlobVal(value2));
    default:
        utExit("Unknown value type");
    }
    return false; // Dummy return
}

// Skip white space.
static void skipSpace(
    uchar **bytesPtr)
{
    uchar *bytes = *bytesPtr;

    while(*bytes != '\0' && *bytes <= ' ') {
        bytes++;
    }
    *bytesPtr = bytes;
}

// Forward declaraction for recursion.
static vaValue parseValue(uchar **bytesPtr);

// Parse a list, but don't convert it to a value.
static vaList parseGenericList(
    uchar **bytesPtr,
    char lastChar)
{
    vaList list = vaListCreate();
    vaValue value;
    uchar c;

    (*bytesPtr)++;
    skipSpace(bytesPtr);
    c = **bytesPtr;
    while(c != lastChar) {
        value = parseValue(bytesPtr);
        vaListAppendValue(list, value);
        skipSpace(bytesPtr);
        c = **bytesPtr;
        if(c == ',') {
            (*bytesPtr)++;
        } else if(c != lastChar) {
            utExit("Expected %c to end list: %s", lastChar, *bytesPtr);
        }
    }
    (*bytesPtr)++;
    return list;
}

// Parse a list value.
static vaValue parseList(
    uchar **bytesPtr)
{
    vaList list = parseGenericList(bytesPtr, ']');

    return vaListValueCreate(list);
}

// Parse a tuple value.
static vaValue parseTuple(
    uchar **bytesPtr)
{
    vaList list = parseGenericList(bytesPtr, ')');

    return vaTupleValueCreate(list);
}

// Parse a dictionary value.
static vaValue parseDictionary(
    uchar **bytesPtr)
{
    vaDictionary dictionary = vaDictionaryCreate();
    vaValue key, value;
    uchar c;

    (*bytesPtr) += 2;
    skipSpace(bytesPtr);
    c = **bytesPtr;
    while(c != '|' || (*bytesPtr)[1] != ']') {
        key = parseValue(bytesPtr);
        skipSpace(bytesPtr);
        c = **bytesPtr;
        if(c != ':') {
            utExit("Expected a key:value pair: ", *bytesPtr);
        }
        (*bytesPtr)++;
        value = parseValue(bytesPtr);
        vaDictionaryInsertValue(dictionary, key, value);
        skipSpace(bytesPtr);
        c = **bytesPtr;
        if(c == ',') {
            (*bytesPtr)++;
        } else if(c != '|' || (*bytesPtr)[1] != ']') {
            utExit("Expected |] to end dictionary: %s", *bytesPtr);
        }
    }
    (*bytesPtr) += 2;
    return vaDictionaryValueCreate(dictionary);
}

// Add an escaped character to the string buffer.  Only 0 is not allowed.
static void addEscapedChar(
    uchar **bytesPtr)
{
    uchar c = **bytesPtr;

    if(c == 'n') {
        addChar('\n');
        (*bytesPtr)++;
    } else if(c == 'r') {
        addChar('\r');
        (*bytesPtr)++;
    } else if(c == 't') {
        addChar('\t');
        (*bytesPtr)++;
    } else if(c == '\0') {
        utExit("Invalid zero embedded in string.");
    } else {
        addUtf8Char(bytesPtr);
    }
}

// Parse a string.
static vaValue parseString(
    uchar **bytesPtr)
{
    uchar *bytes = *bytesPtr + 1;
    uchar c = *bytes;

    vaStringPos = 0;
    while(c != '"') {
        if(c == '\0') {
            utExit("Unterminated string");
        }
        if(c == '\\') {
            bytes++;
            addEscapedChar(&bytes);
        } else {
            addUtf8Char(&bytes);
        }
        c = *bytes;
    }
    addChar('\0');
    *bytesPtr = bytes + 1;
    return vaStringValueCreate(vaStringCreate(vaStringBuffer));
}

// Convert a hex character into a value.  If not a valid hex character, return 255.
static uint8 fromHex(
    uchar c)
{
    if(isdigit(c)) {
        return c - '0';
    }
    if(c >= 'a' && c <= 'f') {
        return 10 + c - 'a';
    }
    if(c >= 'A' && c <= 'F') {
        return 10 + c - 'A';
    }
    return 255;
}

// Parse a hex integer or object reference.
static vaValue parseHexNumber(
    uchar **bytesPtr)
{
    uchar type = *(*bytesPtr + 1);
    uchar *bytes = *bytesPtr + 2;
    uint64 value = 0;
    uchar c = *bytes++;
    uint8 digit;

    while(true) {
        digit = fromHex(c);
        if(digit == 255) {
            if(type == 'p') {
                *bytesPtr = bytes - 1;
                return vaObjectValueCreate(value);
            }
            utAssert(type == 'x');
            *bytesPtr = bytes - 1;
            return vaPosIntValueCreate(value);
        }
        value = (value << 4) | digit;
        c = *bytes++;
    }
    return vaValueNull; // Dummy return
}

// Parse a blob string.
static vaValue parseBlob(
    uchar **bytesPtr)
{
    uchar type = *(*bytesPtr + 1);
    uchar *bytes = *bytesPtr + 2;
    uchar c = *bytes++;
    uint8 digit1, digit2;

    vaStringPos = 0;
    while(true) {
        digit1 = fromHex(c);
        if(digit1 >= 16) {
            *bytesPtr = bytes - 1;
            if(type == 'b') {
                return vaBlobValueCreate(vaBlobCreate(vaStringPos, vaStringBuffer));
            }
            if(type == 'f') {
                return vaFloatValueCreate(vaDecodeFloat(vaStringBuffer));
            }
            utAssert(type == 'd');
            return vaFloatValueCreate(vaDecodeDouble(vaStringBuffer));
        }
        c = *bytes++;
        digit2 = fromHex(c);
        if(digit1 >= 16) {
            utExit("Expecting an even number of hex digits in blob: %s", *bytesPtr);
        }
        addChar((digit1 << 4) | digit2);
        c = *bytes++;
    }
    return vaValueNull; // Dummy return
}

// Parse a + or - sign, and return true if negating.
static bool parseSign(
    uchar **bytesPtr)
{
    uchar c = **bytesPtr;
    bool negate = false;

    if(c == '-') {
        negate = true;
        (*bytesPtr)++;
        skipSpace(bytesPtr);
    }
    if(c == '+') {
        (*bytesPtr)++;
        skipSpace(bytesPtr);
    }
    return negate;
}

// Parse an integer value.  Return vaValueNull rather than failing if not a valid int.
static vaValue parseInt(
    uchar **bytesPtr)
{
    bool negate = parseSign(bytesPtr);
    uchar *bytes = *bytesPtr;
    uchar c = *bytes++;
    uint64 value = 0;

    if(!isdigit(c)) {
        utExit("Invalid integer: %s", *bytesPtr);
    }
    do {
        value = 10*value + c - '0';
        c = *bytes++;
    } while(isdigit(c));
    *bytesPtr = bytes - 1;
    if(negate) {
        return vaNegIntValueCreate(value);
    }
    return vaPosIntValueCreate(value);
}

// Parse a floating point number.
static vaValue parseFloat(
    uchar **bytesPtr)
{
    char *tail;
    double value = strtod((char *)(*bytesPtr), &tail);
    float floatVal = value;

    *bytesPtr = (uchar *)(tail);
    if(floatVal == value) {
        return vaFloatValueCreate(floatVal);
    }
    return vaDoubleValueCreate(value);
}

// Parse an identifier.
static vaValue parseIdent(
    uchar **bytesPtr)
{
    uchar *bytes = *bytesPtr;
    uchar c = *bytes;
    bool hasEscape = false;

    if(c != '\\' && c != '_' && !isalpha(c)) {
        utExit("Invalid identifier: %s", bytes);
    }
    vaStringPos = 0;
    while(c == '\\' || c == '_' || isalnum(c)) {
        if(c == '\\') {
            bytes++;
            addEscapedChar(&bytes);
            hasEscape = true;
        } else {
            addUtf8Char(&bytes);
        }
        c = *bytes;
    }
    addChar('\0');
    *bytesPtr = bytes;
    if(!hasEscape) {
        if(!strcmp((char *)vaStringBuffer, "true")) {
            return vaBoolValueCreate(true);
        }
        if(!strcmp((char *)vaStringBuffer, "false")) {
            return vaBoolValueCreate(false);
        }
        if(!strcmp((char *)vaStringBuffer, "null")) {
            return vaNullValueCreate();
        }
    }
    return vaIdentValueCreate(utSymCreate((char *)vaStringBuffer));
}

// Determine if the string has what looks like a floating point value.
static bool isFloat(
    uchar *bytes)
{
    uchar c = *bytes++;

    if(c == '-') {
        skipSpace(&bytes);
        c = *bytes++;
    }
    if(c == '0' && *bytes == 'x') {
        // Could be hex coded float.  Is there a . or p?
        bytes++;
        c = *bytes++;
        while(c != '\0' && c != '.' && c != 'p' && c != 'P') {
            if(!isdigit(c) && (c < 'a' || c > 'f') && (c < 'A' || c > 'F')) {
                return false;
            }
            c = *bytes++;
        }
        return c != '\0';
    }
    while(c != '\0' && c != '.' && c != 'e' && c != 'E') {
        if(!isdigit(c)) {
            return false;
        }
        c = *bytes++;
    }
    return c != '\0';
}

// This is a custom high-speed value parser.
static vaValue parseValue(
    uchar **bytesPtr)
{
    uchar *bytes;
    uchar c, d;

    skipSpace(bytesPtr);
    bytes = *bytesPtr;
    c = **bytesPtr;
    if(isFloat(bytes)) {
        return parseFloat(bytesPtr);
    }
    switch(c) {
    case '\0': utExit("Unexpected end of value expression"); return vaValueNull;
    case '[':
        d = bytes[1];
        if(d == '|') {
            return parseDictionary(bytesPtr);
        }
        return parseList(bytesPtr);
    case '(': return parseTuple(bytesPtr);
    case '"': return parseString(bytesPtr);
    case '0':
        d = bytes[1];
        if(d == 'p' || d == 'x') {
            return parseHexNumber(bytesPtr);
        }
        if(d == 'f' || d == 'd' || d == 'b') {
            return parseBlob(bytesPtr);
        }
        break;
    }
    if(isdigit(c) || c == '-' || c == '+') {
        // Must be an integer
        return parseInt(bytesPtr);
    }
    bytes = *bytesPtr;
    // Should only have IDENT left.
    return parseIdent(bytesPtr);
}

// This is a custom high-speed value parser.
vaValue vaParseValue(
    uchar *bytes)
{
    vaValue value = parseValue(&bytes);

    if(*bytes != '\0') {
        utExit("Extra bytes at end of value: %s", bytes);
    }
    return value;
}
