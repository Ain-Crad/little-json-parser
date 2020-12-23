#include <iostream>
#include <iomanip>
#include "litjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;
static LitJson lit;

#define CHECK(expect, actual) Check(expect, actual, __FILE__, __LINE__)

template<typename TypeExpect, typename TypeActual>
static void Check(TypeExpect expect, TypeActual actual, const char *file_name, int line_num){
    ++test_count;
    if(expect == actual){
        ++test_pass;
    }else{
        std::cerr << file_name << ":" << line_num
                  << ": expect: " << expect << " actual: " << actual << std::endl;
    }
}

static void TestParseNull(){
    LitValue v;

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_OK, lit.LitParse(&v, "null"));
    CHECK(LIT_NULL, lit.LitGetType(&v));
}

static void TestParseTrue(){
    LitValue v;

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_OK, lit.LitParse(&v, "true"));
    CHECK(LIT_TRUE, lit.LitGetType(&v));
}

static void TestParseFalse(){
    LitValue v; 

    v.type = LIT_TRUE;
    CHECK(LIT_PARSE_OK, lit.LitParse(&v, "false"));
    CHECK(LIT_FALSE, lit.LitGetType(&v));
}

static void TestParseExpectValue(){
    LitValue v;

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_EXPECT_VALUE, lit.LitParse(&v, ""));
    CHECK(LIT_NULL, lit.LitGetType(&v));

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_EXPECT_VALUE, lit.LitParse(&v, " "));
    CHECK(LIT_NULL, lit.LitGetType(&v));
}

static void TestParseInvalidValue(){
    LitValue v;

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_INVALID_VALUE, lit.LitParse(&v, "nul"));
    CHECK(LIT_NULL, lit.LitGetType(&v));

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_INVALID_VALUE, lit.LitParse(&v, "nulll"));
    CHECK(LIT_NULL, lit.LitGetType(&v));

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_INVALID_VALUE, lit.LitParse(&v, "?"));
    CHECK(LIT_NULL, lit.LitGetType(&v));

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_INVALID_VALUE, lit.LitParse(&v, "fale"));
    CHECK(LIT_NULL, lit.LitGetType(&v));

    v.type = LIT_FALSE; 
    CHECK(LIT_PARSE_INVALID_VALUE, lit.LitParse(&v, "falsefalse"));
    CHECK(LIT_NULL, lit.LitGetType(&v));

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_INVALID_VALUE, lit.LitParse(&v, "tre"));
    CHECK(LIT_NULL, lit.LitGetType(&v));

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_INVALID_VALUE, lit.LitParse(&v, "truetrue"));
    CHECK(LIT_NULL, lit.LitGetType(&v));
}

static void TestParseRootNotSingular(){
    LitValue v;

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_ROOT_NOT_SINGULAR, lit.LitParse(&v, "null x"));
    CHECK(LIT_NULL, lit.LitGetType(&v));

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_ROOT_NOT_SINGULAR, lit.LitParse(&v, "null x ?"));
    CHECK(LIT_NULL, lit.LitGetType(&v));

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_ROOT_NOT_SINGULAR, lit.LitParse(&v, "true null"));
    CHECK(LIT_NULL, lit.LitGetType(&v));

    v.type = LIT_FALSE;
    CHECK(LIT_PARSE_ROOT_NOT_SINGULAR, lit.LitParse(&v, "false true"));
    CHECK(LIT_NULL, lit.LitGetType(&v));
}

static void TestParse(){
    TestParseNull();
    TestParseTrue();
    TestParseFalse();
    TestParseExpectValue();
    TestParseInvalidValue();
    TestParseRootNotSingular();
}

int main(){
    TestParse();

    std::cout << test_pass << "/" << test_count;
    std::cout << std::fixed << std::setprecision(2)
              << " (" << test_pass * 100.0 / test_count << "%) passed" << std::endl;
    std::cout << std::defaultfloat;
    
    return main_ret;
}