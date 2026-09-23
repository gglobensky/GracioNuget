#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <memory>
#include <fstream>

namespace gracio {



// Small primes for fast GCD simplification
static const uint64_t SMALL_PRIMES[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
    157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
    239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317,
    331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419,
    421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499
};

extern bool g_gracio_debug;
inline void log_msg(const std::string& msg) {
    if (!g_gracio_debug) return;
    std::ofstream logfile("gracio_debug.log", std::ios::app);
    if (logfile.is_open()) {
        logfile << msg << std::endl;
        logfile.close();
    }
}

class BigInt {
public:
    using Limb = uint64_t;
    static constexpr size_t SIO_THRESHOLD = 1;

    friend struct Gracio;

    BigInt() : is_small(true), value_small(0), sign(1), size(1) {}
    BigInt(int64_t val) : is_small(true), sign(val < 0 ? -1 : 1) {
        value_small = static_cast<uint64_t>(val < 0 ? -val : val);
        size = 1;
    }
    BigInt(const std::string& s);

    // Destructor and copy/move operations are handled by std::vector defaults

    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const { return !(*this == other); }
    bool operator<(const BigInt& other) const;
    bool operator>(const BigInt& other) const { return other < *this; }
    bool operator<=(const BigInt& other) const { return !(*this > other); }
    bool operator>=(const BigInt& other) const { return !(*this < other); }

    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator*(const BigInt& other) const;
    BigInt operator/(const BigInt& other) const;
    BigInt operator%(const BigInt& other) const;

    void operator+=(const BigInt& other);
    void operator-=(const BigInt& other);
    void operator*=(const BigInt& other);
    void operator/=(const BigInt& other);
    void operator%=(const BigInt& other);

    void shiftRight();
    void shiftLeft();
    bool isEven() const;
    bool isZero() const;

    std::string toString() const;
    int64_t toInt64() const;
    double toDouble() const;

    static BigInt gcd(const BigInt& a, const BigInt& b);
    BigInt pow(uint64_t exp) const;
    // Returns the integer n-th root if it exists exactly, otherwise returns an empty/null result.
    // Using a special value (e.g., isZero() but with sign=0 or similar) to indicate "not found".
    // Actually, let's return a pair or use a boolean flag. 
    // For simplicity in this API, we can return a BigInt and provide a separate check, 
    // or just handle the "null" case by returning a BigInt with size=0/is_small=true/value_small=0 but marked as invalid.
    // Better: Use std::optional if possible, but since this is C++11/14 for compatibility, let's use a custom result struct.

private:
    bool is_small;
    int8_t sign; 
    Limb value_small;
    std::vector<Limb> limbs;
    size_t size;

    void ensureCapacity(size_t new_size);
    void normalize();
    static int compareAbsolute(const BigInt& a, const BigInt& b);
    
    BigInt(int8_t s, Limb val_small) : is_small(true), sign(s), value_small(val_small), size(1) {}
    BigInt(int8_t s, const std::vector<Limb>& l) : is_small(false), sign(s), limbs(l), size(l.size()) { normalize(); }

    static BigInt absAdd(const BigInt& a, const BigInt& b);
    static BigInt absSubtract(const BigInt& a, const BigInt& b);
    static void divMod(const BigInt& a, const BigInt& b, BigInt& q, BigInt& r);

    bool isDivisibleBy(uint64_t p) const;
    uint64_t divideByScalar(uint64_t p);

    // Karatsuba Helpers
    static BigInt schoolbookMultiply(const BigInt& a, const BigInt& b);
    static BigInt shiftLimbs(const BigInt& a, size_t s);
    static void splitBigInt(const BigInt& a, size_t m, BigInt& low, BigInt& high);
    static BigInt karatsubaRecursive(const BigInt& x, const BigInt& y);

};

struct RootResult {
    BigInt value;
    bool found;
    RootResult(const BigInt& v, bool f) : value(v), found(f) {}
    RootResult() : value(0), found(false) {}
};

Gracio continuedFractionApprox(double target);

struct Gracio {
    BigInt numerator;
    BigInt denominator;
    uint32_t precisionLimit = 0;

    Gracio() : numerator(0), denominator(1), precisionLimit(0) {}
    Gracio(BigInt n, BigInt d = BigInt(1), uint32_t limit = 0);
    ~Gracio() = default; 
    
    void simplify();
    Gracio& approximate(uint32_t digits);

    void add(const Gracio& other);
    void subtract(const Gracio& other);
    void multiply(const Gracio& other);
    void divide(const Gracio& other);
    void power(int64_t exp);
    static RootResult integerRoot(const BigInt& val, uint32_t index);
    static Gracio root(uint32_t index, const Gracio& value);
    
    double toDouble() const;
    std::string toString() const;

private:
    void checkPrecisionLimit();
};

} // namespace gracio
