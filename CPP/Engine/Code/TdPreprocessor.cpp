#include "TdPreprocessor.h"

namespace eng {

inline static bool IsAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline static bool IsNumeric(char c)
{
    return c >= '0' && c <= '9';
}

inline static bool IsEndOfLine(char c)
{
    return c == '\n' || c == '\r';
}

inline static bool IsWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\v' || c == '\f' || IsEndOfLine(c);
}

static void EatWhitespace(TdTokenizer* t)
{
    for (;;)
    {
        if (IsWhitespace(t->pos[0]))
        {
            ++t->pos;
        }
        else if (t->pos[0] == '/' && t->pos[1] == '/')
        {
            t->pos += 2;
            while (!IsEndOfLine(t->pos[0])) ++t->pos;
        }
        else if (t->pos[0] == '/' && t->pos[1] == '*')
        {
            t->pos += 2;
            for (;;)
            {
                if (!t->pos[0]) break;
                if (t->pos[0] == '*' && t->pos[1] == '/')
                {
                    t->pos += 2;
                    break;
                }
                ++t->pos;
            }
        }
        else
        {
            break;
        }
    }
}

bool TdTokenEquals(const TdToken* token, const char* text)
{
    for (int32 i = 0; i < token->length; ++i, ++text)
    {
        if (text[0] == '\0') return false;
        if (token->text[i] != text[0]) return false;
    }
    return true;
}

TdToken TdGetToken(TdTokenizer* t)
{
    EatWhitespace(t);

    TdToken token = {};
    token.text = t->pos;
    token.length = 1;

    char c = t->pos[0];
    ++t->pos;
    switch (c)
    {
        case '\0': { token.type = tt_EndOfStream; } break;
        case '(': { token.type = tt_OpenParen; } break;
        case ')': { token.type = tt_CloseParen; } break;
        case '[': { token.type = tt_OpenBracket; } break;
        case ']': { token.type = tt_CloseBracket; } break;
        case '{': { token.type = tt_OpenBrace; } break;
        case '}': { token.type = tt_CloseBrace; } break;
        case ':': { token.type = tt_Colon; } break;
        case ';': { token.type = tt_SemiColon; } break;
        case '*': { token.type = tt_Asterisk; } break;

        case '"':
        {
            token.type = tt_String;
            token.text = t->pos;
            while (t->pos[0] && t->pos[0] != '"')
            {
                if (t->pos[0] == '\\' && t->pos[1]) ++t->pos;
                ++t->pos;
            }
            token.length = t->pos - token.text;
            if (t->pos[0]) ++t->pos;
        } break;

        default:
        {
            if (IsAlpha(c))
            {
                token.type = tt_Identifier;
                while (IsAlpha(t->pos[0]) || IsNumeric(t->pos[0]) || t->pos[0] == '_') ++t->pos;
                token.length = t->pos - token.text;
            }
            else
                token.type = tt_Unknown;
        } break;
    }

    return token;
}

int32 TdProcessFileForIntrospection(const char* filename)
{
    char * data = tdReadAllTextFileAndNullTerminate(filename);
    if (!data) return 0;

    TdTokenizer tokenizer = {};
    tokenizer.pos = data;

    bool parsing = true;
    while (parsing)
    {
        TdToken token = TdGetToken(&tokenizer);
        switch (token.type)
        {
            case tt_EndOfStream: { parsing = false; } break;
            case tt_OpenParen:
            case tt_CloseParen:
            case tt_OpenBracket:
            case tt_CloseBracket:
            case tt_OpenBrace:
            case tt_Colon:
            case tt_SemiColon:
            case tt_Asterisk:
            case tt_String:
                break;

            case tt_CloseBrace:
                break;

            default:
            {
                printf("%d: %.*s\n", token.type, token.length, token.text);
            } break;
        }
    }

    free(data);
    return 0;
}

}
