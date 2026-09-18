#include "gracio_core.hpp"
#include <intrin.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace gracio {

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

BigInt::~BigInt() {
    if (!is_small) delete[] limbs;
}

BigInt::BigInt(const BigInt& other) {
    is_small = other.is_small;
    sign = other.sign;
    size = other.size;
    if (is_small) value_small = other.value_small;
    else {
        limbs = new Limb[size];
        std::copy(other.limbs, other.limbs + size, limbs);
    }
}

BigInt& BigInt::operator=(const BigInt& other) {
    if (this == &other) return *this;
    if (!is_small) delete[] limbs;
    is_small = other.is_small;
    sign = other.sign;
    size = other.size;
    if (is_small) value_small = other.value_small;
    else {
        limbs = new Limb[size];
        std::copy(other.limbs, other.limbs + size, limbs);
    }
    return *this;
}

BigInt::BigInt(BigInt&& other) noexcept : is_small(other.is_small), sign(other.sign), size(other.size) {
    if (is_small) value_small = other.value_small;
    else limbs = other.limbs;
    other.is_small = true; other.value_small = 0; other.sign = 1; other.size = 1; other.limbs = nullptr;
}

BigInt& BigInt::operator=(BigInt&& other) noexcept {
    if (this == &other) return *this;
    if (!is_small) delete[] limbs;
    is_small = other.is_small; sign = other.sign; size = other.size;
    if (is_small) value_small = other.value_small;
    else limbs = other.limbs;
    other.is_small = true; other.value_small = 0; other.sign = 1; other.size = 1; other.limbs = nullptr;
    return *this;
}

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
            Limb* new_limbs = new Limb[size - 1];
            std::copy(limbs, limbs + size - 1, new_limbs);
            delete[] limbs; limbs = new_limbs; size--;
        }
    }
}

void BigInt::shiftLeft() {
    if (isZero()) return;
    if (is_small) {
        if ((value_small & (1ULL << 63)) != 0) {
            Limb val = value_small; is_small = false; size = 2; limbs = new Limb[2];
            limbs[0] = (val << 1); limbs[1] = (val >> 63);
        } else value_small <<= 1;
    } else {
        Limb carry = 0;
        for (size_t i = 0; i < size; ++i) {
            Limb next_carry = (limbs[i] >> 63);
            limbs[i] = (limbs[i] << 1) | carry; carry = next_carry;
        }
        if (carry) {
            Limb* new_limbs = new Limb[size + 1];
            std::copy(limbs, limbs + size, new_limbs);
            new_limbs[size] = carry; delete[] limbs; limbs = new_limbs; size++;
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
    bool sA = a.is_small, sB = b.is_small;
    size_t szA = sA ? 1 : a.size, szB = sB ? 1 : b.size;
    size_t maxS = std::max(szA, szB);
    BigInt res; res.is_small = false; res.limbs = new Limb[maxS + 1]; res.size = maxS; res.sign = 1;
    Limb carry = 0;
    for (size_t i = 0; i < maxS; ++i) {
        Limb vA = (i < szA) ? (sA ? a.value_small : a.limbs[i]) : 0;
        Limb vB = (i < szB) ? (sB ? b.value_small : b.limbs[i]) : 0;
        uint64_t sum = vA + vB + carry;
        carry = (sum < vA || (sum == vA && carry > 0)) ? 1 : 0;
        res.limbs[i] = sum;
    }
    if (carry) { res.limbs[maxS] = carry; res.size++; }
    return res;
}

BigInt BigInt::absSubtract(const BigInt& a, const BigInt& b) {
    bool sA = a.is_small, sB = b.is_small;
    size_t szA = sA ? 1 : a.size, szB = sB ? 1 : b.size;
    BigInt res; res.is_small = false; res.limbs = new Limb[szA]; res.size = szA; res.sign = 1;
    Limb borrow = 0;
    for (size_t i = 0; i < szA; ++i) {
        Limb vA = (i < szA) ? (sA ? a.value_small : a.limbs[i]) : 0;
        Limb vB = (i < szB) ? (sB ? b.value_small : b.limbs[i]) : 0;
        uint64_t diff = vA - vB - borrow;
        borrow = (vA < vB || (vA == vB && borrow > 0)) ? 1 : 0;
        res.limbs[i] = diff;
    }
    while (res.size > 1 && res.limbs[res.size-1] == 0) {
        Limb* nL = new Limb[res.size - 1]; std::copy(res.limbs, res.limbs + res.size - 1, nL);
        delete[] res.limbs; res.limbs = nL; res.size--;
    }
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

BigInt BigInt::operator*(const BigInt& other) const {
    if (isZero() || other.isZero()) return BigInt(0);
    bool sA = is_small, sB = other.is_small;
    size_t szA = sA ? 1 : size, szB = sB ? 1 : other.size;
    BigInt res; res.is_small = false; res.limbs = new Limb[szA + szB]; res.size = szA + szB; res.sign = sign * other.sign;
    std::fill(res.limbs, res.limbs + res.size, 0);

    for (size_t i = 0; i < szA; ++i) {
        Limb vA = sA ? value_small : limbs[i];
        for (size_t j = 0; j < szB; ++j) {
            Limb vB = sB ? other.value_small : other.limbs[j];
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
    while (res.size > 1 && res.limbs[res.size-1] == 0) {
        Limb* nL = new Limb[res.size - 1]; std::copy(res.limbs, res.limbs + res.size - 1, nL);
        delete[] res.limbs; res.limbs = nL; res.size--;
    }
    return res;
}

void BigInt::divMod(const BigInt& a, const BigInt& b, BigInt& q, BigInt& r) {
    if (b.isZero()) throw std::runtime_error("Division by zero");
    
    BigInt absA = a; absA.sign = 1;
    BigInt absB = b; absB.sign = 1;
    
    BigInt quotient(0);
    BigInt remainder(0);

    // Binary Long Division (Shift-and-Subtract) - O(bits^2)
    // TODO: Implement Knuth's Algorithm D for O(n*m) performance.
    for (int i = (absA.is_small ? 63 : (int)(absA.size * 64)) ; i >= 0; --i) {
        remainder.shiftLeft();
        if ((absA.is_small && (absA.value_small & (1ULL << i))) || 
            (!absA.is_small && i < (int)absA.size * 64 && (absA.limbs[i/64] & (1ULL << (i%64))))) {
            remainder += BigInt(1);
        }

        if (BigInt::compareAbsolute(remainder, absB) >= 0) {
            remainder -= absB;
            // Set bit i in quotient. This is a simplification for the prototype.
            // In a real impl, we'd use limbs to set bits.
            if (quotient.is_small) {
                if (i < 63) quotient.value_small |= (1ULL << i);
                else {
                    quotient.is_small = false;
                    quotient.size = 2;
                    quotient.limbs = new Limb[2];
                    quotient.limbs[0] = 0;
                    quotient.limbs[1] = (i == 63) ? 1 : 0;
                }
            } else {
                size_t limbIdx = i / 64;
                if (limbIdx >= quotient.size) {
                    Limb* nL = new Limb[quotient.size + 1];
                    std::copy(quotient.limbs, quotient.limbs + quotient.size, nL);
                    delete[] quotient.limbs;
                    quotient.limbs = nL;
                    quotient.size++;
                }
                quotient.limbs[limbIdx] |= (1ULL << (i % 64));
            }
        }
    }

    q = quotient; r = remainder;
    q.sign = a.sign * b.sign;
    r.sign = a.sign;
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

std::string BigInt::toString() const {
    if (isZero()) return "0";
    if (is_small && size == 1) {
        std::string s = std::to_string(value_small);
        return sign == -1 ? "-" + s : s;
    }

    // Real Base-10 Conversion: Repeated division by 10
    std::string res = "";
    BigInt temp = *this;
    temp.sign = 1;
    BigInt ten(10);

    while (!temp.isZero()) {
        BigInt q, r;
        divMod(temp, ten, q, r);
        res += (char)('0' + r.toInt64());
        temp = q;
    }
    std::reverse(res.begin(), res.end());
    return sign == -1 ? "-" + res : res;
}

int64_t BigInt::toInt64() const {
    if (!is_small) throw std::runtime_error("BigInt too large for int64");
    return sign * static_cast<int64_t>(value_small);
}

// --- Gracio Implementation ---

Gracio::Gracio(BigInt n, BigInt d, uint32_t limit) : numerator(n), denominator(d), precisionLimit(limit) {
    if (denominator.isZero()) throw std::runtime_error("Denominator cannot be zero");
    if (denominator.sign == -1) { numerator.sign *= -1; denominator.sign = 1; }
    simplify();
}

void Gracio::simplify() {
    BigInt g = BigInt::gcd(numerator, denominator);
    if (!(g == BigInt(1))) {
        BigInt q, r;
        divMod(numerator, g, q, r);
        numerator = q;
        divMod(denominator, g, q, r);
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

void Gracio::checkPrecisionLimit() {}
void Gracio::approximate(uint32_t digits) {}

double Gracio::toDouble() const {
    if (numerator.isZero()) return 0.0;
    // To avoid overflow in toInt64, use a double conversion for large numbers
    if (!numerator.is_small || !denominator.is_small) {
        // Approximation: convert top limbs to double
        double n = (double)numerator.is_small ? numerator.toInt64() : (double)numerator.limbs[numerator.size-1];
        double d = (double)denominator.is_small ? denominator.toInt64() : (double)denominator.limbs[denominator.size-1];
        return n / d; 
    }
    return (double)numerator.toInt64() / denominator.toInt64();
}

std::string Gracio::toString() const {
    return numerator.toString() + "/" + denominator.toString();
}

} // namespace gracio
