/*
 * ExtensionClientHelper
 *
 * A module providing a helper API to make it easier to test
 * against the extension client API
 */

open Oni_Core;
open Oni_Extensions;

module JsonInformationMessageFormat = {
  [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
  type t = {
    [@key "type"]
    messageType: string,
    filename: string,
    fullText: string,
  };
};

let empty: unit => ref(list(string)) = () => ref([]);
let emptyInfoMsgs: unit => ref(list(JsonInformationMessageFormat.t)) =
  () => ref([]);

let clear = (r: ref(list(string))) => r := [];

let appendInfoMsg = (r, s) => {
  let json = Yojson.Safe.from_string(s);
  let info = JsonInformationMessageFormat.of_yojson(json);

  switch (info) {
  | Ok(v) => r := [v, ...r^]
  | _ => ()
  };
};

let doesInfoMessageMatch = (r: ref(list(JsonInformationMessageFormat.t)), f) => {
  let l = r^ |> List.filter(f) |> List.length;
  l > 0;
};

let append = (r: ref(list(string)), s: string) => r := [s, ...r^];

let any = (r: ref(list(string)), ()) => List.length(r^) > 0;

let isStringValueInList = (r: ref(list(string)), match: string) => {
  let l = r^ |> List.filter(id => String.equal(id, match)) |> List.length;
  l > 0;
};

module Waiters = {
  let wait = (f, client: ExtHostClient.t) => {
    Oni_Core.Utility.waitForCondition(
      ~timeout=10.0,
      () => {
        ExtHostClient.pump(client);
        f();
      },
    );
  };
};

let noop1 = _ => ();
let withExtensionClient2 =
    (
      ~onStatusBarSetEntry=noop1,
      ~onDidActivateExtension=noop1,
      ~onShowMessage=noop1,
      ~onRegisterCommand=noop1,
      f: ExtHostClient.t => unit,
    ) => {
  let setup = Setup.init();

  let rootPath = Rench.Environment.getWorkingDirectory();
  let testExtensionsPath = Rench.Path.join(rootPath, "test/test_extensions");

  let extensions =
    ExtensionScanner.scan(testExtensionsPath)
    |> List.map(ext => ExtHostInitData.ExtensionInfo.ofScannedExtension(ext));

  let initData = ExtHostInitData.create(~extensions, ());

  let initialized = ref(false);
  let closed = ref(false);

  let onClosed = () => closed := true;
  let onInitialized = () => initialized := true;

  let v =
    ExtHostClient.start(
      ~initData,
      ~onInitialized,
      ~onStatusBarSetEntry,
      ~onDidActivateExtension,
      ~onRegisterCommand,
      ~onShowMessage,
      ~onClosed,
      setup,
    );

  Oni_Core.Utility.waitForCondition(() => {
    ExtHostClient.pump(v);
    initialized^;
  });

  if (! initialized^) {
    failwith("extension host client did not initialize successfully");
  } else {
    f(v);
    ExtHostClient.close(v);
  };
};

