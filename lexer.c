/* Parse the input tokens.  I switched from flex because input files are in UTF-8,
   and it looks like flex doesn't yet support it.  Check operators  while
   parsing so we keep multi-character operators together.  Otherwize, just split
   tokens at spaces and boundaries between alnum and non-alnum characters. */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "co.h"

static uchar *coLine;
static uchar *coText;
static size_t coTextSize, coTextPos;
static uint32 coParenDepth, coBracketDepth, coBraceDepth;
static bool coLastWasNewline;
static uint32 coIndentDepth;
static coToken coLastToken;
static uint32 coPendingEndTokens;

// Print out an error message and exit.
void coError(
    coToken token,
    char *message,
    ...)
{
    char *buff;
    va_list ap;

    va_start(ap, message);
    buff = utVsprintf((char *)message, ap);
    va_end(ap);
    utError("Line %d, token \"%s\": %s", coTokenGetLineNum(token),
            coTokenGetText(token), buff);
}

// Initialize lexer.
void coLexerStart(void)
{
    coLine = NULL;
    coTextSize = 256;
    coText = (uchar *)calloc(coTextSize, sizeof(uchar));
    coParenDepth = 0;
    coBracketDepth = 0;
    coBraceDepth = 0;
    coIndentDepth = 0;
    coLastWasNewline = true;
    coLastToken = coTokenNull;
    coPendingEndTokens = 0;
}

// Stop the lexer.
void coLexerStop(void)
{
}

// Just print the contents of the token
void coPrintToken(
    coToken token)
{
    printf("%-6u ", coTokenGetLineNum(token));
    switch(coTokenGetType(token)) {
    case CO_TOK_INTEGER:
        printf("INTEGER: %llu\n", coTokenGetIntVal(token));
        break;
    case CO_TOK_FLOAT:
        printf("FLOAT: %g\n", coTokenGetFloatVal(token));
        break;
    case CO_TOK_STRING:
        printf("STRING: %s\n", coTokenGetText(token));
        break;
    case CO_TOK_NEWLINE:
        printf("NEWLINE\n");
        break;
    case CO_TOK_TAB:
        printf("TAB\n");
        break;
    case CO_TOK_CHAR:
        printf("CHAR: %s\n", coTokenGetText(token));
        break;
    case CO_TOK_IDENT:
        printf("IDENT: %s\n", coTokenGetText(token));
        break;
    case CO_TOK_OPERATOR:
        printf("OPERATOR: %s\n", coTokenGetText(token));
        break;
    case CO_TOK_COMMENT:
        printf("COMMENT: %s\n", coTokenGetText(token));
        break;
    case CO_TOK_KEYWORD:
        printf("KEYWORD: %s\n", coTokenGetText(token));
        break;
    case CO_TOK_BEGIN:
        printf("BEGIN\n");
        break;
    case CO_TOK_END:
        printf("END\n");
        break;
    default:
        utExit("Unknown token type");
    }
}

// Create a token object.
static inline coToken coTokenCreate(
    coTokenType type,
    uchar *text)
{
    coToken token = coTokenAlloc();

    coTokenSetType(token, type);
    coTokenSetText(token, text, strlen((char *)text) + 1);
    coTokenSetLineNum(token, coLineNum);
    coLastToken = token;
    return token;
}

// Create a new integer token.
static inline coToken coIntTokenCreate(
    uint64 intVal,
    uchar *text)
{
    coToken token = coTokenCreate(CO_TOK_INTEGER, text);

    coTokenSetIntVal(token, intVal);
    return token;
}

// Create a new float token.
static inline coToken coFloatTokenCreate(
    double floatVal,
    uchar *text)
{
    coToken token = coTokenCreate(CO_TOK_FLOAT, text);

    coTokenSetFloatVal(token, floatVal);
    return token;
}

// Create a new operator or keyword token.  Just check to see if the keyword is
// owned by an operator or staterule.
static inline coToken coKeywordTokenCreate(
    coKeyword keyword,
    uchar *text)
{
    coPattern pattern = coElementGetPattern(coKeywordGetFirstElement(keyword));
    coToken token;

    if(coPatternGetOperator(pattern) == coOperatorNull) {
        token = coTokenCreate(CO_TOK_KEYWORD, text);
    } else {
        token = coTokenCreate(CO_TOK_OPERATOR, text);
    }
    coTokenSetKeywordVal(token, keyword);
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

// Add a character to coText from coLine.
static inline void addChar(void)
{
    int length = utf8FindLength(*coLine);

    if(coTextPos + length > coTextSize) {
        coTextSize <<= 1;
        coText = (uchar *)realloc(coText, coTextSize*sizeof(uchar));
    }
    while(length--) {
        coText[coTextPos++] = *coLine++;
    }
}

// Add an ASCII character to coText from coLine.
static inline void addAscii(
    uchar c)
{
    if(coTextPos >= coTextSize) {
        coTextSize <<= 1;
        coText = (uchar *)realloc(coText, coTextSize*sizeof(uchar));
    }
    coText[coTextPos++] = c;
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
    if(*coLine != '#') {
        return false;
    }
    while(*coLine != '\0') {
        addChar();
    }
    addAscii('\0');
    return true;
}

// Try to read an integer, but if a parsed float is longer, do that.
static inline coToken readNumber(void)
{
    uchar c = *coLine;
    char *floatTail, *intTail;
    double floatVal;
    uint64 intVal;

    if(!isdigit(c) && c != '.') {
        return 0;
    }
    floatVal = strtod((char *)coLine, &floatTail);
    if(floatTail == (char *)coLine) {
        return 0;
    }
    intVal = strtoll((char *)coLine, &intTail, 0);
    if(intTail >= floatTail) {
        addChars(intTail - (char *)coLine);
        addAscii('\0');
        return coIntTokenCreate(intVal, coText);
    }
    addChars(floatTail - (char *)coLine);
    addAscii('\0');
    return coFloatTokenCreate(floatVal, coText);
}

// Try to read a string.
static inline bool readString(void)
{
    if(*coLine != '"') {
        return false;
    }
    coLine++; // Skip " character
    while(*coLine != '\0' && *coLine != '"') {
        if(*coLine == '\\') {
            coLine++;
            if(*coLine == 'n') {
                addAscii('\n');
                coLine++;
            } else if(*coLine == 'r') {
                addAscii('\r');
                coLine++;
            } else if(*coLine == 't') {
                addAscii('\t');
                coLine++;
            } else {
                addChar();
            }
        } else {
            addChar();
        }
    }
    if(*coLine != '"') {
        utError("Invalid string termination");
    }
    coLine++;
    addAscii('\0');
    return true;
}

// Try to read an operator.  We check up to 4-character long strings of ASCII
// punctuation characters, and accept the longest found.
static inline coToken readOperator(void)
{
    coKeyword keyword;
    uchar opString[5];
    int length;

    for(length = 0; length < 4 && ispunct(coLine[length]); length++) {
        opString[length] = coLine[length];
    }
    // Now check in the operator hash table for each size > 0, starting at
    // length.
    while(length) {
        opString[length] = '\0';
        keyword = coSyntaxFindKeyword(coCurrentSyntax, utSymCreate((char *)opString));
        if(keyword != coKeywordNull) {
            coLine += length;
            return coKeywordTokenCreate(keyword, opString);
        }
        length--;
    }
    return coTokenNull;
}

// Try to read a keyword.
static inline coToken lookForKeyword(void)
{
    coKeyword keyword = coSyntaxFindKeyword(coCurrentSyntax, utSymCreate((char *)coText));

    if(keyword == coKeywordNull) {
        return coTokenNull;
    }
    return coKeywordTokenCreate(keyword, coText);
}

// Read an identifier.  This should work so long as the first character is alpha
// numeric or is not a plain ASCII character (has it's high bit set).
static inline bool readIdentifier(void)
{
    uchar c = *coLine;

    if(!(c & 0x80) && !isalnum(c) && c != '\\') {
        return false;
    }
    while((c & 0x80) || isalnum(c) || c == '\\') {
        addChar();
        c = *coLine;
    }
    addAscii('\0');
    return true;
}

// Read one token, now that we know we've got some text to parse.
static coToken readToken(void)
{
    coToken token;

    if(readComment()){
        return coTokenCreate(CO_TOK_COMMENT, coText);
    }
    if(readString()) {
        return coTokenCreate(CO_TOK_STRING, coText);
    }
    token = readNumber();
    if(token != coTokenNull) {
        return token;
    }
    token = readOperator();
    if(token != coTokenNull) {
        return token;
    }
    if(readIdentifier()) {
        token = lookForKeyword();
        if(token != coTokenNull) {
            return token;
        }
        return coTokenCreate(CO_TOK_IDENT, coText);
    }
    // Must just be a single punctuation character
    addChar();
    addAscii('\0');
    if(*coText == '\t') {
        return coTokenCreate(CO_TOK_TAB, coText);
    }
    return coTokenCreate(CO_TOK_CHAR, coText);
}

// Determine if coLine is nothing but spaces, tabs, and a single backslash.
static inline bool lineIsSlash(void)
{
    bool hasBackslash = false;
    uchar *p = coLine;
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
static coToken lexRawToken(void)
{
    coTextPos = 0;
    if(coLine == NULL) {
        coLine = utf8ReadLine(coFile);
        if(coLine == NULL) {
            return coTokenNull;
        }
        coLineNum++;
    }
    while(lineIsSlash()) {
        coLine = utf8ReadLine(coFile);
        if(coLine == NULL) {
            return coTokenNull;
        }
        coLineNum++;
    }
    coLine = skipSpace(coLine);
    if(*coLine == '\0') {
        coLine = NULL;
        return coTokenCreate(CO_TOK_NEWLINE, (uchar *)"\n");
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
    p = coLine;
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
    utAssert(coLine == NULL);
    coLine = utf8ReadLine(coFile);
    while(coLine != NULL && lineIsBlank()) {
        coLineNum++;
        coLine = utf8ReadLine(coFile);
    }
    if(coLine != NULL) {
        coLineNum++;
    }
}

// Eat NEWLINE and TAB tokens inside parens, brackets, or braces.
static coToken readTokenWithoutEmbeddedNewlines(void)
{
    // TODO: Should we use the group operators to determine when to drop NEWLINES?
    coToken token;
    coTokenType type;
    char *text;

    token = lexRawToken();
    if(token == coTokenNull) {
        return token;
    }
    type = coTokenGetType(token);
    // Eat newlines inside grouping operators
    while((type == CO_TOK_NEWLINE || type == CO_TOK_TAB) &&
            (coParenDepth > 0 || coBracketDepth > 0 || coBraceDepth > 0)) {
        coTokenDestroy(token);
        token = lexRawToken();
        type = coTokenGetType(token);
    }
    if(type == CO_TOK_OPERATOR) {
        text = (char *)coTokenGetText(token);
        if(!strcmp(text, "(")) {
            coParenDepth++;
        } else if(!strcmp(text, "[")) {
            coBracketDepth++;
        } else if(!strcmp(text, "{")) {
            coBraceDepth++;
        } else if(!strcmp(text, ")")) {
            coParenDepth--;
        } else if(!strcmp(text, "]")) {
            coBracketDepth--;
        } else if(!strcmp(text, "}")) {
            coBraceDepth--;
        }
    }
    return token;
}

// Count TAB tokens in the input stream, throw them away, and return the number
// thrown out.  This can only be called after finishing reading a line.
static uint32 readTabTokens(void)
{
    uint32 numTabs = 0;
    uint32 numSpaces = 0;

    skipBlankLines();
    if(coLine == NULL) {
        return 0;
    }
    while(*coLine == '\t') {
        numTabs++;
        coLine++;
    }
    while(*coLine == ' ') {
        numSpaces++;
        coLine++;
    }
    if(*coLine == '\t') {
        utError("Line %d: spaces before tab found.  It makes my brain hurt!.");
    }
    if((numSpaces & 0x3) != 0) {
        utError("Line %d: Indentation must be by 4 spaces.  Try using tabs instead.");
    }
    return numTabs + (numSpaces >> 2);
}

// Detect NEWLINE TAB* and use the tab depth to introduce BEGIN/END tokens.
coToken coLex(void)
{
    coToken token;
    uint32 depth;

    if(coPendingEndTokens != 0) {
        coPendingEndTokens--;
        return coTokenCreate(CO_TOK_END, (uchar *)"");
    }
    if(!coLastWasNewline) {
        token = readTokenWithoutEmbeddedNewlines();
        if(token == coTokenNull) {
            return coTokenNull;
        }
        coLastWasNewline = coTokenGetType(token) == CO_TOK_NEWLINE;
        return token;
    }
    depth = readTabTokens();
    coLastWasNewline = false; // readTabTokens reads until non-NEWLINE
    if(depth > coIndentDepth) {
        if(depth > coIndentDepth + 1) {
            coError(coLastToken, "Indentation too deep");
        }
        coIndentDepth = depth;
        return coTokenCreate(CO_TOK_BEGIN, (uchar *)"");
    } else if(depth < coIndentDepth) {
        coPendingEndTokens = coIndentDepth - depth - 1;
        coIndentDepth = depth;
        return coTokenCreate(CO_TOK_END, (uchar *)"");
    }
    token = readTokenWithoutEmbeddedNewlines();
    if(token == coTokenNull) {
        return coTokenNull;
    }
    coLastWasNewline = coTokenGetType(token) == CO_TOK_NEWLINE;
    return token;
}
