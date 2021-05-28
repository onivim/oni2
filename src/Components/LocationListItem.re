open EditorCoreTypes;
open Revery;
open Revery.UI;
open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Oni2.UI.LocationListItem"));

module Colors = Feature_Theme.Colors;

type t = {
  file: string,
  location: CharacterPosition.t,
  text: string,
  // TODO: Change Index.t -> ByteIndex. to properly type this
  highlight: option((Index.t, Index.t)),
};

let toTrees: list(t) => list(Tree.t(string, t)) =
  (items: list(t)) => {
    let fileToHits =
      items
      |> List.fold_left(
           (acc, curr) => {
             acc
             |> StringMap.update(
                  curr.file,
                  fun
                  | None => Some([curr])
                  | Some(existingList) => Some([curr, ...existingList]),
                )
           },
           StringMap.empty,
         );

    StringMap.fold(
      (filePath, hits, acc) => {
        let children = hits |> List.map(Tree.leaf) |> List.rev;

        let node = Tree.node(~expanded=true, ~children, filePath);
        [node, ...acc];
      },
      fileToHits,
      [],
    );
  };

module Styles = {
  open Style;

  let clickable = [cursor(Revery.MouseCursors.pointer)];

  let result = [
    flexDirection(`Row),
    overflow(`Hidden),
    paddingVertical(4),
    paddingHorizontal(8),
  ];

  let snippet = (~theme, ~isHighlighted) => [
    color(
      isHighlighted
        ? Colors.Oni.normalModeBackground.from(theme)
        : Colors.foreground.from(theme),
    ),
    textWrap(TextWrapping.NoWrap),
  ];
};

module View = {
  let make =
      (~showPosition=false, ~theme, ~uiFont: UiFont.t, ~width, ~item, ()) => {
    let unstyled = (~text, ()) =>
      <Text
        style={Styles.snippet(~theme, ~isHighlighted=false)}
        fontFamily={uiFont.family}
        fontSize={uiFont.size}
        text
      />;

    let position =
      showPosition
        ? <Opacity opacity=0.75>
            <unstyled
              text={Printf.sprintf(
                " [%d, %d]",
                // For UI: We should be showing one-based locations
                item.location.line |> EditorCoreTypes.LineNumber.toOneBased,
                (item.location.character |> CharacterIndex.toInt) + 1,
              )}
            />
          </Opacity>
        : React.empty;

    let content = () => {
      let highlighted = (~text, ()) =>
        <Text
          style={Styles.snippet(~theme, ~isHighlighted=true)}
          fontFamily={uiFont.family}
          fontSize={uiFont.size}
          text
        />;

      switch (item.highlight) {
      | Some((indexStart, indexEnd)) =>
        let textWidth =
          Revery.Draw.Text.dimensions(
            ~smoothing=Revery.Font.Smoothing.default,
            ~fontSize=uiFont.size,
            ~fontFamily=uiFont.family,
            ~fontWeight=Normal,
            item.text,
          ).
            width;

        let averageCharacterWidth =
          textWidth /. float(String.length(item.text));

        let availableWidth = float(width);
        let maxLength = int_of_float(availableWidth /. averageCharacterWidth);
        let charStart = Index.toZeroBased(indexStart);
        let charEnd = Index.toZeroBased(indexEnd);

        try({
          let (text, charStart, charEnd) =
            StringEx.extractSnippet(
              ~maxLength,
              ~charStart,
              ~charEnd,
              item.text,
            );
          let before = String.sub(text, 0, charStart);
          let matchedText = String.sub(text, charStart, charEnd - charStart);
          let after =
            String.sub(text, charEnd, String.length(text) - charEnd);

          <View style=Style.[flexDirection(`Row)]>
            <unstyled text=before />
            <highlighted text=matchedText />
            <unstyled text={StringEx.trimRight(after)} />
          </View>;
        }) {
        | Invalid_argument(message) =>
          // TODO: This shouldn't happen, but you never know. Consider a sane implementation of `String.sub` instead, to avoid this
          Log.errorf(m =>
            m("\"%s\" - (%n, %n)\n%!", message, charStart, charEnd)
          );
          <unstyled text={item.text} />;
        };
      | None => <unstyled text={item.text} />
      };
    };

    <View style=Styles.clickable>
      <View style=Styles.result> <content /> position </View>
    </View>;
  };
};
