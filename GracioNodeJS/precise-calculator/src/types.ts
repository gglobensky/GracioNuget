export interface Token {
  type: 'number' | 'operator' | '(' | ')' | 'function';
  value: string;
}