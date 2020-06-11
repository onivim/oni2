/* Feature_Hover.re
     This feature project contains logic related to Hover
   */
open Oni_Core;
open Revery;
open Revery.UI;
open Revery.UI.Components;
open EditorCoreTypes;

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
  let scrollWheelMultiplier = 25;
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

  let diagnostic = (~theme) => [
    textOverflow(`Ellipsis),
    color(Colors.Editor.foreground.from(theme)),
    backgroundColor(Colors.EditorHoverWidget.background.from(theme)),
  ];

  let contents = (~theme, ~showScrollbar, ~scrollTop) => [
    backgroundColor(Colors.EditorHoverWidget.background.from(theme)),
    Style.maxWidth(maxWidth),
    top(scrollTop),
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

  let hr = [
    flexGrow(1),
    height(2),
    backgroundColor(Color.rgba(0., 0., 0., 0.25)),
    marginTop(4),
    marginBottom(4),
  ];
};

module View = {
  let horizontalRule = () => <Row> <View style=Styles.hr /> </Row>;

  type state = {
    scrollTop: int,
    maybeHeight: option(int),
  };
  let initialState = {scrollTop: 0, maybeHeight: None};
  type action =
    | SetScrollTop(int)
    | SetHeight(int);

  let%component hover =
                (
                  ~x,
                  ~y,
                  ~colorTheme,
                  ~tokenTheme,
                  ~languageInfo,
                  ~uiFont: UiFont.t,
                  ~editorFont: Service_Font.font,
                  ~model,
                  ~grammars,
                  ~diagnostic,
                  (),
                ) => {
    let reducer = (action, state) =>
      switch (action) {
      | SetScrollTop(scrollTop) => {...state, scrollTop}
      | SetHeight(height) => {...state, maybeHeight: Some(height)}
      };
    let%hook (state, dispatch) = Hooks.reducer(~initialState, reducer);

    let%hook () =
      Hooks.effect(
        If((!=), model.contents),
        () => {
          dispatch(SetScrollTop(0));
          None;
        },
      );

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
        ~codeBlockStyle=Style.[flexGrow(1)],
      );

    let hoverDiagnostic =
        (~diagnostic: Feature_LanguageSupport.Diagnostic.t, ()) => {
      <Text
        text={diagnostic.message}
        fontFamily={editorFont.fontFamily}
        fontSize={editorFont.fontSize}
        style={Styles.diagnostic(~theme=colorTheme)}
      />;
    };

    let showScrollbar =
      switch (state.maybeHeight) {
      | None => false
      | Some(height) => height >= Styles.maxHeight
      };

    let scrollbar = () =>
      switch (state.maybeHeight) {
      | None => React.empty
      | Some(height) =>
        let thumbLength = Styles.maxHeight * Styles.maxHeight / height;
        <View style={Styles.scrollBar(~theme=colorTheme)}>
          <Slider
            onValueChanged={v => dispatch(SetScrollTop(int_of_float(v)))}
            value={float(state.scrollTop)}
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

    let scroll = (wheelEvent: NodeEvents.mouseWheelEventParams) =>
      switch (state.maybeHeight, showScrollbar) {
      | (Some(height), true) =>
        let delta =
          int_of_float(wheelEvent.deltaY) * Constants.scrollWheelMultiplier;
        dispatch(
          SetScrollTop(
            state.scrollTop
            + delta
            |> Oni_Core.Utility.IntEx.clamp(
                 ~hi=0,
                 ~lo=Styles.maxHeight - height,
               ),
          ),
        );

      | _ => ()
      };

    <View style={Styles.outer(~x, ~y)}>
      <View style=Styles.container>
        <View
          style={Styles.contents(
            ~theme=colorTheme,
            ~showScrollbar,
            ~scrollTop=state.scrollTop,
          )}
          onMouseWheel=scroll
          onDimensionsChanged={({height, _}) =>
            dispatch(SetHeight(height))
          }>
          {List.map(markdown => <hoverMarkdown markdown />, model.contents)
           |> React.listToElement}
          <horizontalRule />
          {List.map(diag => <hoverDiagnostic diagnostic=diag />, diagnostic)
           |> React.listToElement}
        </View>
      </View>
      {showScrollbar ? <scrollbar /> : React.empty}
    </View>;
  };

  let make =
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
        ~diagnostics,
        (),
      ) => {
    let (maybeCoords, maybeDiagnostic): (
      option((int, int)),
      option(list(Feature_LanguageSupport.Diagnostic.t)),
    ) =
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

        let diagnostic =
          Feature_LanguageSupport.Diagnostics.getDiagnosticsAtPosition(
            diagnostics,
            buffer,
            range.start,
          );

        (Some((x, y)), Some(diagnostic));
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

        let diagnostic =
          Feature_LanguageSupport.Diagnostics.getDiagnosticsAtPosition(
            diagnostics,
            buffer,
            cursorPosition,
          );

        (Some((x, y)), Some(diagnostic));
      | _ => (None, None)
      };
    switch (maybeCoords, maybeDiagnostic) {
    | (Some((x, y)), Some(diagnostic)) =>
      <hover
        x
        y
        colorTheme
        tokenTheme
        languageInfo
        uiFont
        editorFont
        model
        grammars
        diagnostic
      />
    | _ => React.empty
    };
  };
};
