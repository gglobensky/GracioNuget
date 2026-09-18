#include "gracio_api.h"
#include "gracio_core.hpp"
#include <string>

extern "C" {

GRACIO_API gracio_t* gracio_create_int(int64_t numerator, int64_t denominator) {
    try {
        auto* g = new gracio::Gracio(gracio::BigInt(numerator), gracio::BigInt(denominator));
        return reinterpret_cast<gracio_t*>(g);
    } catch (...) {
        return nullptr;
    }
}

GRACIO_API gracio_t* gracio_create_float(double value) {
    // For the prototype, we'll use a simple conversion. 
    // A professional impl would parse the float as a string to avoid drift.
    try {
        // Simple approach: multiply by 10^9 to get a ratio
        int64_t n = static_cast<int64_t>(value * 1e9);
        auto* g = new gracio::Gracio(gracio::BigInt(n), gracio::BigInt(1000000000));
        return reinterpret_cast<gracio_t*>(g);
    } catch (...) {
        return nullptr;
    }
}

GRACIO_API void gracio_destroy(gracio_t* g) {
    delete reinterpret_cast<gracio::Gracio*>(g);
}

GRACIO_API void gracio_set_precision_limit(gracio_t* g, uint32_t limit) {
    if (g) {
        reinterpret_cast<gracio::Gracio*>(g)->precisionLimit = limit;
    }
}

GRACIO_API void gracio_add(gracio_t* a, const gracio_t* b) {
    if (a && b) {
        reinterpret_cast<gracio::Gracio*>(a)->add(*reinterpret_cast<const gracio::Gracio*>(b));
    }
}

GRACIO_API void gracio_subtract(gracio_t* a, const gracio_t* b) {
    if (a && b) {
        reinterpret_cast<gracio::Gracio*>(a)->subtract(*reinterpret_cast<const gracio::Gracio*>(b));
    }
}

GRACIO_API void gracio_multiply(gracio_t* a, const gracio_t* b) {
    if (a && b) {
        reinterpret_cast<gracio::Gracio*>(a)->multiply(*reinterpret_cast<const gracio::Gracio*>(b));
    }
}

GRACIO_API void gracio_divide(gracio_t* a, const gracio_t* b) {
    if (a && b) {
        reinterpret_cast<gracio::Gracio*>(a)->divide(*reinterpret_cast<const gracio::Gracio*>(b));
    }
}

GRACIO_API double gracio_to_double(const gracio_t* g) {
    if (!g) return 0.0;
    return reinterpret_cast<const gracio::Gracio*>(g)->toDouble();
}

GRACIO_API char* gracio_to_string(const gracio_t* g) {
    if (!g) return nullptr;
    std::string s = reinterpret_cast<const gracio::Gracio*>(g)->toString();
    char* res = (char*)malloc(s.length() + 1);
    std::copy(s.begin(), s.end(), res);
    res[s.length()] = '\0';
    return res;
}

GRACIO_API void gracio_free_string(char* s) {
    free(s);
}

}
