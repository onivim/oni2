open Oni_Core;
open Oni_Core.Utility;

module Log = (
  val Oni_Core.Log.withNamespace("Oni2.LanguageSupport.CodeLens")
);

// MODEL

type codeLens = Exthost.CodeLens.lens;

let lineNumber = (lens: Exthost.CodeLens.lens) =>
  Exthost.OneBasedRange.(lens.range.startLineNumber - 1);

let textFromExthost = (lens: Exthost.CodeLens.lens) => {
  Exthost.Command.(
    lens.command
    |> OptionEx.flatMap(command => command.label)
    |> Option.map(Exthost.Label.toString)
    |> Option.value(~default="(null)")
  );
};

let text = (lens: Exthost.CodeLens.lens) => textFromExthost(lens);

type provider = {
  handle: int,
  maybeEventHandle: option(int),
  selector: Exthost.DocumentSelector.t,
  // [eventTick] is incremented for each [emitCodeLens] event
  eventTick: int,
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
      lenses: list(Exthost.CodeLens.lens),
    });

let register = (~handle: int, ~selector, ~maybeEventHandle, model) => {
  providers: [
    {handle, selector, maybeEventHandle, eventTick: 0},
    ...model.providers,
  ],
};

let unregister = (~handle: int, model) => {
  providers: model.providers |> List.filter(prov => prov.handle != handle),
};

let emit = (~eventHandle: int, model) => {
  {
    // Increment event tick for any matching providers
    providers:
      model.providers
      |> List.map(provider =>
           if (provider.maybeEventHandle == Some(eventHandle)) {
             {...provider, eventTick: provider.eventTick + 1};
           } else {
             provider;
           }
         ),
  };
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
           |> List.map(({handle, eventTick, _}) => {
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
                  ~eventTick,
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
  let make = (~theme, ~uiFont: Oni_Core.UiFont.t, ~codeLens, ()) => {
    let foregroundColor = CodeLensColors.foreground.from(theme);
    let label =
      Exthost.CodeLens.(codeLens.command)
      |> OptionEx.flatMap((command: Exthost.Command.t) => command.label)
      |> Option.value(~default=Exthost.Label.ofString("(null)"));

    <View
      style=Style.[
        marginTop(4),
        marginBottom(0),
        flexGrow(1),
        flexShrink(0),
      ]>
      <Oni_Components.Label font=uiFont color=foregroundColor label />
    </View>;
  };
};
