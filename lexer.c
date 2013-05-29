/* Parse the input tokens.  I switched from flex because input files are in UTF-8,
   and it looks like flex doesn't yet support it.  Check operators  while
   parsing so we keep multi-character operators together.  Otherwize, just split
   tokens at spaces and boundaries between alnum and non-alnum characters. */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "pa.h"

static uchar *paLine;
static uchar *paText;
static size_t paTextSize, paTextPos;
static uint32 paParenDepth, paBracketDepth;
static bool paLastWasNewline;
static paToken paLastToken;

// Print out an error message and exit.
void paError(
    paToken token,
    char *message,
    ...)
{
    char *buff;
    va_list ap;

    va_start(ap, message);
    buff = utVsprintf((char *)message, ap);
    va_end(ap);
    utError("Line %d, token \"%s\": %s", paTokenGetLineNum(token),
            paTokenGetText(token), buff);
}

// Initialize lexer.
void paLexerStart(void)
{
    paLine = NULL;
    paTextSize = 256;
    paText = (uchar *)calloc(paTextSize, sizeof(uchar));
    paParenDepth = 0;
    paBracketDepth = 0;
    paLastWasNewline = true;
    paLastToken = paTokenNull;
}

// Stop the lexer.
void paLexerStop(void)
{
}

// Just print the contents of the token
void paPrintToken(
    paToken token)
{
    printf("%-6u ", paTokenGetLineNum(token));
    switch(paTokenGetType(token)) {
    case PA_TOK_INTEGER:
        printf("INTEGER: %llu\n", paTokenGetIntVal(token));
        break;
    case PA_TOK_FLOAT:
        printf("FLOAT: %g\n", paTokenGetFloatVal(token));
        break;
    case PA_TOK_STRING:
        printf("STRING: %s\n", paTokenGetText(token));
        break;
    case PA_TOK_NEWLINE:
        printf("NEWLINE\n");
        break;
    case PA_TOK_CHAR:
        printf("CHAR: %s\n", paTokenGetText(token));
        break;
    case PA_TOK_IDENT:
        printf("IDENT: %s\n", paTokenGetText(token));
        break;
    case PA_TOK_OPERATOR:
        printf("OPERATOR: %s\n", paTokenGetText(token));
        break;
    case PA_TOK_COMMENT:
        printf("COMMENT: %s\n", paTokenGetText(token));
        break;
    case PA_TOK_KEYWORD:
        printf("KEYWORD: %s\n", paTokenGetText(token));
        break;
    case PA_TOK_BEGIN:
        printf("BEGIN\n");
        break;
    case PA_TOK_END:
        printf("END\n");
        break;
    default:
        utExit("Unknown token type");
    }
}

// Create a token object.
static inline paToken paTokenCreate(
    paTokenType type,
    uchar *text)
{
    paToken token = paTokenAlloc();

    paTokenSetType(token, type);
    paTokenSetText(token, text, strlen((char *)text) + 1);
    paTokenSetLineNum(token, paLineNum);
    paLastToken = token;
    return token;
}

// Create a new integer token.
static inline paToken paIntTokenCreate(
    uint64 intVal,
    uchar *text)
{
    paToken token = paTokenCreate(PA_TOK_INTEGER, text);

    paTokenSetIntVal(token, intVal);
    return token;
}

// Create a new float token.
static inline paToken paFloatTokenCreate(
    double floatVal,
    uchar *text)
{
    paToken token = paTokenCreate(PA_TOK_FLOAT, text);

    paTokenSetFloatVal(token, floatVal);
    return token;
}

// Create a new operator or keyword token.  Just check to see if the keyword is
// owned by an operator or staterule.
static inline paToken paKeywordTokenCreate(
    paKeyword keyword,
    uchar *text)
{
    paPattern pattern = paElementGetPattern(paKeywordGetFirstElement(keyword));
    paToken token;

    if(paPatternGetOperator(pattern) == paOperatorNull) {
        token = paTokenCreate(PA_TOK_KEYWORD, text);
    } else {
        token = paTokenCreate(PA_TOK_OPERATOR, text);
    }
    paTokenSetKeywordVal(token, keyword);
    return token;
}

// Skip blanks and control chars other than tab and newline.
static uchar *skipSpace(
    uchar *p)
{
    uchar c = *p;

    while(c <= ' ' && c != '\0' && c != '\n') {
        c = *++p;
    }
    return p;
}

// Add a character to paText from paLine.
static inline void addChar(void)
{
    int length = utf8FindLength(*paLine);

    if(paTextPos + length > paTextSize) {
        paTextSize <<= 1;
        paText = (uchar *)realloc(paText, paTextSize*sizeof(uchar));
    }
    while(length--) {
        paText[paTextPos++] = *paLine++;
    }
}

// Add an ASCII character to paText from paLine.
static inline void addAscii(
    uchar c)
{
    if(paTextPos >= paTextSize) {
        paTextSize <<= 1;
        paText = (uchar *)realloc(paText, paTextSize*sizeof(uchar));
    }
    paText[paTextPos++] = c;
}

static inline void addChars(
    int numChars)
{
    while(numChars-- != 0) {
        addChar();
    }
}

// Try to parse a comment.
static inline bool readComment(void)
{
    if(*paLine != '#') {
        return false;
    }
    while(*paLine != '\0') {
        addChar();
    }
    addAscii('\0');
    return true;
}

// Try to read an integer, but if a parsed float is longer, do that.
static inline paToken readNumber(void)
{
    uchar c = *paLine;
    char *floatTail, *intTail;
    double floatVal;
    uint64 intVal;

    if(!isdigit(c) && c != '.') {
        return 0;
    }
    floatVal = strtod((char *)paLine, &floatTail);
    if(floatTail == (char *)paLine) {
        return 0;
    }
    intVal = strtoll((char *)paLine, &intTail, 0);
    if(intTail >= floatTail) {
        addChars(intTail - (char *)paLine);
        addAscii('\0');
        return paIntTokenCreate(intVal, paText);
    }
    addChars(floatTail - (char *)paLine);
    addAscii('\0');
    return paFloatTokenCreate(floatVal, paText);
}

// Try to read a string.
static inline bool readString(void)
{
    if(*paLine != '"') {
        return false;
    }
    paLine++; // Skip " character
    while(*paLine != '\0' && *paLine != '"') {
        if(*paLine == '\\') {
            paLine++;
            if(*paLine == 'n') {
                addAscii('\n');
                paLine++;
            } else if(*paLine == 'r') {
                addAscii('\r');
                paLine++;
            } else if(*paLine == 't') {
                addAscii('\t');
                paLine++;
            } else {
                addChar();
            }
        } else {
            addChar();
        }
    }
    if(*paLine != '"') {
        utError("Invalid string termination");
    }
    paLine++;
    addAscii('\0');
    return true;
}

// Try to read an operator.  We check up to 4-character long strings of ASCII
// punctuation characters, and accept the longest found.
static inline paToken readOperator(void)
{
    paKeyword keyword;
    uchar opString[5];
    int length;

    for(length = 0; length < 4 && ispunct(paLine[length]); length++) {
        opString[length] = paLine[length];
    }
    // Now check in the operator hash table for each size > 0, starting at
    // length.
    while(length) {
        opString[length] = '\0';
        keyword = paSyntaxFindKeyword(paCurrentSyntax, utSymCreate((char *)opString));
        if(keyword != paKeywordNull) {
            paLine += length;
            return paKeywordTokenCreate(keyword, opString);
        }
        length--;
    }
    return paTokenNull;
}

// Try to read a keyword.
static inline paToken lookForKeyword(void)
{
    paKeyword keyword = paSyntaxFindKeyword(paCurrentSyntax, utSymCreate((char *)paText));

    if(keyword == paKeywordNull) {
        return paTokenNull;
    }
    return paKeywordTokenCreate(keyword, paText);
}

// Read an identifier.  This should work so long as the first character is alpha
// numeric or is not a plain ASCII character (has it's high bit set).
static inline bool readIdentifier(void)
{
    uchar c = *paLine;

    if(!(c & 0x80) && !isalnum(c) && c != '\\') {
        return false;
    }
    while((c & 0x80) || isalnum(c) || c == '\\') {
        addChar();
        c = *paLine;
    }
    addAscii('\0');
    return true;
}

// Read one token, now that we know we've got some text to parse.
static paToken readToken(void)
{
    paToken token;

    if(readComment()){
        return paTokenCreate(PA_TOK_COMMENT, paText);
    }
    if(readString()) {
        return paTokenCreate(PA_TOK_STRING, paText);
    }
    token = readNumber();
    if(token != paTokenNull) {
        return token;
    }
    token = readOperator();
    if(token != paTokenNull) {
        return token;
    }
    if(readIdentifier()) {
        token = lookForKeyword();
        if(token != paTokenNull) {
            return token;
        }
        return paTokenCreate(PA_TOK_IDENT, paText);
    }
    // Must just be a single punctuation character
    addChar();
    addAscii('\0');
    return paTokenCreate(PA_TOK_CHAR, paText);
}

// Determine if paLine is nothing but spaces, tabs, and a single backslash.
static inline bool lineIsSlash(void)
{
    bool hasBackslash = false;
    uchar *p = paLine;
    uchar c;

    while((c = *p++) != '\0') {
        if(c == '\\') {
            if(hasBackslash) {
                return false;
            }
            hasBackslash = true;
        } else if(c != ' ' && c != '\t') {
            return false;
        }
    }
    return hasBackslash;
}

// Parse one token.
static paToken lexRawToken(void)
{
    paTextPos = 0;
    if(paLine == NULL) {
        paLine = utf8ReadLine(paFile);
        if(paLine == NULL) {
            return paTokenNull;
        }
        paLineNum++;
    }
    while(lineIsSlash()) {
        paLine = utf8ReadLine(paFile);
        if(paLine == NULL) {
            return paTokenNull;
        }
        paLineNum++;
    }
    paLine = skipSpace(paLine);
    if(*paLine == '\0') {
        paLine = NULL;
        return paTokenCreate(PA_TOK_NEWLINE, (uchar *)"\n");
    }
    return readToken();
}

// Determine if the input line is blank.
static inline bool lineIsBlank(void)
{
    uchar *p;
    uchar c;

    if(lineIsSlash()) {
        return true;
    }
    p = paLine;
    while((c = *p++) != '\0') {
        if(c != ' ' && c != '\t') {
            return false;
        }
    }
    return true;
}

// Skip blank lines in the input.
static void skipBlankLines(void)
{
    utAssert(paLine == NULL);
    paLine = utf8ReadLine(paFile);
    while(paLine != NULL && lineIsBlank()) {
        paLineNum++;
        paLine = utf8ReadLine(paFile);
    }
    if(paLine != NULL) {
        paLineNum++;
    }
}

// Parse a single token.
paToken paLex(void)
{
    paToken token;
    paTokenType type;
    char *text;

    token = lexRawToken();
    if(token == paTokenNull) {
        return token;
    }
    type = paTokenGetType(token);
    // Eat newlines inside grouping operators
    while(type == PA_TOK_NEWLINE && (paParenDepth > 0 || paBracketDepth > 0)) {
        paTokenDestroy(token);
        token = lexRawToken();
        type = paTokenGetType(token);
    }
    if(type == PA_TOK_OPERATOR) {
        text = (char *)paTokenGetText(token);
        if(!strcmp(text, "(")) {
            paParenDepth++;
        } else if(!strcmp(text, "[")) {
            paBracketDepth++;
        } else if(!strcmp(text, ")")) {
            paParenDepth--;
        } else if(!strcmp(text, "]")) {
            paBracketDepth--;
        }
    }
    return token;
}
