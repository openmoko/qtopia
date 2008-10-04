/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include <QList>
#include <QByteArray>

struct Keyword {
    const char *lexem;
    const char *token;
};

struct State
{
    State(const char* token) : token(token)
    {
        ::memset(next, 0, sizeof(next));
    }
    State(const State &other) : token(other.token)
    {
        ::memcpy(next, other.next, sizeof(next));
    }
    State &operator=(const State &other)
    {
        token = other.token;
        ::memcpy(next, other.next, sizeof(next));
        return *this;
    }

    QByteArray token;
    int next[128];
};

Keyword keywords[] =
{
    {"<", "LANGLE" },
    {">", "RANGLE" },
    {"(", "LPAREN" },
    {")", "RPAREN" },
    {"{", "LBRACE"},
    {"}", "RBRACE"},
    {"[", "LBRACKET"},
    {"]", "RBRACKET"},
    {",", "COMMA" },
    {"\n", "NEWLINE" },
    {"\n\r", "NEWLINE" },
    {"\r\n", "NEWLINE" },
    {"+", "PLUS" },
    {"-", "MINUS" },
    {"*", "STAR" },
    {"=", "EQUAL" },
    {"+=", "PLUS_EQ" },
    {"-=", "MINUS_EQ" },
    {"*=", "STAR_EQ" },
    {"~=", "TILDE_EQ" },
    {".", "DOT"},
    {"?", "QUESTION"},
    {"@", "AT"},
    {"&", "AMPERSAND"},
    {"\"", "QUOTE"},
    {" ", "WHITESPACE"},
    {"\t", "WHITESPACE"},
    {"$", "DOLLAR"},
    {"$$", "DOLLAR_DOLLAR"},
    {"#", "HASH"},
    {"\\", "BACKSLASH"},
    {"/", "SLASH"},
    {":", "COLON"},
    {";", "SEMICOLON"},
    {"!", "NOT"},
    {"|", "PIPE"},
    {"'", "SINGLE_QUOTE"},
    {"`", "BACK_QUOTE"},
    {"^", "CARROT"},
    {"%", "PERCENT"},
    {"else", "ELSE"},
    {"<script>", "SCRIPT_START"},
    {"</script>", "SCRIPT_END"},
    {0, 0}
};

bool is_character(char s)
{
    return (s >= 'a' && s <= 'z') ||
           (s >= 'A' && s <= 'Z') ||
           (s >= '0' && s <= '9') ||
           s == '_';
}

void newState(QList<State> &states, const char *token, const char *lexem)
{
    int state = 0;
    bool character = is_character(*lexem);

    while(*lexem) {
        int next = states[state].next[(int)*lexem];

        if(!next) {
            next = states.size();
            states += State(character?"CHARACTER":"INCOMPLETE");
            states[state].next[(int)*lexem] = next;
        }

        state = next;
        ++lexem;
        character = character && is_character(*lexem);
    }

    states[state].token = token;
}

void newState(QList<State> &states, const char *token, char lexem)
{
    int next = states[0].next[(int)lexem];
    if(!next) {
        next = states.size();
        states += State(token);
        states[0].next[(int)lexem] = next;
    } else {
        states[next].token = token;
    }
}

int main()
{
    QList<State> states;
    states += State("NOTOKEN");

    // identifiers
    for (int cc = 'a'; cc <= 'z'; ++cc)
        newState(states, "CHARACTER", cc);
    for (int cc = 'A'; cc <= 'Z'; ++cc)
        newState(states, "CHARACTER", cc);
    newState(states, "CHARACTER", '_');

    // add digits
    for(int cc = '0'; cc <= '9'; ++cc)
        newState(states, "DIGIT", cc);

    // keywords
    for(int ii = 0; keywords[ii].lexem; ++ii)
        newState(states, keywords[ii].token, keywords[ii].lexem);

    ::printf("static const struct\n{\n"
             "    Token token;\n"
             "    char next[128];\n"
             "} keywords[] = {\n");

    for(int ii = 0; ii < states.size(); ++ii) {
        printf("%s    { %s, { ", ii?",\n":"", states[ii].token.data());
        for(int jj = 0; jj < 128; jj++) 
            printf("%s%d", jj?",":"", states[ii].next[jj]);
        printf(" } }");
    }

    printf("\n};\n");
}
