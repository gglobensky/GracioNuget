// test/basic-arithmetic.ts
import { expect } from 'chai';
import { Gracio } from '../src/gracio.js';
import { calculateDrift } from '../src/audit-util.js';
describe('Basic Arithmetic Operations', () => {
    describe('Addition', () => {
        it('should add two large numbers accurately', async () => {
            const a = Gracio.fromInt(999999999999999);
            const b = Gracio.fromInt(1);
            const result = a.add(b);
            expect(result.toString()).to.equal("1000000000000000/1");
            expect(calculateDrift(result, 999999999999999 + 1)).to.be.below(1e-15);
        });
        it('should add decimal fractions accurately', async () => {
            const a = new Gracio(1n, 10n);
            const b = new Gracio(2n, 10n);
            const result = a.add(b);
            expect(result.toString()).to.equal("3/10");
            expect(calculateDrift(result, 0.1 + 0.2)).to.be.below(1e-15);
        });
    });
    describe('Multiplication', () => {
        it('should multiply large numbers accurately', async () => {
            const a = Gracio.fromInt(10000000000000000);
            const b = Gracio.fromInt(10000000000000000);
            const result = a.multiply(b);
            expect(result.toString()).to.equal("100000000000000000000000000000000/1");
            expect(calculateDrift(result, 10000000000000000 * 10000000000000000)).to.be.below(1e-15);
        });
    });
    describe('Division', () => {
        it('should divide accurately', async () => {
            const a = Gracio.fromInt(10);
            const b = Gracio.fromInt(3);
            const result = a.divide(b);
            expect(result.toString()).to.equal("10/3");
            expect(calculateDrift(result, 10 / 3)).to.be.below(1e-15);
        });
    });
});
