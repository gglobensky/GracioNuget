#include "gracio_core.hpp"
#include <intrin.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <fstream>
#include <iostream>

namespace gracio {

bool g_gracio_debug = false;

// --- BigInt Implementation ---

BigInt::BigInt(const std::string& s) : is_small(true), sign(1), size(1) {
    if (s.empty()) throw std::invalid_argument("Empty string");
    size_t start = 0;
    if (s[0] == '-') { sign = -1; start = 1; }
    else if (s[0] == '+') { start = 1; }

    BigInt res(0);
    BigInt ten(10);
    for (size_t i = start; i < s.length(); ++i) {
        if (!isdigit(s[i])) throw std::invalid_argument("Invalid character in BigInt string");
        res *= ten;
        res += BigInt(s[i] - '0');
    }
    *this = res;
    this->sign = sign; 
}

// Manual memory management removed in favor of std::vector

bool BigInt::isZero() const {
    if (is_small) return value_small == 0;
    for (size_t i = 0; i < size; ++i) if (limbs[i] != 0) return false;
    return true;
}

bool BigInt::isEven() const {
    if (is_small) return (value_small & 1) == 0;
    return (limbs[0] & 1) == 0;
}

void BigInt::shiftRight() {
    if (isZero()) return;
    if (is_small) value_small >>= 1;
    else {
        Limb carry = 0;
        for (int i = (int)size - 1; i >= 0; --i) {
            Limb next_carry = (limbs[i] & 1) << 63;
            limbs[i] >>= 1; limbs[i] |= carry; carry = next_carry;
        }
        if (size > 1 && limbs[size-1] == 0) {
            limbs.pop_back();
            size--;
        }
    }
}

void BigInt::shiftLeft() {
    if (isZero()) return;
    if (is_small) {
        if ((value_small & (1ULL << 63)) != 0) {
            Limb val = value_small; is_small = false; size = 2; 
            limbs.assign({(val << 1), (val >> 63)});
        } else value_small <<= 1;
    } else {
        Limb carry = 0;
        for (size_t i = 0; i < size; ++i) {
            Limb next_carry = (limbs[i] >> 63);
            limbs[i] = (limbs[i] << 1) | carry; carry = next_carry;
        }
        if (carry) {
            limbs.push_back(carry);
            size++;
        }
    }
}

int BigInt::compareAbsolute(const BigInt& a, const BigInt& b) {
    if (a.isZero() && b.isZero()) return 0;
    if (a.isZero()) return -1; if (b.isZero()) return 1;
    size_t sizeA = a.is_small ? 1 : a.size;
    size_t sizeB = b.is_small ? 1 : b.size;
    if (sizeA > sizeB) return 1; if (sizeA < sizeB) return -1;
    for (int i = (int)sizeA - 1; i >= 0; --i) {
        Limb valA = a.is_small ? a.value_small : a.limbs[i];
        Limb valB = b.is_small ? b.value_small : b.limbs[i];
        if (valA > valB) return 1; if (valA < valB) return -1;
    }
    return 0;
}

bool BigInt::operator<(const BigInt& other) const {
    if (sign != other.sign) return sign < other.sign;
    int cmp = compareAbsolute(*this, other);
    return sign == 1 ? cmp < 0 : cmp > 0;
}

bool BigInt::operator==(const BigInt& other) const {
    if (sign != other.sign) return false;
    return compareAbsolute(*this, other) == 0;
}

BigInt BigInt::absAdd(const BigInt& a, const BigInt& b) {
    log_msg("Entering absAdd");
    bool sA = a.is_small, sB = b.is_small;
    size_t szA = sA ? 1 : a.size, szB = sB ? 1 : b.size;
    size_t maxS = std::max(szA, szB);
    BigInt res; res.is_small = false; res.sign = 1;
    res.limbs.resize(maxS + 1, 0);
    res.size = maxS;
    Limb carry = 0;
    for (size_t i = 0; i < maxS; ++i) {
        Limb vA = (i < szA) ? (sA ? a.value_small : a.limbs[i]) : 0;
        Limb vB = (i < szB) ? (sB ? b.value_small : b.limbs[i]) : 0;
        uint64_t sum = vA + vB + carry;
        carry = (sum < vA || (sum == vA && carry > 0)) ? 1 : 0;
        res.limbs[i] = sum;
    }
    if (carry) { res.limbs[maxS] = carry; res.size++; }
    res.normalize();
    log_msg("Exiting absAdd");
    return res;
}

BigInt BigInt::absSubtract(const BigInt& a, const BigInt& b) {
    log_msg("Entering absSubtract");
    bool sA = a.is_small, sB = b.is_small;
    size_t szA = sA ? 1 : a.size, szB = sB ? 1 : b.size;
    BigInt res; res.is_small = false; res.sign = 1;
    res.limbs.resize(szA, 0);
    res.size = szA;
    Limb borrow = 0;
    for (size_t i = 0; i < szA; ++i) {
        Limb vA = (i < szA) ? (sA ? a.value_small : a.limbs[i]) : 0;
        Limb vB = (i < szB) ? (sB ? b.value_small : b.limbs[i]) : 0;
        uint64_t diff = vA - vB - borrow;
        borrow = (vA < vB || (vA == vB && borrow > 0)) ? 1 : 0;
        res.limbs[i] = diff;
    }
    res.normalize();
    log_msg("Exiting absSubtract");
    return res;
}

BigInt BigInt::operator+(const BigInt& other) const {
    if (sign == other.sign) {
        BigInt res = absAdd(*this, other); res.sign = sign; return res;
    }
    if (compareAbsolute(*this, other) >= 0) {
        BigInt res = absSubtract(*this, other); res.sign = sign; return res;
    } else {
        BigInt res = absSubtract(other, *this); res.sign = other.sign; return res;
    }
}

BigInt BigInt::operator-(const BigInt& other) const {
    BigInt negOther = other; negOther.sign *= -1;
    return *this + negOther;
}

// Helper for schoolbook multiplication (Absolute values)
BigInt BigInt::schoolbookMultiply(const BigInt& a, const BigInt& b) {
    bool sA = a.is_small, sB = b.is_small;
    size_t szA = sA ? 1 : a.size, szB = sB ? 1 : b.size;
    BigInt res; res.is_small = false; res.sign = 1;
    res.limbs.assign(szA + szB + 1, 0);
    res.size = szA + szB;

    for (size_t i = 0; i < szA; ++i) {
        Limb vA = sA ? a.value_small : a.limbs[i];
        for (size_t j = 0; j < szB; ++j) {
            Limb vB = sB ? b.value_small : b.limbs[j];
            uint64_t high;
            uint64_t low = _umul128(vA, vB, &high);
            uint64_t sumLow = low + res.limbs[i+j];
            res.limbs[i+j] = sumLow;
            uint64_t carry = high + (sumLow < low ? 1 : 0);
            size_t k = i + j + 1;
            while (carry) {
                uint64_t old = res.limbs[k];
                res.limbs[k] += carry;
                carry = (res.limbs[k] < old) ? 1 : 0;
                k++;
            }
        }
    }
    res.normalize();
    return res;
}

// Helper to shift BigInt by limbs (B^s)
BigInt BigInt::shiftLimbs(const BigInt& a, size_t s) {
    if (a.isZero()) return BigInt(0);
    std::vector<Limb> newLimbs;
    newLimbs.reserve(a.size + s);
    for(size_t i=0; i<s; ++i) newLimbs.push_back(0);
    if (a.is_small) {
        newLimbs.push_back(a.value_small);
    } else {
        newLimbs.insert(newLimbs.end(), a.limbs.begin(), a.limbs.end());
    }
    return BigInt(a.sign, newLimbs);
}

// Helper to split BigInt into low and high parts
void BigInt::splitBigInt(const BigInt& a, size_t m, BigInt& low, BigInt& high) {
    if (a.is_small) {
        low = BigInt(1, a.value_small);
        high = BigInt(0);
        return;
    }
    std::vector<Limb> lLimbs, hLimbs;
    size_t actualSize = a.size;
    for (size_t i = 0; i < std::min(m, actualSize); ++i) {
        lLimbs.push_back(a.limbs[i]);
    }
    if (actualSize > m) {
        for (size_t i = m; i < actualSize; ++i) {
            hLimbs.push_back(a.limbs[i]);
        }
    }
    low = BigInt(1, lLimbs);
    high = BigInt(1, hLimbs);
}

// Recursive Karatsuba implementation (Absolute values)
BigInt BigInt::karatsubaRecursive(const BigInt& x, const BigInt& y) {
    size_t szX = x.is_small ? 1 : x.size;
    size_t szY = y.is_small ? 1 : y.size;
    
    // Base case: use schoolbook for small inputs
    if (szX < 32 || szY < 32) {
        return schoolbookMultiply(x, y);
    }

    size_t n = std::max(szX, szY);
    size_t m = n / 2;

    BigInt x0, x1, y0, y1;
    splitBigInt(x, m, x0, x1);
    splitBigInt(y, m, y0, y1);

    // z0 = x0 * y0
    BigInt z0 = karatsubaRecursive(x0, y0);
    // z2 = x1 * y1
    BigInt z2 = karatsubaRecursive(x1, y1);
    // z1 = (x0 + x1) * (y0 + y1) - z0 - z2
    BigInt sumX = x0 + x1;
    BigInt sumY = y0 + y1;
    BigInt z1 = karatsubaRecursive(sumX, sumY);
    z1 = z1 - z0 - z2;

    // Result = z2 * B^(2m) + z1 * B^m + z0
    BigInt res = shiftLimbs(z2, 2 * m);
    res = res + shiftLimbs(z1, m);
    res = res + z0;

    res.normalize();
    return res;
}

BigInt BigInt::operator*(const BigInt& other) const {
    log_msg("Entering operator*");
    if (isZero() || other.isZero()) return BigInt(0);
    
    // Work with absolute values for Karatsuba
    BigInt absThis = *this; absThis.sign = 1;
    BigInt absOther = other; absOther.sign = 1;
    
    BigInt res = karatsubaRecursive(absThis, absOther);
    res.sign = sign * other.sign;
    
    log_msg("Exiting operator*");
    return res;
}

// Helper for Algorithm D to add in place (used for correction loop)
void addBack(std::vector<BigInt::Limb>& rLimbs, const std::vector<BigInt::Limb>& vLimbs, size_t offset) {
    BigInt::Limb carry = 0;
    for (size_t i = 0; i < vLimbs.size(); ++i) {
        if (offset + i >= rLimbs.size()) rLimbs.resize(offset + i + 1, 0);
        uint64_t sum = rLimbs[offset + i] + vLimbs[i] + carry;
        carry = (sum < rLimbs[offset + i] || (sum == rLimbs[offset + i] && carry > 0)) ? 1 : 0;
        rLimbs[offset + i] = sum;
    }
    size_t k = offset + vLimbs.size();
    while (carry && k < rLimbs.size()) {
        uint64_t sum = rLimbs[k] + carry;
        carry = (sum < rLimbs[k]) ? 1 : 0;
        rLimbs[k] = sum;
        k++;
    }
    if (carry) rLimbs.push_back(carry);
}

// Helper for Algorithm D to multiply and subtract in place. Returns true if underflow occurred.
bool multiplySubtract(std::vector<BigInt::Limb>& rLimbs, const std::vector<BigInt::Limb>& vLimbs, BigInt::Limb q, size_t offset) {
    if (q == 0) return false;
    BigInt::Limb borrow = 0;
    for (size_t i = 0; i < vLimbs.size(); ++i) {
        uint64_t high;
        uint64_t prodLow = _umul128(vLimbs[i], q, &high);
        uint64_t sub = prodLow + borrow;
        borrow = high + (sub < prodLow ? 1 : 0);
        
        if (offset + i >= rLimbs.size()) rLimbs.resize(offset + i + 1, 0);
        
        BigInt::Limb oldVal = rLimbs[offset + i];
        rLimbs[offset + i] = oldVal - sub;
        borrow += (oldVal < sub ? 1 : 0);
    }
    size_t k = offset + vLimbs.size();
    while (borrow && k < rLimbs.size()) {
        BigInt::Limb oldVal = rLimbs[k];
        rLimbs[k] -= borrow;
        borrow = (oldVal < borrow ? 1 : 0);
        k++;
    }
    if (borrow) {
        // To restore, we'd need to add back. But for Algorithm D,
        // if it underflows, the qHat was too large.
        return true; 
    }
    return false;
}

// Portable 128-bit by 64-bit division for MSVC (returns quotient)
BigInt::Limb div128(BigInt::Limb high, BigInt::Limb low, BigInt::Limb divisor) {
    if (divisor == 0) return 0;
    BigInt::Limb q = 0;
    BigInt::Limb rHigh = high;
    BigInt::Limb rLow = low;

    for (int i = 63; i >= 0; --i) {
        BigInt::Limb carry = (rLow >> 63);
        rLow <<= 1;
        rHigh = (rHigh << 1) | carry;

        if (rHigh >= divisor) {
            rHigh -= divisor;
            q |= (1ULL << i);
        }
    }
    return q;
}

void BigInt::divMod(const BigInt& a, const BigInt& b, BigInt& q, BigInt& r) {
    log_msg("Entering divMod (Algorithm D)");
    if (b.isZero()) throw std::runtime_error("Division by zero");
    
    BigInt absA = a; absA.sign = 1;
    BigInt absB = b; absB.sign = 1;

    if (BigInt::compareAbsolute(absA, absB) < 0) {
        q = BigInt(0); r = absA;
        q.sign = a.sign * b.sign; r.sign = a.sign;
        return;
    }

    std::vector<Limb> u = absA.is_small ? std::vector<Limb>{absA.value_small} : absA.limbs;
    std::vector<Limb> v = absB.is_small ? std::vector<Limb>{absB.value_small} : absB.limbs;

    size_t n = v.size();
    // Normalization: d = floor(B / (v[n-1] + 1))
    Limb vn_1 = v[n - 1];
    Limb d = (0xFFFFFFFFFFFFFFFFULL / (vn_1 + 1)) + 1;
    if (d == 0) d = 1;

    std::vector<Limb> un(u.size());
    Limb carryU = 0;
    for (size_t i = 0; i < u.size(); ++i) {
        uint64_t high;
        uint64_t low = _umul128(u[i], d, &high);
        un[i] = low + carryU;
        carryU = high;
    }
    if (carryU) un.push_back(carryU);

    std::vector<Limb> vn(v.size());
    Limb carryV = 0;
    for (size_t i = 0; i < v.size(); ++i) {
        uint64_t high;
        uint64_t low = _umul128(v[i], d, &high);
        vn[i] = low + carryV;
        carryV = high;
    }

    size_t m = un.size() - n;
    std::vector<Limb> qLimbs(m, 0);
    std::vector<Limb> rem = un;

    for (int j = (int)m - 1; j >= 0; --j) {
        // Estimation: qHat = (rem[j+n]*B + rem[j+n-1]) / vn[n-1]
        Limb highPart = 0, lowPart = 0;
        if (rem.size() > (size_t)j + n) {
            highPart = rem[j + n];
            lowPart = rem[j + n - 1];
        } else if (rem.size() == (size_t)j + n && j+n-1 < rem.size()) {
             lowPart = rem[j + n - 1];
        }
        Limb qHat = div128(highPart, lowPart, vn[n - 1]);

        // Refinement loop: if qHat * v > currentRem window, decrement and restore
        while (qHat > 0) {
            if (multiplySubtract(rem, vn, qHat, j)) {
                addBack(rem, vn, j); // Restore remainder
                qHat--;
            } else {
                break; 
            }
        }

        if (qHat > 0) {
            qLimbs[j] = qHat;
        } else {
            // If qHat became 0 but multiplySubtract was called, we already restored.
            // But if it started as 0, no subtraction happened.
        }
    }

    // Denormalize Remainder: r = rem / d
    // Use the divideByScalar helper for efficiency and correctness.
    BigInt resR(1, rem);
    resR.divideByScalar(d);

    BigInt resQ(1, qLimbs);
    q = resQ; r = resR;
    q.sign = a.sign * b.sign;
    r.sign = a.sign;
    log_msg("Exiting divMod");
}

BigInt BigInt::operator/(const BigInt& other) const {
    BigInt q, r;
    divMod(*this, other, q, r);
    return q;
}

BigInt BigInt::operator%(const BigInt& other) const {
    BigInt q, r;
    divMod(*this, other, q, r);
    return r;
}

void BigInt::operator+=(const BigInt& other) { *this = *this + other; }
void BigInt::operator-=(const BigInt& other) { *this = *this - other; }
void BigInt::operator*=(const BigInt& other) { *this = *this * other; }
void BigInt::operator/=(const BigInt& other) { *this = *this / other; }
void BigInt::operator%=(const BigInt& other) { *this = *this % other; }

void BigInt::normalize() {
    log_msg("Entering normalize");
    if (is_small) return;
    while (size > 1 && limbs[size - 1] == 0) {
        limbs.pop_back();
        size--;
    }
    if (size == 1) {
        value_small = limbs[0];
        limbs.clear();
        is_small = true;
    }
    log_msg("Exiting normalize");
}

bool BigInt::isDivisibleBy(uint64_t p) const {
    if (isZero()) return true;
    if (is_small) return (value_small % p) == 0;
    BigInt temp = *this;
    temp.divideByScalar(p);
    // If divisible, the final remainder would be 0. 
    // Since divideByScalar doesn't return the remainder, we can check if 
    // (temp * p) == (*this). But that's slow.
    // Let's just implement a simple mod helper.
    BigInt::Limb rem = 0;
    for (int i = (int)size - 1; i >= 0; --i) {
        BigInt::Limb rHigh = rem;
        BigInt::Limb rLow = limbs[i];
        for (int j = 63; j >= 0; --j) {
            BigInt::Limb carry = (rLow >> 63);
            rLow <<= 1;
            rHigh = (rHigh << 1) | carry;
            if (rHigh >= p) rHigh -= p;
        }
        rem = rHigh;
    }
    return rem == 0;
}

uint64_t BigInt::divideByScalar(uint64_t p) {
    if (isZero()) return 0;
    BigInt::Limb carry = 0;
    if (is_small) {
        uint64_t rem = value_small % p;
        value_small /= p;
        return rem;
    }
    for (int i = (int)size - 1; i >= 0; --i) {
        // Divide (carry * B + limbs[i]) by p
        BigInt::Limb high = carry;
        BigInt::Limb low = limbs[i];
        BigInt::Limb q = 0;
        for (int j = 63; j >= 0; --j) {
            BigInt::Limb c = (low >> 63);
            low <<= 1;
            high = (high << 1) | c;
            if (high >= p) {
                high -= p;
                q |= (1ULL << j);
            }
        }
        limbs[i] = q;
        carry = high;
    }
    normalize();
    return carry;
}

BigInt BigInt::gcd(const BigInt& a, const BigInt& b) {
    if (a.isZero()) return b; if (b.isZero()) return a;
    BigInt n = a; BigInt d = b; n.sign = 1; d.sign = 1;
    uint64_t shift = 0;
    while (n.isEven() && d.isEven()) { n.shiftRight(); d.shiftRight(); shift++; }
    while (n.isEven()) n.shiftRight();
    do {
        while (d.isEven()) d.shiftRight();
        if (n > d) { BigInt t = n; n = d; d = t; }
        d -= n;
    } while (!d.isZero());
    for (uint64_t i = 0; i < shift; ++i) n.shiftLeft();
    return n;
}

BigInt BigInt::pow(uint64_t exp) const {
    if (exp == 0) return BigInt(1);
    if (exp == 1) return *this;

    BigInt res(1);
    BigInt base = *this;
    while (exp > 0) {
        if (exp % 2 == 1) res = res * base;
        base = base * base;
        exp /= 2;
    }
    return res;
}

std::string BigInt::toString() const {
    if (isZero()) return "0";
    if (is_small) {
        std::string s = std::to_string(value_small);
        return sign == -1 ? "-" + s : s;
    }

    std::vector<std::string> chunks;
    BigInt temp = *this;
    temp.sign = 1;
    const uint64_t base = 1000000000; // 10^9

    while (!temp.isZero()) {
        uint64_t rem = temp.divideByScalar(base);
        std::string chunk = std::to_string(rem);
        chunks.push_back(chunk);
    }

    // Pad all but the last (most significant) chunk
    for (size_t i = 0; i < chunks.size() - 1; ++i) {
        if (chunks[i].length() < 9) {
            chunks[i] = std::string(9 - chunks[i].length(), '0') + chunks[i];
        }
    }

    std::string res = "";
    for (int i = (int)chunks.size() - 1; i >= 0; --i) {
        res += chunks[i];
    }
    return sign == -1 ? "-" + res : res;
}

int64_t BigInt::toInt64() const {
    if (is_small) return sign * static_cast<int64_t>(value_small);
    if (size == 1) return sign * static_cast<int64_t>(limbs[0]);
    throw std::runtime_error("BigInt too large for int64");
}

// --- Gracio Implementation ---

Gracio::Gracio(BigInt n, BigInt d, uint32_t limit) : numerator(n), denominator(d), precisionLimit(limit) {
    log_msg("Constructing Gracio");
    if (denominator.isZero()) throw std::runtime_error("Denominator cannot be zero");
    if (denominator.sign == -1) { numerator.sign *= -1; denominator.sign = 1; }
    simplify();
}

void Gracio::simplify() {
    // 1. Trial division with small primes for fast initial reduction
    for (uint64_t p : SMALL_PRIMES) {
        while (numerator.isDivisibleBy(p) && denominator.isDivisibleBy(p)) {
            numerator.divideByScalar(p);
            denominator.divideByScalar(p);
        }
    }

    // 2. Fallback to Binary GCD for remaining large factors
    BigInt g = BigInt::gcd(numerator, denominator);
    if (!(g == BigInt(1))) {
        BigInt q, r;
        BigInt::divMod(numerator, g, q, r);
        numerator = q;
        BigInt::divMod(denominator, g, q, r);
        denominator = q;
    }
}

void Gracio::add(const Gracio& other) {
    if (denominator == other.denominator) numerator += other.numerator;
    else {
        BigInt n = numerator * other.denominator + other.numerator * denominator;
        BigInt d = denominator * other.denominator;
        numerator = n; denominator = d;
    }
    simplify(); checkPrecisionLimit();
}

void Gracio::subtract(const Gracio& other) {
    if (denominator == other.denominator) numerator -= other.numerator;
    else {
        BigInt n = numerator * other.denominator - other.numerator * denominator;
        BigInt d = denominator * other.denominator;
        numerator = n; denominator = d;
    }
    simplify(); checkPrecisionLimit();
}

void Gracio::multiply(const Gracio& other) {
    numerator *= other.numerator; denominator *= other.denominator;
    simplify(); checkPrecisionLimit();
}

void Gracio::divide(const Gracio& other) {
    if (other.numerator.isZero()) throw std::runtime_error("Divide by zero");
    numerator *= other.denominator; denominator *= other.numerator;
    if (denominator.sign == -1) { numerator.sign *= -1; denominator.sign = 1; }
    simplify(); checkPrecisionLimit();
}

void Gracio::power(int64_t exp) {
    if (exp == 0) {
        numerator = BigInt(1); denominator = BigInt(1);
        return;
    }
    if (exp < 0) {
        std::swap(numerator, denominator);
        exp = -exp;
    }
    uint64_t uExp = static_cast<uint64_t>(exp);
    numerator = numerator.pow(uExp);
    denominator = denominator.pow(uExp);
    simplify(); checkPrecisionLimit();
}

RootResult Gracio::integerRoot(const BigInt& val, uint32_t index) {
    if (val.isZero()) return {BigInt(0), true};
    if (index == 1) return {val, true};
    if (val.sign == -1 && index % 2 == 0) return {BigInt(0), false};

    BigInt absVal = val; absVal.sign = 1;
    
    // Initial guess x0: Use double precision to get a close start if possible
    double approx = std::pow(absVal.toDouble(), 1.0 / index);
    BigInt x;
    if (std::isfinite(approx) && approx < 1e18) {
        x = BigInt(static_cast<int64_t>(approx) + 1);
    } else {
        // For extremely large numbers, use a bit-based guess: 2^(ceil(bits/index))
        size_t bits = (absVal.is_small ? 64 : absVal.size * 64);
        size_t guessBits = (bits + index - 1) / index;
        x = BigInt(1);
        for(size_t i=0; i<guessBits; ++i) x.shiftLeft();
    }

    while (true) {
        // Integer Newton's Method: x_{n+1} = floor( ((k-1)*x_n + A / x_n^{k-1}) / k )
        BigInt term1 = x * BigInt(index - 1);
        BigInt term2 = absVal / x.pow(index - 1);
        BigInt sum = term1 + term2;
        sum.divideByScalar(index);
        
        if (sum >= x) break;
        x = sum;
    }

    // The root is either x or x-1. We check if it's an exact root.
    BigInt res = x; 
    bool found = (res.pow(index) == absVal);
    if (!found) {
        BigInt prev = x - BigInt(1);
        if (prev.pow(index) == absVal) {
            res = prev;
            found = true;
        }
    }

    res.sign = val.sign;
    return RootResult(res, found);
}

// Internal helper for root approximations
Gracio continuedFractionApprox(double target) {
    double x = target;
    long long h_prev2 = 0, h_prev1 = 1;
    long long k_prev2 = 1, k_prev1 = 0;

    for (int i = 0; i < 30; ++i) { // Max iterations to prevent overflow of long long
        double a = std::floor(x);
        long long curr_h = (long long)a * h_prev1 + h_prev2;
        long long curr_k = (long long)a * k_prev1 + k_prev2;

        // Check for overflow of long long in convergents
        if (curr_k < 0 || curr_h < 0) break; 

        h_prev2 = h_prev1; h_prev1 = curr_h;
        k_prev2 = k_prev1; k_prev1 = curr_k;

        if (x - a == 0) break;
        double next_x = 1.0 / (x - a);
        if (std::isinf(next_x)) break;
        x = next_x;
    }
    return Gracio(BigInt(h_prev1), BigInt(k_prev1));
}

Gracio Gracio::root(uint32_t index, const Gracio& value) {
    if (index == 0) throw std::runtime_error("Root index cannot be zero");
    if (index == 1) return value;

    // Step 1: Exact Power Check
    RootResult resN = integerRoot(value.numerator, index);
    RootResult resD = integerRoot(value.denominator, index);

    if (resN.found && resD.found) {
        return Gracio(resN.value, resD.value);
    }

    // Step 2: Approximation via Continued Fractions
    double target = std::pow(value.toDouble(), 1.0 / index);
    return continuedFractionApprox(target);
}

void Gracio::checkPrecisionLimit() {
    if (precisionLimit == 0) return;
    BigInt absDen = denominator; absDen.sign = 1;
    if (absDen.toString().length() > precisionLimit * 2) {
        approximate(precisionLimit);
    }
}

Gracio& Gracio::approximate(uint32_t digits) {
    BigInt a = numerator; a.sign = 1;
    BigInt b = denominator; b.sign = 1;
    bool originalSign = (numerator.sign != denominator.sign);

    if (a.isZero()) return *this;

    BigInt h_prev2(0), h_prev1(1);
    BigInt k_prev2(1), k_prev1(0);

    while (!b.isZero()) {
        BigInt q = a / b;
        BigInt r = a % b;

        BigInt h = q * h_prev1 + h_prev2;
        BigInt k = q * k_prev1 + k_prev2;

        if (k.toString().length() > digits) {
            break;
        }

        h_prev2 = h_prev1;
        h_prev1 = h;
        k_prev2 = k_prev1;
        k_prev1 = k;

        a = b;
        b = r;
    }

    numerator = h_prev1;
    numerator.sign = originalSign ? -1 : 1;
    denominator = k_prev1;
    return *this;
}

double BigInt::toDouble() const {
    if (isZero()) return 0.0;
    double signVal = (sign == -1) ? -1.0 : 1.0;
    if (is_small) return signVal * static_cast<double>(value_small);

    // To get a precise double, we need the most significant ~53 bits.
    // We take the top two limbs to be safe and shift them.
    uint64_t high = limbs[size - 1];
    uint64_t low = (size > 1) ? limbs[size - 2] : 0;

    double val = static_cast<double>(high);
    val += static_cast<double>(low) / 18446744073709551616.0; // divide by 2^64

    // Scale by (2^64)^(size-1)
    // log2(val * 2^(64*(size-1))) = log2(val) + 64*(size-1)
    // We can use ldexp for this.
    return signVal * std::ldexp(val, 64 * (int)(size - 1));
}

double Gracio::toDouble() const {
    if (numerator.isZero()) return 0.0;
    return numerator.toDouble() / denominator.toDouble();
}

std::string Gracio::toString() const {
    return numerator.toString() + "/" + denominator.toString();
}

} // namespace gracio
