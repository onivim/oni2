/* Feature_Hover.re
     This feature project contains logic related to Hover
   */
open Oni_Core;
open Revery;
open Revery.UI;
open Revery.UI.Components;
open EditorCoreTypes;
open Utility;

module Log = (val Log.withNamespace("Oni.Feature.Hover"));

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: list(Exthost.DocumentFilter.t),
};

type model = {
  shown: bool,
  providers: list(provider),
  contents: list(string),
  range: option(EditorCoreTypes.Range.t),
};

let initial = {shown: false, providers: [], contents: [], range: None};

[@deriving show({with_path: false})]
type command =
  | Show;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | KeyPressed(string)
  | ProviderRegistered(provider)
  | HoverInfoReceived({
      contents: list(string),
      range: option(EditorCoreTypes.Range.t),
    })
  | HoverRequestFailed(string);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update = (~maybeBuffer, ~maybeEditor, ~extHostClient, model, msg) =>
  switch (msg) {
  | Command(Show) =>
    switch (maybeBuffer, maybeEditor) {
    | (Some(buffer), Some(editor)) =>
      let filetype =
        buffer
        |> Oni_Core.Buffer.getFileType
        |> Option.value(~default="plaintext");

      let matchingProviders =
        model.providers
        |> List.filter(({selector, _}) =>
             Exthost.DocumentSelector.matches(~filetype, selector)
           );

      let position = Feature_Editor.Editor.getPrimaryCursor(~buffer, editor);

      let effects =
        matchingProviders
        |> List.map(provider =>
             Service_Exthost.Effects.LanguageFeatures.provideHover(
               ~handle=provider.handle,
               ~uri=Oni_Core.Buffer.getUri(buffer),
               ~position,
               extHostClient,
               res =>
               switch (res) {
               | Ok({contents, range}) =>
                 HoverInfoReceived({
                   contents,
                   range: Option.map(Exthost.OneBasedRange.toRange, range),
                 })
               | Error(s) => HoverRequestFailed(s)
               }
             )
           )
        |> Isolinear.Effect.batch;

      ({...model, shown: true, contents: []}, Effect(effects));

    | _ => (model, Nothing)
    }
  | KeyPressed(_) => ({...model, shown: false}, Nothing)
  | ProviderRegistered(provider) => (
      {...model, providers: [provider, ...model.providers]},
      Nothing,
    )
  | HoverInfoReceived({contents, range}) => (
      {...model, contents, range},
      Nothing,
    )
  | _ => (model, Nothing)
  };

module Commands = {
  open Feature_Commands.Schema;

  let show =
    define(
      ~category="Hover",
      ~title="Show hover panel",
      "editor.action.showHover",
      Command(Show),
    );
};

module Contributions = {
  let commands = Commands.[show];
};

module Constants = {
  let scrollBarThickness = 10;
  let scrollTrackColor = Color.rgba(0., 0., 0., 0.4);
  let scrollThumbColor = Color.rgba(0.5, 0.5, 0.5, 0.4);
};

module Styles = {
  open Style;
  module Colors = Feature_Theme.Colors;

  let outer = (~x, ~y) => [
    position(`Absolute),
    left(x),
    top(y),
    border(~width=1, ~color=Revery.Colors.black),
  ];

  let maxHeight = 200;
  let maxWidth = 500;

  let container = [
    position(`Relative),
    Style.maxWidth(maxWidth + Constants.scrollBarThickness),
    Style.maxHeight(maxHeight),
    overflow(`Scroll),
  ];

  let contents = (~theme, ~showScrollbar) => [
    backgroundColor(Colors.EditorHoverWidget.background.from(theme)),
    Style.maxWidth(maxWidth),
    paddingLeft(6),
    {
      showScrollbar
        ? paddingRight(6 + Constants.scrollBarThickness) : paddingRight(6);
    },
    paddingBottom(4),
    paddingTop(4),
  ];

  let scrollBar = (~theme) => [
    right(0),
    top(0),
    bottom(0),
    position(`Absolute),
    backgroundColor(Colors.EditorHoverWidget.background.from(theme)),
    width(Constants.scrollBarThickness),
  ];
};

module View = {
  let%component make =
                (
                  ~colorTheme,
                  ~tokenTheme,
                  ~languageInfo,
                  ~uiFont: UiFont.t,
                  ~editorFont: Service_Font.font,
                  ~model,
                  ~editor: Feature_Editor.Editor.t,
                  ~buffer,
                  ~gutterWidth,
                  ~cursorOffset,
                  ~grammars,
                  (),
                ) => {
    let%hook (maybeContents, setMaybeContents) = Hooks.state(None);
    let setContents = c => setMaybeContents(_ => Some(c));

    let hoverMarkdown = (~markdown) =>
      Oni_Components.Markdown.make(
        ~colorTheme,
        ~tokenTheme,
        ~languageInfo,
        ~fontFamily={
          uiFont.family;
        },
        ~codeFontFamily={
          editorFont.fontFamily;
        },
        ~grammars,
        ~markdown,
        ~baseFontSize=14.,
        ~codeBlockStyle=[],
      );

    let showScrollbar =
      switch (maybeContents) {
      | None => false
      | Some(contents) =>
        let {height, _}: Revery.UI.Dimensions.t = contents#measurements();
        height >= Styles.maxHeight;
      };

    let scrollbar = () =>
      switch (maybeContents) {
      | None => React.empty
      | Some(contents) =>
        let {height, _}: Revery.UI.Dimensions.t = contents#measurements();
        let thumbLength = Styles.maxHeight * Styles.maxHeight / height;
        <View style={Styles.scrollBar(~theme=colorTheme)}>
          <Slider
            minimumValue=0.
            maximumValue={float(Styles.maxHeight - height)}
            sliderLength=Styles.maxHeight
            thumbLength
            trackThickness=Constants.scrollBarThickness
            thumbThickness=Constants.scrollBarThickness
            minimumTrackColor=Constants.scrollTrackColor
            maximumTrackColor=Constants.scrollTrackColor
            thumbColor=Constants.scrollThumbColor
            vertical=true
          />
        </View>;
      };

    let maybeCoords: option((int, int)) =
      switch (model.range, model.shown) {
      | (Some(range), true) =>
        let y =
          int_of_float(
            editorFont.measuredHeight
            *. float(Index.toZeroBased(range.start.line) + 1)
            -. editor.scrollY
            +. 0.5,
          );

        let x =
          int_of_float(
            gutterWidth
            +. editorFont.measuredWidth
            *. float(Index.toZeroBased(range.start.column))
            -. editor.scrollX
            +. 0.5,
          );
        Some((x, y));
      | (None, true) =>
        let cursorPosition =
          Feature_Editor.Editor.getPrimaryCursor(~buffer, editor);
        let y =
          int_of_float(
            editorFont.measuredHeight
            *. float(Index.toZeroBased(cursorPosition.line) + 1)
            -. editor.scrollY
            +. 0.5,
          );
        let x =
          int_of_float(
            gutterWidth
            +. editorFont.measuredWidth
            *. float(cursorOffset)
            -. editor.scrollX
            +. 0.5,
          );
        Some((x, y));
      | _ => None
      };

    switch (maybeCoords) {
    | Some((x, y)) =>
      <View style={Styles.outer(~x, ~y)}>
        <View style=Styles.container>
          <View
            style={Styles.contents(~theme=colorTheme, ~showScrollbar)}
            ref={node => setContents(node)}>
            {List.map(markdown => <hoverMarkdown markdown />, model.contents)
             |> React.listToElement}
          </View>
        </View>
        {showScrollbar ? <scrollbar /> : React.empty}
      </View>
    | None => React.empty
    };
  };
};
