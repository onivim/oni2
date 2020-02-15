open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Extensions.SCM"));

// MODEL

[@deriving show({with_path: false})]
type command = {
  id: string,
  title: string,
  arguments: list([@opaque] Json.t),
};

module Resource = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    uri: Uri.t,
    icons: list(string),
    tooltip: string,
    strikeThrough: bool,
    faded: bool,
    source: option(string),
    letter: option(string),
    color: option(string),
  };
};

module ResourceGroup = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    hideWhenEmpty: bool,
    resources: list(Resource.t),
  };
};

module Provider = {
  [@deriving show({with_path: false})]
  type t = {
    handle: int,
    id: string,
    label: string,
    rootUri: option(Uri.t),
    resourceGroups: list(ResourceGroup.t),
    hasQuickDiffProvider: bool,
    count: int,
    commitTemplate: string,
    acceptInputCommand: option(command),
  };
};

module Decode = {
  let resource =
    fun
    | `List([
        `Int(handle),
        uri,
        `List(icons),
        `String(tooltip),
        `Bool(strikeThrough),
        `Bool(faded),
        source,
        letter,
        color,
      ]) =>
      Resource.{
        handle,
        uri: Uri.of_yojson(uri) |> Stdlib.Result.get_ok,
        icons:
          List.filter_map(
            fun
            | `String(icon) => Some(icon)
            | _ => None,
            icons,
          ),
        tooltip,
        strikeThrough,
        faded,
        source: source |> Yojson.Safe.Util.to_string_option,
        letter: letter |> Yojson.Safe.Util.to_string_option,
        color: Yojson.Safe.Util.(color |> member("id") |> to_string_option),
      }

    | _ => failwith("Unexpected json for scm resource");
};

// UPDATE

type msg =
  | RegisterSourceControl({
      handle: int,
      id: string,
      label: string,
      rootUri: option(Uri.t),
    })
  | UnregisterSourceControl({handle: int})
  | UpdateSourceControl({
      handle: int,
      hasQuickDiffProvider: option(bool),
      count: option(int),
      commitTemplate: option(string),
      acceptInputCommand: option(command),
    })
  // statusBarCommands: option(_),
  | RegisterSCMResourceGroup({
      provider: int,
      handle: int,
      id: string,
      label: string,
    })
  | UnregisterSCMResourceGroup({
      provider: int,
      handle: int,
    })
  | SpliceSCMResourceStates({
      provider: int,
      group: int,
      start: int,
      deleteCount: int,
      additions: list(Resource.t),
    });

let handleMessage = (~dispatch, method, args) =>
  switch (method) {
  | "$registerSourceControl" =>
    switch (args) {
    | [`Int(handle), `String(id), `String(label), rootUri] =>
      let rootUri = Uri.of_yojson(rootUri) |> Stdlib.Result.to_option;
      dispatch(RegisterSourceControl({handle, id, label, rootUri}));

    | _ => Log.error("Unexpected arguments for $registerSourceControl")
    }

  | "$unregisterSourceControl" =>
    switch (args) {
    | [`Int(handle)] => dispatch(UnregisterSourceControl({handle: handle}))

    | _ => Log.error("Unexpected arguments for $unregisterSourceControl")
    }

  | "$updateSourceControl" =>
    switch (args) {
    | [`Int(handle), features] =>
      Yojson.Safe.Util.(
        dispatch(
          UpdateSourceControl({
            handle,
            hasQuickDiffProvider:
              features |> member("hasQuickDiffProvider") |> to_bool_option,
            count: features |> member("count") |> to_int_option,
            commitTemplate:
              features |> member("commitTemplate") |> to_string_option,
            acceptInputCommand:
              features
              |> member("acceptInputCommand")
              |> (
                fun
                | `Assoc(_) as obj =>
                  Some({
                    id: obj |> member("id") |> to_string,
                    title: obj |> member("title") |> to_string,
                    arguments: obj |> member("arguments") |> to_list,
                  })
                | _ => None
              ),
          }),
        )
      )

    | _ => Log.error("Unexpected arguments for $updateSourceControl")
    }

  | "$registerGroup" =>
    switch (args) {
    | [`Int(provider), `Int(handle), `String(id), `String(label)] =>
      dispatch(RegisterSCMResourceGroup({provider, handle, id, label}))

    | _ => Log.error("Unexpected arguments for $registerGroup")
    }

  | "$unregisterGroup" =>
    switch (args) {
    | [`Int(handle), `Int(provider)] =>
      dispatch(UnregisterSCMResourceGroup({provider, handle}))

    | _ => Log.error("Unexpected arguments for $unregisterGroup")
    }

  | "$spliceResourceStates" =>
    switch (args) {
    | [`Int(provider), `List(groupSplices)] =>
      List.iter(
        fun
        | `List([`Int(group), `List(splices)]) =>
          List.iter(
            splice =>
              switch (splice) {
              | `List([`Int(start), `Int(deleteCount), `List(additions)]) =>
                let additions = List.map(Decode.resource, additions);
                dispatch(
                  SpliceSCMResourceStates({
                    provider,
                    group,
                    start,
                    deleteCount,
                    additions,
                  }),
                );

              | _ => Log.warn("spliceResourceStates: Unexpected json")
              },
            splices,
          )
        | _ => Log.warn("spliceResourceStates: Unexpected json"),
        groupSplices,
      )

    | _ => Log.error("Unexpected arguments for $spliceResourceStates")
    }
  | _ =>
    Log.warnf(m =>
      m(
        "Unhandled SCM message - %s: %s",
        method,
        Yojson.Safe.to_string(`List(args)),
      )
    )
  };

// REQUESTS

module Requests = {
  let provideOriginalResource = (handle, uri, client) =>
    ExtHostTransport.request(
      ~msgType=MessageType.requestJsonArgsWithCancellation,
      client,
      ExtHostProtocol.OutgoingNotifications._buildNotification(
        "ExtHostSCM",
        "$provideOriginalResource",
        `List([`Int(handle), Uri.to_yojson(uri)]),
      ),
      json =>
      Uri.of_yojson(json) |> Stdlib.Result.get_ok
    );

  let onInputBoxValueChange = (handle, value, client) =>
    ExtHostTransport.send(
      ~msgType=MessageType.requestJsonArgsWithCancellation,
      client,
      ExtHostProtocol.OutgoingNotifications._buildNotification(
        "ExtHostSCM",
        "$onInputBoxValueChange",
        `List([`Int(handle), `String(value)]),
      ),
    );
};

module Effects = {
  let provideOriginalResource = (extHostClient, providers, path, toMsg) =>
    Isolinear.Effect.createWithDispatch(~name="scm.getOriginalUri", dispatch => {
      // Try our luck with every provider. If several returns Last-Writer-Wins
      // TODO: Is there a better heuristic? Perhaps use rootUri to choose the "nearest" provider?
      providers
      |> List.iter((provider: Provider.t) => {
           let promise =
             Requests.provideOriginalResource(
               provider.handle,
               Uri.fromPath(path),
               extHostClient,
             );

           Lwt.on_success(promise, uri => dispatch(toMsg(uri)));
         })
    });

  let onInputBoxValueChange = (extHostClient, provider: Provider.t, value) =>
    Isolinear.Effect.create(~name="scm.onInputBoxValueChange", () =>
      Requests.onInputBoxValueChange(provider.handle, value, extHostClient)
    );
};
