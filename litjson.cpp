#include "litjson.h"

#include "cassert"

void LitJson::LitParseWhitespace(LitContext* c) {
    const char* p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
    c->json = p;
}

ParseResultType LitJson::LitParseRear(LitContext* c) {
    if (*c->json != '\0' && *c->json != ' ' && *c->json != '\t' && *c->json != '\n' && *c->json != '\r') {
        return LIT_PARSE_INVALID_VALUE;
    }

    LitParseWhitespace(c);
    if (*c->json != '\0') {
        return LIT_PARSE_ROOT_NOT_SINGULAR;
    }

    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseNull(LitContext* c, LitValue* v) {
    assert(c->json[0] == 'n');
    if (c->json[1] != 'u' || c->json[2] != 'l' || c->json[3] != 'l') {
        return LIT_PARSE_INVALID_VALUE;
    }

    c->json += 4;
    ParseResultType ret = LitParseRear(c);
    if (ret != LIT_PARSE_OK) {
        return ret;
    }

    v->type = LIT_NULL;
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseTrue(LitContext* c, LitValue* v) {
    assert(c->json[0] == 't');
    if (c->json[1] != 'r' || c->json[2] != 'u' || c->json[3] != 'e') {
        return LIT_PARSE_INVALID_VALUE;
    }

    c->json += 4;
    ParseResultType ret = LitParseRear(c);
    if (ret != LIT_PARSE_OK) {
        return ret;
    }

    v->type = LIT_TRUE;
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseFalse(LitContext* c, LitValue* v) {
    assert(c->json[0] == 'f');
    if (c->json[1] != 'a' || c->json[2] != 'l' || c->json[3] != 's' || c->json[4] != 'e') {
        return LIT_PARSE_INVALID_VALUE;
    }

    c->json += 5;
    ParseResultType ret = LitParseRear(c);
    if (ret != LIT_PARSE_OK) {
        return ret;
    }

    v->type = LIT_FALSE;
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
            return LIT_PARSE_INVALID_VALUE;
    }
}

ParseResultType LitJson::LitParse(LitValue* v, const char* json) {
    LitContext c;
    assert(v != nullptr);
    c.json = json;
    v->type = LIT_NULL;
    LitParseWhitespace(&c);
    return LitParseValue(&c, v);
}

LitType LitJson::LitGetType(const LitValue* v) {
    assert(v != nullptr);
    return v->type;
}