import { Gracio } from './gracio.js';
export class Constants {
    /**
     * Pi approximation using a high-precision convergent (355/113 is the most famous,
     * but we can go further for scientific use).
     * Convergent: 5419351 / 1724137 (~10 digits)
     */
    static get PI() {
        return new Gracio(5419351n, 1724137n);
    }
    /**
     * Euler's number e approximation.
     * Convergent: 2718281828459 / 1000000000000
     */
    static get E() {
        return new Gracio(2718281828459n, 1000000000000n);
    }
    /**
     * Golden Ratio phi = (1 + sqrt(5)) / 2.
     * We calculate this using the engine's root approximation for maximum consistency.
     */
    static get PHI() {
        const sqrt5 = Gracio.root(2n, new Gracio(5n, 1n));
        return new Gracio(1n, 1n).add(sqrt5).divide(new Gracio(2n, 1n));
    }
    /**
     * Gravitational Constant G approx 6.67430e-11
     */
    static get GRAVITY() {
        return Gracio.fromFloat('6.67430e-11');
    }
}
