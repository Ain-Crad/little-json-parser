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
    if (type == LIT_ARRAY) arr.~vector();

    type = (b ? LIT_TRUE : LIT_FALSE);
    return *this;
}

LitValue& LitValue::operator=(double d) {
    if (type == LIT_STRING) str.~basic_string();
    if (type == LIT_ARRAY) arr.~vector();

    n = d;
    type = LIT_NUMBER;
    return *this;
}

LitValue& LitValue::operator=(const std::string& s) {
    if (type == LIT_ARRAY) arr.~vector();

    if (type == LIT_STRING) {
        str = s;
    } else {
        new (&str) std::string(s);
    }
    type = LIT_STRING;
    return *this;
}

LitValue& LitValue::operator=(const std::vector<std::shared_ptr<LitValue>>& a) {
    if (type == LIT_STRING) str.~basic_string();

    if (type == LIT_ARRAY) {
        arr = a;
    } else {
        new (&arr) std::vector<std::shared_ptr<LitValue>>(a);
    }
    type = LIT_STRING;
    return *this;
}

void LitValue::CopyUnion(const LitValue& v) {
    switch (v.type) {
        case LIT_NUMBER: n = v.n; break;
        case LIT_STRING: new (&str) std::string(v.str); break;
        case LIT_ARRAY: new (&arr) std::vector<std::shared_ptr<LitValue>>(v.arr); break;
    }
}

LitValue& LitValue::operator=(const LitValue& v) {
    if (type == LIT_STRING && v.type != LIT_STRING) str.~basic_string();
    if (type == LIT_ARRAY && v.type != LIT_ARRAY) arr.~vector();

    if (type == LIT_STRING && v.type == LIT_STRING) {
        str = v.str;
    } else if (type == LIT_ARRAY && v.type == LIT_ARRAY) {
        arr = v.arr;
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

ParseResultType DealStringError(ParseResultType t, LitContext* c) {
    c->str.clear();
    return t;
}

ParseResultType LitJson::LitParseString(LitContext* c, LitValue* v) {
    assert(c->json[0] == '\"');
    unsigned uh = 0, ul = 0;
    const char* p = c->json;
    ++p;
    while (true) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                lit_set_string(v, c->str);
                c->str.clear();
                c->json = p;
                return LIT_PARSE_OK;
            case '\\':
                switch (*p++) {
                    case '\"': c->str.push_back('\"'); break;
                    case '\\': c->str.push_back('\\'); break;
                    case '/': c->str.push_back('/'); break;
                    case 'b': c->str.push_back('\b'); break;
                    case 'f': c->str.push_back('\f'); break;
                    case 'n': c->str.push_back('\n'); break;
                    case 'r': c->str.push_back('\r'); break;
                    case 't': c->str.push_back('\t'); break;
                    case 'u':
                        if (!(p = LitParseUnicode(p, &uh))) return DealStringError(LIT_PARSE_INVALID_UNICODE_HEX, c);
                        if (uh >= 0xDC00 && uh <= 0xDFFF) return DealStringError(LIT_PARSE_INVALID_UNICODE_HEX, c);
                        if (uh >= 0xD800 && uh <= 0xDBFF) {
                            if (*p++ != '\\') return DealStringError(LIT_PARSE_INVALID_UNICODE_SURROGATE, c);
                            if (*p++ != 'u') return DealStringError(LIT_PARSE_INVALID_UNICODE_SURROGATE, c);
                            if (!(p = LitParseUnicode(p, &ul)))
                                return DealStringError(LIT_PARSE_INVALID_UNICODE_HEX, c);
                            if (ul < 0xDC00 || ul > 0xDFFF)
                                return DealStringError(LIT_PARSE_INVALID_UNICODE_SURROGATE, c);
                            uh = 0x10000 + (((uh - 0xD800) << 10) | (ul - 0xDC00));
                        }
                        LitEncodeUTF8(c, uh);
                        break;
                    default: return LIT_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            case '\0': return DealStringError(LIT_PARSE_MISS_QUOTATION_MARK, c);
            default:
                if (static_cast<unsigned char>(ch) < 0x20) return DealStringError(LIT_PARSE_INVALID_STRING_CHAR, c);
                c->str.push_back(ch);
        }
    }
}

const char* LitJson::LitParseUnicode(const char* p, unsigned int* u) {
    *u = 0;
    for (int i = 0; i < 4; ++i) {
        char ch = *p++;
        *u <<= 4;
        if (ch >= '0' && ch <= '9') {
            *u |= ch - '0';
        } else if (ch >= 'A' && ch <= 'F') {
            *u |= ch - 'A' + 10;
        } else if (ch >= 'a' && ch <= 'f') {
            *u |= ch - 'a' + 10;
        } else {
            return nullptr;
        }
    }
    return p;
}

void LitJson::LitEncodeUTF8(LitContext* c, unsigned int u) {
    if (u <= 0x7F) {
        c->str.push_back(u);
    } else if (u <= 0x7FF) {
        c->str.push_back(0xC0 | ((u >> 6) & 0xFF));
        c->str.push_back(0x80 | (u & 0x3F));
    } else if (u <= 0xFFFF) {
        c->str.push_back(0xE0 | ((u >> 12) & 0xFF));
        c->str.push_back(0x80 | ((u >> 6) & 0x3F));
        c->str.push_back(0x80 | (u & 0x3F));
    } else {
        assert(u <= 0x10FFFF);
        c->str.push_back(0xF0 | ((u >> 18) & 0xFF));
        c->str.push_back(0x80 | ((u >> 12) & 0x3F));
        c->str.push_back(0x80 | ((u >> 6) & 0x3F));
        c->str.push_back(0x80 | (u & 0x3F));
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