#include <node_api.h>
#include <gracio_api.h>
#include <string>
#include <vector>

/* -------------------------------------------------------------------------- */
/*                                 HELPERS                                    */
/* -------------------------------------------------------------------------- */

static gracio_t* get_handle(napi_env env, napi_value js_obj) {
    void* data = nullptr;
    if (napi_unwrap(env, js_obj, &data) != napi_ok) return nullptr;
    return (gracio_t*)data;
}

static void finalize_gracio(napi_env env, void* data, void* edge) {
    if (data) gracio_destroy((gracio_t*)data);
}

/* -------------------------------------------------------------------------- */
/*                               INSTANCE METHODS                             */
/* -------------------------------------------------------------------------- */

static napi_value method_add(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_value argv[16]; // Fixed size for safety
    napi_value this_val = nullptr;
    void* data = nullptr;
    // Corrected: pass argv (the array pointer), not &argv
    napi_get_cb_info(env, info, &argc, argv, &this_val, &data);

    gracio_t* a = (gracio_t*)data;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "Argument missing");
        return nullptr;
    }
    gracio_t* b = get_handle(env, argv[0]);

    if (!a || !b) {
        napi_throw_type_error(env, nullptr, "Invalid Gracio objects");
        return nullptr;
    }

    gracio_add(a, b);
    return this_val;
}

static napi_value method_subtract(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_value argv[16];
    napi_value this_val = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &this_val, &data);

    gracio_t* a = (gracio_t*)data;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "Argument missing");
        return nullptr;
    }
    gracio_t* b = get_handle(env, argv[0]);

    if (!a || !b) {
        napi_throw_type_error(env, nullptr, "Invalid Gracio objects");
        return nullptr;
    }

    gracio_subtract(a, b);
    return this_val;
}

static napi_value method_multiply(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_value argv[16];
    napi_value this_val = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &this_val, &data);

    gracio_t* a = (gracio_t*)data;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "Argument missing");
        return nullptr;
    }
    gracio_t* b = get_handle(env, argv[0]);

    if (!a || !b) {
        napi_throw_type_error(env, nullptr, "Invalid Gracio objects");
        return nullptr;
    }

    gracio_multiply(a, b);
    return this_val;
}

static napi_value method_divide(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_value argv[16];
    napi_value this_val = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &this_val, &data);

    gracio_t* a = (gracio_t*)data;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "Argument missing");
        return nullptr;
    }
    gracio_t* b = get_handle(env, argv[0]);

    if (!a || !b) {
        napi_throw_type_error(env, nullptr, "Invalid Gracio objects");
        return nullptr;
    }

    int res = try_gracio_divide(a, b);
    if (res == 1) {
        napi_throw_error(env, nullptr, "Division by zero");
        return nullptr;
    }
    return this_val;
}

static napi_value method_pow(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_value argv[16];
    napi_value this_val = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &this_val, &data);

    gracio_t* a = (gracio_t*)data;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "Argument missing");
        return nullptr;
    }

    int64_t exp;
    bool lossless;
    if (napi_get_value_bigint_int64(env, argv[0], &exp, &lossless) != napi_ok) {
        napi_throw_type_error(env, nullptr, "Exponent must be a BigInt");
        return nullptr;
    }

    if (!a) return nullptr;
    gracio_power(a, exp);
    return this_val;
}

static napi_value method_root(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_value argv[16];
    napi_value this_val = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &this_val, &data);

    gracio_t* a = (gracio_t*)data;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "Argument missing");
        return nullptr;
    }

    double d_index;
    napi_get_value_double(env, argv[0], &d_index);
    uint32_t index = (uint32_t)d_index;

    if (!a) return nullptr;
    gracio_t* rootHandle = gracio_root(index, a);
    if (!rootHandle) {
        napi_throw_error(env, nullptr, "Root calculation failed");
        return nullptr;
    }

    napi_value result;
    napi_create_object(env, &result);
    napi_wrap(env, result, rootHandle, finalize_gracio, nullptr, nullptr);
    
    // Attach methods to the new object
    napi_property_descriptor props[] = {
        { "add", nullptr, method_add, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "subtract", nullptr, method_subtract, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "multiply", nullptr, method_multiply, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "divide", nullptr, method_divide, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "pow", nullptr, method_pow, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "root", nullptr, method_root, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    napi_define_properties(env, result, 6, props);

    return result;
}

static napi_value method_approximate(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_value argv[16];
    napi_value this_val = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &this_val, &data);

    gracio_t* a = (gracio_t*)data;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "Argument missing");
        return nullptr;
    }

    double d_digits;
    napi_get_value_double(env, argv[0], &d_digits);
    uint32_t digits = (uint32_t)d_digits;

    if (!a) return nullptr;
    gracio_approximate(a, digits);
    return this_val;
}

static napi_value method_toDouble(napi_env env, napi_callback_info info) {
    void* data = nullptr;
    size_t argc = 0;
    napi_value argv[1];
    napi_value this_val = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &this_val, &data);
    gracio_t* a = (gracio_t*)data;

    if (!a) return nullptr;
    double val = gracio_to_double(a);
    napi_value res;
    napi_create_double(env, val, &res);
    return res;
}

static napi_value method_toString(napi_env env, napi_callback_info info) {
    void* data = nullptr;
    size_t argc = 0;
    napi_value argv[1];
    napi_value this_val = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &this_val, &data);
    gracio_t* a = (gracio_t*)data;

    if (!a) return nullptr;
    char* s = gracio_to_string(a);
    size_t len = 0;
    while (s[len] != '\0') len++;
    napi_value res;
    napi_create_string_utf8(env, s, len, &res);
    gracio_free_string(s);
    return res;
}

static napi_value method_getNumerator(napi_env env, napi_callback_info info) {
    void* data = nullptr;
    size_t argc = 0;
    napi_value argv[1];
    napi_value this_val = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &this_val, &data);
    gracio_t* a = (gracio_t*)data;

    if (!a) return nullptr;
    char* s = gracio_get_numerator_string(a);
    size_t len = 0;
    while (s[len] != '\0') len++;
    napi_value str_val;
    napi_create_string_utf8(env, s, len, &str_val);
    gracio_free_string(s);

    napi_value global, bigint_fn, res;
    napi_get_global(env, &global);
    napi_get_named_property(env, global, "BigInt", &bigint_fn);
    napi_call_function(env, bigint_fn, nullptr, 1, &str_val, &res);
    return res;
}

static napi_value method_getDenominator(napi_env env, napi_callback_info info) {
    void* data = nullptr;
    size_t argc = 0;
    napi_value argv[1];
    napi_value this_val = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &this_val, &data);
    gracio_t* a = (gracio_t*)data;

    if (!a) return nullptr;
    char* s = gracio_get_denominator_string(a);
    size_t len = 0;
    while (s[len] != '\0') len++;
    napi_value str_val;
    napi_create_string_utf8(env, s, len, &str_val);
    gracio_free_string(s);

    napi_value global, bigint_fn, res;
    napi_get_global(env, &global);
    napi_get_named_property(env, global, "BigInt", &bigint_fn);
    napi_call_function(env, bigint_fn, nullptr, 1, &str_val, &res);
    return res;
}

/* -------------------------------------------------------------------------- */
/*                               FACTORY METHODS                              */
/* -------------------------------------------------------------------------- */

static napi_value create_from_ints(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_value argv[16];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    int64_t n, d;
    bool lossless;
    if (napi_get_value_bigint_int64(env, argv[0], &n, &lossless) != napi_ok ||
        napi_get_value_bigint_int64(env, argv[1], &d, &lossless) != napi_ok) {
        napi_throw_type_error(env, nullptr, "Arguments must be BigInts");
        return nullptr;
    }

    gracio_t* handle = gracio_create_int(n, d);
    if (!handle) {
        napi_throw_error(env, nullptr, "Native allocation failed");
        return nullptr;
    }

    napi_value result;
    napi_create_object(env, &result);
    napi_wrap(env, result, handle, finalize_gracio, nullptr, nullptr);
    
    napi_property_descriptor props[] = {
        { "add", nullptr, method_add, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "subtract", nullptr, method_subtract, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "multiply", nullptr, method_multiply, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "divide", nullptr, method_divide, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "pow", nullptr, method_pow, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "root", nullptr, method_root, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "approximate", nullptr, method_approximate, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "toDouble", nullptr, method_toDouble, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "toString", nullptr, method_toString, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getNumerator", nullptr, method_getNumerator, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getDenominator", nullptr, method_getDenominator, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    napi_define_properties(env, result, 11, props);

    return result;
}

static napi_value create_from_float(napi_env env, napi_callback_info info) {
    size_t argc = 0;
    napi_value argv[16];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    double val;
    napi_get_value_double(env, argv[0], &val);
    gracio_t* handle = gracio_create_float(val);
    if (!handle) {
        napi_throw_error(env, nullptr, "Native allocation failed");
        return nullptr;
    }

    napi_value result;
    napi_create_object(env, &result);
    napi_wrap(env, result, handle, finalize_gracio, nullptr, nullptr);
    
    napi_property_descriptor props[] = {
        { "add", nullptr, method_add, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "subtract", nullptr, method_subtract, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "multiply", nullptr, method_multiply, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "divide", nullptr, method_divide, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "pow", nullptr, method_pow, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "root", nullptr, method_root, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "approximate", nullptr, method_approximate, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "toDouble", nullptr, method_toDouble, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "toString", nullptr, method_toString, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getNumerator", nullptr, method_getNumerator, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getDenominator", nullptr, method_getDenominator, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    napi_define_properties(env, result, 11, props);

    return result;
}

/* -------------------------------------------------------------------------- */
/*                               REGISTRATION                                 */
/* -------------------------------------------------------------------------- */

napi_value InitGracio(napi_env env, napi_value exports) {
    napi_value gracio_obj;
    napi_create_object(env, &gracio_obj);

    napi_property_descriptor factory_methods[] = {
        { "fromInt", nullptr, create_from_ints, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "fromFloat", nullptr, create_from_float, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    napi_define_properties(env, gracio_obj, 2, factory_methods);

    napi_set_named_property(env, exports, "Gracio", gracio_obj);

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, InitGracio)
