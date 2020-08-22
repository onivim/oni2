open Oni_Core;
open Oni_Core.Utility;

// MODEL

type codeLens = Exthost.CodeLens.t;

let lineNumber = (codeLens: Exthost.CodeLens.t) => Exthost.OneBasedRange.(
  codeLens.range.startLineNumber - 1
);

let text = (codeLens: Exthost.CodeLens.t) => Exthost.Command.(
  codeLens.command
  |> OptionEx.flatMap(command => command.label)
  |> Option.map(Exthost.Label.toString)
  |> Option.value(~default="(null)")
);

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

type handleToLenses = IntMap.t(list(Exthost.CodeLens.t));

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
  |> List.flatten
}

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

let addLenses = (handle, lenses, handleToLenses) => {
  IntMap.add(handle, lenses, handleToLenses)
};

let update = (msg, model) => switch (msg) {
| CodeLensesError(_) => model
| CodeLensesReceived({ handle, bufferId, lenses }) => {
  let bufferToLenses = model.bufferToLenses
  |> IntMap.update(bufferId, fun
  | None => IntMap.empty |> addLenses(handle, lenses) |> Option.some
  | Some(existing) => existing |> addLenses(handle, lenses) |> Option.some
  );
  {...model, bufferToLenses}
}
}

// SUBSCRIPTION

module Sub = {
  let create = (~visibleBuffers, ~client, model) => {

    visibleBuffers
    |> List.map(buffer => {
      model.providers
      |> List.filter(({selector, _}) => Exthost.DocumentSelector.matchesBuffer(~buffer, selector))
      |> List.map(({handle, _}) => {

      let toMsg = fun
      | Error(msg) => CodeLensesError(msg)
      | Ok(lenses) => CodeLensesReceived({handle, bufferId: buffer |> Oni_Core.Buffer.getId, lenses});
      
      Service_Exthost.Sub.codeLenses(~handle, ~buffer, ~toMsg, client)
    })
    })
    |> List.flatten
    |> Isolinear.Sub.batch;
  };
};

let sub = Sub.create;
