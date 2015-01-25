#include "vadatabase.h"

// Value methods.
void vaValueStart(void);
void vaValueStop(void);
vaValue vaIntValueCreate(int64 val);
vaValue vaUintValueCreate(int64 val);
vaValue vaPosIntValueCreate(uint64 val);
vaValue vaNegIntValueCreate(uint64 val);
vaValue vaObjectValueCreate(int64 val);
vaValue vaStringValueCreate(vaString string);
vaValue vaFloatValueCreate(float val);
vaValue vaDoubleValueCreate(double val);
vaValue vaBoolValueCreate(bool val);
vaValue vaIdentValueCreate(utSym val);
vaValue vaTupleValueCreate(vaList vals);
vaValue vaListValueCreate(vaList vals);
vaValue vaDictionaryValueCreate(vaDictionary dictionary);
vaValue vaNullValueCreate(void);
vaValue vaBlobValueCreate(vaBlob blob);
bool vaValuesEqual(vaValue value1, vaValue value2);
void vaPrintValue(vaValue value);
uint8 *vaValue2String(vaValue value);
uint8 *vaValue2PreciseString(vaValue value);
vaValue vaParseValue(uchar *string);

// List methods
bool vaListsEqual(vaList list1, vaList list2);
static inline vaList vaListCreate(void) {return vaListAlloc();}

// Dictionary methods
bool vaDictionariesEqual(vaDictionary dict1, vaDictionary dict2);
vaDictionary vaDictionaryCreate(void);
void vaDictionaryInsertValue(vaDictionary dictionary, vaValue key, vaValue value);

// Blob methods
vaBlob vaBlobCreate(uint64 length, uint8 *bytes);
void vaFreeBlobData(vaBlob blob);
bool vaBlobsEqual(vaBlob blob1, vaBlob blob2);

// String methods
vaString vaStringCreate(uchar *value);
vaString vaStrcat(vaString string1, vaString string2);

// Bencode module
void vaBencodeStart(void);
void vaBencodeStop(void);
vaValue vaBdecode(uchar *bytes);
uchar *vaBencode(vaValue value, uint64 *length);
uchar *vaBencodeUint(uint64 value);
uint64 vaFindEncodedValueLength(uint8 *bytes);
void vaPrintEncodedValue(uint8 *bytes);
float vaDecodeFloat(uint8 *bytes);
double vaDecodeDouble(uint8 *bytes);
uint64 vaDecodeUint(uint8 *bytes);
void vaBencodeSelfTest(void);
uchar *vaMungeString(uint8 *bytes);
uchar *vaEscapeIdent(uint8 *bytes);

// Convert to a hex digit.
static inline uchar toHex(uint8 value) { return value < 10? value + '0' : value - 10 + 'A'; }

// Globals
extern vaRoot vaTheRoot;
extern utSym vaTrueSym, vaFalseSym, vaNullSym;
extern FILE *vaFile; // Used by utf8 to provide a read-line function
