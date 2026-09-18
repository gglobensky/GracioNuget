import { Token } from './types.js';

export class Lexer {
  private position = 0;

  tokenize(expression: string): Token[] {
    const tokens: Token[] = [];
    
    while (this.position < expression.length) {
      const char = expression[this.position];
      
      // Skip whitespace
      if (char.match(/\s/)) {
        this.position++;
        continue;
      }
      
      // Match numbers (including decimals and negatives)
      if (char.match(/[0-9.]/) || (char === '-' && 
          (tokens.length === 0 || 
           tokens[tokens.length-1].type === 'operator' ||
           tokens[tokens.length-1].type === '('))) {
        let numberStr = '';
        if (char === '-') {
          numberStr += char;
          this.position++;
        }
        
        while (this.position < expression.length && 
               expression[this.position].match(/[0-9.]/)) {
          numberStr += expression[this.position];
          this.position++;
        }
        tokens.push({ type: 'number', value: numberStr });
      }
      
      // Match operators
      else if (['+', '-', '*', '/', '(', ')', '^', ','].includes(char)) {
        const tokenType = ['(', ')'].includes(char) ? char : 'operator';
        tokens.push({ 
          type: tokenType as Token['type'],
          value: char
        });
        this.position++;
      }
      // Match function names (e.g., "root")
      else if (char.match(/[a-zA-Z]/)) {
        let name = '';
        while (this.position < expression.length && 
               expression[this.position].match(/[a-zA-Z0-9]/)) {
          name += expression[this.position];
          this.position++;
        }
        tokens.push({ type: 'function', value: name });
      }
    }

    return tokens;
  }
}