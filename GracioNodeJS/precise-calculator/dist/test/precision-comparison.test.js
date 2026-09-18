import { expect } from 'chai';
import { Gracio } from '../src/gracio.js';
import { auditOperation } from '../src/audit-util.js';
describe('Precision & Performance Audit', () => {
    it('should audit simple additive loops (The Common Denominator Case)', async () => {
        const iterations = 10_000;
        const stepValue = 0.1;
        const exactResult = stepValue * iterations;
        // Ratio setup: we want to measure the .add() loop specifically
        let rSum = Gracio.fromInt(0);
        const stepRatio = new Gracio(1n, 10n);
        // Float setup
        let fSum = 0;
        const metrics = await auditOperation('Additive Loop (0.1 * N)', () => {
            rSum.add(stepRatio);
            return rSum; // Return the object so we can extract value at end
        }, () => {
            fSum += stepValue;
            return fSum;
        }, exactResult, (res) => res.toFloat(), iterations);
        expect(metrics.performanceTax).to.be.below(10); // Should be very efficient now
    });
    it('should audit multiplicative chains', async () => {
        const iterations = 1_000;
        const stepValue = 1.1;
        const exactResult = Math.pow(stepValue, iterations);
        let rVal = Gracio.fromInt(1);
        const stepRatio = new Gracio(11n, 10n);
        let fVal = 1.0;
        const metrics = await auditOperation('Multiplicative Chain (1.1 ^ N)', () => {
            rVal.multiply(stepRatio);
            return rVal;
        }, () => {
            fVal *= stepValue;
            return fVal;
        }, exactResult, (res) => res.toFloat(), iterations);
        // Multiplicative chains grow numbers fast, so the tax will be higher than additive
        expect(Number.isFinite(metrics.performanceTax)).to.be.true;
    });
});
