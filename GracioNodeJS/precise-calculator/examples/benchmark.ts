import { Gracio } from '../src/gracio.js';
import Big from 'big.js';
import Decimal from 'decimal.js';
import Fraction from 'fraction.js';

function benchmark(name: string, fn: () => void) {
    const start = performance.now();
    fn();
    const end = performance.now();
    console.log(`${name.padEnd(30)}: ${(end - start).toFixed(2)}ms`);
}

async function runBenchmarks() {
    console.log("--- 1. Additive Chain (1M iterations of +0.1) ---");
    const addIterations = 1_000_000;

    benchmark("Standard Float", () => {
        let sum = 0;
        for (let i = 0; i < addIterations; i++) sum += 0.1;
    });

    benchmark("Gracio (Mutable)", () => {
        const sum = Gracio.fromInt(0);
        const inc = Gracio.fromFloat(0.1);
        for (let i = 0; i < addIterations; i++) sum.add(inc);
    });

    benchmark("big.js", () => {
        let sum = new (Big as any)(0);
        const inc = new (Big as any)(0.1);
        for (let i = 0; i < addIterations; i++) sum = sum.plus(inc);
    });

    benchmark("decimal.js", () => {
        let sum = new (Decimal as any)(0);
        const inc = new (Decimal as any)(0.1);
        for (let i = 0; i < addIterations; i++) sum = sum.plus(inc);
    });

    benchmark("fraction.js", () => {
        let sum = new Fraction(0);
        const inc = new Fraction(0.1);
        for (let i = 0; i < addIterations; i++) sum = sum.add(inc);
    });

    console.log("\n--- 2. Multiplicative Chain (1K iterations of * 1.1) ---");
    const multIterations = 1_000;

    benchmark("Standard Float", () => {
        let val = 1.0;
        for (let i = 0; i < multIterations; i++) val *= 1.1;
    });

    benchmark("Gracio (No Limit)", () => {
        const val = Gracio.fromInt(1);
        const mul = Gracio.fromFloat(1.1);
        for (let i = 0; i < multIterations; i++) val.multiply(mul);
    });

    benchmark("Gracio (Limit 50)", () => {
        const val = Gracio.fromInt(1);
        val.precisionLimit = 50;
        const mul = Gracio.fromFloat(1.1);
        for (let i = 0; i < multIterations; i++) val.multiply(mul);
    });

    benchmark("big.js", () => {
        let val = new (Big as any)(1);
        const mul = new (Big as any)(1.1);
        for (let i = 0; i < multIterations; i++) val = val.times(mul);
    });

    benchmark("decimal.js", () => {
        let val = new (Decimal as any)(1);
        const mul = new (Decimal as any)(1.1);
        for (let i = 0; i < multIterations; i++) val = val.times(mul);
    });

    console.log("\n--- 3. Chaotic Loop (Logistic Map, 500 iterations) ---");
    const chaosIterations = 500;

    benchmark("Standard Float", () => {
        let x = 0.7;
        for (let i = 0; i < chaosIterations; i++) x = 4 * x * (1 - x);
    });

    benchmark("Gracio (Limit 100)", () => {
        const x = Gracio.fromFloat(0.7);
        x.precisionLimit = 100;
        const r = Gracio.fromInt(4);
        const one = Gracio.fromInt(1);
        for (let i = 0; i < chaosIterations; i++) {
            const temp = one.clone().subtract(x);
            x.multiply(r).multiply(temp);
        }
    });

    console.log("fraction.js                  : N/A (Digit Explosion - would hang process)");
}

runBenchmarks().catch(console.error);
