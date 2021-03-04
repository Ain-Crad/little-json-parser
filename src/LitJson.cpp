#include "litjson.h"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <iostream>

// parse
void LitJson::LitParseWhitespace() {
    assert(cur != nullptr);
    while (*cur == ' ' || *cur == '\t' || *cur == '\n' || *cur == '\r') ++cur;
}

ParseResultType LitJson::LitParseNull(LitValue* v) {
    assert(cur != nullptr && cur[0] == 'n');
    if (cur[1] != 'u' || cur[2] != 'l' || cur[3] != 'l') {
        return LIT_PARSE_INVALID_VALUE;
    }

    cur += 4;

    lit_set_null(v);
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseTrue(LitValue* v) {
    assert(cur != nullptr && cur[0] == 't');
    if (cur[1] != 'r' || cur[2] != 'u' || cur[3] != 'e') {
        return LIT_PARSE_INVALID_VALUE;
    }

    cur += 4;

    lit_set_boolean(v, true);
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseFalse(LitValue* v) {
    assert(cur != nullptr && cur[0] == 'f');
    if (cur[1] != 'a' || cur[2] != 'l' || cur[3] != 's' || cur[4] != 'e') {
        return LIT_PARSE_INVALID_VALUE;
    }

    cur += 5;

    lit_set_boolean(v, false);
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseNumber(LitValue* v) {
    const char* p = cur;

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

    lit_set_number(v, strtod(cur, nullptr));
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
        v->type = LIT_NULL;
        return LIT_PARSE_NUMBER_TOO_BIG;
    }

    cur = p;
    return LIT_PARSE_OK;
}

ParseResultType DealStringError(ParseResultType t, std::string* buff) {
    buff->clear();
    return t;
}

ParseResultType LitJson::LitParseString(LitValue* v) {
    ParseResultType res;
    std::string buff;
    if ((res = LitParseStringRaw(&buff)) == LIT_PARSE_OK) {
        lit_set_string(v, buff);
    }
    return res;
}

ParseResultType LitJson::LitParseStringRaw(std::string* buff) {
    assert(cur != nullptr && cur[0] == '\"');
    unsigned uh = 0, ul = 0;
    const char* p = cur;
    ++p;
    while (true) {
        char ch = *p++;
        switch (ch) {
            case '\"': cur = p; return LIT_PARSE_OK;
            case '\\':
                switch (*p++) {
                    case '\"': buff->push_back('\"'); break;
                    case '\\': buff->push_back('\\'); break;
                    case '/': buff->push_back('/'); break;
                    case 'b': buff->push_back('\b'); break;
                    case 'f': buff->push_back('\f'); break;
                    case 'n': buff->push_back('\n'); break;
                    case 'r': buff->push_back('\r'); break;
                    case 't': buff->push_back('\t'); break;
                    case 'u':
                        if (!(p = LitParseUnicode(p, &uh))) return DealStringError(LIT_PARSE_INVALID_UNICODE_HEX, buff);
                        if (uh >= 0xDC00 && uh <= 0xDFFF) return DealStringError(LIT_PARSE_INVALID_UNICODE_HEX, buff);
                        if (uh >= 0xD800 && uh <= 0xDBFF) {
                            if (*p++ != '\\') return DealStringError(LIT_PARSE_INVALID_UNICODE_SURROGATE, buff);
                            if (*p++ != 'u') return DealStringError(LIT_PARSE_INVALID_UNICODE_SURROGATE, buff);
                            if (!(p = LitParseUnicode(p, &ul)))
                                return DealStringError(LIT_PARSE_INVALID_UNICODE_HEX, buff);
                            if (ul < 0xDC00 || ul > 0xDFFF)
                                return DealStringError(LIT_PARSE_INVALID_UNICODE_SURROGATE, buff);
                            uh = 0x10000 + (((uh - 0xD800) << 10) | (ul - 0xDC00));
                        }
                        LitEncodeUTF8(buff, uh);
                        break;
                    default: return LIT_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            case '\0': return DealStringError(LIT_PARSE_MISS_QUOTATION_MARK, buff);
            default:
                if (static_cast<unsigned char>(ch) < 0x20) return DealStringError(LIT_PARSE_INVALID_STRING_CHAR, buff);
                buff->push_back(ch);
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

void LitJson::LitEncodeUTF8(std::string* buff, unsigned int u) {
    if (u <= 0x7F) {
        buff->push_back(u);
    } else if (u <= 0x7FF) {
        buff->push_back(0xC0 | ((u >> 6) & 0xFF));
        buff->push_back(0x80 | (u & 0x3F));
    } else if (u <= 0xFFFF) {
        buff->push_back(0xE0 | ((u >> 12) & 0xFF));
        buff->push_back(0x80 | ((u >> 6) & 0x3F));
        buff->push_back(0x80 | (u & 0x3F));
    } else {
        assert(u <= 0x10FFFF);
        buff->push_back(0xF0 | ((u >> 18) & 0xFF));
        buff->push_back(0x80 | ((u >> 12) & 0x3F));
        buff->push_back(0x80 | ((u >> 6) & 0x3F));
        buff->push_back(0x80 | (u & 0x3F));
    }
}

ParseResultType LitJson::LitParseArray(LitValue* v) {
    assert(cur != nullptr && cur[0] == '[');
    ++cur;
    LitParseWhitespace();
    if (*cur == ']') {
        ++cur;
        lit_set_array(v, {});
        return LIT_PARSE_OK;
    }

    std::vector<LitValue> aux;
    ParseResultType res = LIT_PARSE_INVALID_VALUE;
    while (true) {
        LitValue t;
        if ((res = LitParseValue(&t)) != LIT_PARSE_OK) return res;

        aux.push_back(t);
        LitParseWhitespace();
        if (*cur == ',') {
            ++cur;
            LitParseWhitespace();
        } else if (*cur == ']') {
            ++cur;
            lit_set_array(v, aux);
            return LIT_PARSE_OK;
        } else {
            return LIT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
        }
    }
}

ParseResultType LitJson::LitParseObject(LitValue* v) {
    assert(cur != nullptr && cur[0] == '{');
    ++cur;
    LitParseWhitespace();
    if (*cur == '}') {
        ++cur;
        lit_set_object(v, {});
        return LIT_PARSE_OK;
    }

    LitValue::Obj aux;
    ParseResultType res = LIT_PARSE_INVALID_VALUE;
    while (true) {
        std::string key;
        LitValue value;
        if (*cur != '\"') return LIT_PARSE_MISS_KEY;
        if ((res = LitParseStringRaw(&key)) != LIT_PARSE_OK) return res;
        LitParseWhitespace();
        if (*cur != ':') return LIT_PARSE_MISS_COLON;
        ++cur;
        LitParseWhitespace();
        if ((res = LitParseValue(&value)) != LIT_PARSE_OK) return res;
        aux.push_back({key, value});
        LitParseWhitespace();
        if (*cur == ',') {
            ++cur;
            LitParseWhitespace();
        } else if (*cur == '}') {
            ++cur;
            lit_set_object(v, aux);
            return LIT_PARSE_OK;
        } else {
            return LIT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
        }
    }
}

ParseResultType LitJson::LitParseValue(LitValue* v) {
    assert(cur != nullptr);
    switch (*cur) {
        case 'n': return LitParseNull(v);
        case 't': return LitParseTrue(v);
        case 'f': return LitParseFalse(v);
        case '\"': return LitParseString(v);
        case '\0': return LIT_PARSE_EXPECT_VALUE;
        case '[': return LitParseArray(v);
        case '{': return LitParseObject(v);
        default: return LitParseNumber(v);
    }
}

ParseResultType LitJson::LitParse(LitValue* v, const char* json) {
    assert(v != nullptr);
    cur = json;
    LitParseWhitespace();
    ParseResultType res = LitParseValue(v);
    if (res == LIT_PARSE_OK) {
        LitParseWhitespace();
        if (*cur != '\0') {
            v->type = LIT_NULL;
            res = LIT_PARSE_ROOT_NOT_SINGULAR;
        }
    } else {
        v->type = LIT_NULL;
    }
    return res;
}

// set and get
LitType LitJson::lit_get_type(const LitValue& v) { return v.type; }

void LitJson::lit_set_null(LitValue* v) {
    assert(v != nullptr);
    if (v->type == LIT_STRING) v->str.~basic_string();
    if (v->type == LIT_ARRAY) v->arr.~vector<LitValue>();
    v->type = LIT_NULL;
}

bool LitJson::lit_get_boolean(const LitValue& v) {
    assert(v.type == LIT_TRUE || v.type == LIT_FALSE);
    return v.type;
}
void LitJson::lit_set_boolean(LitValue* v, bool b) {
    assert(v != nullptr);
    *v = b;
}

double LitJson::lit_get_number(const LitValue& v) {
    assert(v.type == LIT_NUMBER);
    return v.n;
}
void LitJson::lit_set_number(LitValue* v, double n) {
    assert(v != nullptr);
    *v = n;
}

std::string LitJson::lit_get_string(const LitValue& v) {
    assert(v.type == LIT_STRING);
    return v.str;
}
void LitJson::lit_set_string(LitValue* v, const std::string& s) {
    assert(v != nullptr);
    *v = s;
}

LitValue& LitJson::lit_get_array_element(LitValue& v, size_t index) {
    assert(v.type == LIT_ARRAY && index < v.arr.size());
    return v.arr[index];
}
const LitValue& LitJson::lit_get_array_element(const LitValue& v, size_t index) {
    assert(v.type == LIT_ARRAY && index < v.arr.size());
    return v.arr[index];
}

size_t LitJson::lit_get_array_size(const LitValue& v) {
    assert(v.type == LIT_ARRAY);
    return v.arr.size();
}
void LitJson::lit_set_array(LitValue* v, const std::vector<LitValue>& a) {
    assert(v != nullptr);
    *v = a;
}

size_t LitJson::lit_get_object_size(const LitValue& v) {
    assert(v.type == LIT_OBJECT);
    return v.obj.size();
}
const std::string& LitJson::lit_get_object_key(const LitValue& v, size_t index) {
    assert(v.type == LIT_OBJECT && index < v.obj.size());
    return v.obj[index].first;
}
size_t LitJson::lit_get_object_key_length(const LitValue& v, size_t index) {
    assert(v.type == LIT_OBJECT && index < v.obj.size());
    return v.obj[index].first.size();
}
LitValue& LitJson::lit_get_object_value(LitValue& v, size_t index) {
    assert(v.type == LIT_OBJECT && index < v.obj.size());
    return v.obj[index].second;
}
void LitJson::lit_set_object(LitValue* v, const LitValue::Obj& obj) {
    assert(v != nullptr);
    *v = obj;
}

std::string LitJson::LitStringify(const LitValue& v) {
    std::string res;
    LitStringifyValue(v, &res);
    return res;
}

void LitJson::LitStringifyValue(const LitValue& v, std::string* res) {
    switch (v.type) {
        case LIT_NULL: *res += "null"; break;
        case LIT_FALSE: *res += "false"; break;
        case LIT_TRUE: *res += "true"; break;
        case LIT_NUMBER:
            char buff[32];
            sprintf(buff, "%.17g", v.n);
            *res += buff;
            break;
        case LIT_STRING: LitStringifyString(v.str, res); break;
        case LIT_ARRAY:
            res->push_back('[');
            for (int i = 0; i < v.arr.size(); ++i) {
                if (i > 0) res->push_back(',');
                LitStringifyValue(v.arr[i], res);
            }
            res->push_back(']');
            break;
        case LIT_OBJECT:
            res->push_back('{');
            for (int i = 0; i < v.obj.size(); ++i) {
                if (i > 0) res->push_back(',');
                LitStringifyString(v.obj[i].first, res);
                res->push_back(':');
                LitStringifyValue(v.obj[i].second, res);
            }
            res->push_back('}');
            break;
    }
}

void LitJson::LitStringifyString(const std::string& str, std::string* res) {
    res->push_back('\"');
    for (int i = 0; i < str.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(str[i]);
        switch (ch) {
            case '\"': *res += "\\\""; break;
            case '\\': *res += "\\\\"; break;
            case '\b': *res += "\\b"; break;
            case '\f': *res += "\\f"; break;
            case '\n': *res += "\\n"; break;
            case '\r': *res += "\\r"; break;
            case '\t': *res += "\\t"; break;
            default:
                if (ch < 0x20) {
                    char buff[7] = {'\0'};
                    sprintf(buff, "\\u%04X", ch);
                    *res += buff;
                } else {
                    res->push_back(ch);
                }
        }
    }
    res->push_back('\"');
}