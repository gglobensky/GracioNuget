import { Gracio } from '../src/gracio.js';
console.log("--- Precision Drift Demo ---");
// Floating point drift example
let floatSum = 0;
for (let i = 0; i < 10; i++) {
    floatSum += 0.1;
}
console.log(`Floating Point Sum (0.1 * 10): ${floatSum}`);
// Result: 0.9999999999999999
// PreciseNumber example
let preciseSum = Gracio.fromInt(0);
const step = new Gracio(1n, 10n); // 1/10
for (let i = 0; i < 10; i++) {
    preciseSum.add(step);
}
console.log(`PreciseNumber Sum (1/10 * 10): ${preciseSum.toString()}`);
// Result: 1/1
