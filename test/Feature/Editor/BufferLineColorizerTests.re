open Oni_Core;
open TestFramework;
open Revery;

module BufferLineColorizer = Feature_Editor.BufferLineColorizer;

let basicColorizer =
  BufferLineColorizer.create(
    ~defaultBackgroundColor=Colors.black,
    ~defaultForegroundColor=Colors.white,
    ~selectionHighlights=None,
    ~selectionColor=Colors.yellow,
    ~matchingPair=None,
    ~searchHighlights=[],
    ~searchHighlightColor=Colors.orange,
  );

// Still needs ~startIndex, ~endIndex, and tokenColors

let backgroundColor = Colors.black;
let basicTokens = [
  ThemeToken.create(
    ~index=1,
    ~backgroundColor,
    ~foregroundColor=Colors.green,
    ~syntaxScope=SyntaxScope.none,
    (),
  ),
  ThemeToken.create(
    ~index=5,
    ~backgroundColor,
    ~foregroundColor=Colors.red,
    ~syntaxScope=SyntaxScope.none,
    (),
  ),
  ThemeToken.create(
    ~index=10,
    ~backgroundColor,
    ~foregroundColor=Colors.blue,
    ~syntaxScope=SyntaxScope.none,
    (),
  ),
];

describe("BufferLineColorizer", ({test, _}) => {
  test("base case - cover all tokens", ({expect, _}) => {
    let colorize = basicColorizer(~startByte=0, basicTokens);

    let BufferLineColorizer.{color: color0, _} = colorize(0);
    let BufferLineColorizer.{color: color2, _} = colorize(2);
    let BufferLineColorizer.{color: color6, _} = colorize(6);
    let BufferLineColorizer.{color: color11, _} = colorize(11);

    expect.equal(color0, Colors.white);
    expect.equal(color2, Colors.green);
    expect.equal(color6, Colors.red);
    expect.equal(color11, Colors.blue);
  });

  test("out of bounds", ({expect, _}) => {
    let colorize = basicColorizer(~startByte=4, basicTokens);

    let BufferLineColorizer.{color: color0, _} = colorize(0);
    let BufferLineColorizer.{color: color11, _} = colorize(11);

    expect.equal(color0, Colors.green);
    expect.equal(color11, Colors.blue);
  });
});
