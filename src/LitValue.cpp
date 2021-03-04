#include "LitValue.h"

LitValue& LitValue::operator=(const LitValue& v) {
    if (type == LIT_STRING && v.type != LIT_STRING) str.~basic_string();
    if (type == LIT_ARRAY && v.type != LIT_ARRAY) arr.~vector<LitValue>();
    if (type == LIT_OBJECT && v.type != LIT_OBJECT) obj.~vector<std::pair<std::string, LitValue>>();

    if (type == LIT_STRING && v.type == LIT_STRING) {
        str = v.str;
    } else if (type == LIT_ARRAY && v.type == LIT_ARRAY) {
        arr = v.arr;
    } else if (type == LIT_OBJECT && v.type == LIT_OBJECT) {
        obj = v.obj;
    } else {
        CopyUnion(v);
    }
    type = v.type;
    return *this;
}

LitValue& LitValue::operator=(bool b) {
    UnionFree();

    type = (b ? LIT_TRUE : LIT_FALSE);
    return *this;
}

LitValue& LitValue::operator=(double d) {
    UnionFree();

    n = d;
    type = LIT_NUMBER;
    return *this;
}

LitValue& LitValue::operator=(const std::string& s) {
    if (type == LIT_STRING) {
        str = s;
    } else {
        UnionFree();
        new (&str) std::string(s);
    }
    type = LIT_STRING;
    return *this;
}

LitValue& LitValue::operator=(const std::vector<LitValue>& a) {
    if (type == LIT_ARRAY) {
        arr = a;
    } else {
        UnionFree();
        new (&arr) std::vector<LitValue>(a);
    }
    type = LIT_ARRAY;
    return *this;
}

LitValue& LitValue::operator=(const Obj& o) {
    if (type == LIT_OBJECT) {
        obj = o;
    } else {
        UnionFree();
        new (&obj) Obj(o);
    }
    type = LIT_OBJECT;
    return *this;
}

void LitValue::CopyUnion(const LitValue& v) {
    switch (v.type) {
        case LIT_NUMBER: n = v.n; break;
        case LIT_STRING: new (&str) std::string(v.str); break;
        case LIT_ARRAY: new (&arr) std::vector<LitValue>(v.arr); break;
        case LIT_OBJECT: new (&obj) Obj(v.obj); break;
    }
}

void LitValue::UnionFree() {
    if (type == LIT_STRING) str.~basic_string();
    if (type == LIT_ARRAY) arr.~vector<LitValue>();
    if (type == LIT_OBJECT) obj.~vector<std::pair<std::string, LitValue>>();
}