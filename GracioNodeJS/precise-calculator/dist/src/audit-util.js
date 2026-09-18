import { performance } from 'perf_hooks';
/**
 * Compares a PreciseNumber result with a float result.
 * Returns the absolute drift between them.
 */
export function calculateDrift(ratio, floatVal) {
    const ratioFloat = typeof ratio.toFloat === 'function' ? ratio.toFloat() : ratio;
    return Math.abs(ratioFloat - floatVal);
}
export async function auditOperation(name, ratioOp, floatOp, exactValue, getFinalRatioValue, iterations = 10_000) {
    // Benchmark Floats
    const startFloat = performance.now();
    let fRes = 0;
    for (let i = 0; i < iterations; i++) {
        fRes = floatOp();
    }
    const endFloat = performance.now();
    // Benchmark Ratios
    const startRatio = performance.now();
    let rRes;
    for (let i = 0; i < iterations; i++) {
        rRes = ratioOp();
    }
    const endRatio = performance.now();
    const floatTime = endFloat - startFloat;
    const ratioTime = endRatio - startRatio;
    // The "Truth" is the Ratio result (which we assume is exact)
    const finalRatioValue = getFinalRatioValue(rRes);
    const drift = Math.abs(fRes - finalRatioValue);
    console.log(`\n[Audit: ${name}]`);
    console.log(`  Exact Result: ${finalRatioValue}`);
    console.log(`  Float result: ${fRes}`);
    console.log(`  Precision: Float drifted by ${drift.toExponential(4)}`);
    console.log(`  Performance: Ratio is ${(ratioTime / floatTime).toFixed(2)}x slower than Float`);
    return {
        precisionDrift: drift,
        performanceTax: ratioTime / floatTime
    };
}
