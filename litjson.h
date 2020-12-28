#ifndef LITJSON_H_
#define LITJSON_H_

enum LitType { LIT_NULL, LIT_FALSE, LIT_TRUE, LIT_NUMBER, LIT_STRING, LIT_ARRAY, LIT_OBJECT };

struct LitValue {
    double n;
    LitType type;
};

struct LitContext {
    const char* json;
};

enum ParseResultType { LIT_PARSE_OK = 0, LIT_PARSE_EXPECT_VALUE, LIT_PARSE_INVALID_VALUE, LIT_PARSE_ROOT_NOT_SINGULAR };

class LitJson {
public:
    LitJson() = default;

    void LitParseWhitespace(LitContext* c);
    ParseResultType LitParseRear(LitContext* c);
    ParseResultType LitParseNull(LitContext* c, LitValue* v);
    ParseResultType LitParseTrue(LitContext* c, LitValue* v);
    ParseResultType LitParseFalse(LitContext* c, LitValue* v);
    ParseResultType LitParseValue(LitContext* c, LitValue* v);
    ParseResultType LitParse(LitValue* v, const char* json);
    LitType LitGetType(const LitValue* v);
    double LitGetNumber(const LitValue* v);
};

#endif