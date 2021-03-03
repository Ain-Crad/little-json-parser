#ifndef LITJSON_H_
#define LITJSON_H_

#include <string>
#include <vector>

class LitJson;

enum LitType { LIT_NULL, LIT_FALSE, LIT_TRUE, LIT_NUMBER, LIT_STRING, LIT_ARRAY, LIT_OBJECT };

class LitValue {
    friend class LitJson;
    typedef std::vector<std::pair<std::string, LitValue>> Obj;

public:
    LitValue() : n(0.0), type(LIT_NUMBER) {}
    LitValue(const LitValue& v) : type(v.type) { CopyUnion(v); }
    ~LitValue() {
        if (type == LIT_STRING) str.~basic_string();
        if (type == LIT_ARRAY) arr.~vector();
    }

    LitValue& operator=(const LitValue& v);
    LitValue& operator=(bool);
    LitValue& operator=(double);
    LitValue& operator=(const std::string&);
    LitValue& operator=(const std::vector<LitValue>&);
    LitValue& operator=(const Obj&);

private:
    void CopyUnion(const LitValue&);
    void UnionFree();

    union {
        double n;
        std::string str;
        std::vector<LitValue> arr;
        Obj obj;
    };
    LitType type;
};

struct LitContext {
    const char* json;
    // std::string str;
};

enum ParseResultType {
    LIT_PARSE_OK = 0,
    LIT_PARSE_EXPECT_VALUE,
    LIT_PARSE_INVALID_VALUE,
    LIT_PARSE_ROOT_NOT_SINGULAR,
    LIT_PARSE_NUMBER_TOO_BIG,
    LIT_PARSE_MISS_QUOTATION_MARK,
    LIT_PARSE_INVALID_STRING_ESCAPE,
    LIT_PARSE_INVALID_STRING_CHAR,
    LIT_PARSE_INVALID_UNICODE_HEX,
    LIT_PARSE_INVALID_UNICODE_SURROGATE,
    LIT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    LIT_PARSE_MISS_KEY,
    LIT_PARSE_MISS_COLON,
    LIT_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

class LitJson {
public:
    LitJson() = default;

    ParseResultType LitParse(LitValue* v, const char* json);

    LitType lit_get_type(const LitValue& v);

    void lit_set_null(LitValue* v);

    bool lit_get_boolean(const LitValue& v);
    void lit_set_boolean(LitValue* v, bool b);

    double lit_get_number(const LitValue& v);
    void lit_set_number(LitValue* v, double n);

    std::string lit_get_string(const LitValue& v);
    void lit_set_string(LitValue* v, const std::string& s);

    LitValue& lit_get_array_element(LitValue& v, size_t index);
    const LitValue& lit_get_array_element(const LitValue& v, size_t index);

    size_t lit_get_array_size(const LitValue& v);
    void lit_set_array(LitValue* v, const std::vector<LitValue>& a);

    size_t lit_get_object_size(const LitValue& v);
    const std::string& lit_get_object_key(const LitValue& v, size_t index);
    size_t lit_get_object_key_length(const LitValue& v, size_t index);
    LitValue& lit_get_object_value(LitValue& v, size_t index);
    void lit_set_object(LitValue* v, const LitValue::Obj& obj);

private:
    void LitParseWhitespace(LitContext* c);
    ParseResultType LitParseNull(LitContext* c, LitValue* v);
    ParseResultType LitParseTrue(LitContext* c, LitValue* v);
    ParseResultType LitParseFalse(LitContext* c, LitValue* v);
    ParseResultType LitParseValue(LitContext* c, LitValue* v);
    ParseResultType LitParseNumber(LitContext* c, LitValue* v);
    ParseResultType LitParseStringRaw(LitContext* c, std::string* cache);
    ParseResultType LitParseString(LitContext* c, LitValue* v);
    ParseResultType LitParseArray(LitContext* c, LitValue* v);
    ParseResultType LitParseObject(LitContext* c, LitValue* v);
    const char* LitParseUnicode(const char* p, unsigned int* u);
    void LitEncodeUTF8(std::string* cache, unsigned int u);
};

#endif