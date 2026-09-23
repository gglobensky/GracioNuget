using System;

namespace Gracio {
    public enum TokenType {
        Number,
        Operator,
        Function,
        LeftParen,
        RightParen,
        Comma
    }

    public struct Token {
        public TokenType Type { get; }
        public string Value { get; }

        public Token(TokenType type, string value) {
            Type = type;
            Value = value;
        }

        public override string ToString() => $"Token({Type}, \"{Value}\")";
    }
}
