import { Gracio } from '../src/gracio.js';
import Big from 'big.js';
import Decimal from 'decimal.js';
import Fraction from 'fraction.js';

function validate(testName: string, preciseResult: number, others: { [key: string]: number }) {
    console.log(`\n--- ${testName} ---`);
    console.log(`Gracio: ${preciseResult.toFixed(20)}`);
    
    for (const [lib, val] of Object.entries(others)) {
        const diff = Math.abs(preciseResult - val);
        const status = diff === 0 ? "✅ MATCH" : `❌ DRIFT (${diff.toExponential(4)})`;
        console.log(`${lib.padEnd(15)}: ${val.toFixed(20)} ${status}`);
    }
}

async function runValidation() {
    console.log("🚀 Starting Cross-Library Precision Validation...");

    // Test 1: The Reciprocal Trap (1/3 * 3)
    const t1Precise = new Gracio(1n, 3n).multiply(Gracio.fromInt(3)).toFloat();
    const t1Big = new (Big as any)(1).div(3).times(3).toNumber();
    const t1Dec = new (Decimal as any)(1).div(3).times(3).toNumber();
    const t1Frac = Number(new Fraction(1).div(3).mul(3));

    validate("Reciprocal Trap (1/3 * 3)", t1Precise, {
        "big.js": t1Big,
        "decimal.js": t1Dec,
        "fraction.js": t1Frac
    });

    // Test 2: Compound Interest Chain (1.05 ^ 100)
    const iterations = 100;
    const rate = 1.05;
    
    const t2PreciseObj = Gracio.fromInt(1);
    const t2Mul = Gracio.fromFloat(rate);
    for (let i = 0; i < iterations; i++) t2PreciseObj.multiply(t2Mul);
    const t2Precise = t2PreciseObj.toFloat();

    let t2Big = new (Big as any)(1);
    const t2BigMul = new (Big as any)(rate);
    for (let i = 0; i < iterations; i++) t2Big = t2Big.times(t2BigMul);

    let t2Dec = new (Decimal as any)(1);
    const t2DecMul = new (Decimal as any)(rate);
    for (let i = 0; i < iterations; i++) t2Dec = t2Dec.times(t2DecMul);

    let t2Frac = new Fraction(1);
    const t2FracMul = new Fraction(rate);
    for (let i = 0; i < iterations; i++) t2Frac = t2Frac.mul(t2FracMul);

    validate(`Compound Growth (${rate}^${iterations})`, t2Precise, {
        "big.js": t2Big.toNumber(),
        "decimal.js": t2Dec.toNumber(),
        "fraction.js": Number(t2Frac)
    });

    // Test 3: The "Salami Slicing" (Small increments on large base)
    const base = 1000000;
    const inc = 0.0000001;
    const steps = 10000;

    const t3PreciseObj = Gracio.fromInt(base);
    const t3Inc = Gracio.fromFloat(inc);
    for (let i = 0; i < steps; i++) t3PreciseObj.add(t3Inc);
    const t3Precise = t3PreciseObj.toFloat();

    let t3Big = new (Big as any)(base);
    const t3BigInc = new (Big as any)(inc);
    for (let i = 0; i < steps; i++) t3Big = t3Big.plus(t3BigInc);

    let t3Dec = new (Decimal as any)(base);
    const t3DecInc = new (Decimal as any)(inc);
    for (let i = 0; i < steps; i++) t3Dec = t3Dec.plus(t3DecInc);

    let t3Frac = new Fraction(base);
    const t3FracInc = new Fraction(inc);
    for (let i = 0; i < steps; i++) t3Frac = t3Frac.add(t3FracInc);

    validate(`Salami Slicing (${steps} x ${inc})`, t3Precise, {
        "big.js": t3Big.toNumber(),
        "decimal.js": t3Dec.toNumber(),
        "fraction.js": Number(t3Frac)
    });

    console.log("\n✅ Validation Complete.");
}

runValidation().catch(console.error);
