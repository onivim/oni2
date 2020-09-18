open Revery;
open Revery.UI;
open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Oni2.UI.LocationListView"));

module Colors = Feature_Theme.Colors;

// TODO: move to Revery
let getFontAdvance = (fontFamily, fontSize) => {
  let font =
    Service_Font.resolveWithFallback(Revery.Font.Weight.Normal, fontFamily);
  Revery.Font.FontRenderer.measure(
    ~smoothing=Revery.Font.Smoothing.default,
    font,
    fontSize,
    "x",
  );
};

module Styles = {
  open Style;

  let clickable = [cursor(Revery.MouseCursors.pointer)];

  let result = (~theme, ~isHovered) => [
    flexDirection(`Row),
    overflow(`Hidden),
    paddingVertical(4),
    paddingHorizontal(8),
    backgroundColor(
      isHovered
        ? Colors.Menu.selectionBackground.from(theme)
        : Revery.Colors.transparentWhite,
    ),
  ];

  let locationText = (~theme) => [
    color(Colors.EditorLineNumber.activeForeground.from(theme)),
    textWrap(TextWrapping.NoWrap),
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

let%component make =
              (
                ~theme,
                ~uiFont: UiFont.t,
                ~editorFont: Service_Font.font,
                ~items: array(LocationListItem.t),
                ~onSelectItem: LocationListItem.t => unit,
                ~workingDirectory,
                (),
              ) => {
  let%hook outerRef = Hooks.ref(None);
  let%hook (hovered, setHovered) = Hooks.state(-1);

  let editorFont = {
    ...editorFont,
    fontSize: uiFont.size,
    spaceWidth: getFontAdvance(editorFont.fontFamily, uiFont.size).width,
    // measuredHeight:
    //   editorFont.measuredHeight
    //   *. (float(uiFont.fontSize) /. float(editorFont.fontSize)),
  };

  let width =
    outerRef^
    |> Option.map(node => node#measurements().Dimensions.width)
    |> Option.value(
         ~default=
           Revery.UI.getActiveWindow()
           |> Option.map((window: Window.t) =>
                Revery.Window.getSize(window).width
              )
           |> Option.value(~default=4000),
       );

  let renderItem = i => {
    <LocationListItem.View
      theme
      uiFont
      editorFont
      width
      //    let onMouseOver = _ => setHovered(_ => i);
      //    let onMouseOut = _ => setHovered(hovered => i == hovered ? (-1) : hovered);
      //      onMouseOver
      //      onMouseOut
      isHovered={hovered == i}
      item={Array.get(items, i)}
      workingDirectory
      //      onSelect=onSelectItem
    />;
  };

  <FlatList
    rowHeight=20
    count={Array.length(items)}
    focused=None
    theme
    ref={ref => outerRef := Some(ref)}>
    ...renderItem
  </FlatList>;
};
