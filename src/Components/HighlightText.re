open Revery.UI;

module Log = (val Oni_Core.Log.withNamespace("Oni2.Component.HighlightText"));

let make =
    (
      ~text: string,
      ~highlights: list((int, int)),
      ~style,
      ~fontFamily=?,
      ~fontSize=?,
      ~highlightStyle,
      (),
    ) => {
  let highlighted = {
    let textLength = String.length(text);

    // Assumes ranges are sorted low to high
    let rec highlighter = (last, rest) => {
      switch (rest) {
      | [] => [
          <Text
            ?fontFamily
            ?fontSize
            style
            text={String.sub(text, last, textLength - last)}
          />,
        ]

      | [(low, high), ...rest] => [
          <Text
            style
            ?fontFamily
            ?fontSize
            text={String.sub(text, last, low - last)}
          />,
          <Text
            style=highlightStyle
            ?fontFamily
            ?fontSize
            fontWeight=Revery.Font.Weight.Bold
            text={String.sub(text, low, high + 1 - low)}
          />,
          ...highlighter(high + 1, rest),
        ]
      };
    };

    try(highlighter(0, highlights) |> React.listToElement) {
    // There was an error creating the highlight text - don't crash, but fallback to un-highlighted-text.
    | exn =>
      Log.errorf(m => m("HighlightText error: %s", Printexc.to_string(exn)));
      <Text style ?fontFamily ?fontSize text />;
    };
  };

  <View style=Style.[flexDirection(`Row)]> highlighted </View>;
};
