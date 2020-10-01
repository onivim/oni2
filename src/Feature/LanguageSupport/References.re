type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

type state =
  | Empty
  | InProgress
  | Found(list(Exthost.Location.t));

type model = {
  state,
  providers: list(provider),
};

let initial = {state: Empty, providers: []};

[@deriving show]
type command =
  | FindAll;

[@deriving show]
type msg =
  | Command(command)
  | ReferencesNotAvailable
  | ReferencesFound([@opaque] list(Exthost.Location.t));

let get = ({state, _}) =>
  switch (state) {
  | Empty
  | InProgress => []
  | Found(items) => items
  };

let update = (~maybeBuffer, ~cursorLocation, ~client, msg, model) => {
  switch (msg) {
  | Command(FindAll) =>
    let eff =
      switch (maybeBuffer) {
      | None => Isolinear.Effect.none
      | Some(buffer) =>
        model.providers
        |> List.filter(({selector, _}) =>
             Exthost.DocumentSelector.matchesBuffer(~buffer, selector)
           )
        |> List.map(({handle, _}) =>
             Service_Exthost.Effects.LanguageFeatures.provideReferences(
               ~handle,
               ~uri=Oni_Core.Buffer.getUri(buffer),
               ~position=cursorLocation,
               ~context=Exthost.ReferenceContext.{includeDeclaration: true},
               client,
               fun
               | Ok(locations) => ReferencesFound(locations)
               | Error(_msg) => ReferencesNotAvailable,
             )
           )
        |> Isolinear.Effect.batch
      };
    ({...model, state: InProgress}, Outmsg.Effect(eff));
  | ReferencesNotAvailable => (model, Outmsg.Nothing)
  | ReferencesFound(locations) =>
    let state' =
      switch (model.state) {
      | Empty
      | InProgress => Found(locations)
      | Found(prevLocations) => Found(locations @ prevLocations)
      };
    ({...model, state: state'}, Outmsg.ReferencesAvailable);
  };
};

let register = (~handle, ~selector, model) => {
  ...model,
  providers: [{handle, selector}, ...model.providers],
};

let unregister = (~handle, model) => {
  ...model,
  providers: model.providers |> List.filter(prov => prov.handle != handle),
};

module Commands = {
  open Feature_Commands.Schema;

  let findAll =
    define(
      ~category="References",
      ~title="Find All References",
      "editor.action.goToReferences",
      Command(FindAll),
    );
};

module Keybindings = {
  open Oni_Input.Keybindings;

  let condition = "editorTextFocus && normalMode" |> WhenExpr.parse;

  // TODO: Fix this
  let shiftF12 = {key: "<S-F12>", command: Commands.findAll.id, condition};
};

module Contributions = {
  let commands = Commands.[findAll];
  let keybindings = Keybindings.[shiftF12];
};
