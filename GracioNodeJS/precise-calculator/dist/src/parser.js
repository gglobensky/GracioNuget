import { Gracio } from './gracio.js';
export class Parser {
    position = 0;
    tokens;
    logger;
    constructor(tokens, logger) {
        this.tokens = tokens;
        this.logger = logger;
    }
    parse() {
        const result = this.parseExpression();
        const final = this.collapse(result);
        return new Gracio(final.n, final.d);
    }
    collapse(res) {
        if (res.type === 'ratio')
            return { n: res.n, d: res.d };
        // Evaluate root as a ratio using the core engine's approximation
        const cn = new Gracio(res.value.n, res.value.d);
        const rootRes = Gracio.root(res.index, cn);
        return { n: rootRes.numerator, d: rootRes.denominator };
    }
    parseExpression() {
        let result = this.parseTerm();
        while (this.position < this.tokens.length &&
            this.currentToken().type === 'operator' &&
            ['+', '-'].includes(this.currentToken().value)) {
            const op = this.currentToken().value;
            this.position++;
            const term = this.parseTerm();
            // Addition and subtraction force a collapse of any pending roots
            const left = this.collapse(result);
            const right = this.collapse(term);
            if (op === '+') {
                result = {
                    type: 'ratio',
                    n: left.n * right.d + right.n * left.d,
                    d: left.d * right.d
                };
            }
            else {
                result = {
                    type: 'ratio',
                    n: left.n * right.d - right.n * left.d,
                    d: left.d * right.d
                };
            }
        }
        return result;
    }
    parseTerm() {
        let result = this.parseExponent();
        while (this.position < this.tokens.length &&
            this.currentToken().type === 'operator' &&
            ['*', '/'].includes(this.currentToken().value)) {
            const op = this.currentToken().value;
            this.position++;
            const factor = this.parseExponent();
            if (op === '*') {
                // ROOT FOLDING: if both are roots of same index, combine values
                if (result.type === 'root' && factor.type === 'root' && result.index === factor.index) {
                    result = {
                        type: 'root',
                        index: result.index,
                        value: {
                            n: result.value.n * factor.value.n,
                            d: result.value.d * factor.value.d
                        }
                    };
                }
                else {
                    const left = this.collapse(result);
                    const right = this.collapse(factor);
                    result = {
                        type: 'ratio',
                        n: left.n * right.n,
                        d: left.d * right.d
                    };
                }
            }
            else {
                // ROOT FOLDING: if both are roots of same index, combine values (division)
                if (result.type === 'root' && factor.type === 'root' && result.index === factor.index) {
                    result = {
                        type: 'root',
                        index: result.index,
                        value: {
                            n: result.value.n * factor.value.d,
                            d: result.value.d * factor.value.n
                        }
                    };
                }
                else {
                    const left = this.collapse(result);
                    const right = this.collapse(factor);
                    result = {
                        type: 'ratio',
                        n: left.n * right.d,
                        d: left.d * right.n
                    };
                }
            }
        }
        return result;
    }
    parseExponent() {
        let result = this.parseFactor();
        while (this.position < this.tokens.length &&
            this.currentToken().type === 'operator' &&
            this.currentToken().value === '^') {
            this.position++;
            const exponentRes = this.parseFactor();
            // Exponents must be ratios that resolve to integers for current BigInt support
            const expComp = this.collapse(exponentRes);
            if (expComp.d !== 1n) {
                throw new Error("Exponents must be integers in the current implementation");
            }
            const exp = expComp.n;
            // If we are raising a root to a power, we can fold it: (root(i, v))^p = root(i, v^p)
            if (result.type === 'root') {
                const absExp = exp < 0n ? -exp : exp;
                let newN = result.value.n ** absExp;
                let newD = result.value.d ** absExp;
                if (exp < 0n)
                    [newN, newD] = [newD, newN];
                result = {
                    type: 'root',
                    index: result.index,
                    value: { n: newN, d: newD }
                };
            }
            else {
                const ratio = this.collapse(result);
                const absExp = exp < 0n ? -exp : exp;
                let n = ratio.n ** absExp;
                let d = ratio.d ** absExp;
                if (exp < 0n)
                    [n, d] = [d, n];
                result = { type: 'ratio', n, d };
            }
        }
        return result;
    }
    parseFactor() {
        const token = this.currentToken();
        if (token.type === 'number') {
            this.position++;
            const cn = Gracio.fromFloat(token.value);
            return { type: 'ratio', n: cn.numerator, d: cn.denominator };
        }
        if (token.type === 'function' && token.value === 'root') {
            this.position++; // skip 'root'
            if (this.currentToken().type !== '(')
                throw new Error("Expected '(' after root");
            this.position++; // skip '('
            const index = this.parseExpression();
            if (this.currentToken().type !== 'operator' || this.currentToken().value !== ',') {
                throw new Error("Expected ',' separating root index and value");
            }
            this.position++; // skip ','
            const value = this.parseExpression();
            if (this.currentToken().type !== ')')
                throw new Error("Expected ')' after root arguments");
            this.position++; // skip ')'
            return {
                type: 'root',
                index: this.collapse(index).n,
                value: this.collapse(value)
            };
        }
        if (token.type === '(') {
            this.position++; // skip '('
            const result = this.parseExpression();
            if (this.currentToken().type !== ')')
                throw new Error("Expected closing parenthesis");
            this.position++; // skip ')'
            return result;
        }
        throw new Error(`Unexpected token: ${JSON.stringify(token)}`);
    }
    currentToken() {
        if (this.position >= this.tokens.length) {
            throw new Error("Unexpected end of input");
        }
        return this.tokens[this.position];
    }
}
