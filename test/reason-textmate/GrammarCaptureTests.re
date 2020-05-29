/**
 GrammarCaptureTests.re

 Tests specific to 'capture' behavior
*/
open TestFramework;

module Grammar = Textmate.Grammar;
module GrammarRepository = Textmate.GrammarRepository;
module RegExp = Textmate.RegExp;
module RegExpFactory = Textmate.RegExpFactory;
module Token = Textmate.Token;

let createRegex = (~allowBackReferences=true, str) => {
  RegExpFactory.create(~allowBackReferences, str);
};

describe("GrammarCaptureTests", ({test, _}) => {
  let grammar =
    Grammar.create(
      ~scopeName="source.hello",
      ~patterns=[
        Match({
          matchRegex: createRegex("hello"),
          matchName: Some("prefix.hello"),
          captures: [],
        }),
        Match({
          matchRegex: createRegex("world(!?)"),
          matchName: Some("suffix.hello"),
          captures: [(1, "emphasis.hello")],
        }),
        MatchRange({
          beginRegex: createRegex("<(\\w+)>"),
          endRegex: createRegex(~allowBackReferences=false, "</\\1>"),
          beginCaptures: [(0, "html.tag.open")],
          endCaptures: [(0, "html.tag.close")],
          name: Some("html.tag.contents"),
          contentName: None,
          patterns: [],
          applyEndPatternLast: false,
        }),
      ],
      ~repository=[],
      (),
    );

  let grammarRepository = _ => None;

  test(
    "match with both name + capture gets both scopes applied", ({expect, _}) => {
    let (tokens, _) =
      Grammar.tokenize(~grammarRepository, ~grammar, "world!");

    expect.int(List.length(tokens)).toBe(2);

    let firstToken = List.hd(tokens);
    expect.bool(firstToken.scopes == ["suffix.hello", "source.hello"]).toBe(
      true,
    );
    expect.int(firstToken.position).toBe(0);
    expect.int(firstToken.length).toBe(5);

    let secondToken = List.nth(tokens, 1);
    expect.bool(
      secondToken.scopes == ["emphasis.hello", "suffix.hello", "source.hello"],
    ).
      toBe(
      true,
    );
    expect.int(secondToken.position).toBe(5);
    expect.int(secondToken.length).toBe(1);
  });

  test("capture with back-reference", ({expect, _}) => {
    let (tokens, _) =
      Grammar.tokenize(~grammarRepository, ~grammar, "<HERE> abc </HERE>");

    expect.int(List.length(tokens)).toBe(3);
    List.iter(t => prerr_endline(Token.show(t)), tokens);

    let firstToken = List.hd(tokens);
    expect.bool(
      firstToken.scopes
      == ["html.tag.open", "html.tag.contents", "source.hello"],
    ).
      toBe(
      true,
    );

    let thirdToken = List.nth(tokens, 2);
    expect.bool(
      thirdToken.scopes
      == ["html.tag.close", "html.tag.contents", "source.hello"],
    ).
      toBe(
      true,
    );
  });
});
