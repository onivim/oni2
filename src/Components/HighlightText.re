open Revery.UI;

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
    prerr_endline ("TOTAL LENGTH: " ++ string_of_int(textLength));

    // Assumes ranges are sorted low to high
    let rec highlighter = (last, rest) =>  {
      prerr_endline(Printf.sprintf("highligher - last: %d", last)); 
      switch (rest) {
      | [] => 
        prerr_endline ("-- empty");
        [
          <Text
            ?fontFamily
            ?fontSize
            style
            text={String.sub(text, last, textLength - last)}
          />,
        ]

      | [(low, high), ...rest] => 
        prerr_endline (Printf.sprintf("-- Not empty - low: %d high: %d", low, high));
        [
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
        ];
    }
    };

    highlighter(0, highlights) |> React.listToElement;
  };

  <View style=Style.[flexDirection(`Row)]> highlighted </View>;
};
