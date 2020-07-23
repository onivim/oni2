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
  requestLocation: Location.t,
  definition: Exthost.DefinitionLink.t,
};

type model = {
  maybeDefinition: option(definition),
  providers: list(provider),
};

[@deriving show]
type msg =
  | DefinitionAvailable(definition)
  | DefinitionNotAvailable;

let initial = {maybeDefinition: None, providers: []};

let update = (msg, model) =>
  switch (msg) {
  | DefinitionNotAvailable => {...model, maybeDefinition: None}
  | DefinitionAvailable(definition) => {
      ...model,
      maybeDefinition: Some(definition),
    }
  };

let register = (~handle, ~selector, model) => {
  ...model,
  providers: [{handle, selector}, ...model.providers],
};

let unregister = (~handle: int, model) => {
  ...model,
  providers: model.providers |> List.filter(prov => prov.handle != handle),
};

let get = (~bufferId, {maybeDefinition, _}) => {
  maybeDefinition
  |> OptionEx.flatMap(({bufferId as lastBufferId, definition}) =>
       if (bufferId == lastBufferId) {
         Some(definition);
       } else {
         None;
       }
     );
};

let getAt = (~bufferId, ~range, {maybeDefinition, _}) => {
  maybeDefinition
  |> OptionEx.flatMap(({bufferId as lastBufferId, definition}) =>
       if (bufferId == lastBufferId) {
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
