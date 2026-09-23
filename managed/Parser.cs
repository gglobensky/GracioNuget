using System;
using System.Collections.Generic;
using System.Linq;

namespace Gracio {
    public abstract class ParserResult { }

    public class RatioResult : ParserResult {
        public Ratio Value { get; }
        public RatioResult(Ratio value) => Value = value;
    }

    public class RootResult : ParserResult {
        public long Index { get; }
        public Ratio Value { get; }
        public RootResult(long index, Ratio value) {
            Index = index;
            Value = value;
        }
    }

    public class Parser {
        private int _position = 0;
        private readonly List<Token> _tokens;

        public Parser(List<Token> tokens) {
            _tokens = tokens;
        }

        public Ratio Parse() {
            var result = ParseExpression();
            return Collapse(result).Value;
        }

        private RatioResult Collapse(ParserResult res) {
            if (res is RatioResult rr) return rr;
            if (res is RootResult rootRes) {
                // Use native root implementation
                IntPtr handle = Ratio.NativeRoot((uint)rootRes.Index, rootRes.Value);
                if (handle == IntPtr.Zero) throw new Exception("Native root calculation failed.");
                return new RatioResult(new Ratio(handle));
            }
            throw new Exception("Unknown ParserResult type");
        }

        private ParserResult ParseExpression() {
            var result = ParseTerm();

            while (_position < _tokens.Count && 
                   _tokens[_position].Type == TokenType.Operator && 
                   (_tokens[_position].Value == "+" || _tokens[_position].Value == "-")) {
                
                string op = _tokens[_position].Value;
                _position++;
                var term = ParseTerm();

                var left = Collapse(result).Value;
                var right = Collapse(term).Value;

                if (op == "+") {
                    result = new RatioResult(left + right);
                } else {
                    result = new RatioResult(left - right);
                }
            }
            return result;
        }

        private ParserResult ParseTerm() {
            var result = ParseExponent();

            while (_position < _tokens.Count && 
                   _tokens[_position].Type == TokenType.Operator && 
                   (_tokens[_position].Value == "*" || _tokens[_position].Value == "/")) {
                
                string op = _tokens[_position].Value;
                _position++;
                var factor = ParseExponent();

                if (op == "*") {
                    // Root Folding: if both are roots of same index, combine values
                    if (result is RootResult r1 && factor is RootResult r2 && r1.Index == r2.Index) {
                        result = new RootResult(r1.Index, r1.Value * r2.Value);
                    } else {
                        var left = Collapse(result).Value;
                        var right = Collapse(factor).Value;
                        result = new RatioResult(left * right);
                    }
                } else {
                    // Root Folding: if both are roots of same index, combine values (division)
                    if (result is RootResult r1 && factor is RootResult r2 && r1.Index == r2.Index) {
                        result = new RootResult(r1.Index, r1.Value / r2.Value);
                    } else {
                        var left = Collapse(result).Value;
                        var right = Collapse(factor).Value;
                        result = new RatioResult(left / right);
                    }
                }
            }
            return result;
        }

        private ParserResult ParseExponent() {
            var result = ParseFactor();

            while (_position < _tokens.Count && 
                   _tokens[_position].Type == TokenType.Operator && 
                   _tokens[_position].Value == "^") {
                
                _position++;
                var exponentRes = ParseFactor();
                var expRatio = Collapse(exponentRes).Value;

                // Exponents must be integers (denominator = 1)
                if (expRatio.ToString() != "1" && !IsInteger(expRatio)) {
                    throw new Exception("Exponents must be integers in the current implementation");
                }
                long exp = ParseIntFromRatio(expRatio);

                if (result is RootResult rootRes) {
                    // Fold: (root(i, v))^p = root(i, v^p)
                    var newValue = rootRes.Value ^ exp;
                    result = new RootResult(rootRes.Index, newValue);
                } else {
                    var ratio = Collapse(result).Value;
                    result = new RatioResult(ratio ^ exp);
                }
            }
            return result;
        }

        private ParserResult ParseFactor() {
            Token token = CurrentToken();

            if (token.Type == TokenType.Number) {
                _position++;
                // Handle float-like strings by creating a ratio
                Ratio r = Ratio.ParseNumber(token.Value);
                return new RatioResult(r);
            }

            if (token.Type == TokenType.Function && token.Value == "root") {
                _position++; // skip 'root'
                if (CurrentToken().Type != TokenType.LeftParen) throw new Exception("Expected '(' after root");
                _position++; // skip '('

                var index = ParseExpression();
                if (CurrentToken().Type != TokenType.Comma) throw new Exception("Expected ',' separating root index and value");
                _position++; // skip ','

                var value = ParseExpression();

                if (CurrentToken().Type != TokenType.RightParen) throw new Exception("Expected ')' after root arguments");
                _position++; // skip ')'

                return new RootResult(ParseIntFromRatio(Collapse(index).Value), Collapse(value).Value);
            }

            if (token.Type == TokenType.LeftParen) {
                _position++; // skip '('
                var result = ParseExpression();
                if (CurrentToken().Type != TokenType.RightParen) throw new Exception("Expected closing parenthesis");
                _position++; // skip ')'
                return result;
            }

            throw new Exception($"Unexpected token: {token}");
        }

        private Token CurrentToken() {
            if (_position >= _tokens.Count) throw new Exception("Unexpected end of input");
            return _tokens[_position];
        }

        private bool IsInteger(Ratio r) {
            // Simple check: if toString doesn't contain '/', it's an integer. 
            // A better way would be to expose the denominator from native core.
            return !r.ToString().Contains("/");
        }

        private long ParseIntFromRatio(Ratio r) {
            string s = r.ToString();
            if (s.Contains("/")) throw new Exception("Value is not an integer");
            return long.Parse(s);
        }
    }
}
