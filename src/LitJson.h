#ifndef LITJSON_H_
#define LITJSON_H_

#include <string>
#include <vector>

#include "LitValue.h"

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

    // Json Parse
    ParseResultType LitParse(LitValue* v, const char* json);
    // Json Stringify
    std::string LitStringify(const LitValue& v);

    // setter and getter function
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
    // parse
    void LitParseWhitespace();
    ParseResultType LitParseNull(LitValue* v);
    ParseResultType LitParseTrue(LitValue* v);
    ParseResultType LitParseFalse(LitValue* v);
    ParseResultType LitParseValue(LitValue* v);
    ParseResultType LitParseNumber(LitValue* v);
    ParseResultType LitParseStringRaw(std::string* buff);
    ParseResultType LitParseString(LitValue* v);
    ParseResultType LitParseArray(LitValue* v);
    ParseResultType LitParseObject(LitValue* v);
    const char* LitParseUnicode(const char* p, unsigned int* u);
    void LitEncodeUTF8(std::string* buff, unsigned int u);

    // stringify
    void LitStringifyValue(const LitValue& v, std::string* res);
    void LitStringifyString(const std::string& str, std::string* res);

    const char* cur = nullptr;
};

#endif