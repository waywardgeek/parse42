// Dictionary methods.  Note that this is an ordered dictionary.
#include "value.h"

// Compare two entries.
static inline bool dictEntriesEqual(
    vaDictEntry dictEntry1,
    vaDictEntry dictEntry2)
{
    return vaValuesEqual(vaDictEntryGetKey(dictEntry1), vaDictEntryGetKey(dictEntry2)) &&
        vaValuesEqual(vaDictEntryGetValue(dictEntry1), vaDictEntryGetValue(dictEntry2));
}

// Compare two dictionaries.  They are not equal if the entries are not in the same order.
bool vaDictionariesEqual(
    vaDictionary dict1,
    vaDictionary dict2)
{
    vaDictEntry dictEntry1 = vaDictionaryGetFirstDictEntry(dict1);
    vaDictEntry dictEntry2 = vaDictionaryGetFirstDictEntry(dict2);

    while(dictEntry1 != vaDictEntryNull && dictEntry2 != vaDictEntryNull) {
        if(!dictEntriesEqual(dictEntry1, dictEntry2)) {
            return false;
        }
        dictEntry1 = vaDictEntryGetNextDictionaryDictEntry(dictEntry1);
        dictEntry2 = vaDictEntryGetNextDictionaryDictEntry(dictEntry2);
    }
    return dictEntry1 == dictEntry2;
}

// Create a new empty dictionary.
vaDictionary vaDictionaryCreate(void)
{
    vaDictionary dictionary = vaDictionaryAlloc();

    // Initialize entry hash table
    vaDictionarySetNumEntries(dictionary, 0);
    vaDictionaryResizeEntrys(dictionary, 42);
    return dictionary;
}

// Insert a value into the dictionary.
void vaDictionaryInsertValue(
    vaDictionary dictionary,
    vaValue key,
    vaValue value)
{
    vaDictEntry dictEntry = vaDictEntryAlloc();

    vaDictEntrySetKey(dictEntry, key);
    vaDictEntrySetValue(dictEntry, value);
    vaDictionaryAppendDictEntry(dictionary, dictEntry);
}

