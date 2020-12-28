#include <iomanip>
#include <iostream>

#include "litjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;
static LitJson lit;

#define CHECK(expect, actual) Check(expect, actual, __FILE__, __LINE__)
#define CHECK_ERROR(error, json) CheckError(error, json, __FILE__, __LINE__)

template <typename T>
static void Check(T expect, T actual, const char *file_name, int line_num) {
    ++test_count;
    if (expect == actual) {
        ++test_pass;
    } else {
        std::cerr << file_name << ":" << line_num << ": expect: " << expect << " actual: " << actual << std::endl;
    }
}

static void CheckError(ParseResultType error, const char *json, const char *file_name, int line_num) {
    LitValue v;
    v.type = LIT_FALSE;
    Check(error, lit.LitParse(&v, json), file_name, line_num);
    Check(LIT_NULL, lit.LitGetType(&v), file_name, line_num);
}

static void TestParseNull() {
    LitValue v;

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_OK, lit.LitParse(&v, "null"));
    CHECK(LIT_NULL, lit.LitGetType(&v));
}

static void TestParseTrue() {
    LitValue v;

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_OK, lit.LitParse(&v, "true"));
    CHECK(LIT_TRUE, lit.LitGetType(&v));
}

static void TestParseFalse() {
    LitValue v;

    v.type = LIT_TRUE;
    CHECK(LIT_PARSE_OK, lit.LitParse(&v, "false"));
    CHECK(LIT_FALSE, lit.LitGetType(&v));
}

static void TestParseNumber() {}

static void TestParseExpectValue() {
    CHECK_ERROR(LIT_PARSE_EXPECT_VALUE, "");
    CHECK_ERROR(LIT_PARSE_EXPECT_VALUE, " ");
}

static void TestParseInvalidValue() {
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "nul");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "nulll");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "?");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "fale");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "falsefalse");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "tre");
    CHECK_ERROR(LIT_PARSE_INVALID_VALUE, "truetrue");
}

static void TestParseRootNotSingular() {
    CHECK_ERROR(LIT_PARSE_ROOT_NOT_SINGULAR, "null x");
    CHECK_ERROR(LIT_PARSE_ROOT_NOT_SINGULAR, "true null");
    CHECK_ERROR(LIT_PARSE_ROOT_NOT_SINGULAR, "false true");
}

static void TestParse() {
    TestParseNull();
    TestParseTrue();
    TestParseFalse();
    TestParseExpectValue();
    TestParseInvalidValue();
    TestParseRootNotSingular();
}

int main() {
    TestParse();

    std::cout << test_pass << "/" << test_count;
    std::cout << std::fixed << std::setprecision(2) << " (" << test_pass * 100.0 / test_count << "%) passed"
              << std::endl;
    std::cout << std::defaultfloat;

    return main_ret;
}