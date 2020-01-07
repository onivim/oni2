open Oni_Core;
open TestFramework;
open Revery;

module BufferLineColorizer = Oni_Model.BufferLineColorizer;

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
  ColorizedToken.create(
    ~index=1,
    ~backgroundColor,
    ~foregroundColor=Colors.green,
    ~syntaxScope=SyntaxScope.None,
    (),
  ),
  ColorizedToken.create(
    ~index=5,
    ~backgroundColor,
    ~foregroundColor=Colors.red,
    ~syntaxScope=SyntaxScope.None,
    (),
  ),
  ColorizedToken.create(
    ~index=10,
    ~backgroundColor,
    ~foregroundColor=Colors.blue,
    ~syntaxScope=SyntaxScope.None,
    (),
  ),
];

describe("BufferLineColorizer", ({test, _}) => {
  test("base case - cover all tokens", ({expect, _}) => {
    let colorize = basicColorizer(~startIndex=0, ~endIndex=11, basicTokens);

    let (_, color0) = colorize(0);
    let (_, color2) = colorize(2);
    let (_, color6) = colorize(6);
    let (_, color11) = colorize(11);

    expect.equal(color0, Colors.white);
    expect.equal(color2, Colors.green);
    expect.equal(color6, Colors.red);
    expect.equal(color11, Colors.blue);
  });

  test("out of bounds", ({expect, _}) => {
    let colorize = basicColorizer(~startIndex=4, ~endIndex=6, basicTokens);

    let (_, color0) = colorize(0);
    let (_, color11) = colorize(11);

    expect.equal(color0, Colors.green);
    expect.equal(color11, Colors.red);
  });
});
