#include "litjson.h"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>

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

    v->type = LIT_NULL;
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseTrue(LitContext* c, LitValue* v) {
    assert(c->json[0] == 't');
    if (c->json[1] != 'r' || c->json[2] != 'u' || c->json[3] != 'e') {
        return LIT_PARSE_INVALID_VALUE;
    }

    c->json += 4;

    v->type = LIT_TRUE;
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseFalse(LitContext* c, LitValue* v) {
    assert(c->json[0] == 'f');
    if (c->json[1] != 'a' || c->json[2] != 'l' || c->json[3] != 's' || c->json[4] != 'e') {
        return LIT_PARSE_INVALID_VALUE;
    }

    c->json += 5;

    v->type = LIT_FALSE;
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
    v->n = strtod(c->json, nullptr);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
        return LIT_PARSE_NUMBER_TOO_BIG;
    }

    c->json = p;
    v->type = LIT_NUMBER;
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseValue(LitContext* c, LitValue* v) {
    switch (*c->json) {
        case 'n':
            return LitParseNull(c, v);
        case 't':
            return LitParseTrue(c, v);
        case 'f':
            return LitParseFalse(c, v);
        case '\0':
            return LIT_PARSE_EXPECT_VALUE;
        default:
            return LitParseNumber(c, v);
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

LitType LitJson::LitGetType(const LitValue* v) {
    assert(v != nullptr);
    return v->type;
}

double LitJson::LitGetNumber(const LitValue* v) {
    assert(v != nullptr && v->type == LIT_NUMBER);
    return v->n;
}