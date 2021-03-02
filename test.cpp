#include <iomanip>
#include <iostream>

#include "litjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;
static LitJson lit;

#define CHECK_EQ(expect, actual) CheckEquality(expect, actual, __FILE__, __LINE__)
#define CHECK_ERROR(error, json) CheckError(error, json, __FILE__, __LINE__)
#define CHECK_NUMBER(expect, json) CheckNumber(expect, json, __FILE__, __LINE__)
#define CHECK_STRING(expect, json) CheckString(expect, json, __FILE__, __LINE__)

template <typename T>
static void CheckEquality(T expect, T actual, const char *file_name, int line_num) {
    ++test_count;
    if (expect == actual) {
        ++test_pass;
    } else {
        std::cerr << file_name << ":" << line_num << ": expect: " << expect << " actual: " << actual << std::endl;
    }
}

static void CheckError(ParseResultType error, const char *json, const char *file_name, int line_num) {
    LitValue v;
    lit.lit_set_boolean(&v, false);

    CheckEquality(error, lit.LitParse(&v, json), file_name, line_num);
    CheckEquality(LIT_NULL, lit.lit_get_type(&v), file_name, line_num);
}

static void CheckNumber(double expect, const char *json, const char *file_name, int line_num) {
    LitValue v;
    lit.lit_set_null(&v);

    CheckEquality(LIT_PARSE_OK, lit.LitParse(&v, json), file_name, line_num);
    CheckEquality(LIT_NUMBER, lit.lit_get_type(&v), file_name, line_num);
    CheckEquality(expect, lit.lit_get_number(&v), file_name, line_num);
}

static void CheckString(std::string expect, const char *json, const char *file_name, int line_num) {
    LitValue v;
    lit.lit_set_null(&v);

    CheckEquality(LIT_PARSE_OK, lit.LitParse(&v, json), file_name, line_num);
    CheckEquality(LIT_STRING, lit.lit_get_type(&v), file_name, line_num);
    CheckEquality(expect, lit.lit_get_string(&v), file_name, line_num);
}

static void TestParseNull() {
    LitValue v;
    lit.lit_set_boolean(&v, false);

    CHECK_EQ(LIT_PARSE_OK, lit.LitParse(&v, "null"));
    CHECK_EQ(LIT_NULL, lit.lit_get_type(&v));
}

static void TestParseTrue() {
    LitValue v;
    lit.lit_set_boolean(&v, false);

    CHECK_EQ(LIT_PARSE_OK, lit.LitParse(&v, "true"));
    CHECK_EQ(LIT_TRUE, lit.lit_get_type(&v));
}

static void TestParseFalse() {
    LitValue v;
    lit.lit_set_boolean(&v, true);

    CHECK_EQ(LIT_PARSE_OK, lit.LitParse(&v, "false"));
    CHECK_EQ(LIT_FALSE, lit.lit_get_type(&v));
}

static void TestParseNumber() {
    CHECK_NUMBER(0.0, "0");
    CHECK_NUMBER(0.0, "-0");
    CHECK_NUMBER(0.0, "-0.0");
    CHECK_NUMBER(1.0, "1");
    CHECK_NUMBER(-1.0, "-1");
    CHECK_NUMBER(1.5, "1.5");
    CHECK_NUMBER(-1.5, "-1.5");
    CHECK_NUMBER(3.1416, "3.1416");
    CHECK_NUMBER(1E10, "1E10");
    CHECK_NUMBER(1e10, "1e10");
    CHECK_NUMBER(1E+10, "1E+10");
    CHECK_NUMBER(1E-10, "1E-10");
    CHECK_NUMBER(1.234E+10, "1.234E+10");
    CHECK_NUMBER(1.234E-10, "1.234E-10");
    CHECK_NUMBER(0.0, "1e-10000");

    CHECK_NUMBER(1.0000000000000002, "1.0000000000000002");            // the smallest number > 1
    CHECK_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324");  // minimum denormal positive
    CHECK_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    CHECK_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");  // maximum denormal positive
    CHECK_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    CHECK_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");  // minimum normal positive
    CHECK_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    CHECK_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");  // maximum normal positive
    CHECK_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void TestParseString() {
    CHECK_STRING("", "\"\"");
    CHECK_STRING("Hello World", "\"Hello World\"");
    CHECK_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    CHECK_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    // CHECK_STRING("Hello\0World", "\"Hello\\u0000World\""); to be fixed
    CHECK_STRING("\x24", "\"\\u0024\"");
    CHECK_STRING("\xC2\xA2", "\"\\u00A2\"");
    CHECK_STRING("\xE2\x82\xAC", "\"\\u20AC\"");
    CHECK_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");
    CHECK_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");
}

static void TestParseExpectValue() {
    CHECK_ERROR(LIT_PARSE_EXPECT_VALUE, "");
    CHECK_ERROR(LIT_PARSE_EXPECT_VALUE, " ");
}

static void TestParseInvalidValue() {
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "nul");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "?");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "fale");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "tre");

    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "+0");  // positive sign is not allowed
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "+1");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, ".123");  // at least one digit before '.'
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "1.");    // at least one digit after '.'
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "INF");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "inf");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "NAN");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "nan");
}

static void TestParseRootNotSingular() {
    CHECK_ERROR(LIT_PARSE_ROOT_NOT_SINGULAR, "null x");
    CHECK_ERROR(LIT_PARSE_ROOT_NOT_SINGULAR, "nulll");
    CHECK_ERROR(LIT_PARSE_ROOT_NOT_SINGULAR, "falsefalse");
    CHECK_ERROR(LIT_PARSE_ROOT_NOT_SINGULAR, "true null");
    CHECK_ERROR(LIT_PARSE_ROOT_NOT_SINGULAR, "false true");

    CHECK_ERROR(LIT_PARSE_ROOT_NOT_SINGULAR, "0123");  // after zero should be '.', 'E', 'e' or nothing
    CHECK_ERROR(LIT_PARSE_ROOT_NOT_SINGULAR, "0x0");
    CHECK_ERROR(LIT_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void TestParseNumberTooBig() {
    CHECK_ERROR(LIT_PARSE_NUMBER_TOO_BIG, "1e309");
    CHECK_ERROR(LIT_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void TestParseMissingQuotationMark() {
    CHECK_ERROR(LIT_PARSE_MISS_QUOTATION_MARK, "\"");
    CHECK_ERROR(LIT_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void TestParseInvalidStringEscape() {
    CHECK_ERROR(LIT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    CHECK_ERROR(LIT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    CHECK_ERROR(LIT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    CHECK_ERROR(LIT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void TestParseInvalidStringChar() {
    CHECK_ERROR(LIT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    CHECK_ERROR(LIT_PARSE_INVALID_STRING_CHAR, "\"\x1f\"");
}

static void TestParseInValidUnicodeHex() {
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

// static void TestParseInvalidUnicodeSurrogate() {
//     CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
//     CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
//     CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
//     CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
//     CHECK_ERROR(LIT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
// }

static void TestAccessNull() {
    LitValue v;
    lit.lit_set_string(&v, "access null");
    lit.lit_set_null(&v);
    CHECK_EQ(LIT_NULL, lit.lit_get_type(&v));
}

static void TestAccessBoolean() {
    LitValue v;
    lit.lit_set_string(&v, "access boolean");
    lit.lit_set_boolean(&v, true);
    CHECK_EQ(LIT_TRUE, lit.lit_get_type(&v));
    lit.lit_set_boolean(&v, false);
    CHECK_EQ(LIT_FALSE, lit.lit_get_type(&v));
}

static void TestAccessNumber() {
    LitValue v;
    lit.lit_set_string(&v, "access number");
    lit.lit_set_number(&v, 100.1);
    CHECK_EQ(LIT_NUMBER, lit.lit_get_type(&v));
}

static void TestAccessString() {
    LitValue v;
    lit.lit_set_null(&v);
    lit.lit_set_string(&v, "access string");
    CHECK_EQ(LIT_STRING, lit.lit_get_type(&v));
}

static void TestParse() {
    // test type
    TestParseNull();
    TestParseTrue();
    TestParseFalse();
    TestParseNumber();
    TestParseString();

    // test error
    TestParseExpectValue();
    TestParseInvalidValue();
    TestParseRootNotSingular();
    TestParseNumberTooBig();
    TestParseMissingQuotationMark();
    TestParseInvalidStringEscape();
    TestParseInvalidStringChar();
    TestParseInValidUnicodeHex();
    // TestParseInvalidUnicodeSurrogate();

    // test access/memory management
    TestAccessNull();
    TestAccessBoolean();
    TestAccessNumber();
    TestAccessString();
}

int main() {
    TestParse();

    std::cout << test_pass << "/" << test_count;
    std::cout << std::fixed << std::setprecision(2) << " (" << test_pass * 100.0 / test_count << "%) passed"
              << std::endl;
    std::cout << std::defaultfloat;

    return main_ret;
}