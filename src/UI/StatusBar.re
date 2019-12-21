/*
 * StatusBar.re
 *
 * Container for StatusBar
 */

open EditorCoreTypes;
open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
open Oni_Model;

open Oni_Model.StatusBarModel;

module Option = Utility.Option;

module Styles = {
  open Style;

  let text = (uiFont: UiFont.t) => [
    fontFamily(uiFont.fontFile),
    fontSize(11),
    textWrap(TextWrapping.NoWrap),
  ];

  let view = (bgColor, transition) => [
    backgroundColor(bgColor),
    flexDirection(`Row),
    alignItems(`Center),
    flexGrow(1),
    justifyContent(`FlexEnd),
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
    transform(Transform.[TranslateY(transition)]),
  ];

  let section = alignment => [
    flexDirection(`Row),
    justifyContent(alignment),
    alignItems(alignment),
    flexGrow(1),
  ];

  let item = (height, bg) => [
    flexDirection(`Column),
    justifyContent(`Center),
    alignItems(`Center),
    Style.height(height),
    backgroundColor(bg),
    paddingHorizontal(10),
    minWidth(50),
  ];
};

let positionToString =
  fun
  | Some((loc: Location.t)) =>
    Printf.sprintf(
      "%n,%n",
      Index.toOneBased(loc.line),
      Index.toOneBased(loc.column),
    )
  | None => "";

let section = (~children=React.empty, ~align, ()) =>
  <View style={Styles.section(align)}> children </View>;

let item = (~children, ~height, ~backgroundColor, ()) =>
  <View style={Styles.item(height, backgroundColor)}> children </View>;

let textItem = (~height, ~font, ~theme: Theme.t, ~text, ()) =>
  <item height backgroundColor={theme.statusBarBackground}>
    <Text
      style=Style.[
        backgroundColor(theme.statusBarBackground),
        color(theme.statusBarForeground),
        ...Styles.text(font),
      ]
      text
    />
  </item>;

let diagnosticsItem = (~height, ~font, ~theme: Theme.t, ~diagnostics, ()) => {
  let count =
    diagnostics |> Diagnostics.count |> string_of_int;

  <item height backgroundColor={theme.statusBarBackground}>
    <View
      style=Style.[
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
      ]>
      <FontIcon
        icon=FontAwesome.timesCircle
        backgroundColor={theme.statusBarBackground}
        color={theme.statusBarForeground}
        margin=4
      />
      <Text
        style=Style.[
          backgroundColor(theme.statusBarBackground),
          color(theme.statusBarForeground),
          ...Styles.text(font),
        ]
        text=count
      />
    </View>
  </item>
};

let modeItem = (~height, ~font, ~theme, ~mode, ()) => {
  let (background, foreground) = Theme.getColorsForMode(theme, mode);

  <item height backgroundColor=background>
    <Text
      style=Style.[
        backgroundColor(background),
        color(foreground),
        ...Styles.text(font),
      ]
      text={Vim.Mode.show(mode)}
    />
  </item>;
};

let animation =
  Revery.UI.Animation.(
    animate(Revery.Time.milliseconds(150))
    |> ease(Easing.ease)
    |> tween(50.0, 0.)
    |> delay(Revery.Time.milliseconds(0))
  );

let%component make = (~height, ~state: State.t, ()) => {
  let State.{mode, theme, uiFont: font, diagnostics, } = state;

  let%hook (transition, _animationState, _reset) =
    Hooks.animation(animation, ~active=true);

  let toStatusBarElement = (statusBarItem: Item.t) =>
    <textItem height font theme text={statusBarItem.text} />;

  let leftItems =
    state.statusBar
    |> List.filter((item: Item.t) => item.alignment == Alignment.Left)
    |> List.map(toStatusBarElement)
    |> items => List.append(items, [<diagnosticsItem height font theme diagnostics />])
    |> React.listToElement;

  let rightItems =
    state.statusBar
    |> List.filter((item: Item.t) => item.alignment == Alignment.Right)
    |> List.map(toStatusBarElement)
    |> React.listToElement;

  let indentationItem = () => {
    let text = Indentation.getForActiveBuffer(state) |> Indentation.toStatusString;

    <textItem height font theme text />
  };

  let fileTypeItem = () => {
    let text =
      state
      |> Selectors.getActiveBuffer
      |> Option.bind(Buffer.getFileType)
      |> Option.value(~default="plaintext");

    <textItem height font theme text />
  };

  let positionItem = () => {
    let text =
      state
      |> Selectors.getActiveEditorGroup
      |> Selectors.getActiveEditor
      |> Option.map(Editor.getPrimaryCursor)
      |> positionToString;

      <textItem height font theme text />
  };

  <View style={Styles.view(theme.statusBarBackground, transition)}>
    <section align=`FlexStart> leftItems </section>
    <section align=`Center />
    <section align=`FlexEnd> rightItems </section>
    <section align=`FlexEnd>
      <indentationItem />
      <fileTypeItem />
      <positionItem />
      <modeItem height font theme mode />
    </section>
  </View>;
};
