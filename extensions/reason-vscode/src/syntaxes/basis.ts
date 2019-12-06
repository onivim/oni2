// tslint:disable object-literal-sort-keys

import * as schema from "./schema";

export function ref(f: (...args: any[]) => schema.Rule): string {
  return `#${f.name}`;
}

export function include(f: (...args: any[]) => schema.Rule): { include: string } {
  return { include: ref(f) };
}

export const alt = (...rest: string[]) => rest.join("|");
export const capture = (arg: string) => `(${arg})`;
export const complement = (...rest: string[]) => `[^${rest.join("")}]`;
export const group = (arg: string) => `(?:${arg})`;
export const lookBehind = (arg: string) => `(?<=${arg})`;
export const negativeLookBehind = (arg: string) => `(?<!${arg})`;
export function lastWords(...rest: string[]): string {
  const result: string[] = [];
  for (const token of rest) result.push(`[^[:word:]]${token}`, `^${token}`);
  return group(seq(lookBehind(group(alt(...result))), negativeLookAhead(set(Class.word))));
}
export const many = (arg: string) => `${arg}*`;
export const manyOne = (arg: string) => `${arg}+`;
export const opt = (arg: string) => `${arg}?`;
export const words = (arg: string) => `\\b${arg}\\b`;
export const lookAhead = (arg: string) => `(?=${arg})`;
export const negativeLookAhead = (arg: string) => `(?!${arg})`;
export const seq = (...rest: string[]) => rest.join("");
export const set = (...rest: string[]) => `[${rest.join("")}]`;

export const Class = {
  alnum: "[:alnum:]",
  alpha: "[:alpha:]",
  ascii: "[:ascii:]",
  blank: "[:blank:]",
  cntrl: "[:cntrl:]",
  digit: "[:digit:]",
  graph: "[:graph:]",
  lower: "[:lower:]",
  print: "[:print:]",
  punct: "[:punct:]",
  space: "[:space:]",
  upper: "[:upper:]",
  word: "[:word:]",
  xdigit: "[:xdigit:]",
};

export const Token = {
  AND: "and",
  APOSTROPHE: "'",
  AS: "as",
  ASR: "asr",
  ASSERT: "assert",
  ASTERISK: "\\*",
  BEGIN: "begin",
  CLASS: "class",
  COLON: ":",
  COMMA: ",",
  COMMERCIAL_AT: "@",
  CONSTRAINT: "constraint",
  DO: "do",
  DONE: "done",
  DOWNTO: "downto",
  ELSE: "else",
  END: "end",
  EQUALS_SIGN: "=",
  EXCEPTION: "exception",
  EXTERNAL: "external",
  FALSE: "false",
  FOR: "for",
  FULL_STOP: "\\.",
  FUN: "fun",
  FUNCTION: "function",
  FUNCTOR: "functor",
  GREATER_THAN_SIGN: ">",
  HYPHEN_MINUS: "-",
  IF: "if",
  IN: "in",
  INCLUDE: "include",
  INHERIT: "inherit",
  INITIALIZER: "initializer",
  LAND: "land",
  LAZY: "lazy",
  LEFT_CURLY_BRACKET: "\\{",
  LEFT_PARENTHESIS: "\\(",
  LEFT_SQUARE_BRACKET: "\\[",
  LESS_THAN_SIGN: "<",
  LET: "let",
  LOR: "lor",
  LOW_LINE: "_",
  LSL: "lsl",
  LSR: "lsr",
  LXOR: "lxor",
  MATCH: "match",
  METHOD: "method",
  MOD: "mod",
  MODULE: "module",
  MUTABLE: "mutable",
  NEW: "new",
  NONREC: "nonrec",
  NUMBER_SIGN: "#",
  OBJECT: "object",
  OF: "of",
  OPEN: "open",
  OR: "or",
  PERCENT_SIGN: "%",
  PLUS_SIGN: "\\+",
  PRIVATE: "private",
  QUESTION_MARK: "\\?",
  QUOTATION_MARK: '"',
  REC: "rec",
  REVERSE_SOLIDUS: "\\\\",
  RIGHT_CURLY_BRACKET: "\\}",
  RIGHT_PARENTHESIS: "\\)",
  RIGHT_SQUARE_BRACKET: "\\]",
  SEMICOLON: ";",
  SIG: "sig",
  SOLIDUS: "/",
  STRUCT: "struct",
  THEN: "then",
  TILDE: "~",
  TO: "to",
  TRUE: "true",
  TRY: "try",
  TYPE: "type",
  VAL: "val",
  VERTICAL_LINE: "\\|",
  VIRTUAL: "virtual",
  WHEN: "when",
  WHILE: "while",
  WITH: "with",
};

export class Scope {
  public static ITEM_AND(): string {
    return `${this.STYLE_OPERATOR()} ${this.STYLE_UNDERLINE()}`;
  }

  public static ITEM_CLASS(): string {
    return `entity.name.class constant.numeric ${this.STYLE_UNDERLINE()}`;
  }

  public static ITEM_EXTERNAL(): string {
    return `entity.name.class constant.numeric ${this.STYLE_UNDERLINE()}`;
  }

  public static ITEM_INCLUDE(): string {
    return this.STYLE_OPERATOR();
  }

  public static ITEM_LET(): string {
    return `${this.STYLE_CONTROL()} ${this.STYLE_UNDERLINE()}`;
  }

  public static ITEM_METHOD(): string {
    return `${this.STYLE_BINDER()} ${this.STYLE_UNDERLINE()}`;
  }

  public static ITEM_MODULE(): string {
    return `markup.inserted constant.language support.constant.property-value entity.name.filename ${this.STYLE_UNDERLINE()}`;
  }

  public static ITEM_OPEN(): string {
    return this.STYLE_OPERATOR();
  }

  public static ITEM_TYPE(): string {
    return `${this.STYLE_KEYWORD()} ${this.STYLE_UNDERLINE()}`;
  }

  public static ITEM_VAL(): string {
    return `support.type ${this.STYLE_UNDERLINE()}`;
  }

  public static KEYWORD_AS(): string {
    return this.STYLE_OPERATOR();
  }

  public static KEYWORD_REC(): string {
    return this.STYLE_OPERATOR();
  }

  public static KEYWORD_WHEN(): string {
    return this.STYLE_OPERATOR();
  }

  public static LITERAL_OBJECT(): string {
    return `${this.STYLE_DELIMITER()} ${this.STYLE_ITALICS()}`;
  }

  public static LITERAL_SIGNATURE(): string {
    return `${this.STYLE_DELIMITER()} ${this.STYLE_ITALICS()}`;
  }

  public static LITERAL_STRUCTURE(): string {
    return `${this.STYLE_DELIMITER()} ${this.STYLE_ITALICS()}`;
  }

  public static META_COMMENT(): string {
    return "comment constant.regexp meta.separator.markdown";
  }

  public static MODULE_FUNCTOR(): string {
    return "variable.other.class.js variable.interpolation keyword.operator keyword.control message.error";
  }

  public static MODULE_SIG(): string {
    return this.STYLE_DELIMITER();
  }

  public static MODULE_STRUCT(): string {
    return this.STYLE_DELIMITER();
  }

  public static NAME_FIELD(): string {
    return `markup.inserted constant.language support.constant.property-value entity.name.filename`;
  }

  public static NAME_FUNCTION(): string {
    return `entity.name.function ${this.STYLE_BOLD()} ${this.STYLE_ITALICS()}`;
  }

  public static NAME_METHOD(): string {
    return "entity.name.function";
  }

  public static NAME_MODULE(): string {
    return "entity.name.class constant.numeric";
  }

  public static PUNCTUATION_QUOTE(): string {
    return `markup.punctuation.quote.beginning ${this.STYLE_BOLD()} ${this.STYLE_ITALICS()}`;
  }

  public static SIGNATURE_WITH(): string {
    return `${this.STYLE_OPERATOR()} ${this.STYLE_UNDERLINE()}`;
  }

  public static NAME_TYPE(): string {
    return `entity.name.function ${this.STYLE_BOLD()} ${this.STYLE_ITALICS()}`;
  }

  public static OPERATOR_TYPE(): string {
    return `${this.STYLE_OPERATOR()} ${this.STYLE_BOLD()}`;
  }

  public static PUNCTUATION_APOSTROPHE(): string {
    return `${this.VARIABLE_PATTERN()} ${this.STYLE_BOLD()} ${this.STYLE_ITALICS()}`;
  }

  public static PUNCTUATION_COLON(): string {
    return `${this.STYLE_OPERATOR()} ${this.STYLE_BOLD()}`;
  }

  public static PUNCTUATION_COMMA(): string {
    return `string.regexp ${this.STYLE_BOLD()}`;
  }

  public static PUNCTUATION_DOT(): string {
    return `${this.STYLE_KEYWORD()} ${this.STYLE_BOLD()}`;
  }

  public static PUNCTUATION_EQUALS(): string {
    return `support.type ${this.STYLE_BOLD()}`;
  }

  public static PUNCTUATION_PERCENT_SIGN(): string {
    return `${this.STYLE_OPERATOR()} ${this.STYLE_BOLD()}`;
  }

  public static STYLE_BINDER(): string {
    return "storage.type";
  }

  public static STYLE_BOLD(): string {
    return "strong";
  }

  public static STYLE_CONTROL(): string {
    return "keyword.control";
  }

  public static STYLE_DELIMITER(): string {
    return "punctuation.definition.tag";
  }

  public static STYLE_ITALICS(): string {
    return "emphasis";
  }

  public static STYLE_KEYWORD(): string {
    return "keyword";
  }

  public static STYLE_OPERATOR(): string {
    return "variable.other.class.js message.error variable.interpolation string.regexp";
  }

  public static STYLE_PUNCTUATION(): string {
    return "string.regexp";
  }

  public static STYLE_UNDERLINE(): string {
    return "markup.underline";
  }

  public static TERM_BUILTIN(): string {
    return this.STYLE_OPERATOR();
  }

  public static TERM_CHARACTER(): string {
    return "markup.punctuation.quote.beginning";
  }

  public static TERM_CONSTRUCTOR(): string {
    return `constant.language constant.numeric entity.other.attribute-name.id.css ${this.STYLE_BOLD()}`;
  }

  public static TERM_FUN(): string {
    return this.STYLE_BINDER();
  }

  public static TERM_FUNCTION(): string {
    return this.STYLE_BINDER();
  }

  public static TERM_IF(): string {
    return this.STYLE_CONTROL();
  }

  public static TERM_IN(): string {
    return `${this.STYLE_BINDER()} ${this.STYLE_UNDERLINE()}`;
  }

  public static TERM_LET(): string {
    return `${this.STYLE_BINDER()} ${this.STYLE_UNDERLINE()}`;
  }

  public static TERM_MODULE(): string {
    return "markup.inserted constant.language support.constant.property-value entity.name.filename";
  }

  public static TERM_NUMBER(): string {
    return "constant.numeric";
  }

  public static TERM_STRING(): string {
    return "string beginning.punctuation.definition.quote.markdown";
  }

  public static TYPE_CONSTRUCTOR(): string {
    return `entity.name.function ${this.STYLE_BOLD()}`;
  }

  public static VARIABLE_PATTERN(): string {
    return `string.other.link variable.language variable.parameter ${this.STYLE_ITALICS()}`;
  }

  public static VARIABLE_TYPE(): string {
    return `${this.STYLE_CONTROL()} ${this.STYLE_ITALICS()}`;
  }

  public static VERTICAL_LINE(): string {
    return `support.type ${this.STYLE_BOLD()}`;
  }
}

export interface IGrammar {
  bindClassTerm(): schema.Rule;
  bindClassType(): schema.Rule;
  bindConstructor(): schema.Rule;
  bindSignature(): schema.Rule;
  bindStructure(): schema.Rule;
  bindTerm(): schema.Rule;
  bindTermArgs(): schema.Rule;
  bindType(): schema.Rule;
  boundary(): string;
  comment(): schema.Rule;
  commentBlock(): schema.Rule;
  commentDoc(): schema.Rule;
  decl(): schema.Rule;
  declClass(): schema.Rule;
  declEnd(): string;
  declEndItem(): string;
  declEndItemWith(...rest: string[]): string;
  declEndSans(...rest: string[]): string;
  declEndTokens(): string[];
  declException(): schema.Rule;
  declInclude(): schema.Rule;
  declModule(): schema.Rule;
  declOpen(): schema.Rule;
  declStartTokens(): string[];
  declTerm(): schema.Rule;
  declType(): schema.Rule;
  escapes(...rest: string[]): schema.Rule;
  expEnd(): string;
  functor(ruleBody: () => schema.Rule): schema.Rule[];
  ident(): string;
  identLower(): string;
  identUpper(): string;
  lastOps(...rest: string[]): string;
  literal(): schema.Rule;
  literalArray(): schema.Rule;
  literalBoolean(): schema.Rule;
  literalCharacter(): schema.Rule;
  literalCharacterEscape(): schema.Rule;
  literalList(): schema.Rule;
  literalNumber(): schema.Rule;
  literalObjectTerm(): schema.Rule;
  literalClassType(): schema.Rule;
  literalRecord(): schema.Rule;
  literalString(): schema.Rule;
  literalStringEscape(): schema.Rule;
  literalUnit(): schema.Rule;
  operator(): string;
  operatorTokens(): string[];
  ops(arg: string): string;
  parens(ruleLHS: string, ruleRHS: string): schema.Rule;
  pathModuleExtended(): schema.Rule;
  pathModulePrefixExtended(): schema.Rule;
  pathModulePrefixExtendedParens(): schema.Rule;
  pathModulePrefixSimple(): schema.Rule;
  pathModuleSimple(): schema.Rule;
  pathRecord(): schema.Rule;
  pathType(): schema.Rule;
  pattern(): schema.Rule;
  patternArray(): schema.Rule;
  patternLazy(): schema.Rule;
  patternList(): schema.Rule;
  patternMisc(): schema.Rule;
  patternModule(): schema.Rule;
  patternParens(): schema.Rule;
  patternRecord(): schema.Rule;
  patternType(): schema.Rule;
  pragma(): schema.Rule;
  signature(): schema.Rule;
  signatureConstraints(): schema.Rule;
  signatureFunctor(): schema.Rule;
  signatureLiteral(): schema.Rule;
  signatureParens(): schema.Rule;
  signatureRecovered(): schema.Rule;
  structure(): schema.Rule;
  structureFunctor(): schema.Rule;
  structureLiteral(): schema.Rule;
  structureParens(): schema.Rule;
  structureUnpack(): schema.Rule;
  term(): schema.Rule;
  termAtomic(): schema.Rule;
  termConditional(): schema.Rule;
  termConstructor(): schema.Rule;
  termDelim(): schema.Rule;
  termFor(): schema.Rule;
  termFunction(): schema.Rule;
  termLet(): schema.Rule;
  termMatch(): schema.Rule;
  termMatchRule(): schema.Rule;
  termOperator(): schema.Rule;
  termPun(): schema.Rule;
  termTry(): schema.Rule;
  termWhile(): schema.Rule;
  type(): schema.Rule;
  typeConstructor(): schema.Rule;
  typeLabel(): schema.Rule;
  typeModule(): schema.Rule;
  typeObject(): schema.Rule;
  typeOperator(): schema.Rule;
  typeParens(): schema.Rule;
  typePolymorphicVariant(): schema.Rule;
  typeRecord(): schema.Rule;
  variableModule(): schema.Rule;
  variablePattern(): schema.Rule;
}

export interface IRender {
  render(): schema.IGrammar;
}

export interface ILanguage extends IGrammar, IRender {}
