const SMALL_PRIMES = [
    2n, 3n, 5n, 7n, 11n, 13n, 17n, 19n, 23n, 29n, 31n, 37n, 41n, 43n, 47n, 53n, 59n, 61n, 67n, 71n,
    73n, 79n, 83n, 89n, 97n, 101n, 103n, 107n, 109n, 113n, 127n, 131n, 137n, 139n, 149n, 151n,
    157n, 163n, 167n, 173n, 179n, 181n, 191n, 193n, 197n, 199n, 211n, 223n, 227n, 229n, 233n,
    239n, 241n, 251n, 257n, 263n, 269n, 271n, 277n, 281n, 283n, 293n, 307n, 311n, 313n, 317n,
    331n, 337n, 347n, 349n, 353n, 359n, 367n, 373n, 379n, 383n, 389n, 397n, 401n, 409n, 419n,
    421n, 431n, 433n, 439n, 443n, 449n, 457n, 461n, 463n, 467n, 479n, 487n, 491n, 499n
];
export class Gracio {
    numerator;
    denominator;
    precisionLimit; // Max decimal digits of precision to maintain
    static MIN_SIMPLIFY_THRESHOLD = 10n ** 20n;
    lastSimplifiedDigits = 0;
    constructor(numerator, denominator = 1n, precisionLimit) {
        if (denominator === 0n)
            throw new Error("Denominator cannot be zero");
        // Ensure the denominator is always positive
        if (denominator < 0n) {
            numerator *= -1n;
            denominator *= -1n;
        }
        this.numerator = numerator;
        this.denominator = denominator;
        this.precisionLimit = precisionLimit;
        this.simplify();
    }
    static fromInt(value) {
        return new Gracio(BigInt(value), 1n);
    }
    /**
     * Creates a Gracio instance from a floating-point representation.
     * Uses string parsing to ensure "human-intuitive" exactness (e.g., "0.1" -> 1/10).
     */
    static fromFloat(value) {
        const s = String(value).trim().toLowerCase();
        if (s === 'nan' || s === 'infinity' || s === '-infinity') {
            throw new Error("Cannot represent NaN or Infinity as a ratio");
        }
        // Handle scientific notation: e.g., 1.23e-4
        const eIndex = s.indexOf('e');
        if (eIndex !== -1) {
            const coefficientStr = s.substring(0, eIndex);
            const exponentStr = s.substring(eIndex + 1);
            const exponent = parseInt(exponentStr, 10);
            if (isNaN(exponent))
                throw new Error(`Invalid exponent in float: ${s}`);
            const coeff = Gracio.fromFloat(coefficientStr);
            const scaleNum = exponent >= 0 ? 10n ** BigInt(exponent) : 1n;
            const scaleDen = exponent < 0 ? 10n ** BigInt(-exponent) : 1n;
            return new Gracio(coeff.numerator * scaleNum, coeff.denominator * scaleDen);
        }
        // Handle standard decimal notation: e.g., 123.456
        const dotIndex = s.indexOf('.');
        if (dotIndex === -1) {
            return new Gracio(BigInt(s), 1n);
        }
        const wholePart = s.substring(0, dotIndex);
        const fractionPart = s.substring(dotIndex + 1);
        // Combine parts into a single integer (e.g., "123" + "456" -> 123456)
        // We must handle signs carefully: if wholePart is "-0", the sign still matters.
        const isNegative = s.startsWith('-');
        const cleanWhole = wholePart.replace('-', '');
        const combinedDigits = cleanWhole + fractionPart;
        const numerator = BigInt(combinedDigits || '0');
        const denominator = 10n ** BigInt(fractionPart.length);
        return new Gracio(isNegative ? -numerator : numerator, denominator);
    }
    abs(value) {
        return value < 0n ? -value : value;
    }
    /**
     * Creates a deep copy of the current number.
     * Use this when you need to perform operations without mutating the original object.
     */
    clone() {
        return new Gracio(this.numerator, this.denominator);
    }
    lastSimplifiedBits = 0;
    shouldSimplify() {
        const currentDigits = this.abs(this.numerator).toString().length;
        // Simplify if we've crossed the minimum threshold AND grown by at least 20 digits since last simplification
        return this.abs(this.numerator) > Gracio.MIN_SIMPLIFY_THRESHOLD &&
            currentDigits > (this.lastSimplifiedDigits + 20);
    }
    checkPrecisionLimit() {
        if (this.precisionLimit !== undefined) {
            const currentDigits = this.abs(this.denominator).toString().length;
            // Trigger approximation when growth exceeds 2x the limit to avoid jittery performance
            if (currentDigits > this.precisionLimit * 2) {
                this.pureApproximate(this.precisionLimit);
            }
        }
    }
    simplify() {
        const start = performance.now();
        let a = this.abs(this.numerator);
        let b = this.abs(this.denominator);
        if (a === 0n)
            return this;
        if (b === 0n)
            throw new Error("Denominator cannot be zero");
        // Trial Division: Prune small prime factors first to reduce BigInt magnitude
        for (const p of SMALL_PRIMES) {
            while (a % p === 0n && b % p === 0n) {
                a /= p;
                b /= p;
                this.numerator /= p;
                this.denominator /= p; // denominator is always positive
            }
        }
        // Final Binary GCD for any remaining larger factors
        const gcd = this.calculateGCD(a, b);
        if (gcd > 1n) {
            this.numerator /= gcd;
            this.denominator /= gcd;
        }
        const end = performance.now();
        if (end - start > 10) { // Log if simplification takes more than 10ms
            console.log(`[Simplify] Took ${(end - start).toFixed(2)}ms | Digits: ${a.toString().length}`);
        }
        this.lastSimplifiedDigits = this.abs(this.numerator).toString().length;
        this.lastSimplifiedBits = this.abs(this.numerator).toString(2).length;
        return this;
    }
    // --- Mutable Operations (Modifies current object and returns 'this' for chaining) ---
    add(other) {
        if (this.denominator === other.denominator) {
            this.numerator += other.numerator;
        }
        else {
            this.numerator = this.numerator * other.denominator + other.numerator * this.denominator;
            this.denominator = this.denominator * other.denominator;
        }
        if (this.shouldSimplify())
            this.simplify();
        this.checkPrecisionLimit();
        return this;
    }
    subtract(other) {
        if (this.denominator === other.denominator) {
            this.numerator -= other.numerator;
        }
        else {
            this.numerator = this.numerator * other.denominator - other.numerator * this.denominator;
            this.denominator = this.denominator * other.denominator;
        }
        if (this.shouldSimplify())
            this.simplify();
        this.checkPrecisionLimit();
        return this;
    }
    multiply(other) {
        const n1 = this.abs(this.numerator);
        const d1 = this.abs(this.denominator);
        const n2 = this.abs(other.numerator);
        const d2 = this.abs(other.denominator);
        // ASYMMETRIC FAST PATH: If at least one operand is small, we can afford raw multiplication.
        // We only cross-simplify when BOTH numbers are large to prevent explosive digit growth.
        const SMALL_THRESHOLD = 18446744073709551616n; // 2^64
        if (n1 < SMALL_THRESHOLD || d1 < SMALL_THRESHOLD || n2 < SMALL_THRESHOLD || d2 < SMALL_THRESHOLD) {
            this.numerator *= other.numerator;
            this.denominator *= other.denominator;
            if (this.shouldSimplify())
                this.simplify();
            this.checkPrecisionLimit();
            return this;
        }
        // CROSS-SIMPLIFICATION: Prevents explosive growth for very large BigInts.
        const g1 = this.calculateGCD(n1, d2);
        const g2 = this.calculateGCD(n2, d1);
        this.numerator = (this.numerator / g1) * (other.numerator / g2);
        this.denominator = (this.denominator / g2) * (other.denominator / g1);
        this.checkPrecisionLimit();
        return this;
    }
    divide(other) {
        if (other.numerator === 0n)
            throw new Error("Cannot divide by zero");
        const n1 = this.abs(this.numerator);
        const d1 = this.abs(this.denominator);
        const n2 = this.abs(other.numerator);
        const d2 = this.abs(other.denominator);
        // ASYMMETRIC FAST PATH: If at least one operand is small, avoid expensive GCDs.
        const SMALL_THRESHOLD = 18446744073709551616n; // 2^64
        if (n1 < SMALL_THRESHOLD || d1 < SMALL_THRESHOLD || n2 < SMALL_THRESHOLD || d2 < SMALL_THRESHOLD) {
            this.numerator *= other.denominator;
            this.denominator *= other.numerator;
            if (this.denominator < 0n) {
                this.numerator *= -1n;
                this.denominator *= -1n;
            }
            if (this.shouldSimplify())
                this.simplify();
            this.checkPrecisionLimit();
            return this;
        }
        // CROSS-SIMPLIFICATION: multiply by reciprocal with GCD pruning.
        const g1 = this.calculateGCD(n1, n2); // numerator1 vs reciprocalDenominator (other.numerator)
        const g2 = this.calculateGCD(d2, d1); // reciprocalNumerator (other.denominator) vs denominator1
        this.numerator = (this.numerator / g1) * (d2 / g2);
        this.denominator = (this.denominator / g2) * (n2 / g1);
        if (this.denominator < 0n) {
            this.numerator *= -1n;
            this.denominator *= -1n;
        }
        this.checkPrecisionLimit();
        return this;
    }
    calculateGCD(a, b) {
        let n = a;
        let d = b;
        if (n === 0n)
            return d;
        if (d === 0n)
            return n;
        let shift = 0n;
        while (((n | d) & 1n) === 0n) {
            n >>= 1n;
            d >>= 1n;
            shift++;
        }
        while ((n & 1n) === 0n)
            n >>= 1n;
        do {
            while ((d & 1n) === 0n)
                d >>= 1n;
            if (n > d) {
                let t = n;
                n = d;
                d = t;
            }
            d = d - n;
        } while (d !== 0n);
        return n << shift;
    }
    /**
     * Raises the number to the power of the given exponent.
     * Handles negative exponents by inverting the ratio.
     */
    pow(exponent) {
        if (exponent === 0n) {
            this.numerator = 1n;
            this.denominator = 1n;
            return this;
        }
        const absExp = exponent < 0n ? -exponent : exponent;
        let newNum = this.numerator ** absExp;
        let newDen = this.denominator ** absExp;
        if (exponent < 0n) {
            // Swap numerator and denominator for negative exponents
            [newNum, newDen] = [newDen, newNum];
        }
        this.numerator = newNum;
        this.denominator = newDen;
        return this;
    }
    toFloat() {
        this.simplify();
        const n = this.abs(this.numerator);
        const d = this.abs(this.denominator);
        if (n === 0n)
            return 0;
        // Avoid overflow by scaling down if numbers exceed the safe range for Number (approx 1024 bits)
        const nBits = n.toString(2).length;
        const dBits = d.toString(2).length;
        const maxBits = Math.max(nBits, dBits);
        if (maxBits > 1000) {
            const nShift = BigInt(nBits - 60);
            const dShift = BigInt(dBits - 60);
            // Scale both independently to fit in double-precision floats, then adjust by the difference in exponents
            const nSmall = Number(this.numerator >> nShift);
            const dSmall = Number(this.denominator >> dShift);
            return (nSmall / dSmall) * Math.pow(2, nBits - dBits);
        }
        return Number(this.numerator) / Number(this.denominator);
    }
    /**
     * Approximates the current ratio to a simpler fraction that maintains
     * precision up to the specified number of decimal places.
     * Uses Continued Fraction convergents to find the best rational approximation.
     */
    /**
     * Approximates the current ratio to a simpler fraction using continued fractions.
     * @param digits Maximum number of digits allowed in the denominator.
     */
    approximate(digits = 15) {
        return this.pureApproximate(digits);
    }
    /**
     * Pure rational approximation without converting to float.
     * Finds the best convergent where the denominator does not exceed maxDenominatorDigits.
     */
    pureApproximate(maxDenominatorDigits = 50) {
        // console.log(`[PrecisionLimit] Approximating to ${maxDenominatorDigits} digits...`);
        let a = this.abs(this.numerator);
        let b = this.abs(this.denominator);
        const originalSign = (this.numerator < 0n) !== (this.denominator < 0n);
        if (a === 0n)
            return this;
        let h_prev2 = 0n, h_prev1 = 1n;
        let k_prev2 = 1n, k_prev1 = 0n;
        while (b !== 0n) {
            const q = a / b;
            const r = a % b;
            const h = q * h_prev1 + h_prev2;
            const k = q * k_prev1 + k_prev2;
            // If the new denominator exceeds our digit limit, stop and use the last valid convergent.
            if (k.toString().length > maxDenominatorDigits) {
                break;
            }
            h_prev2 = h_prev1;
            h_prev1 = h;
            k_prev2 = k_prev1;
            k_prev1 = k;
            a = b;
            b = r;
        }
        this.numerator = originalSign ? -h_prev1 : h_prev1;
        this.denominator = k_prev1;
        // Convergents are already in simplest form, no need to simplify().
        return this;
    }
    toString() {
        this.simplify();
        return `${this.numerator}/${this.denominator}`;
    }
    /**
     * Calculates the n-th root of a number.
     * 1. Checks for perfect powers to return exact ratios.
     * 2. Uses continued fractions for high-precision rational approximations of irrationals.
     */
    static root(index, value) {
        console.log(`[Debug] Calculating root(${index}, ${value.toString()})`);
        if (index === 0n)
            throw new Error("Root index cannot be zero");
        if (index === 1n)
            return value.clone();
        const n = value.numerator;
        const d = value.denominator;
        // --- Step 1: Perfect Power Check ---
        const rootN = this.integerRoot(n, index);
        const rootD = this.integerRoot(d, index);
        if (rootN !== null && rootD !== null) {
            return new Gracio(rootN, rootD);
        }
        // --- Step 2: Continued Fraction Approximation ---
        const target = value.toFloat();
        const rootVal = Math.pow(target, 1 / Number(index));
        let p0 = 0n, q0 = 1n; // Convergent -1
        let p1 = 1n, q1 = 0n; // Not quite standard, let's use the iterative approach
        // Standard Continued Fraction algorithm for a float x
        let x = rootVal;
        let a = Math.floor(x);
        let h_prev2 = 0n, h_prev1 = 1n;
        let k_prev2 = 1n, k_prev1 = 0n;
        // We iterate for a few steps to get a very high precision convergent
        for (let i = 0; i < 15; i++) {
            const h = BigInt(Math.floor(x)) * h_prev1 + h_prev2;
            const k = BigInt(Math.floor(x)) * k_prev1 + k_prev2;
            h_prev2 = h_prev1;
            h_prev1 = h;
            k_prev2 = k_prev1;
            k_prev1 = k;
            if (x - Math.floor(x) === 0)
                break;
            x = 1 / (x - Math.floor(x));
        }
        return new Gracio(h_prev1, k_prev1);
    }
    static integerRoot(value, index) {
        if (value === 0n)
            return 0n;
        if (value < 0n && index % 2n === 0n)
            return null; // Even root of negative
        const isNegative = value < 0n;
        const absVal = value < 0n ? -value : value;
        // Fast-path: Prime Sieve. If a prime divides absVal, it must divide it at least 'index' times.
        for (const p of SMALL_PRIMES) {
            if (absVal % p === 0n) {
                let count = 0;
                let temp = absVal;
                while (temp % p === 0n) {
                    temp /= p;
                    count++;
                }
                if (count % Number(index) !== 0)
                    return null;
            }
        }
        let low = 1n;
        let high = absVal;
        // Optimization: for index >= 2, the root is at most sqrt(absVal).
        // For very large numbers, this significantly narrows the search space.
        if (index >= 2n) {
            // We can't easily get a starting 'high' without Math.sqrt, 
            // but we can use a smaller bound for common cases if needed.
        }
        while (low <= high) {
            const mid = (low + high) / 2n;
            if (mid === 0n) {
                low = 1n;
                continue;
            }
            const p = mid ** index;
            if (p === absVal)
                return isNegative ? -mid : mid;
            if (p < absVal)
                low = mid + 1n;
            else
                high = mid - 1n;
        }
        return null;
    }
}
