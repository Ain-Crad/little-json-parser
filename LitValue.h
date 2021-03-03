#ifndef LITVALUE_H_
#define LITVALUE_H_

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

#endif