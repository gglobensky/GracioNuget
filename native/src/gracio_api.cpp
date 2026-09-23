#include "gracio_api.h"
#include "gracio_core.hpp"
#include <string>
#include <cstdlib>

// Explicitly ensuring C linkage for all exported functions


extern "C" {

GRACIO_API gracio_t* gracio_create_int(int64_t numerator, int64_t denominator) {
    gracio::log_msg("API: gracio_create_int");
    try {
        auto* g = new gracio::Gracio(gracio::BigInt(numerator), gracio::BigInt(denominator));
        return reinterpret_cast<gracio_t*>(g);
    } catch (...) {
        gracio::log_msg("API: gracio_create_int - EXCEPTION");
        return nullptr;
    }
}

GRACIO_API gracio_t* gracio_create_float(double value) {
    gracio::log_msg("API: gracio_create_float");
    if (value == 0.0) return gracio_create_int(0, 1);
    try {
        // Use continued fraction approximation to map the double to its simplest rational form.
        // This eliminates the "Float Trap" where 0.3 becomes a complex binary fraction.
        gracio::Gracio res = gracio::continuedFractionApprox(value);
        return reinterpret_cast<gracio_t*>(new gracio::Gracio(res));
    } catch (...) {
        gracio::log_msg("API: gracio_create_float - EXCEPTION");
        return nullptr;
    }
}

GRACIO_API void gracio_destroy(gracio_t* g) {
    gracio::log_msg("API: gracio_destroy");
    if (g) delete reinterpret_cast<gracio::Gracio*>(g);
}

GRACIO_API void gracio_set_debug(bool enabled) {
    gracio::g_gracio_debug = enabled;
}

GRACIO_API gracio_t* gracio_clone(const gracio_t* g) {
    if (!g) return nullptr;
    gracio::log_msg("API: gracio_clone");
    try {
        auto* source = reinterpret_cast<const gracio::Gracio*>(g);
        auto* clone = new gracio::Gracio(*source);
        return reinterpret_cast<gracio_t*>(clone);
    } catch (...) {
        gracio::log_msg("API: gracio_clone - EXCEPTION");
        return nullptr;
    }
}

GRACIO_API void gracio_set_precision_limit(gracio_t* g, uint32_t limit) {
    if (g) {
        reinterpret_cast<gracio::Gracio*>(g)->precisionLimit = limit;
    }
}

GRACIO_API void gracio_add(gracio_t* a, const gracio_t* b) {
    if (!a || !b) return;
    gracio::log_msg("API: gracio_add");
    try { reinterpret_cast<gracio::Gracio*>(a)->add(*reinterpret_cast<const gracio::Gracio*>(b)); } catch (...) {}
    gracio::log_msg("API: gracio_add - DONE");
}

GRACIO_API void gracio_subtract(gracio_t* a, const gracio_t* b) {
    if (!a || !b) return;
    gracio::log_msg("API: gracio_subtract");
    try { reinterpret_cast<gracio::Gracio*>(a)->subtract(*reinterpret_cast<const gracio::Gracio*>(b)); } catch (...) {}
    gracio::log_msg("API: gracio_subtract - DONE");
}

GRACIO_API void gracio_multiply(gracio_t* a, const gracio_t* b) {
    if (!a || !b) return;
    gracio::log_msg("API: gracio_multiply");
    try { reinterpret_cast<gracio::Gracio*>(a)->multiply(*reinterpret_cast<const gracio::Gracio*>(b)); } catch (...) {}
    gracio::log_msg("API: gracio_multiply - DONE");
}

GRACIO_API int try_gracio_divide(gracio_t* a, const gracio_t* b) {
    if (!a || !b) return -1;
    gracio::log_msg("API: gracio_divide");
    try { 
        reinterpret_cast<gracio::Gracio*>(a)->divide(*reinterpret_cast<const gracio::Gracio*>(b)); 
        gracio::log_msg("API: gracio_divide - DONE");
        return 0;
    } catch (...) {
        gracio::log_msg("API: gracio_divide - EXCEPTION (DivByZero)");
        return 1;
    }
}

GRACIO_API void gracio_power(gracio_t* a, int64_t exp) {
    if (!a) return;
    gracio::log_msg("API: gracio_power");
    try { reinterpret_cast<gracio::Gracio*>(a)->power(exp); } catch (...) {}
    gracio::log_msg("API: gracio_power - DONE");
}

GRACIO_API void gracio_approximate(gracio_t* g, uint32_t digits) {
    if (!g) return;
    gracio::log_msg("API: gracio_approximate");
    try { reinterpret_cast<gracio::Gracio*>(g)->approximate(digits); } catch (...) {}
    gracio::log_msg("API: gracio_approximate - DONE");
}

GRACIO_API gracio_t* gracio_root(uint32_t index, const gracio_t* value) {
    if (!value) return nullptr;
    gracio::log_msg("API: gracio_root");
    try {
        gracio::Gracio res = gracio::Gracio::root(index, *reinterpret_cast<const gracio::Gracio*>(value));
        return reinterpret_cast<gracio_t*>(new gracio::Gracio(res));
    } catch (...) {
        gracio::log_msg("API: gracio_root - EXCEPTION");
        return nullptr;
    }
}

GRACIO_API double gracio_to_double(const gracio_t* g) {
    gracio::log_msg("API: gracio_to_double");
    if (!g) return 0.0;
    double res = reinterpret_cast<const gracio::Gracio*>(g)->toDouble();
    gracio::log_msg("API: gracio_to_double - DONE");
    return res;
}

GRACIO_API char* gracio_get_numerator_string(const gracio_t* g) {
    if (!g) return nullptr;
    std::string s = reinterpret_cast<const gracio::Gracio*>(g)->numerator.toString();
    char* res = (char*)std::malloc(s.length() + 1);
    if (res) {
        std::copy(s.begin(), s.end(), res);
        res[s.length()] = '\0';
    }
    return res;
}

GRACIO_API char* gracio_get_denominator_string(const gracio_t* g) {
    if (!g) return nullptr;
    std::string s = reinterpret_cast<const gracio::Gracio*>(g)->denominator.toString();
    char* res = (char*)std::malloc(s.length() + 1);
    if (res) {
        std::copy(s.begin(), s.end(), res);
        res[s.length()] = '\0';
    }
    return res;
}

GRACIO_API char* gracio_to_string(const gracio_t* g) {
    gracio::log_msg("API: gracio_to_string");
    if (!g) return nullptr;
    std::string s = reinterpret_cast<const gracio::Gracio*>(g)->toString();
    char* res = (char*)std::malloc(s.length() + 1);
    if (res) {
        std::copy(s.begin(), s.end(), res);
        res[s.length()] = '\0';
    }
    gracio::log_msg("API: gracio_to_string - DONE");
    return res;
}

GRACIO_API void gracio_free_string(char* s) {
    std::free(s);
}

} // extern "C"
