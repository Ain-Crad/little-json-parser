#ifndef LITJSON_H_
#define LITJSON_H_

#include <string>

class LitJson;

enum LitType { LIT_NULL, LIT_FALSE, LIT_TRUE, LIT_NUMBER, LIT_STRING, LIT_ARRAY, LIT_OBJECT };

class LitValue {
    friend class LitJson;

public:
    LitValue() : n(0.0), type(LIT_NUMBER) {}
    LitValue(const LitValue& v) : type(v.type) { CopyUnion(v); }
    ~LitValue() {
        if (type == LIT_STRING) str.~basic_string();
    }

    LitValue& operator=(const LitValue& v);
    LitValue& operator=(bool);
    LitValue& operator=(double);
    LitValue& operator=(const std::string&);

private:
    void CopyUnion(const LitValue&);

    union {
        double n;
        std::string str;
    };
    LitType type;
};

struct LitContext {
    const char* json;
    std::string str;
};

enum ParseResultType {
    LIT_PARSE_OK = 0,
    LIT_PARSE_EXPECT_VALUE,
    LIT_PARSE_INVALID_VALUE,
    LIT_PARSE_ROOT_NOT_SINGULAR,
    LIT_PARSE_NUMBER_TOO_BIG,
    LIT_PARSE_MISS_QUOTATION_MARK,
    LIT_PARSE_INVALID_STRING_ESCAPE,
    LIT_PARSE_INVALID_STRING_CHAR
};

class LitJson {
public:
    LitJson() = default;

    ParseResultType LitParse(LitValue* v, const char* json);

    LitType lit_get_type(const LitValue* v);

    void lit_set_null(LitValue* v);

    bool lit_get_boolean(const LitValue* v);
    void lit_set_boolean(LitValue* v, bool b);

    double lit_get_number(const LitValue* v);
    void lit_set_number(LitValue* v, double n);

    std::string lit_get_string(const LitValue* v);
    void lit_set_string(LitValue* v, const std::string& s);

private:
    void LitParseWhitespace(LitContext* c);
    ParseResultType LitParseRear(LitContext* c);
    ParseResultType LitParseNull(LitContext* c, LitValue* v);
    ParseResultType LitParseTrue(LitContext* c, LitValue* v);
    ParseResultType LitParseFalse(LitContext* c, LitValue* v);
    ParseResultType LitParseValue(LitContext* c, LitValue* v);
    ParseResultType LitParseNumber(LitContext* c, LitValue* v);
    ParseResultType LitParseString(LitContext* c, LitValue* v);
};

#endif