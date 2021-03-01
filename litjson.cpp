#include "litjson.h"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <iostream>

// assignment && copy-control
LitValue& LitValue::operator=(bool b) {
    if (type == LIT_STRING) str.~basic_string();
    type = (b ? LIT_TRUE : LIT_FALSE);
    return *this;
}

LitValue& LitValue::operator=(double d) {
    if (type == LIT_STRING) str.~basic_string();
    n = d;
    type = LIT_NUMBER;
    return *this;
}

LitValue& LitValue::operator=(const std::string& s) {
    if (type == LIT_STRING) {
        str = s;
    } else {
        new (&str) std::string(s);
    }
    type = LIT_STRING;
    return *this;
}

void LitValue::CopyUnion(const LitValue& v) {
    switch (v.type) {
        case LIT_NUMBER: n = v.n; break;
        case LIT_STRING: new (&str) std::string(v.str); break;
    }
}

LitValue& LitValue::operator=(const LitValue& v) {
    if (type == LIT_STRING && v.type != LIT_STRING) str.~basic_string();
    if (type == LIT_STRING && v.type == LIT_STRING) {
        str = v.str;
    } else {
        CopyUnion(v);
    }
    type = v.type;
    return *this;
}

// parse
void LitJson::LitParseWhitespace(LitContext* c) {
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
    c->json = p;
}

ParseResultType LitJson::LitParseNull(LitContext* c, LitValue* v) {
    assert(c->json[0] == 'n');
    if (c->json[1] != 'u' || c->json[2] != 'l' || c->json[3] != 'l') {
        return LIT_PARSE_INVALID_VALUE;
    }

    c->json += 4;

    lit_set_null(v);
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseTrue(LitContext* c, LitValue* v) {
    assert(c->json[0] == 't');
    if (c->json[1] != 'r' || c->json[2] != 'u' || c->json[3] != 'e') {
        return LIT_PARSE_INVALID_VALUE;
    }

    c->json += 4;

    lit_set_boolean(v, true);
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseFalse(LitContext* c, LitValue* v) {
    assert(c->json[0] == 'f');
    if (c->json[1] != 'a' || c->json[2] != 'l' || c->json[3] != 's' || c->json[4] != 'e') {
        return LIT_PARSE_INVALID_VALUE;
    }

    c->json += 5;

    lit_set_boolean(v, false);
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseNumber(LitContext* c, LitValue* v) {
    const char* p = c->json;

    // skip '-'
    if (*p == '-') ++p;

    // parse number
    if (*p == '0') {
        ++p;
    } else {
        if (!isdigit(*p)) return LIT_PARSE_INVALID_VALUE;
        while (isdigit(*p)) ++p;
    }

    // parse '.'
    if (*p == '.') {
        ++p;
        if (!isdigit(*p)) return LIT_PARSE_INVALID_VALUE;
        while (isdigit(*p)) ++p;
    }

    // parse 'e' or 'E'
    if (*p == 'e' || *p == 'E') {
        ++p;
        if (*p == '-' || *p == '+') ++p;
        if (!isdigit(*p)) return LIT_PARSE_INVALID_VALUE;
        while (isdigit(*p)) ++p;
    }

    // check range error
    errno = 0;

    lit_set_number(v, strtod(c->json, nullptr));
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
        v->type = LIT_NULL;
        return LIT_PARSE_NUMBER_TOO_BIG;
    }

    c->json = p;
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseString(LitContext* c, LitValue* v) {
    assert(c->json[0] == '\"');
    const char* p = c->json;
    ++p;
    while (true) {
        char ch = *p;
        switch (ch) {
            case '\"':
                lit_set_string(v, c->str);
                c->str.clear();
                c->json = ++p;
                return LIT_PARSE_OK;
            case '\\':
                ++p;
                switch (*p) {
                    case '\"': c->str.push_back('\"'); break;
                    case '\\': c->str.push_back('\\'); break;
                    case '/': c->str.push_back('/'); break;
                    case 'b': c->str.push_back('\b'); break;
                    case 'f': c->str.push_back('\f'); break;
                    case 'n': c->str.push_back('\n'); break;
                    case 'r': c->str.push_back('\r'); break;
                    case 't': c->str.push_back('\t'); break;
                    default: return LIT_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            case '\0': c->str.clear(); return LIT_PARSE_MISS_QUOTATION_MARK;
            default:
                if (static_cast<unsigned char>(ch) < 0x20) {
                    c->str.clear();
                    return LIT_PARSE_INVALID_STRING_CHAR;
                }
                c->str.push_back(ch);
        }
        ++p;
    }
}

ParseResultType LitJson::LitParseValue(LitContext* c, LitValue* v) {
    switch (*c->json) {
        case 'n': return LitParseNull(c, v);
        case 't': return LitParseTrue(c, v);
        case 'f': return LitParseFalse(c, v);
        case '\"': return LitParseString(c, v);
        case '\0': return LIT_PARSE_EXPECT_VALUE;
        default: return LitParseNumber(c, v);
    }
}

ParseResultType LitJson::LitParse(LitValue* v, const char* json) {
    LitContext c;
    assert(v != nullptr);
    c.json = json;
    v->type = LIT_NULL;
    LitParseWhitespace(&c);
    ParseResultType res = LitParseValue(&c, v);
    if (res == LIT_PARSE_OK) {
        LitParseWhitespace(&c);
        if (*c.json != '\0') {
            v->type = LIT_NULL;
            res = LIT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return res;
}

// set and get
LitType LitJson::lit_get_type(const LitValue* v) {
    assert(v != nullptr);
    return v->type;
}

void LitJson::lit_set_null(LitValue* v) {
    assert(v != nullptr);
    if (v->type == LIT_STRING) v->str.~basic_string();
    v->type = LIT_NULL;
}

bool LitJson::lit_get_boolean(const LitValue* v) {
    assert(v != nullptr && (v->type == LIT_TRUE || v->type == LIT_FALSE));
    return v->type;
}
void LitJson::lit_set_boolean(LitValue* v, bool b) {
    assert(v != nullptr);
    *v = b;
}

double LitJson::lit_get_number(const LitValue* v) {
    assert(v != nullptr && v->type == LIT_NUMBER);
    return v->n;
}
void LitJson::lit_set_number(LitValue* v, double n) {
    assert(v != nullptr);
    *v = n;
}

std::string LitJson::lit_get_string(const LitValue* v) {
    assert(v != nullptr && v->type == LIT_STRING);
    return v->str;
}
void LitJson::lit_set_string(LitValue* v, const std::string& s) {
    assert(v != nullptr);
    *v = s;
}