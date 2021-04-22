/*
 * Definition.re
 *
 * This module is responsible for recording and managing the 'definition' feature state
 */
open EditorCoreTypes;
open Oni_Core;
open Utility;

type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
};

[@deriving show]
type definition = {
  bufferId: int,
  requestLocation: CharacterPosition.t,
  definition: Exthost.DefinitionLink.t,
};

type model = {
  maybeDefinition: option(definition),
  providers: list(provider),
};

[@deriving show]
type command =
  | GotoDefinition(SplitDirection.t);

[@deriving show]
type msg =
  | DefinitionAvailable(definition)
  | DefinitionNotAvailable
  | Command(command);

let initial = {maybeDefinition: None, providers: []};

let update = (msg, model) =>
  switch (msg) {
  | DefinitionNotAvailable => (
      {...model, maybeDefinition: None},
      Outmsg.Nothing,
    )
  | DefinitionAvailable(definition) => (
      {...model, maybeDefinition: Some(definition)},
      Outmsg.Nothing,
    )
  | Command(GotoDefinition(direction)) =>
    let outmsg =
      switch (model.maybeDefinition) {
      | None => Outmsg.Nothing
      | Some({definition, _}) =>
        let position =
          CharacterPosition.{
            line:
              EditorCoreTypes.LineNumber.ofOneBased(
                definition.range.startLineNumber,
              ),
            character: CharacterIndex.ofInt(definition.range.startColumn - 1),
          };
        Outmsg.OpenFile({
          filePath: definition.uri |> Oni_Core.Uri.toFileSystemPath,
          location: Some(position),
          direction,
        });
      };
    (model, outmsg);
  };

let register = (~handle, ~selector, model) => {
  ...model,
  providers: [{handle, selector}, ...model.providers],
};

let unregister = (~handle: int, model) => {
  ...model,
  providers: model.providers |> List.filter(prov => prov.handle != handle),
};

let get = (~bufferId as currentBufferId, {maybeDefinition, _}) => {
  maybeDefinition
  |> OptionEx.flatMap(({bufferId, definition, _}) =>
       if (currentBufferId == bufferId) {
         Some(definition);
       } else {
         None;
       }
     );
};

let getAt = (~bufferId as currentBufferId, ~range, {maybeDefinition, _}) => {
  maybeDefinition
  |> OptionEx.flatMap(({bufferId, definition, requestLocation}) =>
       if (bufferId == currentBufferId
           && CharacterRange.contains(requestLocation, range)) {
         Some(definition);
       } else {
         None;
       }
     );
};

let handleResponse =
    (
      ~bufferId: int,
      ~location,
      definitionLinks: list(Exthost.DefinitionLink.t),
    ) => {
  switch (definitionLinks) {
  | [] => DefinitionNotAvailable
  | [hd, ..._tail] =>
    DefinitionAvailable({bufferId, definition: hd, requestLocation: location})
  };
};

let sub = (~buffer, ~location, ~client, model) => {
  model.providers
  |> List.filter_map(({handle, selector}) =>
       if (Exthost.DocumentSelector.matchesBuffer(~buffer, selector)) {
         Some(
           Service_Exthost.Sub.definition(
             ~handle,
             ~buffer,
             ~position=location,
             ~toMsg=
               handleResponse(
                 ~location,
                 ~bufferId=Oni_Core.Buffer.getId(buffer),
               ),
             client,
           ),
         );
       } else {
         None;
       }
     )
  |> Isolinear.Sub.batch;
};

module Commands = {
  open Feature_Commands.Schema;

  let gotoDefinition =
    define(
      ~category="Language",
      ~title="Go-to Definition",
      "editor.action.revealDefinition",
      Command(GotoDefinition(SplitDirection.Current)),
    );

  let gotoDefinitionAside =
    define(
      ~category="Language",
      ~title="Go-to Definition Aside",
      "editor.action.revealDefinitionAside",
      Command(GotoDefinition(SplitDirection.Vertical({shouldReuse: true}))),
    );
};

module Keybindings = {
  open Feature_Input.Schema;

  let condition = "normalMode" |> WhenExpr.parse;

  let gotoDefinition =
    bind(~key="<F12>", ~command=Commands.gotoDefinition.id, ~condition);
  // TODO:
  // let gotoDefinitionAside =
  //   bind(
  //     ~key="<C-K>F12",
  //     ~command=Commands.gotoDefinitionAside.id,
  //     ~condition,
  //   );
};

module Contributions = {
  let commands = [Commands.gotoDefinition, Commands.gotoDefinitionAside];

  let keybindings = [
    Keybindings.gotoDefinition,
    //Keybindings.gotoDefinitionAside,
  ];
};
