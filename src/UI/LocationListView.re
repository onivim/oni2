open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Types;
open Oni_Model;

module Styles = {
  open Style;

  let clickable = [cursor(Revery.MouseCursors.pointer)];

  let result = (~theme: Theme.t, ~isHovered) => [
    flexDirection(`Row),
    overflow(`Hidden),
    paddingVertical(4),
    paddingHorizontal(8),
    backgroundColor(
      isHovered ? theme.menuSelectionBackground : Colors.transparentWhite,
    ),
  ];

  let locationText = (~font: Types.UiFont.t, ~theme: Theme.t) => [
    fontFamily(font.fontFile),
    fontSize(font.fontSize),
    color(theme.editorActiveLineNumberForeground),
    textWrap(TextWrapping.NoWrap),
  ];

  let matchText = (~font: Types.UiFont.t, ~theme: Theme.t) => [
    fontFamily(font.fontFile),
    fontSize(font.fontSize),
    color(theme.editorForeground),
    textWrap(TextWrapping.NoWrap),
  ];

  let highlight = (~font: Types.UiFont.t, ~theme: Theme.t) => [
    fontFamily(font.fontFile),
    fontSize(font.fontSize),
    color(theme.oniNormalModeBackground),
    textWrap(TextWrapping.NoWrap),
  ];
};

let item =
    (
      ~theme,
      ~font,
      ~onMouseOver,
      ~onMouseOut,
      ~isHovered,
      ~item: LocationListItem.t,
      (),
    ) => {
  let directory = Rench.Environment.getWorkingDirectory();
  let re = Str.regexp_string(directory ++ Filename.dir_sep);
  let getDisplayPath = fullPath => Str.replace_first(re, "", fullPath);

  let onClick = () => {
    GlobalContext.current().dispatch(
      OpenFileByPath(item.file, None, Some(item.location)),
    );
  };

  let location = () =>
    <Text
      style={Styles.locationText(~font, ~theme)}
      text={Printf.sprintf(
        "%s:%n - ",
        getDisplayPath(item.file),
        Index.toInt1(item.location.line),
      )}
    />;

  let highlightedText = () =>
    switch (item.highlight) {
    | Some((indexStart, indexEnd)) =>
      open Utility.StringUtil;
      let maxLength = 1000;
      let charStart = Index.toInt1(indexStart);
      let charEnd = Index.toInt1(indexEnd);

      try({
        let (text, charStart, charEnd) =
          extractSnippet(~maxLength, ~charStart, ~charEnd, item.text);
        let before = String.sub(text, 0, charStart) |> trimLeft;
        let matchedText = String.sub(text, charStart, charEnd - charStart);
        let after =
          String.sub(text, charEnd, String.length(text) - charEnd)
          |> trimRight;

        <View style=Style.[flexDirection(`Row)]>
          <Text style={Styles.matchText(~font, ~theme)} text=before />
          <Text style={Styles.highlight(~font, ~theme)} text=matchedText />
          <Text style={Styles.matchText(~font, ~theme)} text=after />
        </View>;
      }) {
      | Invalid_argument(message) =>
        Log.error(
          Printf.sprintf(
            "[SearchPane.highlightedText] \"%s\" - (%n, %n)\n%!",
            message,
            charStart,
            charEnd,
          ),
        );
        <Text style={Styles.matchText(~font, ~theme)} text={item.text} />;
      };
    | None =>
      <Text style={Styles.matchText(~font, ~theme)} text={item.text} />
    };

  <Clickable style=Styles.clickable onClick>
    <View style={Styles.result(~theme, ~isHovered)} onMouseOver onMouseOut>
      <location />
      <highlightedText />
    </View>
  </Clickable>;
};

let%component make =
              (
                ~theme: Theme.t,
                ~font: Types.UiFont.t,
                ~items: array(LocationListItem.t),
                (),
              ) => {
  let%hook (hovered, setHovered) = Hooks.state(-1);

  let renderItem = i => {
    let onMouseOver = _ => setHovered(_ => i);
    let onMouseOut = _ => setHovered(hovered => i == hovered ? (-1) : hovered);

    <item
      theme
      font
      onMouseOver
      onMouseOut
      isHovered={hovered == i}
      item={Array.get(items, i)}
    />;
  };

  <FlatList
    rowHeight=20
    count={Array.length(items)}
    focused=None
    render=renderItem
  />;
};