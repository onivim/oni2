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
      decorations: [@opaque] StringMap.t(list(Decoration.t)),
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
  // Round-trip the PATH through a URI, so that it is normalized for Windows (lower-case drive letter, backslahes)
  let path = path |> Uri.fromPath |> Uri.toFileSystemPath;
  model.decorations |> StringMap.find_opt(path) |> Option.value(~default=[]);
};

// EFFECTS

module Effects = {
  let provideDecorations = (~handle, ~uris, ~id, client) => {
    let (nextId, requestMap: IntMap.t(Uri.t)) =
      uris
      |> List.fold_left(
           (acc, curr) => {
             let (lastId, uriMap) = acc;
             (lastId + 1, IntMap.add(lastId + 1, curr, uriMap));
           },
           (id, IntMap.empty),
         );

    let requests =
      requestMap
      |> IntMap.bindings
      |> List.map(((id, uri)) => {
           ({id, uri}: Exthost.Request.Decorations.request)
         });

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
        |> List.fold_left(
             (acc, (requestId, decoration)) => {
               switch (IntMap.find_opt(requestId, requestMap)) {
               | None => acc
               | Some(uri) =>
                 StringMap.add(
                   Uri.toFileSystemPath(uri),
                   [toDecoration(decoration)],
                   acc,
                 )
               }
             },
             StringMap.empty,
           );

      GotDecorations({handle, decorations});
    };

    (
      nextId,
      Service_Exthost.Effects.Decorations.provideDecorations(
        ~handle,
        ~requests,
        ~toMsg,
        client,
      ),
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
      let (nextRequestId, effect) =
        Effects.provideDecorations(~id=requestId, ~handle, ~uris, client);
      ({...model, nextRequestId}, Effect(effect));

    | GotDecorations({handle, decorations}) => (
        {
          ...model,
          decorations:
            StringMap.union(
              (_key, existing, decorations) => {
                let existing =
                  List.filter(
                    (it: Decoration.t) => it.handle != handle,
                    existing,
                  );
                Some(decorations @ existing);
              },
              model.decorations,
              decorations,
            ),
        },
        Nothing,
      )
    }
  );
};
