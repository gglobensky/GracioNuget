using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace Gracio {
    public class Lexer {
        private int _position = 0;

        public List<Token> Tokenize(string expression) {
            var tokens = new List<Token>();
            _position = 0;

            while (_position < expression.Length) {
                char c = expression[_position];

                // Skip whitespace
                if (char.IsWhiteSpace(c)) {
                    _position++;
                    continue;
                }

                // Match numbers (including decimals and negatives)
                if (char.IsDigit(c) || c == '.' || 
                   (c == '-' && (tokens.Count == 0 || 
                                 tokens[tokens.Count - 1].Type == TokenType.Operator || 
                                 tokens[tokens.Count - 1].Type == TokenType.LeftParen))) {
                    
                    string numberStr = "";
                    if (c == '-') {
                        numberStr += c;
                        _position++;
                    }

                    while (_position < expression.Length && (char.IsDigit(expression[_position]) || expression[_position] == '.')) {
                        numberStr += expression[_position];
                        _position++;
                    }
                    tokens.Add(new Token(TokenType.Number, numberStr));
                }
                // Match operators and delimiters
                else if ("+-*/()^,".Contains(c)) {
                    TokenType type;
                    if (c == '(') type = TokenType.LeftParen;
                    else if (c == ')') type = TokenType.RightParen;
                    else if (c == ',') type = TokenType.Comma;
                    else type = TokenType.Operator;

                    tokens.Add(new Token(type, c.ToString()));
                    _position++;
                }
                // Match function names (e.g., "root")
                else if (char.IsLetter(c)) {
                    string name = "";
                    while (_position < expression.Length && char.IsLetterOrDigit(expression[_position])) {
                        name += expression[_position];
                        _position++;
                    }
                    tokens.Add(new Token(TokenType.Function, name));
                }
                else {
                    throw new Exception($"Unexpected character: {c}");
                }
            }

            return tokens;
        }
    }
}
