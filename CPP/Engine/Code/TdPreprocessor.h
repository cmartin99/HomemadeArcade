#pragma once

#define introspect(params)

namespace eng {

enum TdTokenType
{
    tt_Unknown,
    tt_EndOfStream,
    tt_OpenParen,
    tt_CloseParen,
    tt_OpenBracket,
    tt_CloseBracket,
    tt_OpenBrace,
    tt_CloseBrace,
    tt_Colon,
    tt_SemiColon,
    tt_Asterisk,
    tt_String,
    tt_Identifier,
};

struct TdToken
{
    TdTokenType type;
    int32 length;
    char *text;
};

struct TdTokenizer
{
    char *pos;
};

bool TdTokenEquals(const TdToken *, const char *text);
TdToken TdGetToken(TdTokenizer *);
int32 TdProcessFileForIntrospection(const char *filename);

}