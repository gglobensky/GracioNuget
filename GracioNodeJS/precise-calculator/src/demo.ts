import { Parser } from './parser.js';
import { Lexer } from './lexer.js';
import { Gracio } from './gracio.js';
import { Constants } from './constants.js';

async function main() {
  console.log("=== Basic Functionality Tests ===");
  const expressions = [
    "1 + 0.1",               // Testing Float Bridge: should be 11/10
    "0.3333333333 * 3",      // Testing Precision: should be 0.9999999999
    "root(2, 2)",            // Standard root
    "root(2, 0.25)",         // Root of a float (0.5)
    "1 / 3 + 1 / 6",         // Classic ratio addition: should be 1/2
  ];
  
  for (const expr of expressions) {
    console.log(`Evaluating: ${expr}`);
    const lexer = new Lexer();
    const tokens = lexer.tokenize(expr);
    const parser = new Parser(tokens);
    try {
      const result = parser.parse();
      console.log(`  Ratio: ${result.toString()} | Float: ${result.toFloat()}`);
    } catch (error) {
      console.error(`  Error: ${(error as Error).message}`);
    }
  }

  console.log("\n=== Precision Audit: Gracio vs IEEE 754 Floats ===");

  const audit = (label: string, floatResult: number, preciseResult: Gracio) => {
    const pFloat = preciseResult.toFloat();
    const diff = Math.abs(floatResult - pFloat);
    console.log(`${label.padEnd(30)} | Float: ${floatResult} | Precise: ${pFloat} | Drift: ${diff}`);
  };

  // Case 1: Representation Error (The Classic)
  audit(
    "Classic Drift (0.1 + 0.2)",
    0.1 + 0.2,
    new Gracio(1n, 10n).add(new Gracio(2n, 10n))
  );

  // Case 2: The Integer Wall (Beyond MAX_SAFE_INTEGER)
  const bigNum = 2n ** 60n;
  audit(
    "The Integer Wall (2^60 + 1 - 2^60)",
    (Math.pow(2, 60) + 1) - Math.pow(2, 60),
    new Gracio(bigNum + 1n, 1n).subtract(new Gracio(bigNum, 1n))
  );

  // Case 3: Cumulative Absorption (Adding small to large)
  let floatSum = 1.0;
  const incrementFloat = 0.0000001;
  for (let i = 0; i < 10_000_000; i++) {
    floatSum += incrementFloat;
  }

  const preciseSum = new Gracio(1n, 1n);
  const incrementPrecise = new Gracio(1n, 10_000_000n);
  for (let i = 0; i < 10_000_000; i++) {
    preciseSum.add(incrementPrecise);
  }

  audit("Cumulative Absorption", floatSum, preciseSum);

  console.log("\n=== Testing Constants ===");
  console.log(`PI: ${Constants.PI.toString()} (~${Constants.PI.toFloat()})`);
  console.log(`E : ${Constants.E.toString()} (~${Constants.E.toFloat()})`);
  console.log(`PHI: ${Constants.PHI.toString()} (~${Constants.PHI.toFloat()})`);
}

void main();


