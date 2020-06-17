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
  contents: list(Exthost.MarkdownString.t),
  range: option(EditorCoreTypes.Range.t),
  triggeredFrom:
    option([ | `CommandPalette | `Mouse(EditorCoreTypes.Location.t)]),
  lastRequestID: option(int),
};

let initial = {
  shown: false,
  providers: [],
  contents: [],
  range: None,
  triggeredFrom: None,
  lastRequestID: None,
};

module IDGenerator = {
  let current = ref(0);

  let get = () => {
    let id = current^;
    current := id + 1;
    id;
  };
};

[@deriving show({with_path: false})]
type command =
  | Show;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | KeyPressed(string)
  | ProviderRegistered(provider)
  | HoverInfoReceived({
      contents: list(Exthost.MarkdownString.t),
      range: option(EditorCoreTypes.Range.t),
      requestID: int,
    })
  | HoverRequestFailed(string)
  | MouseHovered(EditorCoreTypes.Location.t)
  | MouseMoved(EditorCoreTypes.Location.t);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let getEffectsForLocation =
    (~buffer, ~location, ~extHostClient, ~model, ~requestID) => {
  let filetype =
    buffer
    |> Oni_Core.Buffer.getFileType
    |> Option.value(~default="plaintext");

  let matchingProviders =
    model.providers
    |> List.filter(({selector, _}) =>
         Exthost.DocumentSelector.matches(~filetype, selector)
       );

  matchingProviders
  |> List.map(provider =>
       Service_Exthost.Effects.LanguageFeatures.provideHover(
         ~handle=provider.handle,
         ~uri=Oni_Core.Buffer.getUri(buffer),
         ~position=location,
         extHostClient,
         res =>
         switch (res) {
         | Ok({contents, range}) =>
           HoverInfoReceived({
             contents,
             range: Option.map(Exthost.OneBasedRange.toRange, range),
             requestID,
           })
         | Error(s) => HoverRequestFailed(s)
         }
       )
     )
  |> Isolinear.Effect.batch;
};

let update = (~maybeBuffer, ~maybeEditor, ~extHostClient, model, msg) =>
  switch (msg) {
  | Command(Show) =>
    switch (maybeBuffer, maybeEditor) {
    | (Some(buffer), Some(editor)) =>
      let requestID = IDGenerator.get();
      let effects =
        getEffectsForLocation(
          ~buffer,
          ~location=Feature_Editor.Editor.getPrimaryCursor(~buffer, editor),
          ~extHostClient,
          ~model,
          ~requestID,
        );
      (
        {
          ...model,
          shown: true,
          triggeredFrom: Some(`CommandPalette),
          lastRequestID: Some(requestID),
        },
        Effect(effects),
      );

    | _ => (model, Nothing)
    }
  | MouseHovered(location) =>
    switch (maybeBuffer, maybeEditor) {
    | (Some(buffer), Some(_)) =>
      let requestID = IDGenerator.get();
      let effects =
        getEffectsForLocation(
          ~buffer,
          ~location,
          ~extHostClient,
          ~model,
          ~requestID,
        );
      (
        {
          ...model,
          shown: true,
          triggeredFrom: Some(`Mouse(location)),
          lastRequestID: Some(requestID),
        },
        Effect(effects),
      );
    | _ => (model, Nothing)
    }
  | MouseMoved(location) =>
    let newModel =
      switch (model.range) {
      | Some(range) =>
        if (EditorCoreTypes.Range.contains(location, range)) {
          model;
        } else {
          {
            ...model,
            shown: false,
            range: None,
            contents: [],
            triggeredFrom: None,
          };
        }

      | None => {
          ...model,
          shown: false,
          range: None,
          contents: [],
          triggeredFrom: None,
        }
      };
    (newModel, Nothing);
  | KeyPressed(_) => (
      {
        ...model,
        shown: false,
        contents: [],
        range: None,
        triggeredFrom: None,
      },
      Nothing,
    )
  | ProviderRegistered(provider) => (
      {...model, providers: [provider, ...model.providers]},
      Nothing,
    )
  | HoverInfoReceived({contents, range, requestID}) =>
    switch (model.lastRequestID) {
    | Some(id) when requestID == id => (
        {...model, contents, range, lastRequestID: None},
        Nothing,
      )
    | _ => (model, Nothing)
    }
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

  let outer = (~x, ~y, ~theme) => [
    position(`Absolute),
    left(x),
    top(y),
    border(~width=1, ~color=Colors.EditorHoverWidget.border.from(theme)),
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

  let hr = (~theme) => [
    flexGrow(1),
    height(1),
    backgroundColor(Colors.EditorHoverWidget.border.from(theme)),
    marginTop(3),
    marginBottom(3),
  ];
};

module View = {
  let horizontalRule = (~theme, ()) =>
    <Row> <View style={Styles.hr(~theme)} /> </Row>;

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
        ~markdown=Exthost.MarkdownString.toString(markdown),
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

    <View style={Styles.outer(~x, ~y, ~theme=colorTheme)}>
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
          {model.contents != [] && diagnostic != []
             ? <horizontalRule theme=colorTheme /> : React.empty}
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
        ~grammars,
        ~diagnostics,
        (),
      ) => {
    let (maybeCoords, maybeDiagnostic): (
      option((int, int)),
      option(list(Feature_LanguageSupport.Diagnostic.t)),
    ) =
      switch (model.range, model.triggeredFrom, model.shown) {
      | (Some(range), Some(trigger), true) =>
        let diagLocation =
          switch (trigger) {
          | `Mouse(location) => location
          | `CommandPalette =>
            Feature_Editor.Editor.getPrimaryCursor(~buffer, editor)
          };

        let diagnostic =
          Feature_LanguageSupport.Diagnostics.getDiagnosticsAtPosition(
            diagnostics,
            buffer,
            diagLocation,
          );

        let hoverLocation =
          switch (diagnostic) {
          | [] => range.start
          | [diag, ..._] => diag.range.start
          };

        // TODO: Index instead of byte
        let ({pixelX, pixelY}: Feature_Editor.Editor.pixelPosition, _) =
          Feature_Editor.Editor.bufferLineByteToPixel(
            ~line=hoverLocation.line |> Index.toZeroBased,
            ~byteIndex=hoverLocation.column |> Index.toZeroBased,
            editor,
          );

        let x = int_of_float(pixelX);
        let y = int_of_float(pixelY);

        // TODO: Hover width?

        (Some((x, y)), Some(diagnostic));
      | (None, Some(trigger), true) =>
        let location =
          switch (trigger) {
          | `Mouse(location) => location
          | `CommandPalette =>
            Feature_Editor.Editor.getPrimaryCursor(~buffer, editor)
          };

        let ({pixelX, pixelY}: Feature_Editor.Editor.pixelPosition, _) =
          Feature_Editor.Editor.bufferLineByteToPixel(
            ~line=(location.line |> Index.toZeroBased) + 1,
            ~byteIndex=location.column |> Index.toZeroBased,
            editor,
          );

        let x = int_of_float(pixelX);
        let y = int_of_float(pixelY);

        let diagnostic =
          Feature_LanguageSupport.Diagnostics.getDiagnosticsAtPosition(
            diagnostics,
            buffer,
            location,
          );

        diagnostic == [] ? (None, None) : (Some((x, y)), Some(diagnostic));
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
