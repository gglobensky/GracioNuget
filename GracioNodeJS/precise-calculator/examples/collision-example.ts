import { Gracio } from '../src/gracio.js';

/**
 * Simple example showing how precise coordinates prevent "jitter" 
 * and clipping in collision detection for games.
 */

function getIntersection(x1: bigint, y1: bigint, x2: bigint, y2: bigint, 
                         x3: bigint, y3: bigint, x4: bigint, y4: bigint) {
    
    // Determinant formula for line intersection
    const den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (den === 0n) return null; // Parallel lines

    const pxNum = (x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4);
    const pyNum = (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4);

    return {
        x: new Gracio(pxNum, den),
        y: new Gracio(pyNum, den)
    };
}

console.log("--- Collision Detection Demo ---");

// Two lines that are nearly parallel and very far from origin
// Line 1: (1000000, 1000000) to (1000001, 1000001)
// Line 2: (1000000, 1000001) to (1000001, 1000000)
const intersection = getIntersection(
    1000000n, 1000000n, 1000001n, 1000001n,
    1000000n, 1000001n, 1000001n, 1000000n
);

if (intersection) {
    console.log(`Exact Intersection Point: X=${intersection.x.toString()}, Y=${intersection.y.toString()}`);
} else {
    console.log("Lines are parallel");
}
