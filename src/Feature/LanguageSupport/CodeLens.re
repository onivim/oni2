open Oni_Core;
open Oni_Core.Utility;

module Log = (
  val Oni_Core.Log.withNamespace("Oni2.LanguageSupport.CodeLens")
);

// MODEL

type codeLens = Exthost.CodeLens.t;

let lineNumber = (lens: Exthost.CodeLens.t) =>
  Exthost.OneBasedRange.(lens.range.startLineNumber - 1);

let textFromExthost = (lens: Exthost.CodeLens.t) => {
  Exthost.Command.(
    lens.command
    |> OptionEx.flatMap(command => command.label)
    |> Option.map(Exthost.Label.toString)
    |> Option.value(~default="(null)")
  );
};

let text = (lens: Exthost.CodeLens.t) => textFromExthost(lens);

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

type handleToLenses = IntMap.t(list(codeLens));

type model = {providers: list(provider)};

type outmsg =
  | Nothing
  | CodeLensesChanged({
      handle: int,
      bufferId: int,
      startLine: EditorCoreTypes.LineNumber.t,
      stopLine: EditorCoreTypes.LineNumber.t,
      lenses: list(codeLens),
    });

let initial = {providers: []};

[@deriving show]
type msg =
  | CodeLensesError(string)
  | CodeLensesReceived({
      handle: int,
      bufferId: int,
      startLine: EditorCoreTypes.LineNumber.t,
      stopLine: EditorCoreTypes.LineNumber.t,
      lenses: list(Exthost.CodeLens.t),
    });

let register = (~handle: int, ~selector, model) => {
  providers: [{handle, selector}, ...model.providers],
};

let unregister = (~handle: int, model) => {
  providers: model.providers |> List.filter(prov => prov.handle != handle),
};

// UPDATE

let update = (msg, model) =>
  switch (msg) {
  | CodeLensesError(_) => (model, Nothing)
  | CodeLensesReceived({handle, startLine, stopLine, bufferId, lenses}) => (
      model,
      CodeLensesChanged({handle, bufferId, startLine, stopLine, lenses}),
    )
  };

// SUBSCRIPTION

module Sub = {
  let create =
      (
        ~topVisibleBufferLine,
        ~bottomVisibleBufferLine,
        ~visibleBuffers,
        ~client,
        model,
      ) => {
    // Query below a viewport - grabbing some extra codelenses above and below -
    // to minimize codelens popping in while scrolling
    let delta: int =
      EditorCoreTypes.LineNumber.toOneBased(bottomVisibleBufferLine)
      - EditorCoreTypes.LineNumber.toOneBased(topVisibleBufferLine);
    let (topVisibleBufferLine, bottomVisibleBufferLine) =
      EditorCoreTypes.LineNumber.(
        topVisibleBufferLine - 1,
        bottomVisibleBufferLine + delta,
      );

    let codeLenses =
      visibleBuffers
      |> List.map(buffer => {
           model.providers
           |> List.filter(({selector, _}) =>
                Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
              )
           |> List.map(({handle, _}) => {
                let toMsg =
                  fun
                  | Error(msg) => CodeLensesError(msg)
                  | Ok(lenses) =>
                    CodeLensesReceived({
                      handle,
                      startLine: topVisibleBufferLine,
                      stopLine: bottomVisibleBufferLine,
                      bufferId: buffer |> Oni_Core.Buffer.getId,
                      lenses,
                    });

                Service_Exthost.Sub.codeLenses(
                  ~handle,
                  ~buffer,
                  ~startLine=topVisibleBufferLine,
                  ~stopLine=bottomVisibleBufferLine,
                  ~toMsg,
                  client,
                );
              })
         })
      |> List.flatten;

    codeLenses |> Isolinear.Sub.batch;
  };
};

module Configuration = Feature_Configuration.GlobalConfiguration;

let sub =
    (
      ~config,
      ~isAnimatingScroll,
      ~topVisibleBufferLine,
      ~bottomVisibleBufferLine,
      ~visibleBuffers,
      ~client,
      model,
    ) =>
  if (!isAnimatingScroll && Configuration.Editor.codeLensEnabled.get(config)) {
    Sub.create(
      ~topVisibleBufferLine,
      ~bottomVisibleBufferLine,
      ~visibleBuffers,
      ~client,
      model,
    );
  } else {
    Isolinear.Sub.none;
  };

// COLORS

module Colors = {
  open Revery;
  open ColorTheme.Schema;

  let foreground =
    define("editorCodeLens.foreground", color(Color.hex("#999999")) |> all);
};

// CONTRIBUTIONS

module Contributions = {
  let colors = Colors.[foreground];

  let configuration = [];
};

// VIEW

module View = {
  module CodeLensColors = Colors;
  open Revery.UI;
  let make = (~leftMargin, ~theme, ~uiFont: Oni_Core.UiFont.t, ~codeLens, ()) => {
    let foregroundColor = CodeLensColors.foreground.from(theme);
    let text = text(codeLens);
    <View
      style=Style.[
        marginTop(4),
        marginBottom(0),
        marginLeft(leftMargin),
        flexGrow(1),
        flexShrink(0),
      ]>
      <Text
        text
        fontFamily={uiFont.family}
        fontSize={uiFont.size}
        style=Style.[
          color(foregroundColor),
          flexGrow(1),
          textWrap(Revery.TextWrapping.Wrap),
        ]
      />
    </View>;
  };
};
