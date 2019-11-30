open Revery.UI;

let make =
    (
      ~text: string,
      ~highlights: list((int, int)),
      ~style,
      ~highlightStyle,
      (),
    ) => {
  let highlighted = {
    let textLength = String.length(text);

    // Assumes ranges are sorted low to high
    let rec highlighter = last =>
      fun
      | [] => [
          <Text style text={String.sub(text, last, textLength - last)} />,
        ]

      | [(low, high), ...rest] => [
          <Text style text={String.sub(text, last, low - last)} />,
          <Text
            style=highlightStyle
            text={String.sub(text, low, high + 1 - low)}
          />,
          ...highlighter(high + 1, rest),
        ];

    highlighter(0, highlights) |> React.listToElement;
  };

  <View style=Style.[flexDirection(`Row)]> highlighted </View>;
};
