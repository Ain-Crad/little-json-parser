#include "cassert"
#include "litjson.h"

void LitJson::LitParseWhitespace(LitContext* c){
    const char* p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
    c->json = p;
}

ParseResultType LitJson::LitParseNull(LitContext* c, LitValue* v){
    assert(c->json[0] == 'n');
    if(c->json[1] != 'u' || c->json[2] != 'l' || c->json[3] != 'l'){
        return LIT_PARSE_INVALID_VALUE;
    }

    c->json += 4;

    LitParseWhitespace(c);
    if(*c->json != '\0'){
        return LIT_PARSE_ROOT_NOT_SINGULAR;
    }

    v->type = LIT_NULL;
    return LIT_PARSE_OK;
}

ParseResultType LitJson::LitParseValue(LitContext* c, LitValue* v){
    switch(*c->json){
        case 'n': return LitParseNull(c, v);
        case '\0': return LIT_PARSE_EXPECT_VALUE;
        default: return LIT_PARSE_INVALID_VALUE;
    }
}

ParseResultType LitJson::LitParse(LitValue* v, const char* json){
    LitContext c;
    assert(v != nullptr);
    c.json = json;
    v->type = LIT_NULL;
    LitParseWhitespace(&c);
    return LitParseValue(&c, v);
}

LitType LitJson::LitGetType(const LitValue *v){
    assert(v != nullptr);
    return v->type;
}