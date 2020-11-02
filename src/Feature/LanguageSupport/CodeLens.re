open Oni_Core;
open Oni_Core.Utility;

// MODEL

type codeLens = {
  handle: int,
  lens: Exthost.CodeLens.t,
  uniqueId: string,
};

let lineNumber = ({lens, _}) =>
  Exthost.OneBasedRange.(lens.range.startLineNumber - 1);

let uniqueId = ({uniqueId, _}) => uniqueId;

let text = ({lens, _}: codeLens) =>
  Exthost.Command.(
    lens.command
    |> OptionEx.flatMap(command => command.label)
    |> Option.map(Exthost.Label.toString)
    |> Option.value(~default="(null)")
  );

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

type handleToLenses = IntMap.t(list(codeLens));

type model = {
  providers: list(provider),
  bufferToLenses: IntMap.t(handleToLenses),
};

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
               lens.range.startLineNumber,
             ),
         }
       );
  IntMap.add(handle, internalLenses, handleToLenses);
};

let update = (msg, model) =>
  switch (msg) {
  | CodeLensesError(_) => model
  | CodeLensesReceived({handle, bufferId, lenses}) =>
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
             existing |> addLenses(handle, bufferId, lenses) |> Option.some,
         );
    {...model, bufferToLenses};
  };

// CONFIGURATION

module Configuration = {
  open Config.Schema;

  module Experimental = {
    let enabled =
      setting("experimental.editor.codeLens", bool, ~default=false);
  };
};

// SUBSCRIPTION

module Sub = {
  let create = (~visibleBuffers, ~client, model) => {
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
                    bufferId: buffer |> Oni_Core.Buffer.getId,
                    lenses,
                  });

              Service_Exthost.Sub.codeLenses(
                ~handle,
                ~buffer,
                ~toMsg,
                client,
              );
            })
       })
    |> List.flatten
    |> Isolinear.Sub.batch;
  };
};

let sub = (~config, ~visibleBuffers, ~client, model) =>
  if (Configuration.Experimental.enabled.get(config)) {
    Sub.create(~visibleBuffers, ~client, model);
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

  let configuration = Configuration.[Experimental.enabled.spec];
};

// VIEW

module View = {
  module CodeLensColors = Colors;
  open Revery.UI;
  let make = (~theme, ~uiFont: Oni_Core.UiFont.t, ~codeLens, ()) => {
    let foregroundColor = CodeLensColors.foreground.from(theme);
    let text = text(codeLens);
    <View style=Style.[marginTop(4), marginBottom(-4)]>
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
