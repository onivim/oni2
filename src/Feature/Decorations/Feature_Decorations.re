open Oni_Core;

// MODEL

module Decoration = {
  [@deriving show]
  type t = {
    handle: int, // provider handle
    tooltip: string,
    letter: string,
    color: string // TODO: ThemeColor.t?
  };
};

[@deriving show]
type msg =
  | Exthost(Exthost.Msg.Decorations.msg)
  | GotDecorations({
      handle: int,
      uri: Uri.t,
      decorations: list(Decoration.t),
    });

module Msg = {
  let exthost = msg => Exthost(msg);
};

type provider = {
  handle: int,
  label: string,
};

type model = {
  nextRequestId: int,
  providers: list(provider),
  decorations: StringMap.t(list(Decoration.t)),
};

let initial = {nextRequestId: 0, providers: [], decorations: StringMap.empty};

let getDecorations = (~path: string, model) => {
  model.decorations |> StringMap.find_opt(path) |> Option.value(~default=[]);
};

// EFFECTS

module Effects = {
  let provideDecorations = (~handle, ~uri, ~id, client) => {
    let requests: list(Exthost.Request.Decorations.request) = [{id, uri}];

    let toDecoration: Exthost.Request.Decorations.decoration => Decoration.t =
      decoration => {
        handle,
        tooltip: decoration.title,
        letter: decoration.letter,
        color: decoration.color.id,
      };

    let toMsg = decorations => {
      let decorations =
        decorations
        |> IntMap.bindings
        |> List.to_seq
        |> Seq.map(snd)
        |> Seq.map(toDecoration)
        |> List.of_seq;

      GotDecorations({handle, uri, decorations});
    };

    Service_Exthost.Effects.Decorations.provideDecorations(
      ~handle,
      ~requests,
      ~toMsg,
      client,
    );
  };
};

// UPDATE

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update = (~client: Exthost.Client.t, msg, model) => {
  Exthost.Msg.Decorations.(
    switch (msg) {
    | Exthost(RegisterDecorationProvider({handle, label})) => (
        {...model, providers: [{handle, label}, ...model.providers]},
        Nothing,
      )

    | Exthost(UnregisterDecorationProvider({handle})) => (
        {
          ...model,
          providers:
            List.filter(
              provider => provider.handle != handle,
              model.providers,
            ),
        },
        Nothing,
      )

    | Exthost(DecorationsDidChange({handle, uris})) =>
      let requestId = model.nextRequestId;
      let (lastRequestId, effects) =
        uris
        |> List.fold_left(
             (acc, curr) => {
               let (requestId, effects) = acc;
               (
                 requestId + 1,
                 [
                   Effects.provideDecorations(
                     ~id=requestId,
                     ~handle,
                     ~uri=curr,
                     client,
                   ),
                   ...effects,
                 ],
               );
             },
             (requestId, []),
           );

      (
        {...model, nextRequestId: lastRequestId + 1},
        Effect(effects |> Isolinear.Effect.batch),
      );

    | GotDecorations({handle, uri, decorations}) => (
        {
          ...model,
          decorations:
            StringMap.update(
              Uri.toFileSystemPath(uri),
              fun
              | Some(existing) => {
                  let existing =
                    List.filter(
                      (it: Decoration.t) => it.handle != handle,
                      existing,
                    );
                  Some(decorations @ existing);
                }
              | None => Some(decorations),
              model.decorations,
            ),
        },
        Nothing,
      )
    }
  );
};
