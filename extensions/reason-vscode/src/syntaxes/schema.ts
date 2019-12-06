export interface IPatterns extends Array<Rule> {}

export interface IGrammar {
  name: string;
  scopeName: string;
  fileTypes: string[];
  patterns: IPatterns;
  repository: IRepository;
}

export interface IMatchScopes {
  [key: string]: RuleSimple;
}

export type Rule = RuleSimple | IRuleCapturing | IRuleDelimited | IRuleReference;

export interface IRuleCapturing {
  match: string;
  name?: string;
  captures?: IMatchScopes;
  patterns?: IPatterns;
}

export interface IRuleDelimited {
  begin: string;
  end: string;
  applyEndPatternLast?: boolean;
  name?: string;
  contentName?: string;
  beginCaptures?: IMatchScopes;
  endCaptures?: IMatchScopes;
  patterns?: IPatterns;
}

export interface IRuleReference {
  include: string;
}

export type RuleSimple =
  | {
      name: string;
      patterns?: IPatterns;
    }
  | {
      name?: string;
      patterns: IPatterns;
    };

export interface IRepository {
  [key: string]: Rule;
}
