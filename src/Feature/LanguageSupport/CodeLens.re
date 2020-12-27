open Oni_Core;
open Oni_Core.Utility;

module Log = (
  val Oni_Core.Log.withNamespace("Oni2.LanguageSupport.CodeLens")
);

// MODEL

type codeLens = {
  handle: int,
  lens: Exthost.CodeLens.t,
  uniqueId: string,
};

let lineNumber = ({lens, _}) =>
  Exthost.OneBasedRange.(lens.range.startLineNumber - 1);

let uniqueId = ({uniqueId, _}) => uniqueId;

let textFromExthost = (lens: Exthost.CodeLens.t) => {
  Exthost.Command.(
    lens.command
    |> OptionEx.flatMap(command => command.label)
    |> Option.map(Exthost.Label.toString)
    |> Option.value(~default="(null)")
  );
};

let text = ({lens, _}: codeLens) => textFromExthost(lens);

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

type handleToLenses = IntMap.t(list(codeLens));

type model = {
  providers: list(provider),
  bufferToLenses: IntMap.t(handleToLenses),
};

type outmsg =
  | Nothing
  | CodeLensesChanged({
      bufferId: int,
      startLine: EditorCoreTypes.LineNumber.t,
      stopLine: EditorCoreTypes.LineNumber.t,
      lenses: list(codeLens),
    });

let get = (~bufferId, {bufferToLenses, _}) => {
  bufferToLenses
  |> IntMap.find_opt(bufferId)
  |> Option.value(~default=IntMap.empty)
  |> IntMap.bindings
  |> List.map(snd)
  |> List.flatten;
};

let initial = {providers: [], bufferToLenses: IntMap.empty};

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
  ...model,
  providers: [{handle, selector}, ...model.providers],
};

let unregister = (~handle: int, model) => {
  ...model,
  providers: model.providers |> List.filter(prov => prov.handle != handle),
};

// UPDATE

let addLenses = (handle, bufferId, lenses, handleToLenses) => {
  let internalLenses =
    lenses
    |> List.map(lens =>
         {
           lens,
           handle,
           uniqueId:
             Printf.sprintf(
               "%d%d%d",
               handle,
               bufferId,
               Hashtbl.hash(textFromExthost(lens)),
             ),
         }
       );

  let sort = (lenses: list(codeLens)) =>
    lenses
    |> Base.List.dedup_and_sort(~compare=(lensA, lensB) => {
         Exthost.CodeLens.(
           {
             lensA.lens.range.startLineNumber - lensB.lens.range.startLineNumber;
           }
         )
       });

  handleToLenses
  |> IntMap.update(
       handle,
       fun
       | None => internalLenses |> sort |> Option.some
       | Some(prev) => prev @ internalLenses |> sort |> Option.some,
     );
};

let removeLensesInRange = (startLine, stopLine, handle, handleToLenses) => {
  let start1 = EditorCoreTypes.LineNumber.toOneBased(startLine);
  let stop1 = EditorCoreTypes.LineNumber.toOneBased(stopLine);
  let filter = (lens: codeLens) => {
    Exthost.CodeLens.(
      {
        let line = lens.lens.range.startLineNumber;
        !(line >= start1 && line <= stop1);
      }
    );
  };
  handleToLenses
  |> IntMap.update(
       handle,
       fun
       | None => None
       | Some(lenses) => lenses |> List.filter(filter) |> Option.some,
     );
};

let update = (msg, model) =>
  switch (msg) {
  | CodeLensesError(_) => (model, Nothing)
  | CodeLensesReceived({handle, startLine, stopLine, bufferId, lenses}) =>
    let bufferToLenses =
      model.bufferToLenses
      |> IntMap.update(
           bufferId,
           fun
           | None =>
             IntMap.empty
             |> addLenses(handle, bufferId, lenses)
             |> Option.some
           | Some(existing) =>
             existing
             |> removeLensesInRange(startLine, stopLine, handle)
             |> addLenses(handle, bufferId, lenses)
             |> Option.some,
         );

    let model' = {...model, bufferToLenses};
    let lenses = get(~bufferId, model');
    (model', CodeLensesChanged({bufferId, startLine, stopLine, lenses}));
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
    // Query above and below a viewport - grabbing some extra codelenses above and below -
    // to minimize codelens popping in while scrolling
    let delta: int =
      EditorCoreTypes.LineNumber.toOneBased(bottomVisibleBufferLine)
      - EditorCoreTypes.LineNumber.toOneBased(topVisibleBufferLine);
    let (topVisibleBufferLine, bottomVisibleBufferLine) =
      EditorCoreTypes.LineNumber.(
        topVisibleBufferLine - delta,
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
  if (!isAnimatingScroll
      && Configuration.Experimental.Editor.codeLensEnabled.get(config)) {
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
