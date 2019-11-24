open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Types;
open Oni_Model;

// TODO: Remove with 4.08
module Option = {
  let map = f =>
    fun
    | Some(x) => Some(f(x))
    | None => None;

  let value = (~default) =>
    fun
    | Some(x) => x
    | None => default;
};

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

  let snippet = (~font: Types.EditorFont.t, ~theme: Theme.t, ~isHighlighted) => [
    fontFamily(font.fontFile),
    fontSize(font.fontSize),
    color(
      isHighlighted ? theme.oniNormalModeBackground : theme.editorForeground,
    ),
    textWrap(TextWrapping.NoWrap),
  ];
};

let item =
    (
      ~theme,
      ~uiFont: UiFont.t,
      ~editorFont,
      ~onMouseOver,
      ~onMouseOut,
      ~width,
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

  let locationText = Printf.sprintf(
        "%s:%n - ",
        getDisplayPath(item.file),
        Index.toInt1(item.location.line),
      );

  let locationWidth =
    Revery.Draw.Text.measure(
      ~window=Revery.UI.getActiveWindow(),
      ~fontSize=uiFont.fontSize,
      ~fontFamily=uiFont.fontFile,
      locationText
    ).width;

  let location = () =>
    <Text
      style={Styles.locationText(~font=uiFont, ~theme)}
      text=locationText
    />;

  let content = () => {
    let unstyled = (~text, ()) =>
      <Text
        style={Styles.snippet(~font=editorFont, ~theme, ~isHighlighted=false)}
        text
      />;

    let highlighted = (~text, ()) =>
      <Text
        style={Styles.snippet(~font=editorFont, ~theme, ~isHighlighted=true)}
        text
      />;

    switch (item.highlight) {
    | Some((indexStart, indexEnd)) =>
      open Utility.StringUtil;

      let availableWidth = float(width - locationWidth);
      let maxLength = int_of_float(availableWidth /. editorFont.measuredWidth);
      let charStart = Index.toInt1(indexStart);
      let charEnd = Index.toInt1(indexEnd);

      try({
        let (text, charStart, charEnd) =
          extractSnippet(~maxLength, ~charStart, ~charEnd, item.text);
        let before = String.sub(text, 0, charStart);
        let matchedText = String.sub(text, charStart, charEnd - charStart);
        let after = String.sub(text, charEnd, String.length(text) - charEnd);

        <View style=Style.[flexDirection(`Row)]>
          <unstyled text={trimLeft(before)} />
          <highlighted text=matchedText />
          <unstyled text={trimRight(after)} />
        </View>;
      }) {
      | Invalid_argument(message) =>
        // TODO: This shouldn't happen, but you never know. Consider a sane implementation of `String.sub` instead, to avoid this
        Log.error(
          Printf.sprintf(
            "[SearchPane.highlightedText] \"%s\" - (%n, %n)\n%!",
            message,
            charStart,
            charEnd,
          ),
        );
        <unstyled text={item.text} />;
      };
    | None => <unstyled text={item.text} />
    };
  };

  <Clickable style=Styles.clickable onClick>
    <View style={Styles.result(~theme, ~isHovered)} onMouseOver onMouseOut>
      <location />
      <content />
    </View>
  </Clickable>;
};

let%component make =
              (
                ~theme: Theme.t,
                ~uiFont: Types.UiFont.t,
                ~editorFont: Types.EditorFont.t,
                ~items: array(LocationListItem.t),
                (),
              ) => {
  let%hook (outerRef, setOuterRef) = Hooks.ref(None);
  let%hook (hovered, setHovered) = Hooks.state(-1);

  let editorFont = {...editorFont, fontSize: uiFont.fontSize};
  let width =
    outerRef
    |> Option.map(node => node#measurements().Dimensions.width)
    |> Option.value(
         ~default=
           Revery.UI.getActiveWindow()
           |> Option.map((window: Window.t) => window.metrics.size.width)
           |> Option.value(~default=4000),
       );

  let renderItem = i => {
    let onMouseOver = _ => setHovered(_ => i);
    let onMouseOut = _ => setHovered(hovered => i == hovered ? (-1) : hovered);

    <item
      theme
      uiFont
      editorFont
      onMouseOver
      onMouseOut
      width
      isHovered={hovered == i}
      item={Array.get(items, i)}
    />;
  };

  <FlatList
    rowHeight=20
    count={Array.length(items)}
    focused=None
    render=renderItem
    ref={ref => setOuterRef(Some(ref))}
  />;
};