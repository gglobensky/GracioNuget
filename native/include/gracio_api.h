#ifndef GRACIO_API_H
#define GRACIO_API_H

#ifdef _WIN32
    #define GRACIO_API __declspec(dllexport)
#else
    #define GRACIO_API __attribute__((visibility("default")))
#endif

#include <stdint.h>
#include <stdbool.h>

extern "C" {

// Opaque handle to the Gracio object
typedef struct gracio_t gracio_t;

// Lifecycle
GRACIO_API gracio_t* gracio_create_int(int64_t numerator, int64_t denominator);
GRACIO_API gracio_t* gracio_create_float(double value);
GRACIO_API void gracio_destroy(gracio_t* g);
GRACIO_API void gracio_set_debug(bool enabled);
GRACIO_API gracio_t* gracio_clone(const gracio_t* g);

// Configuration
GRACIO_API void gracio_set_precision_limit(gracio_t* g, uint32_t limit);

// Arithmetic (Mutable - modifies the first argument)
GRACIO_API void gracio_add(gracio_t* a, const gracio_t* b);
GRACIO_API void gracio_subtract(gracio_t* a, const gracio_t* b);
GRACIO_API void gracio_multiply(gracio_t* a, const gracio_t* b);
GRACIO_API int try_gracio_divide(gracio_t* a, const gracio_t* b);
GRACIO_API void gracio_power(gracio_t* a, int64_t exp);
GRACIO_API void gracio_approximate(gracio_t* g, uint32_t digits);
GRACIO_API gracio_t* gracio_root(uint32_t index, const gracio_t* value);

// Output
GRACIO_API double gracio_to_double(const gracio_t* g);
GRACIO_API char* gracio_get_numerator_string(const gracio_t* g);
GRACIO_API char* gracio_get_denominator_string(const gracio_t* g);
GRACIO_API char* gracio_to_string(const gracio_t* g); // Caller must free the returned string
GRACIO_API void gracio_free_string(char* s);

}

#endif // GRACIO_API_H
