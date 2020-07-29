open Oni_Core;
open Exthost;

[@deriving show]
type msg =
  | CompletionResultAvailable({
      handle: int,
      suggestResult: Exthost.SuggestResult.t,
    })
  | CompletionError({
      handle: int,
      errorMsg: string,
    });

[@deriving show]
type provider = {
  handle: int,
  selector: DocumentSelector.t,
  triggerCharacters: list(string),
  supportsResolveDetails: bool,
  extensionId: string,
};

module Session = {
  [@deriving show]
  type state =
    | Waiting
    | Completed(list(Exthost.SuggestItem.t))
    // TODO
    //| Incomplete(list(Exthost.SuggestItem.t))
    | Error(string);

  [@deriving show]
  type t = {
    state,
    buffer: [@opaque] Oni_Core.Buffer.t,
    base: string,
    location: EditorCoreTypes.Location.t,
  };

  let create = (~buffer, ~base, ~location) => {
    state: Waiting,
    buffer,
    base,
    location,
  };

  let refine = (~base, current) => {
    ...current,
    base,
    // TODO: Filter results based on base
  };

  let update = (~buffer, ~base, ~location, previous) =>
    // If different buffer or location... start over!
    if (Oni_Core.Buffer.getId(buffer)
        != Oni_Core.Buffer.getId(previous.buffer)
        || location != previous.location) {
      create(~buffer, ~base, ~location);
    } else {
      // Refine results
      refine(~base, previous);
    };
};

type model = {
  handleToSession: IntMap.t(Session.t),
  providers: list(provider),
};

let initial = {handleToSession: IntMap.empty, providers: []};

let isActive = (_model) => true;

let register =
    (
      ~handle,
      ~selector,
      ~triggerCharacters,
      ~supportsResolveDetails,
      ~extensionId,
      model,
    ) => {
  ...model,
  providers: [
    {
      handle,
      selector,
      triggerCharacters,
      supportsResolveDetails,
      extensionId,
    },
    ...model.providers,
  ],
};

let unregister = (~handle, model) => {
  ...model,
  providers: List.filter(prov => prov.handle != handle, model.providers),
};

let bufferUpdated = (~buffer, ~activeCursor, ~syntaxScope, ~triggerKey, model) => {
  let candidateProviders =
    model.providers
    |> List.filter(prov =>
         Exthost.DocumentSelector.matchesBuffer(~buffer, prov.selector)
       );

  let handleToSession =
    List.fold_left(
      (acc: IntMap.t(Session.t), curr: provider) => {
        let maybeMeet =
          CompletionMeet.fromBufferLocation(
            // TODO: triggerCharacters
            ~location=activeCursor,
            buffer,
          );

        switch (maybeMeet) {
        | None => acc
        | Some({base, location, _}) =>
          acc
          |> IntMap.update(
               curr.handle,
               fun
               | None => Some(Session.create(~buffer, ~base, ~location))
               | Some(previous) =>
                 Some(Session.update(~buffer, ~base, ~location, previous)),
             )
        };
      },
      IntMap.empty,
      candidateProviders,
    );

  {...model, handleToSession};
};

let update = (msg, model) => {
  (model, Outmsg.Nothing);
};

let sub = (~client, model) => {
  model.handleToSession
  |> IntMap.bindings
  |> List.map(((handle: int, meet: Session.t)) => {
       Service_Exthost.Sub.completionItems(
         // TODO: proper trigger kind
         ~context=
           Exthost.CompletionContext.{
             triggerKind: Invoke,
             triggerCharacter: None,
           },
         ~handle,
         ~buffer=meet.buffer,
         ~position=meet.location,
         ~toMsg=
           suggestResult => {
             prerr_endline(
               "Got result for handle: " ++ string_of_int(handle),
             );
             switch (suggestResult) {
             | Ok(v) =>
               prerr_endline(Exthost.SuggestResult.show(v));
               CompletionResultAvailable({handle, suggestResult: v});
             | Error(errorMsg) =>
               prerr_endline(errorMsg);
               CompletionError({handle, errorMsg});
             };
           },
         client,
       )
     })
  |> Isolinear.Sub.batch;
};

module View = {
  open Revery;
  open Revery.UI;
  let%component make = (
    ~x: int,
    ~y: int,
    ~lineHeight: float,
    ~theme: Oni_Core.ColorTheme.Colors.t,
    ~tokenTheme: Oni_Syntax.TokenTheme.t,
    ~editorFont: Service_Font.font,
    ~model: model,
    (),
  ) =>  {
    let%hook () = Hooks.effect(OnMount, () => None);
    <View />
  };
};
