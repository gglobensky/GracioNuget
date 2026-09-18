#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <memory>

namespace gracio {

/**
 * BigInt implements arbitrary-precision integers with Small Integer Optimization (SIO).
 */
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

    ~BigInt();

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

    static BigInt gcd(const BigInt& a, const BigInt& b);

private:
    bool is_small;
    int8_t sign; 
    union {
        Limb value_small;
        Limb* limbs;
    };
    size_t size;

    void ensureCapacity(size_t new_size);
    void normalize();
    static int compareAbsolute(const BigInt& a, const BigInt& b);
    
    BigInt(int8_t s, Limb val_small) : is_small(true), sign(s), value_small(val_small), size(1) {}
    BigInt(int8_t s, Limb* l, size_t sz) : is_small(false), sign(s), limbs(l), size(sz) {}

    static BigInt absAdd(const BigInt& a, const BigInt& b);
    static BigInt absSubtract(const BigInt& a, const BigInt& b);
    static void divMod(const BigInt& a, const BigInt& b, BigInt& q, BigInt& r);

    BigInt(const BigInt& other);
    BigInt& operator=(const BigInt& other);
    BigInt(BigInt&& other) noexcept;
    BigInt& operator=(BigInt&& other) noexcept;
};

/**
 * Gracio handles rational arithmetic using two BigInts.
 */
struct Gracio {
    BigInt numerator;
    BigInt denominator;
    uint32_t precisionLimit = 0;

    Gracio(BigInt n, BigInt d = BigInt(1), uint32_t limit = 0);
    ~Gracio() = default; 
    
    void simplify();
    void approximate(uint32_t digits);

    void add(const Gracio& other);
    void subtract(const Gracio& other);
    void multiply(const Gracio& other);
    void divide(const Gracio& other);
    
    double toDouble() const;
    std::string toString() const;

private:
    void checkPrecisionLimit();
};

} // namespace gracio
