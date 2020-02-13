open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Extensions.SCM"));

// MODEL

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
    })
  // acceptInputCommand: option(_),
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
