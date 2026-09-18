import { expect } from 'chai';
import { Gracio } from '../src/gracio.js';
import { calculateDrift } from '../src/audit-util.js';

describe('Advanced Arithmetic Operations', () => {
  describe('Powers (.pow)', () => {
    it('should calculate positive integer powers correctly', () => {
      const a = Gracio.fromInt(2);
      a.pow(10n);
      expect(a.toString()).to.equal("1024/1");
      expect(calculateDrift(a, Math.pow(2, 10))).to.be.below(1e-15);
    });

    it('should handle zero exponent (x^0 = 1)', () => {
      const a = new Gracio(3n, 7n);
      a.pow(0n);
      expect(a.toString()).to.equal("1/1");
    });

    it('should handle negative exponents by inverting the ratio', () => {
      const a = new Gracio(1n, 2n); // 1/2
      a.pow(-2n); // (1/2)^-2 = 2^2 = 4
      expect(a.toString()).to.equal("4/1");
      expect(calculateDrift(a, Math.pow(0.5, -2))).to.be.below(1e-15);
    });

    it('should handle fractional ratios raised to powers', () => {
      const a = new Gracio(3n, 4n);
      a.pow(2n); // (3/4)^2 = 9/16
      expect(a.toString()).to.equal("9/16");
      expect(calculateDrift(a, Math.pow(0.75, 2))).to.be.below(1e-15);
    });
  });

  describe('Roots (.root)', () => {
    it('should calculate square roots of perfect squares', () => {
      const a = Gracio.fromInt(4);
      const res = Gracio.root(2n, a);
      expect(res.toString()).to.contain("2"); 
    });

    it('should calculate cube roots of perfect cubes', () => {
      const a = Gracio.fromInt(27);
      const res = Gracio.root(3n, a);
      expect(res.toString()).to.contain("3");
    });

    it('should handle approximate roots correctly', () => {
      const a = Gracio.fromInt(2);
      const res = Gracio.root(2n, a);
      // sqrt(2) approx 1.414213...
      expect(res.toFloat()).to.be.closeTo(1.414213, 0.0001);
    });
  });

  describe('Common Denominator Stress Test', () => {
    it('should maintain high performance during long additive chains with same denominator', () => {
      const iterations = 100_000;
      const start = performance.now();
      
      let rSum = Gracio.fromInt(0);
      const step = new Gracio(1n, 3n);
      for (let i = 0; i < iterations; i++) {
        rSum.add(step);
      }
      const end = performance.now();
      
      console.log(`\n  Additive chain (${iterations} ops) took: ${(end - start).toFixed(2)}ms`);
      expect(rSum.toString()).to.equal(`${iterations}/3`);
      expect(end - start).to.be.below(100); // Should be extremely fast now
    });
  });
});
