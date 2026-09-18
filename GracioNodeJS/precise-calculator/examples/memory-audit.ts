import { Gracio } from '../src/gracio.js';
import Big from 'big.js';
import Decimal from 'decimal.js';
import Fraction from 'fraction.js';

async function auditMemory() {
    console.log("--- Memory Usage Audit (1M Additions) ---");
    console.log("Note: Run with 'node --expose-gc' for accurate results.\n");

    const iterations = 1_000_000;
    const stepValue = 0.1;

    const libs = [
        {
            name: "Gracio (Mutable)",
            fn: () => {
                const sum = Gracio.fromInt(0);
                const inc = Gracio.fromFloat(stepValue);
                for (let i = 0; i < iterations; i++) {
                    sum.add(inc);
                }
                return sum;
            }
        },
        {
            name: "big.js",
            fn: () => {
                let sum = new (Big as any)(0);
                const inc = new (Big as any)(stepValue);
                for (let i = 0; i < iterations; i++) {
                    sum = sum.plus(inc);
                }
                return sum;
            }
        },
        {
            name: "decimal.js",
            fn: () => {
                let sum = new (Decimal as any)(0);
                const inc = new (Decimal as any)(stepValue);
                for (let i = 0; i < iterations; i++) {
                    sum = sum.plus(inc);
                }
                return sum;
            }
        },
        {
            name: "fraction.js",
            fn: () => {
                let sum = new Fraction(0);
                const inc = new Fraction(stepValue);
                for (let i = 0; i < iterations; i++) {
                    sum = sum.add(inc);
                }
                return sum;
            }
        }
    ];

    for (const lib of libs) {
        // Force GC if available
        if (global.gc) {
            global.gc();
        } else {
            console.warn(`Warning: global.gc() not found. Run with --expose-gc.`);
        }

        const startMem = process.memoryUsage().heapUsed;
        const result = lib.fn();
        const endMem = process.memoryUsage().heapUsed;
        
        const deltaMB = (endMem - startMem) / 1024 / 1024;
        console.log(`${lib.name.padEnd(20)}: Delta ${deltaMB.toFixed(2)} MB`);
    }
}

auditMemory().catch(console.error);
